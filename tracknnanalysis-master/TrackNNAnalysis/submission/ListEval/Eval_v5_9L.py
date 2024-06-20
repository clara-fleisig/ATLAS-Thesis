################################################
# v5 for 9L                                   #
################################################
def getEvalRuns():
    folderToRun = {}
    folderInfo = {}
    folderInfo["path"]      = "HTTData/may3+4/region0/9L/nominal/"
    folderInfo["nHits"]     = 9
    folderInfo["LayerRegion"] = "R0"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_9L_Nom_v5.onnx"
    folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_9L_Nom_v5"] = folderInfo

    folderInfo = {}
    folderInfo["path"]      = "HTTData/may3+4/region1/9L/nominal/"
    folderInfo["nHits"]     = 9
    folderInfo["LayerRegion"] = "R1"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_9L_Nom_v5.onnx"
    folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_9L_Nom_v5"] = folderInfo

    folderInfo = {}
    folderInfo["path"]      = "HTTData/may3+4/region3/9L/nominal/"
    folderInfo["nHits"]     = 9
    folderInfo["LayerRegion"] = "R3"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_9L_Nom_v5.onnx"
    folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_9L_Nom_v5"] = folderInfo

    return folderToRun

