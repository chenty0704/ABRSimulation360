export module ABRSimulation360.ViewportPredictionSimulator;

import System.Base;
import System.Math;
import System.MDArray;
import System.Parallel;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;
import ABRSimulation360.ViewportPredictors.ViewportPredictorFactory;

using namespace std;
using namespace experimental;

/// Simulates the dynamics of viewport prediction.
export class ViewportPredictionSimulator {
public:
    /// Simulates a viewport predictor on a viewport series.
    /// @param windowLength The number of segments in the prediction window.
    /// @param segmentSeconds The segment duration in seconds.
    /// @param predictorOptions The options for the viewport predictor.
    /// @param viewportSeries A viewport series.
    /// @param out A 2D array of predicted viewport positions. The k-th row represents the predictions for each segment made k segments ago.
    static void Simulate(int windowLength, double segmentSeconds,
                         const BaseViewportPredictorOptions &predictorOptions,
                         ViewportSeriesView viewportSeries,
                         mdspan<SphericalPosition, dims<2>> out) {
        const auto intervalSeconds = viewportSeries.IntervalSeconds;
        const auto segmentLength = Math::Round(segmentSeconds / intervalSeconds);
        const auto segmentCount = Math::Round(viewportSeries.DurationSeconds() / segmentSeconds);
        const auto viewportPredictor = ViewportPredictorFactory::Create(intervalSeconds, predictorOptions);

        for (auto i = 0; i < segmentCount - 1; ++i) {
            viewportPredictor->Update(viewportSeries.Window(i * segmentSeconds, segmentSeconds).Values);
            const auto predictedPositions = viewportPredictor->PredictPositions(0., windowLength * segmentSeconds);
            for (auto k = 0; k < windowLength; ++k)
                if (const auto j = i + k - windowLength + 1; j >= 0 && j < segmentCount - windowLength)
                    copy_n(&predictedPositions[k * segmentLength], segmentLength, &out[k, j * segmentLength]);
        }
    }

    /// Simulates a viewport predictor on a collection of viewport series.
    /// @param windowLength The number of segments in the prediction window.
    /// @param segmentSeconds The segment duration in seconds.
    /// @param predictorOptions The options for the viewport predictor.
    /// @param viewportData A collection of viewport series.
    /// @param out A 3D array of predicted viewport positions.
    static void Simulate(int windowLength, double segmentSeconds,
                         const BaseViewportPredictorOptions &predictorOptions,
                         ViewportDataView viewportData,
                         mdspan<SphericalPosition, dims<3>> out) {
        Parallel::For(0, viewportData.PathCount(), [&](int i) {
            const auto _out = submdspan(out, i, full_extent, full_extent);
            Simulate(windowLength, segmentSeconds, predictorOptions, viewportData[i], _out);
        });
    }
};
