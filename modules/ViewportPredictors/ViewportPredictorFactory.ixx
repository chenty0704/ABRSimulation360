module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.ViewportPredictorFactory;

import System.Base;

import ABRSimulation360.ViewportPredictors.GravitationalPredictor;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;
import ABRSimulation360.ViewportPredictors.LinearPredictor;
import ABRSimulation360.ViewportPredictors.StaticPredictor;

using namespace std;

#define TRY_CREATE(T) \
    if (const auto *const _options = dynamic_cast<const CONCAT(T, Options) *>(&options)) \
        return make_unique<T>(intervalSeconds, *_options);

/// Represents a factory for viewport predictors.
export class ViewportPredictorFactory {
public:
    /// Creates a viewport predictor with the specified configuration and options.
    /// @param intervalSeconds The interval between two samples in seconds.
    /// @param options The options for the viewport predictor.
    /// @returns A viewport predictor with the specified configuration and options.
    [[nodiscard]] static unique_ptr<IViewportPredictor> Create(double intervalSeconds,
                                                               const BaseViewportPredictorOptions &options) {
        FOR_EACH(TRY_CREATE, (
                     GravitationalPredictor,
                     LinearPredictor,
                     StaticPredictor
                 ))
        throw runtime_error("Unknown viewport predictor options.");
    }
};
