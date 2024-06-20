import commonFunctions as CF
import os
#################################################
##              EvalHTT                       ##
#################################################
Eval = []


baseDataPath = "/usatlas/groups/bnl_local2/ckalderon/hough_transform_outputs/"
# baseDataPath = "/data/data/"

folderToRun = {}


#####################################
# Eval
#####################################

from ListExtrap import ACTS_testing as ACTS
folderToRun = CF.merge_two_dicts(folderToRun, ACTS.getMaps())


#####################################
# Run Settings
#####################################

from ListExtrap import runSetting as RS
runTypes = RS.getACTSRunSettings()
for key in folderToRun:
    dataPath = baseDataPath + "/" + folderToRun[key]["path"]
    baseName = folderToRun[key]["modelType"] + "_seed" +folderToRun[key]["windowSize"] 
    fileList = CF.findFile(dataPath, "root")
    overlay = ""
    if(len(fileList) == 0):
        print("Cannot find any files in the path")
        exit(1)
    for runSetting in runTypes:
        counter = 0
        for dataFile in fileList:
            skip = False
            if not "-09" in dataFile: continue
            #skip the files used for training
            for n in range(30):
                if "s{}_".format(n) in dataFile: 
                    skip = True
                    break
            if skip==True: continue
            
            if (("pileupOnly" in runSetting.keys()) and ("muonOnly" in runSetting.keys())):
                print("cannot be both pileupOnly and muonOnly")
                exit(1)
            if "pileupOnly" in runSetting.keys(): overlay = "pileupOnly"
            elif "muonOnly" in runSetting.keys(): overlay = "muonOnly"
            inputDict = {}
            inputDict["baseName"]   = "ExtrapEval_" + key + "_" + overlay
            inputDict["name"]       = "J" + str(counter)
            inputDict["type"]       = "EvalExtrapEff"
            inputDict["inputFile"]  = dataFile

            outPath = "/usatlas/groups/bnl_local2/agekow/EffOutput/raw/" + overlay + "/Extrap_" + key
            outManagerPath = "/usatlas/groups/bnl_local2/agekow/ExtrapTracks/raw/" + overlay +"/ExtrapTracks_" + key
            os.system("mkdir -vp " + outPath)
            os.system("mkdir -vp " + outManagerPath)

            outFileName = dataFile.split("/")[-1]
            #if os.path.isfile(outPath + "/" + outFileName):
              #if( os.stat(outPath + "/" + outFileName).st_size > 10000):
                #print("skipping")
                #continue
            inputDict["outputFile"]       = outPath + "/" + outFileName
            inputDict["outputManagerFile"] = outManagerPath + "/" + outFileName
            # copy from runSettings

            keyList = ["uncertainty", "windowSize", "onnxFile", "modelType", "minHits", "LayerRegion", "saveOutputTracks", 
                "fracOutputTracks", "hitType", "NNCutVal", "pileupOnly", "muonOnly", "nHitOverlapThres", "deltaROverlap"] # "zUncertainty", "phiUncertainty"]

            for dictKey in keyList:
                if(dictKey in runSetting):
                    inputDict[dictKey] = ''.join(runSetting[dictKey].split())

            for dictKey in keyList:
                if(dictKey in folderToRun[key]):
                    inputDict[dictKey] = ''.join(folderToRun[key][dictKey].split())


            #print("inputDict keys", inputDict.keys())
            Eval.append(inputDict)
            counter += 1






