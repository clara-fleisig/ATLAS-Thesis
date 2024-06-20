################################################
# v5 training                                    #
################################################
def getEvalRuns():
    folderToRun = {}

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_HTTTrue_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_HTTTrue_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_8L_Stubs_v5"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_HTTTrue_singleP_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_singleP_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_HTTTrue_singleP_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_HTTTrue_singleP_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/9L/nominal/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_9L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_9L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_9L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_9L_Stubs_v5"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/9L/nominal/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_9L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_9L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region0/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_9L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p1_0p3_HTTFake_singleP_ttbar_9L_Stubs_v5"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_HTTTrue_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrue_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_HTTTrue_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrue_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_HTTTrue_singleP_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrue_singleP_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_HTTTrue_singleP_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_HTTTrue_singleP_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/9L/nominal/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_9L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_9L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_9L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_9L_Stubs_v5"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/nominal/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/9L/nominal/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_9L_Nom_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_9L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region1/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_9L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_0p7_0p9_HTTFake_singleP_ttbar_9L_Stubs_v5"] = folderInfo



    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/nominal/8Lnominal"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_HTTTrue_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrue_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_HTTTrue_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrue_8L_Stubs_v5"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/nominal/8Lnominal"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_HTTTrue_singleP_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrue_singleP_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_HTTTrue_singleP_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_HTTTrue_singleP_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/nominal/8Lnominal"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/9L/nominal/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_9L_Nom_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_9L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_9L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_9L_Stubs_v5"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/nominal/8Lnominal"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_8L_Nom_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_8L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/8L/stubs/"
    # folderInfo["nHits"]     = 8
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_8L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_8L_Stubs_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/9L/nominal/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_9L_Nom_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_9L_Nom_v5"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]      = "HTTData/may3+4/region3/9L/stubs/"
    # folderInfo["nHits"]     = 9
    # folderInfo["onnxFile"]  = "TrackNNAnalysis/ONNXModels/v5/DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_9L_Stubs_v5.onnx"
    # folderToRun["DNN_Region_2p0_2p2_HTTFake_singleP_ttbar_9L_Stubs_v5"] = folderInfo

    return folderToRun

