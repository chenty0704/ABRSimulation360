#include <LibraryLinkUtilities/Macros.h>

import LibraryLinkUtilities.Base;
import LibraryLinkUtilities.MArgumentQueue;
import LibraryLinkUtilities.TimeSeries;
import System.Base;
import System.JSON;
import System.Math;
import System.MDArray;

import ABRSimulation360.Base;
import ABRSimulation360.ThroughputPredictors.EMAPredictor;
import ABRSimulation360.ThroughputPredictors.IThroughputPredictor;
import ABRSimulation360.ThroughputPredictors.MovingAveragePredictor;
import ABRSimulation360.ViewportPredictionSimulator;
import ABRSimulation360.ViewportPredictors.GravitationalPredictor;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;
import ABRSimulation360.ViewportPredictors.LinearPredictor;
import ABRSimulation360.ViewportPredictors.NavGraphPredictor;
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
                                        StaticPredictorOptions
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
