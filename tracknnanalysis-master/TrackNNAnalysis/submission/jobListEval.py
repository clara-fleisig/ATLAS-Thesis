import commonFunctions as CF
import os
#################################################
##              EvalHTT                       ##
#################################################
HTTEval = []


baseDataPath = "/usatlas/groups/bnl_local2/sabidi/"
# baseDataPath = "/usatlas/u/ckalderon/usatlasdata/hough_transform_outputs/"
# baseDataPath = "/data/data/ACTSdata/"
folderToRun = {}


#####################################
# Eval
#####################################
# from ListEval import Eval_8L_Nom as E8LNom
# folderToRun = CF.merge_two_dicts(folderToRun, E8LNom.getEvalRuns())

from ListEval import Eval_9L_SP as E9LSP
folderToRun = CF.merge_two_dicts(folderToRun, E9LSP.getEvalRuns())

# from ListEval import Eval_8L_Stubs as E8LStubs
# folderToRun = CF.merge_two_dicts(folderToRun, E8LStubs.getEvalRuns())

# from ListEval import Eval_11L_Nom as E11LNom
# folderToRun = CF.merge_two_dicts(folderToRun, E11LNom.getEvalRuns())

# from ListEval import Eval_11L_Nom_2ndStage as E11LNom_2ndStage
# folderToRun = CF.merge_two_dicts(folderToRun, E11LNom_2ndStage.getEvalRuns())

# from ListEval import Eval_12L_Nom as E12LNom
# folderToRun = CF.merge_two_dicts(folderToRun, E12LNom.getEvalRuns())


# from ListEval import Eval_ACTS_Nom as ACTS
# folderToRun = CF.merge_two_dicts(folderToRun, ACTS.getEvalRuns())


#####################################
# Run Settings
#####################################

from ListEval import runSettings as RS
runTypes = RS.getRunSettings()
# runTypes = RS.getACTSRunSettings()

for key in folderToRun:
    dataPath = baseDataPath + "/" + folderToRun[key]["path"]
    fileList = CF.findFile(dataPath, "root")
    print key
    if(len(fileList) == 0):
        print("Cannot find any files in the path")
        exit(1)
    for runSetting in runTypes:
        counter = 0
        for dataFile in fileList:
            inputDict = {}
            inputDict["baseName"]   = "HTTEval_" + key + "_" + runSetting["baseName"]
            inputDict["name"]       = "J" + str(counter)
            inputDict["type"]       = "HTTEff"
            inputDict["inputFile"]  = dataFile

            outPath = "EffOutput/raw/" + key +  "/" + runSetting["baseName"]
            os.system("mkdir -vp " + outPath)

            outFileName = dataFile.split("/")[-1]
            # if os.path.isfile(outPath + "/" + outFileName):
            #   if( os.stat(outPath + "/" + outFileName).st_size > 10000):
                # continue
            inputDict["outputFile"]       = outPath + "/" + outFileName
            inputDict["nHits"]            = folderToRun[key]["nHits"]
            inputDict["onnxFile"]         = folderToRun[key]["onnxFile"]
            inputDict["LayerRegion"]   = folderToRun[key]["LayerRegion"]
            inputDict["NNCutVal"]         = ''.join(runSetting["NNVals"].split())
            inputDict["nHitOverlapThres"] = ''.join(runSetting["hitThres"].split())

            # copy from runSettings
            keyList = ["pileupOnly", "lwrPtLim", "uprPtLim", "trackType", "muonOnly", "doACTS", "scaled", "hitType", "order"]
            for dictKey in keyList:
                if(dictKey in runSetting):
                    inputDict[dictKey] = runSetting[dictKey]

            # copy from job settings
            for dictKey in keyList:
                if(dictKey in folderToRun[key]):
                    inputDict[dictKey] = folderToRun[key][dictKey]


            HTTEval.append(inputDict)
            counter += 1








