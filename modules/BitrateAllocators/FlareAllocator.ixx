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
    double AccuracySmoothingWeight = 0.5; ///< The smoothing weight for viewport prediction accuracy.
};

export {
    DESCRIBE_STRUCT(FlareAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        AccuracySmoothingWeight
                    ))
}

/// A Flare allocator fetches a variable number of out-of-sight tiles based on prediction accuracy.
export class FlareAllocator : public BaseBitrateAllocator {
    double _accuracySmoothingWeight;

    double _accuracy = 0.5;
    vector<double> _prevPredictedDistribution;

public:
    /// Creates a Flare allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the Flare allocator.
    explicit FlareAllocator(const StreamingConfig &streamingConfig, const FlareAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _accuracySmoothingWeight(options.AccuracySmoothingWeight) {
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

        _prevPredictedDistribution = vector(from_range, predictedDistribution);
        const auto mixedDistribution = context.DilatedViewportDistribution(1 - _accuracy);
        return FromViewportDistribution(aggregateBitrateMbps, mixedDistribution);
    }
};
