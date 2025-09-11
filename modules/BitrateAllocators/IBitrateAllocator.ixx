module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.IBitrateAllocator;

import System.Base;
import System.Math;

import ABRSimulation360.Base;

using namespace std;

/// Represents the base options for a bitrate allocator.
export struct BaseBitrateAllocatorOptions {
    virtual ~BaseBitrateAllocatorOptions() = default;
};

export {
    DESCRIBE_STRUCT(BaseBitrateAllocatorOptions, (), ())
}

/// Provides context for a bitrate allocator.
export struct BitrateAllocatorContext {
    double AggregateBitrateMbps; ///< The aggregate bitrate in megabits per second.
    double BufferSeconds; ///< The buffer level in seconds.
    span<const double> ViewportDistribution; ///< The predicted viewport distribution.
    span<const double> PrevViewportDistribution; ///< The actual viewport distribution of the previous segment.

    /// Returns the dilated viewport distribution.
    /// @param dilation The dilation factor.
    /// @returns The dilated viewport distribution.
    function<vector<double>(double)> DilatedViewportDistribution;
};

/// Defines the interface of a bitrate allocator.
export class IBitrateAllocator {
public:
    virtual ~IBitrateAllocator() = default;

    /// Gets the bitrate decisions for all tiles given the specified context.
    /// @param context The context for the bitrate allocator.
    /// @returns The bitrate decisions for all tiles given the specified context.
    [[nodiscard]] virtual vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) = 0;
};

/// Provides a skeletal implementation of a bitrate allocator.
export class BaseBitrateAllocator : public IBitrateAllocator {
protected:
    int _tileCount;
    vector<double> _bitratesMbps;
    vector<double> _utilities;

    explicit BaseBitrateAllocator(const StreamingConfig &streamingConfig, const BaseBitrateAllocatorOptions & = {}) {
        const auto tileCountPerFace = streamingConfig.TilingCount * streamingConfig.TilingCount;
        _tileCount = tileCountPerFace * 6;
        _bitratesMbps = streamingConfig.BitratesPerFaceMbps / tileCountPerFace;

        const auto minBitrateMbps = _bitratesMbps.front(), maxBitrateMbps = _bitratesMbps.back();
        const auto utilityNormalizer = Math::Log(maxBitrateMbps / minBitrateMbps);
        _utilities = _bitratesMbps | views::transform([&](double bitrateMbps) {
            return Math::Log(bitrateMbps / minBitrateMbps) / utilityNormalizer;
        }) | ranges::to<vector>();
    }

    [[nodiscard]] int BitrateIDBelow(double bitrateMbps) const {
        const auto it = ranges::upper_bound(_bitratesMbps, bitrateMbps);
        return it != _bitratesMbps.cbegin() ? static_cast<int>(it - _bitratesMbps.cbegin()) - 1 : 0;
    }

    [[nodiscard]] vector<int> FromViewportDistribution(double aggregateBitrateMbps,
                                                       span<const double> distribution) const {
        const auto bitratesMbps = distribution * aggregateBitrateMbps;
        return bitratesMbps | views::transform([&](double bitrateMbps) {
            return BitrateIDBelow(bitrateMbps);
        }) | ranges::to<vector>();
    }
};
