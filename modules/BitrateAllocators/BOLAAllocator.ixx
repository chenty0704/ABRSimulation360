module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.BOLAAllocator;

import System.Base;
import System.MDArray;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;
using namespace experimental;

/// Represents the options for a BOLA allocator.
export struct BOLAAllocatorOptions : BaseBitrateAllocatorOptions {
    double BufferWeight = 1.; ///< The weight of buffer stability.
};

export {
    DESCRIBE_STRUCT(BOLAAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        BufferWeight
                    ))
}

/// A BOLA allocator decides bitrates based on buffer level thresholds.
export class BOLAAllocator : public BaseBitrateAllocator {
    double _segmentSeconds;
    double _maxBufferSeconds;
    double _bufferWeight;
    double _controlFactor;

public:
    /// Creates a BOLA allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the BOLA allocator.
    explicit BOLAAllocator(const StreamingConfig &streamingConfig, const BOLAAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _segmentSeconds(streamingConfig.SegmentSeconds),
        _maxBufferSeconds(streamingConfig.MaxBufferSeconds), _bufferWeight(options.BufferWeight),
        _controlFactor((_maxBufferSeconds / _segmentSeconds - 1) / (_bufferWeight + 1)) {
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto bitrateCount = static_cast<int>(_bitratesMbps.size());
        const auto bufferSeconds = context.BufferSeconds;
        const auto distribution = context.ViewportDistribution;

        return views::iota(0, _tileCount) | views::transform([&](int tileID) {
            const auto probability = distribution[tileID];
            const auto objectives = views::iota(0, bitrateCount) | views::transform([&](int bitrateID) {
                return (_controlFactor * (probability * _utilities[bitrateID] + _bufferWeight)
                    - probability * bufferSeconds / _segmentSeconds) / _bitratesMbps[bitrateID];
            }) | ranges::to<vector>();
            return static_cast<int>(ranges::max_element(objectives) - objectives.cbegin());
        }) | ranges::to<vector>();
    }
};
