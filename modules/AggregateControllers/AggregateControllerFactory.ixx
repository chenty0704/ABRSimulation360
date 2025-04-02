module;

#include <System/Macros.h>

export module ABRSimulation360.AggregateControllers.AggregateControllerFactory;

import System.Base;

import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.AggregateControllers.ModelPredictiveController;
import ABRSimulation360.AggregateControllers.ThroughputBasedController;
import ABRSimulation360.Base;

using namespace std;

#define TRY_CREATE(T) \
    if (const auto *const _options = dynamic_cast<const CONCAT(T, Options) *>(&options)) \
        return make_unique<T>(streamingConfig, *_options);

/// Represents a factory for aggregate controllers.
export class AggregateControllerFactory {
public:
    /// Creates an aggregate controller with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the aggregate controller.
    /// @returns An aggregate controller with the specified configuration and options.
    [[nodiscard]] static unique_ptr<IAggregateController> Create(const StreamingConfig &streamingConfig,
                                                                 const BaseAggregateControllerOptions &options) {
        FOR_EACH(TRY_CREATE, (
                     ModelPredictiveController,
                     ThroughputBasedController
                 ))
        throw runtime_error("Unknown aggregate controller options.");
    }
};
