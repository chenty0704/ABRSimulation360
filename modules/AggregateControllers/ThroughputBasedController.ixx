module;

#include <System/Macros.h>

export module ABRSimulation360.AggregateControllers.ThroughputBasedController;

import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.Base;

/// Represents the options for a throughput-based controller.
export struct ThroughputBasedControllerOptions : BaseAggregateControllerOptions {
};

export {
    DESCRIBE_STRUCT(ThroughputBasedControllerOptions, (BaseAggregateControllerOptions), ())
}

/// A throughput-based controller decides aggregate bitrates using only the predicted throughputs.
export class ThroughputBasedController : public BaseAggregateController {
public:
    /// Creates a throughput-based controller with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the throughput-based controller.
    explicit ThroughputBasedController(const StreamingConfig &streamingConfig,
                                       const ThroughputBasedControllerOptions &options = {}) :
        BaseAggregateController(streamingConfig, options) {
    }

    [[nodiscard]] double GetAggregateBitrateMbps(const AggregateControllerContext &context) override {
        return context.ThroughputMbps * _throughputDiscount;
    }
};
