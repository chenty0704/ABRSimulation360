BeginPackage["ABRSimulation360`", {"LibraryLinkUtilities`", "Utilities`"}];

ViewportPredictionSimulate::usage = UsageString@"ViewportPredictionSimulate[`windowLength`, `segmentSeconds`, `predictor`, `viewportData`] simulates a viewport predictor on a collection of viewport series.";

Begin["`Private`"];

$ContextAliases["LLU`"] = "LibraryLinkUtilities`Private`LLU`";

LLU`InitializePacletLibrary["ABRSimulation360Link"];

LLU`PacletFunctionSet[$ViewportPredictionSimulate,
    {Integer, Real, "TypedOptions", LibraryDataType[TemporalData, Real]}, {Real, 4}];

ViewportPredictionSimulate[windowLength_Integer, segmentSeconds_Real,
    predictor : _String | _List, viewportData_TemporalData] :=
        $ViewportPredictionSimulate[windowLength, segmentSeconds, predictor, viewportData];

End[];

EndPackage[];
