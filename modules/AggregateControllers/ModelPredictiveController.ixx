module;

#include <System/Macros.h>

export module ABRSimulation360.AggregateControllers.ModelPredictiveController;

import System.Base;
import System.Math;

import ABRSimulation360.AggregateControllers.IAggregateController;
import ABRSimulation360.Base;

using namespace std;

/// Represents the options for a model predictive controller.
export struct ModelPredictiveControllerOptions : BaseAggregateControllerOptions {
    int WindowLength = 4; ///< The number of segments in the optimization window.
    double TargetBufferRatio = 0.6; ///< The target buffer level (normalized by the maximum buffer level).
    double BufferCostWeight = 1.; ///< The weight of buffer cost.
    double SwitchingCostWeight = 0.5; ///< The weight of switching cost.
};

export {
    DESCRIBE_STRUCT(ModelPredictiveControllerOptions, (BaseAggregateControllerOptions), (
                        WindowLength,
                        TargetBufferRatio,
                        BufferCostWeight,
                        SwitchingCostWeight
                    ))
}

/// A model predictive controller decides aggregate bitrates by optimizing an objective function over multiple segments.
export class ModelPredictiveController : public BaseDiscreteAggregateController {
    int _windowLength;
    double _targetBufferSeconds;
    double _bufferCostWeight, _switchingCostWeight;

    int _prevBitrateID = 0;

public:
    /// Creates a model predictive controller with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the model predictive controller.
    explicit ModelPredictiveController(const StreamingConfig &streamingConfig,
                                       const ModelPredictiveControllerOptions &options = {}) :
        BaseDiscreteAggregateController(streamingConfig, options), _windowLength(options.WindowLength),
        _targetBufferSeconds(options.TargetBufferRatio * _maxBufferSeconds),
        _bufferCostWeight(options.BufferCostWeight), _switchingCostWeight(options.SwitchingCostWeight) {
    }

    [[nodiscard]] double GetAggregateBitrateMbps(const AggregateControllerContext &context) override {
        const auto throughputMbps = context.ThroughputMbps * _throughputDiscount;
        const auto downloadSeconds = _bitratesMbps | views::transform([&](double bitrateMbps) {
            return bitrateMbps * _segmentSeconds / throughputMbps;
        }) | ranges::to<vector>();

        const function<pair<optional<int>, double>(int, double, int, int)> OptBitrateIDAndObjective =
            [&](int segmentID, double bufferSeconds, int prevBitrateID, int step) {
            const auto endBitrateID = step == -1 ? -1 : static_cast<int>(_bitratesMbps.size());

            optional<int> optBitrateID;
            auto optObjective = -numeric_limits<double>::infinity();
            for (auto bitrateID = prevBitrateID; bitrateID != endBitrateID; bitrateID += step) {
                if (downloadSeconds[bitrateID] > bufferSeconds) continue;

                auto nextBufferSeconds = bufferSeconds - downloadSeconds[bitrateID];
                auto objective = Objective(bitrateID, nextBufferSeconds, prevBitrateID);
                nextBufferSeconds = min(nextBufferSeconds + _segmentSeconds, _maxBufferSeconds);
                if (segmentID < _windowLength - 1) {
                    const auto [optNextBitrateID, optFutureObjective]
                        = OptBitrateIDAndObjective(segmentID + 1, nextBufferSeconds, bitrateID, step);
                    if (!optNextBitrateID) continue;
                    objective += optFutureObjective;
                }
                if (objective > optObjective) optBitrateID = bitrateID, optObjective = objective;
            }
            return pair(optBitrateID, optObjective);
        };

        const auto bufferSeconds = context.BufferSeconds;
        const auto [optBitrateID0, optObjective0] = OptBitrateIDAndObjective(0, bufferSeconds, _prevBitrateID, -1);
        if (!optBitrateID0) return _bitratesMbps[_prevBitrateID = 0];
        const auto [optBitrateID1, optObjective1] = OptBitrateIDAndObjective(0, bufferSeconds, _prevBitrateID, 1);
        return _bitratesMbps[_prevBitrateID = optObjective1 > optObjective0 ? *optBitrateID1 : *optBitrateID0];
    }

private:
    [[nodiscard]] double BufferCost(double bufferSeconds) const {
        const auto deviation = max(1 - bufferSeconds / _targetBufferSeconds, 0.);
        return deviation * deviation;
    }

    [[nodiscard]] double SwitchingCost(int prevBitrateID, int bitrateID) const {
        return Math::Abs(_utilities[bitrateID] - _utilities[prevBitrateID]);
    }

    [[nodiscard]] double Objective(int bitrateID, double bufferSeconds, int prevBitrateID) const {
        return _utilities[bitrateID] * _segmentSeconds - _bufferCostWeight * BufferCost(bufferSeconds)
            - _switchingCostWeight * SwitchingCost(prevBitrateID, bitrateID);
    }
};
