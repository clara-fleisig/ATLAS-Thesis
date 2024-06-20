################################################
# ACTS runs                                    #
################################################
def getMaps():
    folderToRun = {}

    ############## singleMu
    folderInfo = {}
    folderInfo["path"]              = "pdg13-n1000-mp30-pT100_2022-06-30"
    folderToRun["ACTS_SingleMu_pT100"] = folderInfo

    folderInfo = {}
    folderInfo["path"]              = "pdg13-n1000-mp30-pT10_2022-05-23_-090eta090"
    folderToRun["ACTS_SingleMu_pT10"] = folderInfo
  
    return folderToRun


