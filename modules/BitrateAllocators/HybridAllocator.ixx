module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.HybridAllocator;

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

/// Represents the options for a hybrid allocator.
export struct HybridAllocatorOptions : BaseBitrateAllocatorOptions {
    double TrustLevel = 0.5; ///< The trust level in viewport predictions.
};

export {
    DESCRIBE_STRUCT(HybridAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        TrustLevel
                    ))
}

/// A hybrid allocator decides bitrates by optimizing against a convex combination of the uniform viewport distribution and the predicted viewport distribution.
export class HybridAllocator : public BaseBitrateAllocator {
    double _trustLevel;

public:
    /// Creates a hybrid allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the hybrid allocator.
    explicit HybridAllocator(const StreamingConfig &streamingConfig, const HybridAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _trustLevel(options.TrustLevel) {
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto aggregateBitrateMbps = context.AggregateBitrateMbps;
        const auto distribution = context.ViewportDistribution;
        const auto mixedDistribution = views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return distribution[tileID] * _trustLevel + (1 - _trustLevel) / _tileCount;
        }) | ranges::to<vector>();
        return FromViewportDistribution(aggregateBitrateMbps, mixedDistribution);
    }
};
