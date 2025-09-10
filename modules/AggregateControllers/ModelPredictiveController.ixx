module;

#include <System/Macros.h>

export module ABRSimulation360.AggregateControllers.ModelPredictiveController;

import System.Base;

import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.Base;

using namespace std;

/// Represents the options for a model predictive controller.
export struct ModelPredictiveControllerOptions : BaseAggregateControllerOptions {
    int WindowLength = 4; ///< The number of segments in the optimization window.
    double TargetBufferRatio = 0.6; ///< The target buffer level (normalized by the maximum buffer level).
    double BufferCostWeight = 1.; ///< The weight of buffer cost.
};

export {
    DESCRIBE_STRUCT(ModelPredictiveControllerOptions, (BaseAggregateControllerOptions), (
                        WindowLength,
                        TargetBufferRatio,
                        BufferCostWeight
                    ))
}

/// A model predictive controller decides aggregate bitrates by optimizing an objective function over multiple segments.
export class ModelPredictiveController : public BaseAggregateController {
    int _windowLength;
    double _targetBufferSeconds;
    double _bufferCostWeight;

public:
    /// Creates a model predictive controller with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the model predictive controller.
    explicit ModelPredictiveController(const StreamingConfig &streamingConfig,
                                       const ModelPredictiveControllerOptions &options = {}) :
        BaseAggregateController(streamingConfig, options), _windowLength(options.WindowLength),
        _targetBufferSeconds(options.TargetBufferRatio * _maxBufferSeconds),
        _bufferCostWeight(options.BufferCostWeight) {
    }

    [[nodiscard]] double GetAggregateBitrateMbps(const AggregateControllerContext &context) override {
        const auto throughputMbps = context.ThroughputMbps * _throughputDiscount;
        const auto downloadSeconds = _bitratesMbps | views::transform([&](double bitrateMbps) {
            return bitrateMbps * _segmentSeconds / throughputMbps;
        }) | ranges::to<vector>();

        const auto Objective = [&](int bitrateID, double bufferSeconds) {
            return _utilities[bitrateID] * _segmentSeconds - _bufferCostWeight * BufferCost(bufferSeconds);
        };

        const function<pair<optional<int>, double>(int, double, int)> OptBitrateIDAndObjective =
            [&](int segmentID, double bufferSeconds, int prevBitrateID) {
            optional<int> optBitrateID;
            auto optObjective = -numeric_limits<double>::infinity();
            for (auto bitrateID = prevBitrateID; bitrateID < _bitratesMbps.size(); ++bitrateID) {
                if (downloadSeconds[bitrateID] > bufferSeconds) continue;

                auto nextBufferSeconds = bufferSeconds - downloadSeconds[bitrateID];
                auto objective = Objective(bitrateID, nextBufferSeconds);
                nextBufferSeconds = min(nextBufferSeconds + _segmentSeconds, _maxBufferSeconds);
                if (segmentID < _windowLength - 1) {
                    const auto [optNextBitrateID, optFutureObjective]
                        = OptBitrateIDAndObjective(segmentID + 1, nextBufferSeconds, bitrateID);
                    if (!optNextBitrateID) continue;
                    objective += optFutureObjective;
                }
                if (objective > optObjective) optBitrateID = bitrateID, optObjective = objective;
            }
            return pair(optBitrateID, optObjective);
        };

        const auto optBitrateID = OptBitrateIDAndObjective(0, context.BufferSeconds, 0).first;
        return _bitratesMbps[optBitrateID ? *optBitrateID : 0];
    }

private:
    [[nodiscard]] double BufferCost(double bufferSeconds) const {
        const auto deviation = max(1 - bufferSeconds / _targetBufferSeconds, 0.);
        return deviation * deviation;
    }
};
