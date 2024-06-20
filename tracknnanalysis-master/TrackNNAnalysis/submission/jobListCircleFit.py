import commonFunctions as CF
import os
#################################################
##              EvalHTT                       ##
#################################################
Eval = []


baseDataPath = "/usatlas/u/ckalderon/usatlasdata/hough_transform_outputs/"
# baseDataPath = "/data/data/"

folderToRun = {}


#####################################
# Eval
#####################################

from ListCircleFit import ACTS_testing as ACTS
folderToRun = CF.merge_two_dicts(folderToRun, ACTS.getMaps())


#####################################
# Run Settings
#####################################

from ListCircleFit import runSetting as RS
runTypes = RS.getACTSRunSettings()
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
            inputDict["baseName"]   = "CircleFit_" + key
            inputDict["name"]       = "J" + str(counter)
            inputDict["type"]       = "CircleFit"
            inputDict["inputFile"]  = dataFile

            outPath = "CircleFitOutput/raw/CircleFit_" + key
            os.system("mkdir -vp " + outPath)

            outFileName = dataFile.split("/")[-1]
            if os.path.isfile(outPath + "/" + outFileName):
              if( os.stat(outPath + "/" + outFileName).st_size > 10000):
                continue
            inputDict["outputFile"]       = outPath + "/" + outFileName
            # copy from runSettings
            keyList = [ "LayerRegion"] # "zUncertainty", "phiUncertainty"]
            for dictKey in keyList:
                if(dictKey in runSetting):
                    inputDict[dictKey] = ''.join(runSetting[dictKey].split())

            for dictKey in keyList:
                if(dictKey in folderToRun[key]):
                    inputDict[dictKey] = ''.join(folderToRun[key][dictKey].split())

            Eval.append(inputDict)
            counter += 1






