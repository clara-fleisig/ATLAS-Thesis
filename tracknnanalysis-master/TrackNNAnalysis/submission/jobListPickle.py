import commonFunctions as CF
import os
#################################################
##              Pickle Making                  ##
#################################################
PickleList = []
baseDataPath = "/usatlas/groups/bnl_local2/sabidi/"
# baseDataPath = "/data/data/"
# baseDataPath = "/usatlas/groups/bnl_local2/ckalderon/hough_transform_outputs/"
folderToRun = {}


################ 
# 8 Layer setup
################ 
# from ListPickles import R0_v1_8L as R0_8L
# folderToRun = CF.merge_two_dicts(folderToRun, R0_8L.getMaps())

# from ListPickles import R1_v1_8L as R1_8L
# folderToRun = CF.merge_two_dicts(folderToRun, R1_8L.getMaps())

# from ListPickles import R3_v1_8L as R3_8L
# folderToRun = CF.merge_two_dicts(folderToRun, R3_8L.getMaps())

################ 
# 9 Layer setup
################ 
# from ListPickles import R0_v1_9L as R0_9L
# folderToRun = CF.merge_two_dicts(folderToRun, R0_9L.getMaps())

from ListPickles import R0_v2_9L as R0_9L
folderToRun = CF.merge_two_dicts(folderToRun, R0_9L.getMaps())

# from ListPickles import R1_v1_9L as R1_9L
# folderToRun = CF.merge_two_dicts(folderToRun, R1_9L.getMaps())

# from ListPickles import R3_v1_9L as R3_9L
# folderToRun = CF.merge_two_dicts(folderToRun, R3_9L.getMaps())

################ 
# LLP setup
################ 
# from ListPickles import R0_v1_LLP as R0_LLP
# folderToRun = CF.merge_two_dicts(folderToRun, R0_LLP.getMaps())

################ 
# 11L setup
################ 
#from ListPickles import R0_v2_11L as R0_11L
#folderToRun = CF.merge_two_dicts(folderToRun, R0_11L.getMaps())

# from ListPickles import R0_v1_11L_2ndStage as R0_11L_2nd
# folderToRun = CF.merge_two_dicts(folderToRun, R0_11L_2nd.getMaps())


################ 
# 12L setup
################ 
#from ListPickles import R0_v2_12L as R0_12L
#folderToRun = CF.merge_two_dicts(folderToRun, R0_12L.getMaps())


################# 
## ACTS
################# 
# from ListPickles import ACTS_testing as ACTS
# folderToRun = CF.merge_two_dicts(folderToRun, ACTS.getMaps())

################ 
# Athena
################ 
# from ListPickles import AthenaV1 as av1
# folderToRun = CF.merge_two_dicts(folderToRun, av1.getMaps())

for key, folderInfo in folderToRun.items():
    print key
    fileList = CF.findFile(baseDataPath + "/" + folderInfo["path"])
    if(len(fileList) == 0): 
        exit
    
    # for now, just keep one file
    if("HTT" in key):
        fileList = fileList[0:1]

    counter = 0
    for fileName in fileList:

        inputDict = {}
        inputDict["baseName"]   = "Pickle_" + key
        inputDict["name"]       = "J" + str(counter)
        inputDict["type"]       = folderInfo["type"]
        inputDict["inputFile"]  = fileName

        outPath = "PickleOutput/raw/" + key +  "/"
        os.system("mkdir -vp " + outPath)

        outFileName = fileName.split("/")[-1]
        if os.path.isfile(outPath + "/" + outFileName):
           continue
        inputDict["outputFile"]  = outPath + outFileName

        keyList = ["LayerRegion", "HTTLayerHits", "lwrEtaLim", "uprEtaLim", "hitType", "nEvents", "doSubSample"]
        for dictKey in keyList:
            if(dictKey in folderInfo):
                  inputDict[dictKey] = folderInfo[dictKey]


        PickleList.append(inputDict)
        counter += 1



