def getACTSRunSettings():
    runTypes = []
    # All the runs we want to do
    runDict = {}
    runDict["baseName"]     = ""
    runDict["minHits"]      = "7,8,9"
    # runDict["windowSize"]   = "3"
    runDict["uncertainty"]  = "10,15,20"
    runDict["LayerRegion"] = "barrel"
    runDict["hitType"] = "RotatedToZero"
    runDict["muonOnly"] = ""
    runDict["fracOutputTracks"] = "0.1"
    #runDict["saveOutputTracks"] = ""
    #runDict["zUncertainty"]     = "20"
    #runDict["phiUncertainty"]   = "0.05"
    runDict["NNCutVal"]         = "0.5, 0.1, 0.05, 0.01, 0.001, 0.0005"
    runDict["nHitOverlapThres"] = "6, 8"
    runDict["deltaROverlap"]    = "0.2"
    runTypes.append(runDict)

    return runTypes


