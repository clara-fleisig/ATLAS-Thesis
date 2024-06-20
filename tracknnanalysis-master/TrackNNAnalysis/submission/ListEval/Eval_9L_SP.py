################################################
# Nom Hits                                     #
################################################
def getEvalRuns():
    folderToRun = {}

    folderMap = {}
    folderMap["HTF_ST_9L_rSP"] = "TrackNNAnalysis/ONNXModels/v8/DNN_R0_HTF_ST_9L_rSP.onnx"
    folderMap["HTF_ST_9L_rSP_big"] = "TrackNNAnalysis/ONNXModels/v8/DNN_R0_HTF_ST_9L_rSP_big.onnx"
    folderMap["HTF_ST_9L_rSP_prune0.5"] = "TrackNNAnalysis/ONNXModels/v8/DNN_R0_HTF_ST_9L_rSP_prune0.50.onnx"
    folderMap["HTF_ST_9L_rSP_prune0.7"] = "TrackNNAnalysis/ONNXModels/v8/DNN_R0_HTF_ST_9L_rSP_prune0.70.onnx"

    for key in folderMap:
        ########## R0
        folderInfo = {}
        folderInfo["path"]              = "HTTData/9L_region0/singlemuons/"
        folderInfo["nHits"]             = 5
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "SPWithRoughRotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_singleMu_"+key+"_v8"] = folderInfo


        # folderInfo = {}
        # folderInfo["path"]              = "HTTData/9L_region0/ttbar_pileup/extra"
        # folderInfo["nHits"]             = 5
        # folderInfo["LayerRegion"]    = "R0"
        # folderInfo["onnxFile"]          = folderMap[key]
        # folderInfo["scaled"]            = True
        # folderInfo["hitType"]           = "SPWithRoughRotatedToZero"
        # folderInfo["order"]             = "XX0YY0ZZ0"
        # folderToRun["DNN_Region_0p1_0p3_ttbar_"+key+"_v8"] = folderInfo

    return folderToRun

   