BeginPackage["ABRSimulation360`", {"LibraryLinkUtilities`", "Utilities`"}];

ViewportPredictionSimulate::usage = UsageString@"ViewportPredictionSimulate[`windowLength`, `segmentSeconds`, `predictor`, `viewportData`] simulates a viewport predictor on a collection of viewport series.";

ABRSimulate360::usage = UsageString@"ABRSimulate360[`streamingConfig`, {`controller`, `allocator`}, {`networkData`, `viewportData`}] simulates a 360\[Degree] adaptive bitrate streaming configuration on a collection of network series and viewport series.";

Begin["`Private`"];

$ContextAliases["LLU`"] = "LibraryLinkUtilities`Private`LLU`";

LLU`InitializePacletLibrary["ABRSimulation360Link"];

LLU`PacletFunctionSet[$ViewportPredictionSimulate,
    {Integer, Real, "TypedOptions", LibraryDataType[TemporalData, Real]}, {Real, 4}];

ViewportPredictionSimulate[windowLength_Integer, segmentSeconds_Real,
    predictor : _String | _List, viewportData_TemporalData] :=
        $ViewportPredictionSimulate[windowLength, segmentSeconds, predictor, viewportData];

LLU`PacletFunctionSet[$ABRSimulate360, {"Object", "TypedOptions", "TypedOptions",
    LibraryDataType[TemporalData, Real], LibraryDataType[TemporalData, Real],
    "TypedOptions", "TypedOptions"}, "DataStore"];

Options[ABRSimulate360] = {
    "ThroughputPredictor" -> "EMAPredictor",
    "ViewportPredictor" -> "StaticPredictor"
};

ABRSimulate360[streamingConfig_Association, {controller : _String | _List, allocator : _String | _List},
    {networkData_TemporalData, viewportData_TemporalData}, options : OptionsPattern[]] :=
        Association @@ $ABRSimulate360[streamingConfig, controller, allocator, networkData, viewportData,
            OptionValue["ThroughputPredictor"], OptionValue["ViewportPredictor"]];

End[];

EndPackage[];
