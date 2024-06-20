################################################
# 12L Nom                                      #
################################################
def getEvalRuns():

    folderToRun = {}

    folderInfo = {}
    folderInfo["path"]      = "HTTData/region0_12L_stub_d09_ttbar_onemuon/"
    folderInfo["nHits"]     = 11
    folderInfo["LayerRegion"] = "R0"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrue_12L_Nom_v2.onnx"
    folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_12L_Nom_v2"] = folderInfo



    return folderToRun

