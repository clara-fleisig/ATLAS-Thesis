################################################
# ACTS                                     #
################################################
def getEvalRuns():
    folderToRun = {}

    folderMap = {}
    folderMap["SingleMu"] = "TrackNNAnalysis/ONNXModels/v7/DNN_ACTS_SingleMu.onnx"
    folderMap["ttbar"]    = "TrackNNAnalysis/ONNXModels/v7/DNN_ACTS_ttbar.onnx"
    folderMap["All"]      = "TrackNNAnalysis/ONNXModels/v7/DNN_ACTS_all.onnx"

    for key in folderMap:
        # ########## R0
        folderInfo = {}
        folderInfo["path"]              = "pdg13-n1000-mp30-pT10_2022-05-23_-090eta090"
        folderInfo["nHits"]             = 7
        folderInfo["LayerRegion"]    = "barrel"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "RotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderInfo['doACTS']            = True
        folderToRun["DNN_ACTS_singleMu_"+key+"_v7"] = folderInfo


        folderInfo = {}
        folderInfo["path"]              = "Top_all_on-n200-mu0_withOverlay_2022-05-23_-090eta090"
        folderInfo["nHits"]             = 7
        folderInfo["LayerRegion"]    = "barrel"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "RotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderInfo['doACTS']            = True
        folderToRun["DNN_ACTS_ttbarmu0_"+key+"_v7"] = folderInfo

        folderInfo = {}
        folderInfo["path"]              = "Top_all_on-n20-mu40_withOverlay_2022-05-23_-090eta090"
        folderInfo["nHits"]             = 7
        folderInfo["LayerRegion"]    = "barrel"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "RotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderInfo['doACTS']            = True
        folderToRun["DNN_ACTS_ttbarmu40_"+key+"_v7"] = folderInfo

        folderInfo = {}
        folderInfo["path"]              = "Top_all_on-n20-mu200_withOverlay_2022-05-23_-090eta090"
        folderInfo["nHits"]             = 7
        folderInfo["LayerRegion"]    = "barrel"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "RotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderInfo['doACTS']            = True
        folderToRun["DNN_ACTS_ttbarmu200_"+key+"_v7"] = folderInfo



    return folderToRun

