module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.DragonflyAllocator;

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

/// Represents the options for a Dragonfly allocator.
export struct DragonflyAllocatorOptions : BaseBitrateAllocatorOptions {
    double Dilation = 0.25; ///< The dilation factor for the outer viewport.
};

export {
    DESCRIBE_STRUCT(DragonflyAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        Dilation
                    ))
}

/// A Dragonfly allocator decides bitrates by optimizing against two concentric viewports.
export class DragonflyAllocator : public BaseBitrateAllocator {
    double _dilation;

public:
    /// Creates a Dragonfly allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the Dragonfly allocator.
    explicit DragonflyAllocator(const StreamingConfig &streamingConfig, const DragonflyAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _dilation(options.Dilation) {
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto aggregateBitrateMbps = context.AggregateBitrateMbps;
        const auto distribution = context.ViewportDistribution;
        const auto dilatedDistribution = context.DilatedViewportDistribution(_dilation);
        const auto mixedDistribution = views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return (distribution[tileID] + dilatedDistribution[tileID]) / 2;
        }) | ranges::to<vector>();
        return FromViewportDistribution(aggregateBitrateMbps, mixedDistribution);
    }
};
