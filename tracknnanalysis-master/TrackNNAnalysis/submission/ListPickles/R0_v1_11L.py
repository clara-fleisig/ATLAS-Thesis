################################################
# R0 - 11L                                     #
################################################
def getMaps():
    folderToRun = {}
    ############# Single Muon
    folderInfo = {}
    folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900171.PG_singlemu_InvPtFlat1_etaFlat01_03_phiFlat03_05.r12410.v3_1_EXT0"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "R0_11L"
    folderInfo["HTTLayerHits"]      = "8"
    folderToRun["Region_0p1_0p3_SingleP_11L_Nom"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]              = "HitInfo/v1/user.sabidi.999999.singleMuonFlatPt_v5.RDO.v3_1_EXT0"
    # folderInfo["type"]              = "Pickle"
    # folderInfo["LayerRegion"]    = "R0_11L"
    # folderInfo["HTTLayerHits"]      = "8"
    # folderToRun["Region_0p1_0p3_SingleP_Private_11L_Nom"] = folderInfo

    # ############### ttbar
    # folderInfo = {}
    # folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
    # folderInfo["type"]              = "Pickle"
    # folderInfo["LayerRegion"]    = "R0_11L"
    # folderInfo["HTTLayerHits"]      = "8"
    # folderInfo["lwrEtaLim"]         = "0.1"
    # folderInfo["uprEtaLim"]         = "0.3"
    # folderToRun["Region_0p1_0p3_ttbar_11L_Nom"] = folderInfo

    # ############# HTT True
    # folderInfo = {}
    # folderInfo["path"]              = "HTTData/region0_11L-noinnerpix-firstpass_muon-mu0/"
    # folderInfo["type"]              = "PickleHTT"
    # folderInfo["LayerRegion"]    = "R0"
    # folderToRun["Region_0p1_0p3_HTTTrue_11L_Nom"] = folderInfo

    # ############## HTT Fake
    # folderInfo = {}
    # folderInfo["path"]              = "HTTData/region0_11L-noinnerpix-firstpass/"
    # folderInfo["type"]              = "PickleHTT"
    # folderInfo["LayerRegion"]    = "R0"
    # folderToRun["Region_0p1_0p3_HTTFake_11L_Nom"] = folderInfo
    return folderToRun

