################################################
# ACTS runs                                    #
################################################
def getMaps():
    folderToRun = {}

    datasets = {"ACTS_ttbarWithOverlay_mu40":"Top_all_on-n20-mu40_withOverlay_v2_2022-07-04_-090eta090",
    "ACTS_ttbarWithOverlay_mu200":"Top_all_on-n20-mu200_withOverlay_v2_2022-07-04_-090eta090",
    "ACTS_ttbarWithOverlay_mu0":"Top_all_on-n200-mu0_withOverlay_v2_2022-07-04_-090eta090"}
    #"ACTS_Single_Mu": "pdg13-n2500-mp30-pT100_v3_2022-07-08"}

    # models = ["Extrap_MLP_ACTS_ws3_barrel_layerInfo.onnx", "Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx", 
    #     "Extrap_biRNN_ACTS_ws3_barrel_layerInfo.onnx", "Extrap_ESN_ACTS_ws3_barrel_layerInfo.onnx"]
    models = ["Extrap_MLP_ACTS_ws3_barrel_layerInfo.onnx"]
    for ds in datasets:
        for m in models:
            folderInfo = {}
            folderInfo["path"]              = datasets[ds]
            folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/" + m
            modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
            folderInfo["modelType"]         = modelType
            folderInfo["windowSize"]        = windowSize
            folderToRun[ds + "_" + modelType + "_ws" + windowSize] = folderInfo


    # ############## singleMu
    # folderInfo = {}
    # folderInfo["path"]              = "pdg13-n2500-mp30-pT100_v3_2022-07-08"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_MLP_ACTS_ws3_barrel_layerInfo.onnx"
    # modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
    # folderInfo["modelType"]         = modelType
    # folderInfo["windowSize"]        = windowSize
    # folderToRun["ACTS_SingleMu_" + modelType + "_ws" + windowSize] = folderInfo




    # folderInfo = {}
    # folderInfo["path"]              = "pdg13-n1000-mp30-pT10_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx"
    # modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
    # folderInfo["modelType"]         = modelType
    # folderInfo["windowSize"]        = windowSize
    # folderToRun["ACTS_SingleMu"] = folderInfo
    
    # ############## ttbar
    # folderInfo = {}
    # folderInfo["path"]              = "Top_all_on-n200-mu0_withOverlay_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_MLP_ACTS_ws3_barrel_layerInfo.onnx"
    # folderToRun["ACTS_ttbarWithOverlay_mu0"] = folderInfo
    # 
    # ############## ttbar
    # folderInfo = {}
    # folderInfo["path"]              = "Top_all_on-n200-mu0_withOverlay_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_qMLP_ACTS_ws3_barrel_layerInfo.onnx"
    # modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
    # folderInfo["modelType"]         = modelType
    # folderInfo["windowSize"]        = windowSize
    # folderToRun["ACTS_ttbarWithOverlay_mu0"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]              = "Top_all_on-n20-mu40_withOverlay_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_qMLP_ACTS_ws3_barrel_layerInfo.onnx"
    # modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
    # folderInfo["modelType"]         = modelType
    # folderInfo["windowSize"]        = windowSize
    # folderToRun["ACTS_ttbarWithOverlay_mu40"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]              = "Top_all_on-n200-mu0_withOverlay_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx"
    # modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
    # folderInfo["modelType"]         = modelType
    # folderInfo["windowSize"]        = windowSize
    # folderToRun["ACTS_ttbarWithOverlay_mu0"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]              = "Top_all_on-n20-mu40_withOverlay_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx"
    # modelType, windowSize = getModelSubString(folderInfo["onnxFile"])
    # folderInfo["modelType"]         = modelType
    # folderInfo["windowSize"]        = windowSize
    # folderToRun["ACTS_ttbarWithOverlay_mu40"] = folderInfo
    
    

    ############## ttbar/pileup
    # folderInfo = {}
    # folderInfo["path"]              = "Top_all_on-n20-mu200_withOverlay_2022-05-23_-090eta090"
    # folderInfo["onnxFile"]          = "TrackNNAnalysis/ExtrapolationModels/Extrap_MLP_ACTS_ws3_barrel_layerInfo.onnx"
    # folderToRun["ACTS_ttbarWithOverlay_mu200"] = folderInfo
  
    return folderToRun

def getModelSubString(string):
    s = string.split("Extrap_")[1]
    modelType = s.split("_")[0]
    s = string.split("ws")[1]
    windowSize = s.split("_")[0]

    return modelType, windowSize

