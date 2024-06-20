################################################
# Athena runs                                    #
################################################
def getMaps():
    folderToRun = {}

    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900033.PG_singlemu_Pt1_etaFlatnp0_43.r12064.v3_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_Pt1_mu0"] = folderInfo
    
    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900036.PG_singlemu_Pt10_etaFlatnp0_43.r12064.v3_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_Pt10_mu0"] = folderInfo
    
    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900040.PG_singlemu_Pt100_etaFlatnp0_43.r12064.v3_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_Pt100_mu0"] = folderInfo

  
    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900171.PG_singlemu_InvPtFlat1_etaFlat01_03_phiFlat03_05.r12410.v3_1_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_R0_mu0"] = folderInfo


    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900172.PG_singlemu_InvPtFlat1_etaFlat07_09_phiFlat03_05.r12410.v3_1_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_R1_mu0"] = folderInfo

    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900173.PG_singlemu_InvPtFlat1_etaFlat12_14_phiFlat03_05.r12410.v3_1_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_R2_mu0"] = folderInfo
    
    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900174.PG_singlemu_InvPtFlat1_etaFlat20_22_phiFlat03_05.r12410.v3_1_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_r3_mu0"] = folderInfo


    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.900175.PG_singlemu_InvPtFlat1_etaFlat32_34_phiFlat03_05.r12410.v3_1_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["singlemu_r4_mu0"] = folderInfo

    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12064.v3_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["ttbar_mu0"] = folderInfo

    folderInfo = {}
    folderInfo["path"]              = "/HitInfo/v1/user.sabidi.mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.r12063.v3_EXT0/"
    folderInfo["type"]              = "Pickle"
    folderInfo["LayerRegion"]    = "noCut" 
    folderInfo["HTTLayerHits"]      = 5
    folderToRun["ttbar_mu200"] = folderInfo


    return folderToRun
