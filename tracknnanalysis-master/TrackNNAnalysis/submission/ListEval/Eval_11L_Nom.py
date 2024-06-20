################################################
# 1L Nom                                      #
################################################
def getEvalRuns():

    folderToRun = {}


    # ############## 11L
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0_11L-noinnerpix-firstpass//"
    # folderInfo["nHits"]     = 11
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_11L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_11L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0_11L-noinnerpix-firstpass//"
    # folderInfo["nHits"]     = 11
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_v6"] = folderInfo


    folderInfo = {}
    folderInfo["path"]      = "HTTData/region0_11L_stub_d03_singlemu/"
    folderInfo["nHits"]     = 11
    folderInfo["LayerRegion"] = "R0"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_v2.onnx"
    folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_SingleMu_v2"] = folderInfo


    folderInfo = {}
    folderInfo["path"]      = "HTTData/region0_11L_stub_d03_ttbar_onemuon/"
    folderInfo["nHits"]     = 11
    folderInfo["LayerRegion"] = "R0"
    folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_v2.onnx"
    folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_11L_Nom_v2"] = folderInfo


    return folderToRun

