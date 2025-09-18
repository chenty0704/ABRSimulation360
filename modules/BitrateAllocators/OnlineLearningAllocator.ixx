module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.OnlineLearningAllocator;

import System.Base;
import System.Math;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;

using namespace std;

/// Represents the options for an online learning allocator.
export struct OnlineLearningAllocatorOptions : BaseBitrateAllocatorOptions {
    double InitialTrustLevel = 0.5; ///< The initial trust level in viewport predictions.
    double LearningRate = 0.2; ///< The learning rate for the trust level.
    pair<double, double> TrustLevelRange = {0., 0.95}; ///< The clipping range of the trust level.
    filesystem::path LogPath; ///< The path to log trust levels.
};

export {
    DESCRIBE_STRUCT(OnlineLearningAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        InitialTrustLevel,
                        LearningRate,
                        TrustLevelRange,
                        LogPath
                    ))
}

/// An online learning allocator dynamically adapts the trust level in viewport predictions when deciding bitrates.
export class OnlineLearningAllocator : public BaseBitrateAllocator {
    double _trustLevel;
    double _learningRate;
    double _minTrustLevel, _maxTrustLevel;
    ofstream _logStream;

    vector<double> _prevPredictedDistribution;
    vector<double> _prevMixedDistribution;

public:
    /// Creates an online learning allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the hybrid allocator.
    explicit OnlineLearningAllocator(const StreamingConfig &streamingConfig,
                                     const OnlineLearningAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _trustLevel(options.InitialTrustLevel),
        _learningRate(options.LearningRate), _minTrustLevel(options.TrustLevelRange.first),
        _maxTrustLevel(options.TrustLevelRange.second) {
        if (!options.LogPath.empty()) _logStream.open(options.LogPath);
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto aggregateBitrateMbps = context.AggregateBitrateMbps;
        const auto predictedDistribution = context.ViewportDistribution;
        const auto prevDistribution = context.PrevViewportDistribution;

        if (!_prevMixedDistribution.empty()) {
            const auto dPrevMixedDistribution = _prevPredictedDistribution - 1. / _tileCount;
            const auto dByPrevMixedDistribution = prevDistribution / _prevMixedDistribution;
            const auto derivative = Math::Dot(dByPrevMixedDistribution, dPrevMixedDistribution);
            _trustLevel = clamp(_trustLevel + _learningRate * derivative, _minTrustLevel, _maxTrustLevel);
        }
        if (_logStream.is_open()) println(_logStream, "{}", _trustLevel);

        _prevPredictedDistribution = vector(from_range, predictedDistribution);
        const auto mixedDistribution = views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return predictedDistribution[tileID] * _trustLevel + (1 - _trustLevel) / _tileCount;
        }) | ranges::to<vector>();
        return FromViewportDistribution(aggregateBitrateMbps, _prevMixedDistribution = mixedDistribution);
    }
};
