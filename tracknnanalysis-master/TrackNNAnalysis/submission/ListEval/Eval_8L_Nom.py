################################################
# Nom Hits                                     #
################################################
def getEvalRuns():
    folderToRun = {}

    folderMap = {}
    folderMap["HTTAthena_8LNom"] = "TrackNNAnalysis/ONNXModels/v7/DNN_R0_HTTAthena_8LNom.onnx"
    folderMap["HTTOnly_8LNom"]   = "TrackNNAnalysis/ONNXModels/v7/DNN_R0_HTTOnly_8LNom.onnx"

    for key in folderMap:
        # ########## R0
        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_singlemuons/"
        folderInfo["nHits"]             = 8
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "Nominal"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_singleMu_"+key+"_v7"] = folderInfo


        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_30muonsper_ttbarmu200_offlinehitinfo"
        folderInfo["nHits"]             = 8
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "Nominal"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_ttbar_"+key+"_v7"] = folderInfo


    folderMap = {}
    folderMap["HTTAthena_8LRT0"] = "TrackNNAnalysis/ONNXModels/v7/DNN_R0_HTTAthena_8LRT0.onnx"
    folderMap["HTTOnly_8LRT0"]   = "TrackNNAnalysis/ONNXModels/v7/DNN_R0_HTTOnly_8LRT0.onnx"

    for key in folderMap:
        # ########## R0
        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_singlemuons/"
        folderInfo["nHits"]             = 8
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "RotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_singleMu_"+key+"_v7"] = folderInfo


        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_30muonsper_ttbarmu200_offlinehitinfo"
        folderInfo["nHits"]             = 8
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "RotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_ttbar_"+key+"_v7"] = folderInfo

    folderMap = {}
    folderMap["HTTAthena_8LSPRT0"] = "TrackNNAnalysis/ONNXModels/v7/DNN_R0_HTTAthena_8LSPRT0.onnx"
    folderMap["HTTOnly_8LSPRT0"]   = "TrackNNAnalysis/ONNXModels/v7/DNN_R0_HTTOnly_8LSPRT0.onnx"

    for key in folderMap:
        # ########## R0
        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_singlemuons/"
        folderInfo["nHits"]             = 5
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "SPWithRotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_singleMu_"+key+"_v7"] = folderInfo


        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_30muonsper_ttbarmu200_offlinehitinfo"
        folderInfo["nHits"]             = 5
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["onnxFile"]          = folderMap[key]
        folderInfo["scaled"]            = True
        folderInfo["hitType"]           = "SPWithRotatedToZero"
        folderInfo["order"]             = "XX0YY0ZZ0"
        folderToRun["DNN_Region_0p1_0p3_ttbar_"+key+"_v7"] = folderInfo

    # #####################################
    # ## v6 - 8L Nom
    # #####################################
    # ########## R0
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]            = "HTTData/may20_5muonper/8L-nominal_reg0/"
    # folderInfo["nHits"]           = 8
    # folderInfo["LayerRegion"]  = "R0"
    # folderInfo["onnxFile"]        = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_highStat_fidCut_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/june30_more_fixz0/region0_nominal"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_moreStat2_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0-nominal-noetafilter-mu0"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_mu0_v6"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0_nominal_30muonsper_ttbarmu200"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_30mu_mu200_v6"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0_nominal_30muonsper_ttbarmu200_redo"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_8L_Nom_30mu_mu200_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0_nominal_20muonsper_ttbarmu200_redo"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_20mu_mu200_redo_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region0_nominal_10muonsper_ttbarmu200_redo"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R0"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_10mu_mu200_redo_v6"] = folderInfo



    # ########## R1
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R1"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may20_5muonper/8L-nominal_reg1/"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R1"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_highStat_fidCut_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/june30_more_fixz0/region1_nominal"
    # folderInfo["LayerRegion"] = "R1"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_moreStat2_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region1-nominal-noetafilter-mu0"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R1"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_mu0_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region1_nominal_30muonsper_ttbarmu200_redo"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R1"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_8L_Nom_30mu_mu200_v6"] = folderInfo


    # ########## R3
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may11_region3/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R3"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may20_5muonper/8L-nominal_reg3/"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R3"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_highStat_fidCut_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/june30_more_fixz0/region3_nominal"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R3"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_moreStat2_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region3-nominal-noetafilter-mu0_redo"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R3"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_mu0Redo_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/region3_nominal_30muonsper_ttbarmu200_redo"
    # folderInfo["nHits"]     = 8
    # folderInfo["LayerRegion"] = "R3"
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_8L_Nom_30mu_mu200_v6"] = folderInfo

    return folderToRun

