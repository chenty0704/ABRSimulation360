module;

#include <System/Macros.h>

export module ABRSimulation360.ThroughputPredictors.ThroughputPredictorFactory;

import System.Base;

import ABRSimulation360.ThroughputPredictors.EMAPredictor;
import ABRSimulation360.ThroughputPredictors.IThroughputPredictor;
import ABRSimulation360.ThroughputPredictors.MovingAveragePredictor;

using namespace std;

#define TRY_CREATE(T) \
    if (const auto *const _options = dynamic_cast<const CONCAT(T, Options) *>(&options)) \
        return make_unique<T>(*_options);

/// Represents a factory for throughput predictors.
export class ThroughputPredictorFactory {
public:
    /// Creates a throughput predictor with the specified options.
    /// @param options The options for the throughput predictor.
    /// @returns A throughput predictor with the specified options.
    [[nodiscard]] static unique_ptr<IThroughputPredictor> Create(const BaseThroughputPredictorOptions &options) {
        FOR_EACH(TRY_CREATE, (
                     EMAPredictor,
                     MovingAveragePredictor
                 ))
        throw runtime_error("Unknown throughput predictor options.");
    }
};
