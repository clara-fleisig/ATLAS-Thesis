################################################
# ACTS runs                                    #
################################################
def getMaps():
    folderToRun = {}

    ############ single mu
    folderInfo = {}
    folderInfo["path"]              = "pdg13-n2500-mp30-pT100_v3_2022-07-08"
    folderInfo["type"]              = "PickleACTS"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["hitType"]           = "RotatedToZero"
    folderToRun["ACTS_SingleMu"] = folderInfo

    ############ ttbar/ nopileup
    folderInfo = {}
    folderInfo["path"]              = "Top_all_on-n200-mu0_withOverlay_v2_2022-07-04_-090eta090"
    folderInfo["type"]              = "PickleACTS"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["hitType"]           = "RotatedToZero"
    folderToRun["ACTS_ttbarWithOverlay_mu0"] = folderInfo
    
    ############ ttbar
    folderInfo = {}
    folderInfo["path"]              = "Top_all_on-n20-mu40_withOverlay_v2_2022-07-04_-090eta090"
    folderInfo["type"]              = "PickleACTS"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["hitType"]           = "RotatedToZero"
    folderToRun["ACTS_ttbarWithOverlay_mu40"] = folderInfo
    
    ############## ttbar
    folderInfo = {}
    folderInfo["path"]              = "Top_all_on-n20-mu200_withOverlay_v2_2022-07-04_-090eta090"
    folderInfo["type"]              = "PickleACTS"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["hitType"]           = "Nominal"
    folderToRun["ACTS_ttbarWithOverlay_mu200"] = folderInfo


  
    return folderToRun
