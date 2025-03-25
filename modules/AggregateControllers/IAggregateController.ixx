module;

#include <System/Macros.h>

export module ABRSimulation360.AggregateControllers.IAggregateController;

import System.Base;

import ABRSimulation360.Base;

using namespace std;

/// Represents the base options for an aggregate controller.
export struct BaseAggregateControllerOptions {
    double ThroughputDiscount = 1.; ///< The discount factor for predicted throughputs.

    virtual ~BaseAggregateControllerOptions() = default;
};

export {
    DESCRIBE_STRUCT(BaseAggregateControllerOptions, (), (
                        ThroughputDiscount
                    ))
}

/// Provides context for an aggregate controller.
export struct AggregateControllerContext {
    double ThroughputMbps; ///< The predicted throughput in megabits per second.
    double BufferSeconds; ///< The buffer level in seconds.
};

/// Defines the interface of an aggregate controller.
export class IAggregateController {
public:
    virtual ~IAggregateController() = default;

    /// Gets the aggregate bitrate decision in megabits per second given the specified context.
    /// @param context The context for the aggregate controller.
    /// @returns The aggregate bitrate decision in megabits per second given the specified context.
    [[nodiscard]] virtual double GetAggregateBitrateMbps(const AggregateControllerContext &context) = 0;
};

/// Provides a skeletal implementation of an aggregate controller.
export class BaseAggregateController : public IAggregateController {
protected:
    double _segmentSeconds;
    double _maxBufferSeconds;
    double _throughputDiscount;

    explicit BaseAggregateController(const StreamingConfig &streamingConfig,
                                     const BaseAggregateControllerOptions &options = {}) :
        _segmentSeconds(streamingConfig.SegmentSeconds), _maxBufferSeconds(streamingConfig.MaxBufferSeconds),
        _throughputDiscount(options.ThroughputDiscount) {
    }
};
