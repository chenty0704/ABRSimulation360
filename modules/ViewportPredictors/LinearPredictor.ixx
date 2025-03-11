module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.LinearPredictor;

import System.Base;
import System.Math;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;

using namespace std;

/// Represents the options for a linear predictor.
export struct LinearPredictorOptions : BaseViewportPredictorOptions {
    double HistorySeconds = 0.5; ///< The length of the history in seconds.
};

export {
    DESCRIBE_STRUCT(LinearPredictorOptions, (BaseViewportPredictorOptions), (
                        HistorySeconds
                    ))
}

/// A linear predictor predicts viewport positions using linear regression.
export class LinearPredictor : public BaseViewportPredictor {
    int _historyLength;

    deque<SphericalPosition> _positions;

public:
    /// Creates a linear predictor with the specified configuration and options.
    /// @param intervalSeconds The interval between two samples in seconds.
    /// @param options The options for the linear predictor.
    explicit LinearPredictor(double intervalSeconds, const LinearPredictorOptions &options = {}) :
        BaseViewportPredictor(intervalSeconds, options),
        _historyLength(Math::Round(options.HistorySeconds / intervalSeconds)) {
    }

    void Update(span<const SphericalPosition> positions) override {
        _positions.append_range(positions);
        if (_positions.size() > _historyLength) {
            const auto excessCount = _positions.size() - _historyLength;
            _positions.erase(_positions.cbegin(), _positions.cbegin() + excessCount);
        }
    }

    [[nodiscard]] vector<SphericalPosition>
    PredictPositions(double offsetSeconds, double windowSeconds) const override {
        const auto offset = Math::Round(offsetSeconds / _intervalSeconds);
        const auto windowLength = Math::Round(windowSeconds / _intervalSeconds);
        const auto times = views::iota(-static_cast<int>(_positions.size()), 0) | ranges::to<vector<double>>();
        const auto pitchesDegrees = _positions | views::transform(
            &SphericalPosition::PitchDegrees) | ranges::to<vector>();
        const auto yawsDegrees = _positions | views::transform(&SphericalPosition::YawDegrees) | ranges::to<vector>();

        const auto [pitch0Degrees, pitchVelocity] = LinearRegression(times, pitchesDegrees);
        const auto [yaw0Degrees, yawVelocity] = LinearRegression(
            times, SphericalPosition::UnwrapYawsDegrees(yawsDegrees));
        return views::iota(0, windowLength) | views::transform([&](int i) {
            return SphericalPosition{
                SphericalPosition::ClampPitchDegrees(pitch0Degrees + pitchVelocity * (i + offset)),
                SphericalPosition::WrapYawDegrees(yaw0Degrees + yawVelocity * (i + offset))
            };
        }) | ranges::to<vector>();
    }

private:
    [[nodiscard]] static pair<double, double> LinearRegression(span<const double> inputs, span<const double> outputs) {
        const auto meanInput = Math::Mean(inputs), meanOutput = Math::Mean(outputs);
        const auto inputVariance = Math::Variance(inputs, meanInput);
        const auto covariance = Math::Covariance(inputs, outputs, meanInput, meanOutput);
        const auto slope = covariance / inputVariance;
        const auto intercept = meanOutput - slope * meanInput;
        return {intercept, slope};
    }
};
