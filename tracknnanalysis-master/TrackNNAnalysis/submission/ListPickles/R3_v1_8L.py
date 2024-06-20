################################################
# R3                                           #
################################################
def getMaps():
    folderToRun = {}
    ############## Single Muon
    folderInfo = {}
    folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900174.PG_singlemu_InvPtFlat1_etaFlat20_22_phiFlat03_05.r12410.v3_1_EXT0"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "R3_8L"
    folderInfo["HTTLayerHits"]      = "7"
    folderToRun["Region_2p0_2p2_SingleP_8L_Nom"] = folderInfo
    
    #folderInfo = {}
    #folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900174.PG_singlemu_InvPtFlat1_etaFlat20_22_phiFlat03_05.r12410.v3_1_EXT0"
    #folderInfo["type"]              = "Pickle"
    #folderInfo["LayerRegion"]    = "R3_8L"
    #folderInfo["HTTLayerHits"]      = "6"
    #folderToRun["Region_2p0_2p2_SingleP_8L_Stubs"] = folderInfo
    ############## ttbar
    #folderInfo = {}
    #folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
    #folderInfo["type"]              = "Pickle"
    #folderInfo["LayerRegion"]    = "R3_8L"
    #folderInfo["HTTLayerHits"]      = "7"
    #folderInfo["lwrEtaLim"]         = "2.0"
    #folderInfo["uprEtaLim"]         = "2.2"
    #folderToRun["Region_2p0_2p2_ttbar_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
    #folderInfo["type"]              = "Pickle"
    #folderInfo["LayerRegion"]    = "R3_8L"
    #folderInfo["HTTLayerHits"]      = "6"
    #folderInfo["lwrEtaLim"]         = "2.0"
    #folderInfo["uprEtaLim"]         = "2.2"
    #folderToRun["Region_2p0_2p2_ttbar_8L_Stubs"] = folderInfo
    ############## HTT True
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/region3-nominal-noetafilter-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_HTTTrue_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/region3-nominal-noetafilter-pion-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_HTTTrue_8L_NomPion"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/region3-stub-8L-noetafilter-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_HTTTrue_8L_Stub"] = folderInfo

    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may11_region3/region3-nominal-noetafilter-muon-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_v2_HTTTrue_8L_Nom"] = folderInfo

    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may11_region3/region3-nominal-noetafilter-pion-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_v2_HTTTrue_8L_NomPion"] = folderInfo


    # folderInfo = {}
    # folderInfo["path"]              = "HTTData/may11_region3/region3-sp-noetafilter-muon-mu0/"
    # folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    # folderToRun["Region_2p0_2p2_v2_HTTTrue_8L_sp"] = folderInfo

    ############## HTT Fake
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may3+4/region3/8L/nominal/8Lnominal"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_HTTFake_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may3+4/region3/8L/stubs/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_HTTFake_8L_Stubs"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may11_region3/nominal/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_v2_HTTFake_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may11_region3/stub/"
    #folderInfo["type"]              = "PickleHTT"
    # folderInfo["LayerRegion"]    = "R3"
    #folderToRun["Region_2p0_2p2_v2_HTTFake_8L_Stubs"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]              = "HTTData/may11_region3/sp/"
    # folderInfo["type"]              = "PickleHTT"
    # folderInfo["LayerRegion"]    = "R3"
    # folderToRun["Region_2p0_2p2_v2_HTTFake_8L_sp"] = folderInfo

    # folderInfo = {}
    # folderInfo["path"]              = "HTTData/may11_region3/spstub/"
    # folderInfo["type"]              = "PickleHTT"
    # folderInfo["LayerRegion"]    = "R3"
    # folderToRun["Region_2p0_2p2_v2_HTTFake_8L_spstub"] = folderInfo
    return folderToRun




     