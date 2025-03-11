module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.StaticPredictor;

import System.Base;
import System.Math;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;

using namespace std;

/// Represents the options for a static predictor.
export struct StaticPredictorOptions : BaseViewportPredictorOptions {
};

export {
    DESCRIBE_STRUCT(StaticPredictorOptions, (BaseViewportPredictorOptions), ())
}

/// A static predictor predicts viewport positions using only the previous viewport position.
export class StaticPredictor : public BaseViewportPredictor {
    SphericalPosition _prevPosition = {};

public:
    /// Creates a static predictor with the specified configuration and options.
    /// @param intervalSeconds The interval between two samples in seconds.
    /// @param options The options for the static predictor.
    explicit StaticPredictor(double intervalSeconds, const StaticPredictorOptions &options = {}) :
        BaseViewportPredictor(intervalSeconds, options) {
    }

    void Update(span<const SphericalPosition> positions) override {
        _prevPosition = positions.back();
    }

    [[nodiscard]] vector<SphericalPosition> PredictPositions(double, double windowSeconds) const override {
        const auto windowLength = Math::Round(windowSeconds / _intervalSeconds);
        return {static_cast<size_t>(windowLength), _prevPosition};
    }
};
