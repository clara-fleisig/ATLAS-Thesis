################################################
# 11L Nom second stage                         #
################################################
def getEvalRuns():

    folderToRun = {}

    # ############## 11L
    folderInfo = {}
    folderInfo["path"]      = "HTTData/region0_nominal_30muonsper_ttbarmu200_2ndstage_test/"
    folderInfo["nHits"]     = 11
    folderInfo["LayerRegion"] = "R0"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTTrue_HTTFake_11L_V2_Nom_2ndStage.onnx"
    folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_SingleMu_secondStage_v1"] = folderInfo

    return folderToRun

