################################################
# R1                                           #
################################################
def getMaps():
    folderToRun = {}
    ############## Single Muon
    folderInfo = {}
    folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900172.PG_singlemu_InvPtFlat1_etaFlat07_09_phiFlat03_05.r12410.v3_1_EXT0"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "R1_8L"
    folderInfo["HTTLayerHits"]      = "7"
    folderToRun["Region_0p7_0p9_SingleP_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900172.PG_singlemu_InvPtFlat1_etaFlat07_09_phiFlat03_05.r12410.v3_1_EXT0"
    #folderInfo["type"]              = "Pickle"
    #folderInfo["LayerRegion"]    = "R1_8L"
    #folderInfo["HTTLayerHits"]      = "6"
    #folderToRun["Region_0p7_0p9_SingleP_8L_Stubs"] = folderInfo
    ############## ttbar
    #folderInfo = {}
    #folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
    #folderInfo["type"]              = "Pickle"
    #folderInfo["LayerRegion"]    = "R1_8L"
    #folderInfo["HTTLayerHits"]      = "7"
    #folderInfo["lwrEtaLim"]         = "0.7"
    #folderInfo["uprEtaLim"]         = "0.9"
    #folderToRun["Region_0p7_0p9_ttbar_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
    #folderInfo["type"]              = "Pickle"
    #folderInfo["LayerRegion"]    = "R1_8L"
    #folderInfo["HTTLayerHits"]      = "6"
    #folderInfo["lwrEtaLim"]         = "0.7"
    #folderInfo["uprEtaLim"]         = "0.9"
    #folderToRun["Region_0p7_0p9_ttbar_8L_Stubs"] = folderInfo
    ############## HTT True
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/region1-nominal-noetafilter-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R1"
    #folderToRun["Region_0p7_0p9_HTTTrue_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/region1-nominal-noetafilter-pion-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R1"
    #folderToRun["Region_0p7_0p9_HTTTrue_8L_NomPion"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/region1-stub-8L-noetafilter-mu0/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R1"
    #folderToRun["Region_0p7_0p9_HTTTrue_8L_Stub"] = folderInfo
    ############## HTT Fake
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may3+4/region1/8L/nominal/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R1"
    #folderToRun["Region_0p7_0p9_HTTFake_8L_Nom"] = folderInfo
    #
    #folderInfo = {}
    #folderInfo["path"]              = "HTTData/may3+4/region1/8L/stubs/"
    #folderInfo["type"]              = "PickleHTT"
    #folderInfo["LayerRegion"]    = "R1"
    #folderToRun["Region_0p7_0p9_HTTFake_8L_Stubs"] = folderInfo

    return folderToRun