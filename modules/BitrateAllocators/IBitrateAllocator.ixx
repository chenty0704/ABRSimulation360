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
    double _segmentSeconds;
    int _tileCount;
    vector<double> _bitratesMbps;
    double _maxBufferSeconds;

    vector<double> _utilities;

    explicit BaseBitrateAllocator(const StreamingConfig &streamingConfig, const BaseBitrateAllocatorOptions & = {}) :
        _segmentSeconds(streamingConfig.SegmentSeconds), _maxBufferSeconds(streamingConfig.MaxBufferSeconds) {
        const auto tileCountPerFace = streamingConfig.TilingCount * streamingConfig.TilingCount;
        _tileCount = tileCountPerFace * 6;
        _bitratesMbps = streamingConfig.BitratesPerFaceMbps / tileCountPerFace;

        const auto minBitrateMbps = _bitratesMbps.front(), maxBitrateMbps = _bitratesMbps.back();
        const auto utilityNormalizer = Math::Log(maxBitrateMbps / minBitrateMbps);
        _utilities = _bitratesMbps | views::transform([&](double bitrateMbps) {
            return Math::Log(bitrateMbps / minBitrateMbps) / utilityNormalizer;
        }) | ranges::to<vector>();
    }

    [[nodiscard]] vector<int> FromViewportDistribution(double aggregateBitrateMbps,
                                                       span<const double> distribution) const {
        vector bitrateIDs(_tileCount, 0);
        auto totalBitrateMbps = _bitratesMbps.front() * _tileCount;
        auto derivatives = views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return ExpectedUtilityDerivative(bitrateIDs[tileID], distribution[tileID]);
        }) | ranges::to<vector>();
        while (true) {
            const auto tileID = static_cast<int>(ranges::max_element(derivatives) - derivatives.cbegin());
            if (derivatives[tileID] == 0.) break;

            totalBitrateMbps += _bitratesMbps[bitrateIDs[tileID] + 1] - _bitratesMbps[bitrateIDs[tileID]];
            if (totalBitrateMbps > aggregateBitrateMbps) break;

            derivatives[tileID] = ExpectedUtilityDerivative(++bitrateIDs[tileID], distribution[tileID]);
        }
        return bitrateIDs;
    }

    [[nodiscard]] double ExpectedUtilityDerivative(int bitrateID, double probability) const {
        if (bitrateID == _bitratesMbps.size() - 1) return 0.;

        const auto expectedUtilityDiff = probability * (_utilities[bitrateID + 1] - _utilities[bitrateID]);
        const auto bitrateDiffMbps = _bitratesMbps[bitrateID + 1] - _bitratesMbps[bitrateID];
        return expectedUtilityDiff / bitrateDiffMbps;
    }
};
