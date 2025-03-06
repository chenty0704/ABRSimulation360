module;

#include <System/Macros.h>

export module ABRSimulation360.Base;

import LibraryLinkUtilities.TimeSeries;
import System.Base;

using namespace std;

/// Represents a value and a time duration.
/// @tparam T The type of the value.
export template<typename T>
struct TimedValue {
    T Value; ///< A value.
    double Seconds; ///< A time duration in seconds.

    bool operator==(const TimedValue &) const = default;
    bool operator!=(const TimedValue &) const = default;
};

/// Represents a position on a unit sphere.
export struct SphericalPosition {
    double PitchDegrees; ///< The pitch angle in degrees (from -90 to 90).
    double YawDegrees; ///< The yaw angle in degrees (from -180 to 180).

    bool operator==(const SphericalPosition &) const = default;
    bool operator!=(const SphericalPosition &) const = default;
};

/// Represents the configuration for a 360° adaptive bitrate streaming session.
export struct StreamingConfig {
    double SegmentSeconds; ///< The segment duration in seconds.
    vector<double> BitratesMbps; ///< A list of available bitrates in megabits per second (in ascending order).
    double MaxBufferSeconds; ///< The maximum buffer level in seconds.
};

export {
    DESCRIBE_STRUCT(StreamingConfig, (), (
                        SegmentSeconds,
                        BitratesMbps,
                        MaxBufferSeconds
                    ))
}

/// Represents a constant view of a network series.
/// The values of a network series represent throughputs in megabits per second.
export using NetworkSeriesView = LLU::TimeSeriesView<double>;

/// Represents a constant view of a collection of network series.
/// The values of each network series represent throughputs in megabits per second.
export using NetworkDataView = LLU::TemporalDataView<double>;

/// Represents a constant view of a viewport series.
/// The values of a viewport series represent viewport positions.
export using ViewportSeriesView = LLU::TimeSeriesView<SphericalPosition>;

/// Represents a constant view of a collection of viewport series.
/// The values of each viewport series represent viewport positions.
export using ViewportDataView = LLU::TemporalDataView<SphericalPosition>;
