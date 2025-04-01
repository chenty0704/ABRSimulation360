#include <LibraryLinkUtilities/Macros.h>

import LibraryLinkUtilities.Base;
import LibraryLinkUtilities.MArgumentQueue;
import LibraryLinkUtilities.TimeSeries;
import System.Base;
import System.JSON;
import System.Math;
import System.MDArray;

import ABRSimulation360.ABRSimulator360;
import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.AggregateControllers.ThroughputBasedController;
import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.BOLAAllocator;
import ABRSimulation360.BitrateAllocators.DragonflyAllocator;
import ABRSimulation360.BitrateAllocators.FlareAllocator;
import ABRSimulation360.BitrateAllocators.HybridAllocator;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;
import ABRSimulation360.BitrateAllocators.OnlineLearningAllocator;
import ABRSimulation360.ThroughputPredictors.EMAPredictor;
import ABRSimulation360.ThroughputPredictors.IThroughputPredictor;
import ABRSimulation360.ThroughputPredictors.MovingAveragePredictor;
import ABRSimulation360.ViewportPredictionSimulator;
import ABRSimulation360.ViewportPredictors.GravitationalPredictor;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;
import ABRSimulation360.ViewportPredictors.LinearPredictor;
import ABRSimulation360.ViewportPredictors.NavGraphPredictor;
import ABRSimulation360.ViewportPredictors.OfflinePredictor;
import ABRSimulation360.ViewportPredictors.StaticPredictor;

using namespace std;
using namespace experimental;

LLU_GENERATE_TIME_SERIES_VIEW_GETTER(double)

LLU_GENERATE_TIME_SERIES_VIEW_GETTER(SphericalPosition)

LLU_GENERATE_ABSTRACT_STRUCT_GETTER(BaseThroughputPredictorOptions, (
                                        EMAPredictorOptions,
                                        MovingAveragePredictorOptions
                                    ))

LLU_GENERATE_ABSTRACT_STRUCT_GETTER(BaseViewportPredictorOptions, (
                                        GravitationalPredictorOptions,
                                        LinearPredictorOptions,
                                        NavGraphPredictorOptions,
                                        OfflinePredictorOptions,
                                        StaticPredictorOptions
                                    ))

LLU_GENERATE_ABSTRACT_STRUCT_GETTER(BaseAggregateControllerOptions, (
                                        ThroughputBasedControllerOptions
                                    ))

LLU_GENERATE_ABSTRACT_STRUCT_GETTER(BaseBitrateAllocatorOptions, (
                                        BOLAAllocatorOptions,
                                        DragonflyAllocatorOptions,
                                        FlareAllocatorOptions,
                                        HybridAllocatorOptions,
                                        OnlineLearningAllocatorOptions
                                    ))

extern "C" __declspec(dllexport)
int WolframLibrary_initialize(WolframLibraryData libraryData) {
    return LLU::TryInvoke([&] {
        LLU::LibraryData::setLibraryData(libraryData);
        LLU::ErrorManager::registerPacletErrors(LLU::PacletErrors);
    });
}

