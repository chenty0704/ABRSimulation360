module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.FlareAllocator;

import System.Base;
import System.Math;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

/// Represents the options for a Flare allocator.
export struct FlareAllocatorOptions : BaseBitrateAllocatorOptions {
    double SwitchingCostWeight = 0.1; ///< The weight of switching cost.
    double InitialAccuracy = 0.5; ///< The initial estimate of viewport prediction accuracy.
    double AccuracySmoothingWeight = 0.5; ///< The smoothing weight for viewport prediction accuracy.
};

export {
    DESCRIBE_STRUCT(FlareAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        InitialAccuracy,
                        AccuracySmoothingWeight
                    ))
}

/// A Flare allocator fetches a variable number of out-of-sight tiles based on prediction accuracy.
export class FlareAllocator : public BaseBitrateAllocator {
    double _switchingCostWeight;
    double _accuracySmoothingWeight;

    double _accuracy;
    vector<double> _prevPredictedDistribution;
    double _prevUtility360 = 0.;

public:
    /// Creates a Flare allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the Flare allocator.
    explicit FlareAllocator(const StreamingConfig &streamingConfig, const FlareAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _switchingCostWeight(options.SwitchingCostWeight),
        _accuracySmoothingWeight(options.AccuracySmoothingWeight), _accuracy(options.InitialAccuracy) {
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto aggregateBitrateMbps = context.AggregateBitrateMbps;
        const auto predictedDistribution = context.ViewportDistribution;
        const auto prevDistribution = context.PrevViewportDistribution;

        if (!_prevPredictedDistribution.empty()) {
            const auto minProbabilities = views::iota(0, _tileCount) | views::transform([&](int tileID) {
                return min(_prevPredictedDistribution[tileID], prevDistribution[tileID]);
            }) | ranges::to<vector>();
            const auto maxProbabilities = views::iota(0, _tileCount) | views::transform([&](int tileID) {
                return max(_prevPredictedDistribution[tileID], prevDistribution[tileID]);
            }) | ranges::to<vector>();
            const auto accuracy = Math::Total(minProbabilities) / Math::Total(maxProbabilities);
            _accuracy = (1 - _accuracySmoothingWeight) * _accuracy + _accuracySmoothingWeight * accuracy;
        }

        const auto mixedDistribution = context.DilatedViewportDistribution(1 - _accuracy);
        const auto [class1Probability, class0Probability] = ranges::minmax(mixedDistribution);
        const auto class0Count = static_cast<int>(ranges::count(mixedDistribution, class0Probability)),
                   class1Count = _tileCount - class0Count;
        const auto totalClass0Probability = class0Probability * class0Count,
                   totalClass1Probability = class1Probability * class1Count;

        optional<int> optBitrateID0, optBitrateID1;
        auto optUtility360 = 0., optObjective = -numeric_limits<double>::infinity();
        for (auto bitrateID0 = 0; bitrateID0 < _bitratesMbps.size(); ++bitrateID0) {
            const auto totalClass0BitrateMbps = _bitratesMbps[bitrateID0] * class0Count;
            if (totalClass0BitrateMbps > aggregateBitrateMbps) break;

            const auto class0Utility = totalClass0Probability * _utilities[bitrateID0];
            for (auto bitrateID1 = 0; bitrateID1 <= bitrateID0; ++bitrateID1) {
                if (totalClass0BitrateMbps + _bitratesMbps[bitrateID1] * class1Count > aggregateBitrateMbps) break;

                const auto utility360 = class0Utility + totalClass1Probability * _utilities[bitrateID1];
                const auto spatialSwitching =
                    Math::Sqrt(totalClass0Probability * Math::Pow(_utilities[bitrateID0] - utility360, 2)
                        + totalClass1Probability * Math::Pow(_utilities[bitrateID1] - utility360, 2));
                const auto temporalSwitching = Math::Abs(utility360 - _prevUtility360);
                const auto objective = utility360 - _switchingCostWeight * (spatialSwitching + temporalSwitching);
                if (objective > optObjective) {
                    optBitrateID0 = bitrateID0, optBitrateID1 = bitrateID1;
                    optUtility360 = utility360, optObjective = objective;
                }
            }
        }

        _prevPredictedDistribution = vector(from_range, predictedDistribution);
        _prevUtility360 = optUtility360;
        if (!optBitrateID0) return vector(_tileCount, 0);
        return views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return mixedDistribution[tileID] == class0Probability ? *optBitrateID0 : *optBitrateID1;
        }) | ranges::to<vector>();
    }
};
