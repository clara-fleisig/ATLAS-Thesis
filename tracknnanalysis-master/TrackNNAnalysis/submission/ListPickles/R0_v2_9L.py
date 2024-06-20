################################################
# R0                                           #
################################################
def getMaps():
    folderToRun = {}

    hitType = "SPCylindricalRotatedToZero"

    ############# Single Muon
    folderInfo = {}
    folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900171.PG_singlemu_InvPtFlat1_etaFlat01_03_phiFlat03_05.r12410.v3_1_EXT0"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "R0_9L"
    folderInfo["HTTLayerHits"]      = "8"
    folderInfo["hitType"]           = hitType
    folderToRun["Region_0p1_0p3_SingleP_9L_CylToZeroSP"] = folderInfo

    folderInfo = {}
    folderInfo["path"]              = "HitInfo/v1/user.sabidi.999999.singleMuonFlatPt_v5.RDO.v3_1_EXT0"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "R0_9L"
    folderInfo["HTTLayerHits"]      = "8"
    folderInfo["hitType"]           = hitType
    folderToRun["Region_0p1_0p3_SingleP_Private_9L_CylToZeroSP"] = folderInfo
    
    ############## ttbar
    folderInfo = {}
    folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "R0_9L"
    folderInfo["HTTLayerHits"]      = "8"
    folderInfo["lwrEtaLim"]         = "0.1"
    folderInfo["uprEtaLim"]         = "0.3"
    folderInfo["hitType"]           = hitType
    folderToRun["Region_0p1_0p3_ttbar_9L_CylToZeroSP"] = folderInfo

    ############## HTT True
    folderInfo = {}
    folderInfo["path"]              = "HTTData/9L_region0/singlemuons/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderInfo["hitType"]           = hitType
    folderToRun["Region_0p1_0p3_HTTTrue_9L_CylToZeroSP"] = folderInfo

    ############## HTT Fake
    folderInfo = {}
    folderInfo["path"]              = "HTTData/9L_region0/ttbar_pileup/"
    folderInfo["type"]              = "PickleHTT"
    folderInfo["LayerRegion"]    = "R0"
    folderInfo["hitType"]           = hitType
    folderToRun["Region_0p1_0p3_HTTFake_9L_CylToZeroSP"] = folderInfo

    return folderToRun
