module;

#include <System/Macros.h>

export module ABRSimulation360.ViewportPredictors.NavGraphPredictor;

import LibraryLinkUtilities.WXFStream;
import System.Base;
import System.Math;
import System.MDArray;

import ABRSimulation360.Base;
import ABRSimulation360.ViewportPredictors.IViewportPredictor;

using namespace std;
using namespace experimental;

/// Represents the options for a navigation graph predictor.
export struct NavGraphPredictorOptions : BaseViewportPredictorOptions {
    double BinWidthDegrees = 11.25; ///< The width of each bin in degrees.
    filesystem::path NavGraphPath; ///< The path to the navigation graph.
};

export {
    DESCRIBE_STRUCT(NavGraphPredictorOptions, (BaseViewportPredictorOptions), (
                        BinWidthDegrees,
                        NavGraphPath
                    ))
}

/// A navigation graph predictor predicts viewport positions from a navigation graph.
export class NavGraphPredictor : public BaseViewportPredictor {
    using Node = pair<int, int>;

    double _binWidthDegrees;
    vector<map<Node, vector<pair<Node, double>>>> _navGraph;

    int _time = 0;
    Node _prevNode;

public:
    /// Creates a navigation graph predictor with the specified configuration and options.
    /// @param intervalSeconds The interval between two samples in seconds.
    /// @param options The options for the navigation graph predictor.
    explicit NavGraphPredictor(double intervalSeconds, const NavGraphPredictorOptions &options = {}) :
        BaseViewportPredictor(intervalSeconds, options), _binWidthDegrees(options.BinWidthDegrees) {
        LLU::InWXFStream stream(options.NavGraphPath);
        mdarray<int, extents<size_t, dynamic_extent, dynamic_extent, 2>> sourceNodes;
        mdarray<int, extents<size_t, dynamic_extent, dynamic_extent, dynamic_extent, 2>> destinationNodes;
        mdarray<double, dims<3>> probabilities;
        stream >> sourceNodes >> destinationNodes >> probabilities;

        const auto sourceNodeCount = static_cast<int>(sourceNodes.extent(1)),
                   destinationNodeCount = static_cast<int>(destinationNodes.extent(2));
        _navGraph.resize(sourceNodes.extent(0));
        for (auto t = 0; t < _navGraph.size(); ++t) {
            for (auto s = 0; s < sourceNodeCount; ++s) {
                const auto sourceNode = *reinterpret_cast<const Node *>(&sourceNodes[t, s, 0]);
                vector<pair<Node, double>> destinationNodesWithProbabilities;
                for (auto d = 0; d < destinationNodeCount; ++d) {
                    const auto probability = probabilities[t, s, d];
                    if (probability == 0) break;
                    const auto destinationNode = *reinterpret_cast<const Node *>(&destinationNodes[t, s, d, 0]);
                    destinationNodesWithProbabilities.emplace_back(destinationNode, probability);
                }
                if (destinationNodesWithProbabilities.empty()) break;
                _navGraph[t].emplace(sourceNode, move(destinationNodesWithProbabilities));
            }
        }
    }

    void Update(span<const SphericalPosition> positions) override {
        _time += static_cast<int>(positions.size());
        const auto [pitchDegrees, yawDegrees] = positions.back();
        _prevNode = {Math::Round(pitchDegrees / _binWidthDegrees), Math::Round(yawDegrees / _binWidthDegrees)};
    }

    [[nodiscard]] vector<SphericalPosition>
    PredictPositions(double offsetSeconds, double windowSeconds) const override {
        const auto offset = Math::Round(offsetSeconds / _intervalSeconds);
        const auto windowLength = Math::Round(windowSeconds / _intervalSeconds);

        map<Node, double> prevDistribution = {{_prevNode, 1.}};
        vector<SphericalPosition> predictedPositions(windowLength);
        for (auto k = 0; k < offset + windowLength; ++k) {
            map<Node, double> distribution;
            if (const auto t = k + _time; t < _navGraph.size()) {
                const auto &transitions = _navGraph[t];
                for (const auto [prevNode, prevProbability] : prevDistribution)
                    if (const auto it = transitions.find(prevNode); it != transitions.cend())
                        for (auto [destinationNode, probability] : it->second)
                            distribution[destinationNode] += probability * prevProbability;
                    else distribution[prevNode] += prevProbability;
            }
            if (k >= offset)
                for (auto [node, probability] : distribution) {
                    predictedPositions[k - offset].PitchDegrees += node.first * _binWidthDegrees * probability;
                    predictedPositions[k - offset].YawDegrees += node.second * _binWidthDegrees * probability;
                }
            if (!distribution.empty()) prevDistribution = move(distribution);
        }
        return predictedPositions;
    }
};
