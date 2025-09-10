module;

#pragma warning(disable: 4275)

#define XMGLOBALCONST inline const

#include <directxtk12/SimpleMath.h>

export module ABRSimulation360.ViewportSimulator;

import System.Base;
import System.Math;
import System.MDArray;

import ABRSimulation360.Base;

using namespace std;
using namespace experimental;
using namespace DirectX;
using namespace SimpleMath;

/// Simulates the field of view of a viewport.
export class ViewportSimulator {
    using Extents = extents<size_t, 6, dynamic_extent, dynamic_extent>;

    BoundingFrustum _frustum;
    int _tilingCount;
    mdarray<Vector3, Extents> _tileCorners;

public:
    /// Creates a viewport simulator with the specified configuration.
    /// @param config The viewport configuration.
    /// @param tilingCount The number of tiles in each direction on a cubemap face.
    ViewportSimulator(ViewportConfig config, int tilingCount) : _tilingCount(tilingCount) {
        const auto fovRadians = XMConvertToRadians(static_cast<float>(config.FoVDegrees));
        const auto aspectRatio = static_cast<float>(config.AspectRatio);
        const auto projection = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, 0.01f, 1000.f);
        BoundingFrustum::CreateFromMatrix(_frustum, projection);

        const auto implTilingCount = max(_tilingCount, 2);
        const auto tileWidth = 1 / static_cast<float>(implTilingCount);
        _tileCorners = mdarray<Vector3, Extents>(implTilingCount + 1, implTilingCount + 1);
        for (const auto sign : {-1, 1}) {
            const auto tileCorners = submdspan(_tileCorners.to_mdspan(), sign == -1 ? 0 : 1, full_extent, full_extent);
            for (auto x = 0; x <= implTilingCount; ++x)
                for (auto y = 0; y <= implTilingCount; ++y)
                    tileCorners[x, y] = {sign * 0.5f, -0.5f + y * tileWidth, sign * (0.5f - x * tileWidth)};
        }
        for (const auto sign : {-1, 1}) {
            const auto tileCorners = submdspan(_tileCorners.to_mdspan(), sign == -1 ? 2 : 3, full_extent, full_extent);
            for (auto x = 0; x <= implTilingCount; ++x)
                for (auto y = 0; y <= implTilingCount; ++y)
                    tileCorners[x, y] = {-0.5f + x * tileWidth, sign * 0.5f, sign * (0.5f - y * tileWidth)};
        }
        for (const auto sign : {-1, 1}) {
            const auto tileCorners = submdspan(_tileCorners.to_mdspan(), sign == -1 ? 4 : 5, full_extent, full_extent);
            for (auto x = 0; x <= implTilingCount; ++x)
                for (auto y = 0; y <= implTilingCount; ++y)
                    tileCorners[x, y] = {-sign * (0.5f - x * tileWidth), -0.5f + y * tileWidth, sign * 0.5f};
        }
    }

    /// Converts a viewport position to viewport distribution.
    /// @param position A viewport position.
    /// @returns The viewport distribution corresponding to the viewport position.
    [[nodiscard]] vector<double> ToDistribution(SphericalPosition position) {
        const auto yawRadians = XMConvertToRadians(static_cast<float>(position.YawDegrees)),
                   pitchRadians = XMConvertToRadians(static_cast<float>(position.PitchDegrees));
        _frustum.Orientation = Quaternion::CreateFromYawPitchRoll(yawRadians, pitchRadians, 0.f);

        mdarray<bool, Extents> cornerVisibilities(_tileCorners.extents());
        ranges::transform(_tileCorners.container(), cornerVisibilities.container().begin(),
                          [&](const Vector3 &corner) { return _frustum.Contains(corner) == CONTAINS; });

        const auto implTilingCount = max(_tilingCount, 2);
        mdarray<bool, Extents> tileVisibilities(implTilingCount, implTilingCount);
        for (auto faceID = 0; faceID < 6; ++faceID)
            for (auto x = 0; x < implTilingCount; ++x)
                for (auto y = 0; y < implTilingCount; ++y)
                    tileVisibilities[faceID, x, y] =
                        cornerVisibilities[faceID, x, y] || cornerVisibilities[faceID, x, y + 1]
                        || cornerVisibilities[faceID, x + 1, y] || cornerVisibilities[faceID, x + 1, y + 1];

        vector distribution(6 * _tilingCount * _tilingCount, 0.);
        if (_tilingCount > 1)
            ranges::transform(tileVisibilities.container(), distribution.begin(),
                              [](bool isVisible) { return static_cast<double>(isVisible); });
        else
            for (auto faceID = 0; faceID < 6; ++faceID)
                distribution[faceID] = tileVisibilities[faceID, 0, 0] || tileVisibilities[faceID, 0, 1]
                    || tileVisibilities[faceID, 1, 0] || tileVisibilities[faceID, 1, 1];
        distribution /= Math::Total(distribution);
        return distribution;
    }

    /// Converts a list of viewport positions to viewport distribution.
    /// @param positions A list of viewport positions.
    /// @returns The viewport distribution corresponding to the list of viewport positions.
    [[nodiscard]] vector<double> ToDistribution(span<const SphericalPosition> positions) {
        vector distribution(6 * _tilingCount * _tilingCount, 0.);
        for (const auto position : positions) distribution += ToDistribution(position);
        distribution /= static_cast<double>(positions.size());
        return distribution;
    }
};
