################################################
# R0 - 11L second stage                        #
################################################
def getMaps():
    folderToRun = {}
    ############# HTT True
    folderInfo = {}
    folderInfo["path"]              = "HTTData/region0_11L_stub_d03_singlemu/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderToRun["Region_0p1_0p3_HTTTrue_11L_V2_Nom"] = folderInfo

    ############## HTT Fake
    folderInfo = {}
    folderInfo["path"]              = "HTTData/region0_nominal_30muonsper_ttbarmu200_2ndstage_test/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderToRun["Region_0p1_0p3_HTTFake_11L_V2_Nom_2ndStage"] = folderInfo

    return folderToRun

