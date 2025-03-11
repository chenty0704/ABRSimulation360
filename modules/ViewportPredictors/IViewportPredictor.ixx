module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.IViewportPredictor;

import System.Base;

import ABRSimulation360.Base;

using namespace std;

/// Represents the base options for a viewport predictor.
export struct BaseViewportPredictorOptions {
    virtual ~BaseViewportPredictorOptions() = default;
};

export {
    DESCRIBE_STRUCT(BaseViewportPredictorOptions, (), ())
}

/// Defines the interface of a viewport predictor.
export class IViewportPredictor {
public:
    virtual ~IViewportPredictor() = default;

    /// Updates the viewport predictor with the latest viewport positions.
    /// @param positions A list of viewport positions.
    virtual void Update(span<const SphericalPosition> positions) = 0;

    /// Predicts a list of viewport positions within the specified prediction window.
    /// @param offsetSeconds The offset of the prediction window in seconds.
    /// @param windowSeconds The length of the prediction window in seconds.
    /// @returns A list of predicted viewport positions within the specified prediction window.
    [[nodiscard]] virtual vector<SphericalPosition>
    PredictPositions(double offsetSeconds, double windowSeconds) const = 0;
};

/// Provides a skeletal implementation of a viewport predictor.
export class BaseViewportPredictor : public IViewportPredictor {
protected:
    double _intervalSeconds;

    explicit BaseViewportPredictor(double intervalSeconds, const BaseViewportPredictorOptions & = {}) :
        _intervalSeconds(intervalSeconds) {
    }
};
