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
    double ViewportRatio = 0.2; ///< The (approximate) viewport coverage ratio.
    double SwitchingCostWeight = 0.1; ///< The weight of switching cost.
    double InitialTrustLevel = 0.5; ///< The initial trust level in viewport predictions.
    double LearningRate = 0.2; ///< The learning rate for the trust level.
    pair<double, double> TrustLevelRange = {0., 0.85}; ///< The clipping range of the trust level.
    filesystem::path LogPath; ///< The path to log trust levels.
};

export {
    DESCRIBE_STRUCT(OnlineLearningAllocatorOptions, (BaseBitrateAllocatorOptions), (
                        ViewportRatio,
                        SwitchingCostWeight,
                        InitialTrustLevel,
                        LearningRate,
                        TrustLevelRange,
                        LogPath
                    ))
}

/// An online learning allocator dynamically adapts the trust level in viewport predictions when deciding bitrates.
export class OnlineLearningAllocator : public BaseBitrateAllocator {
    double _viewportRatio;
    double _switchingCostWeight;
    double _learningRate;
    double _minTrustLevel, _maxTrustLevel;
    ofstream _logStream;

    double _trustLevel;
    vector<double> _prevPredictedDistribution;
    vector<double> _prevMixedDistribution;

public:
    /// Creates an online learning allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the hybrid allocator.
    explicit OnlineLearningAllocator(const StreamingConfig &streamingConfig,
                                     const OnlineLearningAllocatorOptions &options = {}) :
        BaseBitrateAllocator(streamingConfig, options), _viewportRatio(options.ViewportRatio),
        _switchingCostWeight(options.SwitchingCostWeight), _learningRate(options.LearningRate),
        _minTrustLevel(options.TrustLevelRange.first), _maxTrustLevel(options.TrustLevelRange.second),
        _trustLevel(options.InitialTrustLevel) {
        if (!options.LogPath.empty()) _logStream.open(options.LogPath);
    }

    [[nodiscard]] vector<int> GetBitrateIDs(const BitrateAllocatorContext &context) override {
        const auto aggregateBitrateMbps = context.AggregateBitrateMbps;
        const auto predictedDistribution = context.ViewportDistribution;
        const auto prevDistribution = context.PrevViewportDistribution;

        if (!_prevPredictedDistribution.empty()) {
            const auto dPrevMixedDistribution = _prevPredictedDistribution - 1. / _tileCount;
            const auto dByPrevMixedDistribution = prevDistribution / _prevMixedDistribution;
            const auto utilityDerivative = Math::Dot(dByPrevMixedDistribution, dPrevMixedDistribution);
            const auto switchingCostDerivative =
                1 / ((1 - _trustLevel) * (_viewportRatio * (1 - _trustLevel) + _trustLevel));
            const auto derivative = utilityDerivative - _switchingCostWeight * switchingCostDerivative;
            _trustLevel = clamp(_trustLevel + _learningRate * derivative, _minTrustLevel, _maxTrustLevel);
        }
        if (_logStream.is_open()) println(_logStream, "{}", _trustLevel);

        const auto mixedDistribution = views::iota(0, _tileCount) | views::transform([&](int tileID) {
            return predictedDistribution[tileID] * _trustLevel + (1 - _trustLevel) / _tileCount;
        }) | ranges::to<vector>();
        _prevPredictedDistribution = vector(from_range, predictedDistribution);
        return FromViewportDistribution(aggregateBitrateMbps, _prevMixedDistribution = mixedDistribution);
    }
};
