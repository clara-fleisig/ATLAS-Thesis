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

from ListPredError import ACTS_testing as ACTS
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
    print key
    if(len(fileList) == 0):
        print("Cannot find any files in the path")
        exit(1)
    for runSetting in runTypes:
        counter = 0
        for dataFile in fileList:
            #skip the files used for training
            skip = False
            for n in range(30):
                if "s{}_".format(str(n)) in dataFile: 
                    skip = True
                    break
            if skip==True: continue
                
            inputDict = {}
            inputDict["baseName"]   = "PredError_" + key
            inputDict["name"]       = "J" + str(counter)
            inputDict["type"]       = "EvalPredError"
            inputDict["inputFile"]  = dataFile

            outPath = "/usatlas/groups/bnl_local2/agekow/PredictionOutput/raw/PredError_" + key
            os.system("mkdir -vp " + outPath)

            outFileName = dataFile.split("/")[-1]
            if os.path.isfile(outPath + "/" + outFileName):
              if( os.stat(outPath + "/" + outFileName).st_size > 10000):
                continue
            inputDict["outputFile"]       = outPath + "/" + outFileName
            # copy from runSettings
            keyList = ["windowSize", "onnxFile", "LayerRegion"]
            for dictKey in keyList:
                if(dictKey in runSetting):
                    inputDict[dictKey] = ''.join(runSetting[dictKey].split())

            for dictKey in keyList:
                if(dictKey in folderToRun[key]):
                    inputDict[dictKey] = ''.join(folderToRun[key][dictKey].split())

            Eval.append(inputDict)
            counter += 1






