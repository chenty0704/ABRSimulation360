module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.GravitationalPredictor;

import LibraryLinkUtilities.WXFStream;
import System.Base;
import System.Math;
import System.MDArray;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;

using namespace std;
using namespace experimental;

/// Represents the options for a gravitational predictor.
export struct GravitationalPredictorOptions : BaseViewportPredictorOptions {
    double SmoothingHalfLifeSeconds = 0.5; ///< The half life for velocity smoothing in seconds.
    double GravitationalSpeedDps = 90.; ///< The speed induced by gravity in degrees / second.
    double GravitationalFieldRangeDegrees = 45.; ///< The range of gravitational fields in degrees.
    filesystem::path ClusterDataPath; ///< The path to the cluster data.
};

export {
    DESCRIBE_STRUCT(GravitationalPredictorOptions, (BaseViewportPredictorOptions), (
                        SmoothingHalfLifeSeconds,
                        GravitationalSpeedDps,
                        GravitationalFieldRangeDegrees,
                        ClusterDataPath
                    ))
}

/// A gravitational predictor predicts viewport positions by simulating the kinematics between a viewport and viewport clusters.
export class GravitationalPredictor : public BaseViewportPredictor {
    double _smoothingWeight;
    double _gravitationalSpeed;
    double _gravitationalFieldRangeDegrees;
    mdarray<SphericalPosition, dims<2>> _clusterCenters;
    mdarray<double, dims<2>> _clusterWeights;

    int _time = 0;
    SphericalPosition _prevPosition = {};
    double _pitchVelocity = 0., _yawVelocity = 0.;

public:
    /// Creates a gravitational predictor with the specified configuration and options.
    /// @param intervalSeconds The interval between two samples in seconds.
    /// @param options The options for the gravitational predictor.
    explicit GravitationalPredictor(double intervalSeconds, const GravitationalPredictorOptions &options = {}) :
        BaseViewportPredictor(intervalSeconds, options),
        _smoothingWeight(pow(0.5, intervalSeconds / options.SmoothingHalfLifeSeconds)),
        _gravitationalSpeed(options.GravitationalSpeedDps * intervalSeconds),
        _gravitationalFieldRangeDegrees(options.GravitationalFieldRangeDegrees) {
        LLU::InWXFStream stream(options.ClusterDataPath);
        mdarray<double, extents<size_t, dynamic_extent, dynamic_extent, 2>> clusterCenters;
        stream >> clusterCenters >> _clusterWeights;
        _clusterCenters = mdarray<SphericalPosition, dims<2>>(
            dims<2>(clusterCenters.extent(0), clusterCenters.extent(1)),
            vector(reinterpret_cast<const SphericalPosition *>(clusterCenters.data()),
                   reinterpret_cast<const SphericalPosition *>(clusterCenters.data()) + clusterCenters.size()));
    }

    void Update(span<const SphericalPosition> positions) override {
        _time += static_cast<int>(positions.size());
        UpdateVelocityEstimates(_prevPosition, positions.front());
        for (auto i = 1; i < positions.size(); ++i) UpdateVelocityEstimates(positions[i - 1], positions[i]);
        _prevPosition = positions.back();
    }

    [[nodiscard]] vector<SphericalPosition>
    PredictPositions(double offsetSeconds, double windowSeconds) const override {
        const auto offset = Math::Round(offsetSeconds / _intervalSeconds);
        const auto windowLength = Math::Round(windowSeconds / _intervalSeconds);
        const auto clusterCount = static_cast<int>(_clusterCenters.extent(1));

        auto position = _prevPosition;
        vector<SphericalPosition> predictedPositions(windowLength);
        for (auto k = 0; k < offset + windowLength; ++k) {
            auto pitchVelocity = _pitchVelocity, yawVelocity = _yawVelocity;
            if (const auto t = k + _time; t < _clusterCenters.extent(0)) {
                const span clusterCenters(&_clusterCenters[t, 0], clusterCount);
                const span clusterWeights(&_clusterWeights[t, 0], clusterCount);
                for (auto i = 0; i < clusterCount; ++i) {
                    const auto pitchDiffDegrees = clusterCenters[i].PitchDegrees - position.PitchDegrees,
                               yawDiffDegrees = SphericalPosition::YawDifferenceDegrees(
                                   position.YawDegrees, clusterCenters[i].YawDegrees);
                    pitchVelocity += GravitationalVelocity(pitchDiffDegrees) * clusterWeights[i];
                    yawVelocity += GravitationalVelocity(yawDiffDegrees) * clusterWeights[i];
                }
            }
            position.PitchDegrees = SphericalPosition::ClampPitchDegrees(position.PitchDegrees + pitchVelocity);
            position.YawDegrees = SphericalPosition::WrapYawDegrees(position.YawDegrees + yawVelocity);
            if (k >= offset) predictedPositions[k - offset] = position;
        }
        return predictedPositions;
    }

private:
    void UpdateVelocityEstimates(SphericalPosition prevPosition, SphericalPosition position) {
        const auto pitchVelocity = position.PitchDegrees - prevPosition.PitchDegrees,
                   yawVelocity = SphericalPosition::YawDifferenceDegrees(prevPosition.YawDegrees, position.YawDegrees);
        _pitchVelocity = _smoothingWeight * _pitchVelocity + (1 - _smoothingWeight) * pitchVelocity;
        _yawVelocity = _smoothingWeight * _yawVelocity + (1 - _smoothingWeight) * yawVelocity;
    }

    [[nodiscard]] double GravitationalVelocity(double diffDegrees) const {
        auto direction = 0;
        if (diffDegrees > 0 && diffDegrees <= _gravitationalFieldRangeDegrees) direction = 1;
        else if (diffDegrees >= -_gravitationalFieldRangeDegrees && diffDegrees < 0) direction = -1;
        return _gravitationalSpeed * direction;
    }
};