/// Simulates a viewport predictor on a collection of viewport series.
/// @param windowLength [Integer] The number of segments in the prediction window.
/// @param segmentSeconds [Real] The segment duration in seconds.
/// @param predictorOptions ["TypedOptions"] The options for the viewport predictor.
/// @param viewportData [LibraryDataType[TemporalData, Real]] A collection of viewport series.
/// @returns [{Real, 4}] A 3D array of predicted viewport positions.
extern "C" __declspec(dllexport)
int ViewportPredictionSimulate(WolframLibraryData, int64_t argc, MArgument *args, MArgument out) {
    return LLU::TryInvoke([&] {
        LLU::MArgumentQueue argQueue(argc, args, out);
        const auto windowLength = argQueue.Pop<int>();
        const auto segmentSeconds = argQueue.Pop<double>();
        const auto predictorOptions = argQueue.Pop<unique_ptr<BaseViewportPredictorOptions>>();
        const auto viewportData = argQueue.Pop<ViewportDataView>();

        const auto sessionCount = viewportData.PathCount();
        const auto segmentLength = Math::Round(segmentSeconds / viewportData.IntervalSeconds);
        LLU::Tensor simulationData(0., {
                                       sessionCount, windowLength,
                                       viewportData.PathLength() - windowLength * segmentLength, 2
                                   });
        const mdspan<SphericalPosition, dims<3>> _simulationData(
            reinterpret_cast<SphericalPosition *>(simulationData.data()),
            simulationData.dimension(0), simulationData.dimension(1), simulationData.dimension(2));
        ViewportPredictionSimulator::Simulate(windowLength, segmentSeconds, *predictorOptions, viewportData,
                                              _simulationData);
        argQueue.SetOutput(simulationData);
    });
}

/// Simulates a 360° adaptive bitrate streaming configuration on a collection of network series and viewport series.
/// @param streamingConfig ["Object"] The adaptive bitrate streaming configuration.
/// @param controllerOptions ["TypedOptions"] The options for the aggregate controller.
/// @param allocatorOptions ["TypedOptions"] The options for the bitrate allocator.
/// @param networkData [LibraryDataType[TemporalData, Real]] A collection of network series.
/// @param viewportData [LibraryDataType[TemporalData, Real]] A collection of viewport series.
/// @param throughputPredictorOptions ["TypedOptions"] The options for the throughput predictor.
/// @param viewportPredictorOptions ["TypedOptions"] The options for the viewport predictor.
/// @returns ["DataStore"] A collection of simulation series.
extern "C" __declspec(dllexport)
int ABRSimulate360(WolframLibraryData, int64_t argc, MArgument *args, MArgument out) {
    return LLU::TryInvoke([&] {
        LLU::MArgumentQueue argQueue(argc, args, out);
        const auto streamingConfig = argQueue.Pop<StreamingConfig>();
        const auto controllerOptions = argQueue.Pop<unique_ptr<BaseAggregateControllerOptions>>();
        const auto allocatorOptions = argQueue.Pop<unique_ptr<BaseBitrateAllocatorOptions>>();
        const auto networkData = argQueue.Pop<NetworkDataView>();
        const auto viewportData = argQueue.Pop<ViewportDataView>();
        const auto throughputPredictorOptions = argQueue.Pop<unique_ptr<BaseThroughputPredictorOptions>>();
        const auto viewportPredictorOptions = argQueue.Pop<unique_ptr<BaseViewportPredictorOptions>>();

        const auto sessionCount = viewportData.PathCount();
        const auto segmentCount = Math::Round(viewportData.DurationSeconds() / streamingConfig.SegmentSeconds);
        const auto tileCount = streamingConfig.TilingCount * streamingConfig.TilingCount * 6;
        LLU::Tensor rebufferingSeconds(0., {sessionCount});
        LLU::Tensor bufferedBitratesMbps(0., {sessionCount, segmentCount, tileCount});
        LLU::Tensor distributions(0., {sessionCount, segmentCount, tileCount});
        const SimulationDataRef simulationData = {
            rebufferingSeconds, LLU::ToMDSpan<double, dims<3>>(bufferedBitratesMbps),
            LLU::ToMDSpan<double, dims<3>>(distributions)
        };
        ABRSimulator360::Simulate(streamingConfig, *controllerOptions, *allocatorOptions,
                                  networkData, viewportData, simulationData,
                                  {*throughputPredictorOptions, *viewportPredictorOptions});

        LLU::DataList<LLU::NodeType::Any> _out;
        _out.push_back("RebufferingSeconds", move(rebufferingSeconds));
        _out.push_back("BufferedBitratesMbps", move(bufferedBitratesMbps));
        _out.push_back("ViewportDistributions", move(distributions));
        argQueue.SetOutput(_out);
    });
}
