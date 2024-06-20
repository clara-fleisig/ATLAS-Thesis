################################################
# R0 - 12L                                     #
################################################
def getMaps():
    folderToRun = {}

    ############# HTT True
    folderInfo = {}
    folderInfo["path"]              = "HTTData/region0_12L_stub_d09_singlemu/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderToRun["Region_0p1_0p3_HTTTrue_12L_V2_Nom"] = folderInfo

    ############## HTT Fake
    folderInfo = {}
    folderInfo["path"]              = "HTTData/region0_12L_stub_d09_ttbar_onemuon/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderToRun["Region_0p1_0p3_HTTFake_12L_V2_Nom"] = folderInfo
    
    return folderToRun

