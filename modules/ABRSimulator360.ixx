export module ABRSimulation360.ABRSimulator360;

import System.Base;
import System.Math;
import System.MDArray;
import System.Parallel;

import ABRSimulation360.AggregateControllers.AggregateControllerFactory;
import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.BitrateAllocatorFactory;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;
import ABRSimulation360.NetworkSimulator;
import ABRSimulation360.ThroughputPredictors.EMAPredictor;
import ABRSimulation360.ThroughputPredictors.IThroughputPredictor;
import ABRSimulation360.ThroughputPredictors.ThroughputPredictorFactory;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;
import ABRSimulation360.ViewportPredictors.OfflinePredictor;
import ABRSimulation360.ViewportPredictors.StaticPredictor;
import ABRSimulation360.ViewportPredictors.ViewportPredictorFactory;
import ABRSimulation360.ViewportSimulator;

using namespace std;
using namespace experimental;

/// Refers to a simulation series.
export struct SimulationSeriesRef {
    double &RebufferingSeconds; ///< The total rebuffering duration in seconds.
    mdspan<double, dims<2>> BufferedBitratesMbps; ///< A 2D array of buffered bitrates in megabits per second.
    mdspan<double, dims<2>> ViewportDistributions; ///< A list of viewport distributions.
    mdspan<double, dims<2>> PredictedViewportDistributions; ///< A list of predicted viewport distributions.
};

/// Refers to a collection of simulation series.
export struct SimulationDataRef {
    span<double> RebufferingSeconds; ///< A list of total rebuffering durations in seconds.
    mdspan<double, dims<3>> BufferedBitratesMbps; ///< A 3D array of buffered bitrates in megabits per second.
    mdspan<double, dims<3>> ViewportDistributions; ///< A 2D array of viewport distributions.
    mdspan<double, dims<3>> PredictedViewportDistributions; ///< A 2D array of predicted viewport distributions.

    /// Returns the path at the specified index.
    /// @param index The index of the path.
    /// @returns The path at the specified index.
    [[nodiscard]] SimulationSeriesRef operator[](int index) const {
        const auto bitratesMbps = submdspan(BufferedBitratesMbps, index, full_extent, full_extent);
        const auto distributions = submdspan(ViewportDistributions, index, full_extent, full_extent);
        const auto predictedDistribution = submdspan(PredictedViewportDistributions, index, full_extent, full_extent);
        return {RebufferingSeconds[index], bitratesMbps, distributions, predictedDistribution};
    }
};

/// Represents the options for 360° adaptive bitrate streaming simulation.
export struct ABRSimulation360Options {
    /// The options for the throughput predictor.
    const BaseThroughputPredictorOptions &ThroughputPredictorOptions = EMAPredictorOptions();
    /// The options for the viewport predictor.
    const BaseViewportPredictorOptions &ViewportPredictorOptions = StaticPredictorOptions();
};

