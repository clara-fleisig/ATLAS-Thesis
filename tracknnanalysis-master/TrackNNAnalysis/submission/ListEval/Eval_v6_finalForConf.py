################################################
# Final v6 for support note                    #
################################################
def getEvalRuns():
    folderToRun = {}
    #####################################
    ## Final Training
    #####################################
    #
    ########## R0
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_8L_Nom_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Nom_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may20_5muonper/8L-nominal_reg0/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Nom_highStat_v6"] = folderInfo

    #
    #
    ########## R1
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_8L_Nom_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Nom_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_8L_Nom_v6"] = folderInfo
    # #
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may20_5muonper/8L-nominal_reg1/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Nom_highStat_v6"] = folderInfo

    #
    ########## R1
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may11_region3/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMuPi_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMuPi_8L_Nom_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may11_region3/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Nom_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may11_region3/nominal/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_Nom_v6.onnx"
    #folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_Nom_v6"] = folderInfo
    #
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may11_region3/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may20_5muonper/8L-nominal_reg3/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Nom_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_highStat_v6"] = folderInfo

    #
    #
    #
    #
    #####################################
    ## v6 - 8L stub
    #####################################
    #
    ########## R0
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6"] = folderInfo
    # #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_8L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_8L_Stubs_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_8L_Stubs_v6"] = folderInfo
    #
    #
    ########## R1
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6"] = folderInfo
    # #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_8L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_8L_Stubs_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_8L_Stubs_v6"] = folderInfo
    #
    #
    ########## R1
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may11_region3/stub/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may11_region3/stub/"
    #folderInfo["nHits"]     = 8
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_Stubs_v6"] = folderInfo
    #
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may11_region3/stub/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Stubs_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_SingleP_8L_Stubs_v6"] = folderInfo
    # #
    #####################################
    ## v6 - 9L stub
    #####################################
    #
    #
    ########## R0
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_9L_Stubs_v6.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_9L_Stubs_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/9L/stubs/"
    #folderInfo["nHits"]     = 9
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_9L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_9L_Stubs_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region0/9L/stubs/"
    #folderInfo["nHits"]     = 9
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_9L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrueMu_SingleP_9L_Stubs_v6"] = folderInfo
    #
    #
    ########## R1
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_SingleP_ttbar_9L_Stubs_v6.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMuPi_SingleP_ttbar_9L_Stubs_v6"] = folderInfo
    # #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/9L/stubs/"
    #folderInfo["nHits"]     = 9
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_9L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_9L_Stubs_v6"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]      = "HTTData/may3+4/region1/9L/stubs/"
    #folderInfo["nHits"]     = 9
    #folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_9L_Stubs_v6.onnx"
    #folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_9L_Stubs_v6"] = folderInfo

    ####################################
    # v6 - 8L SP & SP+Stubs
    ####################################
    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may11_region3/sp/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_sp_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_sp_v6"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may11_region3/spstub/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v6/DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_spstub_v6.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrueMu_8L_spstub_v6"] = folderInfo

    return folderToRun

