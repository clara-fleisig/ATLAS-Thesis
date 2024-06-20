# ################################################
# # LLP R0                                       #
# ################################################
def getMaps():
    folderToRun = {}

    ############# HTT True
    folderInfo = {}
    folderInfo["path"]              = "LLPdata/llp-mu0/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderToRun["LLP_Region_0p1_0p3_HTTTrue_8L_Nom"] = folderInfo

    # ############## HTT Fake
    # folderInfo = {}
    # folderInfo["path"]              = "LLPdata/llp-mu200/"
    # folderInfo["type"]              = "PickleHTT"
    # folderInfo["LayerRegion"]    = "R0"
    # folderToRun["LLP_Region_0p1_0p3_HTTFake_8L_Nom"] = folderInfo

    return folderToRun