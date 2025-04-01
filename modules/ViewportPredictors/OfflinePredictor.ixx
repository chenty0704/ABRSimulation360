module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.OfflinePredictor;

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;

using namespace std;

/// Represents the options for an offline predictor.
export struct OfflinePredictorOptions : BaseViewportPredictorOptions {
    double Randomness = 0.; ///< The randomness in viewport predictions.
};

export {
    DESCRIBE_STRUCT(OfflinePredictorOptions, (BaseViewportPredictorOptions), (
                        Randomness
                    ))
}

/// An offline predictor predicts viewport positions with controlled errors.
export class OfflinePredictor : public BaseViewportPredictor {
    ViewportSeriesView _viewportSeries;
    double _randomness;

    double _seconds = 0.;
    mutable default_random_engine _randomEngine{random_device{}()};

public:
    /// Creates an offline predictor with the specified configuration and options.
    /// @param intervalSeconds The interval between two samples in seconds.
    /// @param options The options for the offline predictor.
    explicit OfflinePredictor(double intervalSeconds, const OfflinePredictorOptions &options = {}) :
        BaseViewportPredictor(intervalSeconds, options), _viewportSeries{intervalSeconds},
        _randomness(options.Randomness) {
    }

    /// Initializes the offline predictor with a viewport series.
    /// @param viewportSeries A viewport series.
    void Initialize(ViewportSeriesView viewportSeries) {
        _viewportSeries = viewportSeries;
    }

    void Update(span<const SphericalPosition> positions) override {
        _seconds += positions.size() * _intervalSeconds;
    }

    [[nodiscard]] vector<SphericalPosition>
    PredictPositions(double offsetSeconds, double windowSeconds) const override {
        const auto positions = _viewportSeries.Window(_seconds + offsetSeconds, windowSeconds).Values;
        if (_randomness == 0.) return {from_range, positions};

        const SphericalPosition randomPosition = {
            uniform_real_distribution(-90., 90.)(_randomEngine),
            uniform_real_distribution(-180., 180.)(_randomEngine)
        };
        return positions | views::transform([&](const SphericalPosition position) {
            return SphericalPosition{
                (1 - _randomness) * position.PitchDegrees + _randomness * randomPosition.PitchDegrees,
                (1 - _randomness) * position.YawDegrees + _randomness * randomPosition.YawDegrees
            };
        }) | ranges::to<vector>();
    }
};
