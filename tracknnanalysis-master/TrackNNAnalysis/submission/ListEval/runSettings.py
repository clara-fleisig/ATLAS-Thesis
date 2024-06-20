def getRunSettings():
    runTypes = []
    # All the runs we want to do
    runDict = {}
    runDict["baseName"]   = "offline_pileupOnly"
    runDict["NNVals"]     = "0.75, 0.5, 0.3, 0.1, 0.05, 0.01, 0.005, 0.001, 0.00001, 0.000001"
    runDict["hitThres"]   = "-4, 2, 4, 5, 100"
    runDict["trackType"]  = "offline"
    runDict["pileupOnly"]   =  True
    runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_muonOnly"
    # runDict["NNVals"]     = "0.75, 0.5, 0.3, 0.1, 0.05, 0.01, 0.005, 0.001, 0.00001, 0.000001"
    # runDict["hitThres"]   = "-4, 2, 4, 5, 100"
    # runDict["trackType"]  = "offline"
    # runDict["muonOnly"]   = True
    # runTypes.append(runDict)


    #runDict = {}
    #runDict["baseName"]   = "offline_All_pt2_Inf"
    #runDict["NNVals"]     = [0.5, 0.2, 0.1, 0.05, 0.025, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    #runDict["hitThres"]   = [4, 5, 6, 7, 10]
    #runDict["trackType"]  = "offline"
    #runDict["lwrPtLim"]   = 2
    #runDict["pileupOnly"]= False
    #runTypes.append(runDict)
    #
    #runDict = {}
    #runDict["baseName"]   = "offline_pileupOnly_pt2_Inf"
    #runDict["NNVals"]     = [0.5, 0.2, 0.1, 0.05, 0.025, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    #runDict["hitThres"]   = [4, 5, 6, 7, 10]
    #runDict["trackType"]  = "offline"
    #runDict["lwrPtLim"]   = 2
    #runDict["pileupOnly"]= True
    #runTypes.append(runDict)
    #
    # runDict = {}
    # runDict["baseName"]   = "offline_pileupOnly_pt1_2"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.025, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 1
    # runDict["uprPtLim"]   = 2
    # runDict["pileupOnly"]= True
    # runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_muonOnly_pt1_2"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 1
    # runDict["uprPtLim"]   = 2
    # runDict["muonOnly"]= True
    # runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_pileupOnly_pt2_4"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.025, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 2
    # runDict["uprPtLim"]   = 4
    # runDict["pileupOnly"]= True
    # runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_muonOnly_pt2_4"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 2
    # runDict["uprPtLim"]   = 4
    # runDict["muonOnly"]= True
    # runTypes.append(runDict)


    # runDict = {}
    # runDict["baseName"]   = "offline_pileupOnly_pt4_6"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.025, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 4
    # runDict["uprPtLim"]   = 6
    # runDict["pileupOnly"]= True
    # runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_muonOnly_pt4_6"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 4
    # runDict["uprPtLim"]   = 6
    # runDict["muonOnly"]= True
    # runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_pileupOnly_pt6_Inf"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.025, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 6
    # runDict["uprPtLim"]   = 1000000
    # runDict["pileupOnly"]= True
    # runTypes.append(runDict)

    # runDict = {}
    # runDict["baseName"]   = "offline_muonOnly_pt6_Inf"
    # runDict["NNVals"]     = [0.75, 0.5, 0.2, 0.1, 0.05, 0.01, 0.0075, 0.005, 0.0025, 0.001, 0.00075, 0.0005, 0.00025, 0.00001, 0.000005]
    # runDict["hitThres"]   = [5]
    # runDict["trackType"]  = "offline"
    # runDict["lwrPtLim"]   = 6
    # runDict["uprPtLim"]   = 1000000
    # runDict["muonOnly"]= True
    # runTypes.append(runDict)
    return runTypes

def getACTSRunSettings():
    runTypes = []
    # All the runs we want to do
    runDict = {}
    runDict["baseName"]   = "offline_allTracks"
    runDict["NNVals"]     = "0.75, 0.5, 0.1, 0.05,0.01, 0.005, 0.001, 0.0005, 0.0001, 0.00005, 0.00001, 0.000005, 0.000001"
    runDict["hitThres"]   = "5, 7, 10"
    runDict["trackType"]  = "offline"
    runTypes.append(runDict)

    runDict = {}
    runDict["baseName"]   = "truth_allTracks"
    runDict["NNVals"]     = "0.75, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001"
    runDict["hitThres"]   = "5, 10"
    runDict["trackType"]  = "truth"
    runTypes.append(runDict)

    return runTypes