/// Simulates the dynamics of 360° adaptive bitrate streaming.
export class ABRSimulator360 {
public:
    /// Simulates a 360° adaptive bitrate streaming configuration on a network series and viewport series.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param controllerOptions The options for the aggregate controller.
    /// @param allocatorOptions The options for the bitrate allocator.
    /// @param networkSeries A network series.
    /// @param viewportSeries A viewport series.
    /// @param out The simulation series output.
    /// @param options The options for 360° adaptive bitrate streaming simulation.
    static void Simulate(const StreamingConfig &streamingConfig,
                         const BaseAggregateControllerOptions &controllerOptions,
                         const BaseBitrateAllocatorOptions &allocatorOptions,
                         NetworkSeriesView networkSeries,
                         ViewportSeriesView viewportSeries,
                         SimulationSeriesRef out,
                         const ABRSimulation360Options &options = {}) {
        const auto segmentSeconds = streamingConfig.SegmentSeconds;
        const auto segmentCount = Math::Round(viewportSeries.DurationSeconds() / segmentSeconds);
        const auto tilingCount = streamingConfig.TilingCount;
        const auto tileCountPerFace = tilingCount * tilingCount, tileCount = tileCountPerFace * 6;
        const auto bitratesMbps = streamingConfig.BitratesPerFaceMbps / tileCountPerFace;
        const auto viewportConfig = streamingConfig.ViewportConfig;
        const auto maxBufferSeconds = streamingConfig.MaxBufferSeconds;

        const auto throughputPredictor = ThroughputPredictorFactory::Create(options.ThroughputPredictorOptions);
        const auto viewportPredictor = ViewportPredictorFactory::Create(
            viewportSeries.IntervalSeconds, options.ViewportPredictorOptions);
        const auto controller = AggregateControllerFactory::Create(streamingConfig, controllerOptions);
        const auto allocator = BitrateAllocatorFactory::Create(streamingConfig, allocatorOptions);
        NetworkSimulator networkSimulator(networkSeries);
        ViewportSimulator viewportSimulator(viewportConfig, tilingCount);

        // Initializes offline components.
        if (auto *const _viewportPredictor = dynamic_cast<OfflinePredictor *>(viewportPredictor.get()))
            _viewportPredictor->Initialize(viewportSeries);

        auto beginSegmentID = 0;
        auto secondsInSegment = 0.;
        out.RebufferingSeconds = 0.;

        // Computes the viewport distributions.
        for (auto segmentID = 0; segmentID < segmentCount; ++segmentID) {
            const auto positions = viewportSeries.Window(segmentID * segmentSeconds, segmentSeconds).Values;
            const auto distribution = viewportSimulator.ToDistribution(positions);
            ranges::copy(distribution, &out.ViewportDistributions[segmentID, 0]);
        }

        const auto DownloadSegment = [&](int segmentID, span<const int> bitrateIDs) {
            const auto _bitratesMbps = views::iota(0, tileCount) | views::transform([&](int tileID) {
                return bitratesMbps[bitrateIDs[tileID]];
            }) | ranges::to<vector>();
            const auto totalSizeMB = Math::Total(_bitratesMbps) * segmentSeconds / 8;
            const auto downloadInfo = networkSimulator.Download(totalSizeMB);
            ranges::copy(_bitratesMbps, &out.BufferedBitratesMbps[segmentID, 0]);
            throughputPredictor->Update(downloadInfo.Value, downloadInfo.Seconds);
            return downloadInfo;
        };

        const auto PlayVideo = [&](double seconds) {
            const auto currentSeconds = beginSegmentID * segmentSeconds + secondsInSegment;
            const auto positions = viewportSeries.Window(currentSeconds, seconds).Values;
            if (!positions.empty()) viewportPredictor->Update(positions);

            beginSegmentID += Math::Floor(seconds / segmentSeconds);
            secondsInSegment += fmod(seconds, segmentSeconds);
            if (secondsInSegment >= segmentSeconds) ++beginSegmentID, secondsInSegment -= segmentSeconds;
        };

        // Downloads the first segment at the lowest bitrates.
        DownloadSegment(0, vector(tileCount, 0));

        // Downloads the remaining segments.
        for (auto endSegmentID = 1; endSegmentID < segmentCount; ++endSegmentID) {
            const auto bufferSeconds = (endSegmentID - beginSegmentID) * segmentSeconds - secondsInSegment;
            if (bufferSeconds > maxBufferSeconds - segmentSeconds) {
                const auto idleSeconds = bufferSeconds + segmentSeconds - maxBufferSeconds;
                networkSimulator.WaitFor(idleSeconds);
                PlayVideo(idleSeconds);
            }

            const auto throughputMbps = throughputPredictor->PredictThroughputMbps();
            const AggregateControllerContext controllerContext = {throughputMbps, bufferSeconds};
            const auto aggregateBitrateMbps = controller->GetAggregateBitrateMbps(controllerContext);

            const auto positions = viewportPredictor->PredictPositions(bufferSeconds, segmentSeconds);
            const auto distribution = viewportSimulator.ToDistribution(positions);
            ranges::copy(distribution, &out.PredictedViewportDistributions[endSegmentID - 1, 0]);
            const span prevDistribution(&out.ViewportDistributions[endSegmentID - 1, 0], tileCount);
            const auto DilatedDistribution = [&](double dilation) {
                const auto dilatedFovDegrees = (1 - dilation) * viewportConfig.FoVDegrees + dilation * 180;
                ViewportSimulator _viewportSimulator({dilatedFovDegrees, viewportConfig.AspectRatio}, tilingCount);
                return _viewportSimulator.ToDistribution(positions);
            };
            const BitrateAllocatorContext allocatorContext =
                {aggregateBitrateMbps, bufferSeconds, distribution, prevDistribution, DilatedDistribution};
            const auto bitrateIDs = allocator->GetBitrateIDs(allocatorContext);

            const auto downloadSeconds = DownloadSegment(endSegmentID, bitrateIDs).Seconds;
            if (downloadSeconds <= bufferSeconds) PlayVideo(downloadSeconds);
            else PlayVideo(bufferSeconds), out.RebufferingSeconds += downloadSeconds - bufferSeconds;
        }

        // Plays the remaining buffer content.
        PlayVideo((segmentCount - beginSegmentID) * segmentSeconds - secondsInSegment);
    }

    /// Simulates a 360° adaptive bitrate streaming configuration on a collection of network series and viewport series.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param controllerOptions The options for the aggregate controller.
    /// @param allocatorOptions The options for the bitrate allocator.
    /// @param networkData A collection of network series.
    /// @param viewportData A collection of viewport series.
    /// @param out The simulation data output.
    /// @param options The options for 360° adaptive bitrate streaming simulation.
    static void Simulate(const StreamingConfig &streamingConfig,
                         const BaseAggregateControllerOptions &controllerOptions,
                         const BaseBitrateAllocatorOptions &allocatorOptions,
                         NetworkDataView networkData,
                         ViewportDataView viewportData,
                         SimulationDataRef out,
                         const ABRSimulation360Options &options = {}) {
        Parallel::For(0, viewportData.PathCount(), [&](int i) {
            Simulate(streamingConfig, controllerOptions, allocatorOptions,
                     networkData[i], viewportData[i], out[i], options);
        });
    }
};
