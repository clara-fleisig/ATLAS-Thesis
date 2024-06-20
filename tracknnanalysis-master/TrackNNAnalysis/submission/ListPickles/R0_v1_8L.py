################################################
# R0                                           #
################################################
def getMaps():

    hitTypeOpt = {}
    hitTypeOpt["SPRT0"] = "SPWithRotatedToZero"
    hitTypeOpt["RT0"]   = "RotatedToZero"
    hitTypeOpt["Nom"]   = "Nominal"

    folderToRun = {}

    for key, hitType in hitTypeOpt.items():

        ############## HTT True
        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_singlemuons/"
        folderInfo["type"]              = "PickleHTT"
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_HTTTrue_8L" + key] = folderInfo

        ############## HTT Fake
        folderInfo = {}
        folderInfo["path"]              = "HTTData/8L_region0/region0_nominal_30muonsper_ttbarmu200_offlinehitinfo/"
        folderInfo["type"]              = "PickleHTT"
        folderInfo["LayerRegion"]    = "R0"
        folderInfo["doSubSample"]       = True
        folderInfo["hitType"]           = hitType
        folderToRun["R0_HTTFake_8L" + key] = folderInfo


        # ############## Single Muon
        folderInfo = {}
        folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900171.PG_singlemu_InvPtFlat1_etaFlat01_03_phiFlat03_05.r12410.v3_1_EXT0"
        folderInfo["type"]              = "Pickle"
        folderInfo["LayerRegion"]    = "R0_8L"
        folderInfo["HTTLayerHits"]      = "7"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_SingleP_8L" + key] = folderInfo
        
        folderInfo = {}
        folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900033.PG_singlemu_Pt1_etaFlatnp0_43.r12064.v3_EXT0"
        folderInfo["type"]              = "Pickle"
        folderInfo["LayerRegion"]    = "R0_8L"
        folderInfo["HTTLayerHits"]      = "7"
        folderInfo["lwrEtaLim"]         = "0.1"
        folderInfo["uprEtaLim"]         = "0.3"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_SingleP_8L_pT1" + key] = folderInfo

        folderInfo = {}
        folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900036.PG_singlemu_Pt10_etaFlatnp0_43.r12064.v3_EXT0"
        folderInfo["type"]              = "Pickle"
        folderInfo["LayerRegion"]    = "R0_8L"
        folderInfo["HTTLayerHits"]      = "7"
        folderInfo["lwrEtaLim"]         = "0.1"
        folderInfo["uprEtaLim"]         = "0.3"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_SingleP_8L_pT10" + key] = folderInfo

        folderInfo = {}
        folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.900040.PG_singlemu_Pt100_etaFlatnp0_43.r12064.v3_EXT0"
        folderInfo["type"]              = "Pickle"
        folderInfo["LayerRegion"]    = "R0_8L"
        folderInfo["HTTLayerHits"]      = "7"
        folderInfo["lwrEtaLim"]         = "0.1"
        folderInfo["uprEtaLim"]         = "0.3"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_SingleP_8L_pT100" + key] = folderInfo

        
        ############### ttbar
        folderInfo = {}
        folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0"
        folderInfo["type"]              = "Pickle"
        folderInfo["LayerRegion"]    = "R0_8L"
        folderInfo["HTTLayerHits"]      = "7"
        folderInfo["lwrEtaLim"]         = "0.1"
        folderInfo["uprEtaLim"]         = "0.3"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_ttbar_8L" + key] = folderInfo

        folderInfo = {}
        folderInfo["path"]              = "HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12063.v3_EXT0"
        folderInfo["type"]              = "Pickle"
        folderInfo["LayerRegion"]    = "R0_8L"
        folderInfo["HTTLayerHits"]      = "7"
        folderInfo["lwrEtaLim"]         = "0.1"
        folderInfo["uprEtaLim"]         = "0.3"
        folderInfo["hitType"]           = hitType
        folderToRun["R0_ttbar_mu200_8L" + key] = folderInfo



    return folderToRun
