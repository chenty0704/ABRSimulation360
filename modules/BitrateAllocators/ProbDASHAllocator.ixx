module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.ProbDASHAllocator;

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

/// Represents the options for a ProbDASH allocator.
export struct ProbDASHAllocatorOptions : BaseBitrateAllocatorOptions {
    double DilationStandardDeviation = 0.15; ///< The standard deviation of the dilation factor.
};

export {
    DESCRIBE_STRUCT(ProbDASHAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        DilationStandardDeviation
                    ))
}

/// A ProbDASH allocator decides bitrates by optimizing against a Gaussian mixture of viewport distributions.
export class ProbDASHAllocator : public BaseBitrateAllocator {
    double _dilationStandardDeviation;

public:
    /// Creates a ProbDASH allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the ProbDASH allocator.
    explicit ProbDASHAllocator(const StreamingConfig &streamingConfig, const ProbDASHAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _dilationStandardDeviation(options.DilationStandardDeviation) {
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto aggregateBitrateMbps = context.AggregateBitrateMbps;
        const auto distribution = context.ViewportDistribution;
        const auto dilatedDistribution1 = context.DilatedViewportDistribution(_dilationStandardDeviation);
        const auto dilatedDistribution2 = context.DilatedViewportDistribution(2 * _dilationStandardDeviation);
        const auto gaussianDistribution = views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return 0.57 * distribution[tileID]
                + 0.35 * dilatedDistribution1[tileID]
                + 0.08 * dilatedDistribution2[tileID];
        }) | ranges::to<vector>();
        return FromViewportDistribution(aggregateBitrateMbps, gaussianDistribution);
    }
};
