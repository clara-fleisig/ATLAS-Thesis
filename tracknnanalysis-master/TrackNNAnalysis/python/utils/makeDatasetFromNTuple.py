import os
import sys
import numpy as np
import pandas as pd
import math
import time
import random
import argparse
import matplotlib  
import pickle
matplotlib.use('Agg')
import matplotlib.pyplot as plt  
from mpl_toolkits.mplot3d import Axes3D
import copy
from sklearn.model_selection import train_test_split
from tensorflow.keras.utils import to_categorical

import functools
import operator
import uproot



import argparse
parser = argparse.ArgumentParser()
parser.add_argument("-i", "--inFile",     type = str, required = True, action="append", nargs="+",    help = "InFile")
parser.add_argument("-o", "--outName",    type = str, required = True,                                help = "Out name without pkl")
parser.add_argument("-t", "--trackType",  type = str, default="ExtrapolatedTrueTracks,ExtrapolatedFakeTracks",             help = "Type of track wanted")
parser.add_argument("-s", "--subsample",  type = int, default=100000,                                     help = "sampling for partial processing")
parser.add_argument("-m", "--maxEvents",  type = int, default=-1,                                           help = "Max events from each tree")
parser.add_argument("-lpT", "--doLowPt",              default = False, action='store_true',           help = "lowPt < 2")
parser.add_argument("-hpT", "--doHighPt",             default = False, action='store_true',           help = "high pT > 2")

args = parser.parse_args()

def main():
    ########################
    # define the variable
    ########################
    trainVarList = ["x", "y", "z"]
    layerVarList = []
    print (args.inFile[0][0])
    if "ACTS" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitX5", "hitX6", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitY5", "hitY6", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4", "hitZ5", "hitZ6"]
        layerVarList     = ["detectorID0", "detectorID1", "detectorID2", "detectorID3", "detectorID4", "detectorID5", "detectorID6", "layerID0", "layerID1", "layerID2", "layerID3", "layerID4", "layerID5", "layerID6"]
    elif "8LNom" in args.inFile[0][0] or "8LRT0" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitX5", "hitX6", "hitX7", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitY5", "hitY6", "hitY7", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4", "hitZ5", "hitZ6", "hitZ7"]
        layerVarList     = ["detectorID0", "detectorID1", "detectorID2", "detectorID3", "detectorID4", "detectorID5", "detectorID6", "detectorID7", "layerID0", "layerID1", "layerID2", "layerID3", "layerID4", "layerID5", "layerID6", "layerID7"]
    elif "8LSPRT0" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4"]
        layerVarList     = ["detectorID0", "detectorID1", "detectorID2", "detectorID3", "detectorID4", "layerID0", "layerID1", "layerID2", "layerID3", "layerID4"]
    elif "9L_roughSP" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4"]
        layerVarList     = ["layerID8"]
    elif "9L" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitX5", "hitX6", "hitX7", "hitX8", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitY5", "hitY6", "hitY7", "hitY8", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4", "hitZ5", "hitZ6", "hitZ7", "hitZ8"]
        layerVarList     = ["detectorID0", "detectorID1", "detectorID2", "detectorID3", "detectorID4", "detectorID5", "detectorID6", "detectorID7", "detectorID8", "layerID0", "layerID1", "layerID2", "layerID3", "layerID4", "layerID5", "layerID6", "layerID7", "layerID8"]
    elif "11L" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitX5", "hitX6", "hitX7", "hitX8", "hitX9", "hitX10", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitY5", "hitY6", "hitY7", "hitY8", "hitY9", "hitY10", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4", "hitZ5", "hitZ6", "hitZ7", "hitZ8", "hitZ9", "hitZ10"]
        layerVarList     = ["detectorID0", "detectorID1", "detectorID2", "detectorID3", "detectorID4", "detectorID5", "detectorID6", "detectorID7", "detectorID8", "detectorID9", "detectorID10", "layerID0", "layerID1", "layerID2", "layerID3", "layerID4", "layerID5", "layerID6", "layerID7", "layerID8", "layerID9", "layerID10"]
    elif "12L" in args.inFile[0][0]:
        trainFlatVarList = ["hitX0", "hitX1", "hitX2", "hitX3", "hitX4", "hitX5", "hitX6", "hitX7", "hitX8", "hitX9", "hitX10", "hitX11", "hitY0", "hitY1", "hitY2", "hitY3", "hitY4", "hitY5", "hitY6", "hitY7", "hitY8", "hitY9", "hitY10", "hitY11", "hitZ0", "hitZ1", "hitZ2", "hitZ3", "hitZ4", "hitZ5", "hitZ6", "hitZ7", "hitZ8", "hitZ9", "hitZ10", "hitZ11"]
        layerVarList     = ["detectorID0", "detectorID1", "detectorID2", "detectorID3", "detectorID4", "detectorID5", "detectorID6", "detectorID7", "detectorID8", "detectorID9", "detectorID10", "detectorID11", "layerID0", "layerID1", "layerID2", "layerID3", "layerID4", "layerID5", "layerID6", "layerID7", "layerID8", "layerID9", "layerID10", "layerID11"]
    else:
        lx = ["hitX"+str(i) for i in range(20)]
        ly = ["hitY"+str(i) for i in range(20)]
        lz = ["hitZ"+str(i) for i in range(20)]
        trainFlatVarList = lx+ly+lz
        # ldetector = ["detectorID"+str(i) for i in range(20)]
        # llayer = ["layerID"+str(i) for i in range(20)]
        # layerVarList=ldetector + llayer
        layerVarList = ["hitDet"+str(i) for i in range(20)]
        
        #"pixOrsct0", "pixOrsct1", "pixOrsct2", "pixOrsct3", "pixOrsct4", "pixOrsct5", "pixOrsct6", "pixOrsct7", "pixOrsct8", "pixOrsct9", "pixOrsct10", "pixOrsct11", "pixOrsct12", "pixOrsct13", "pixOrsct14"]

    print (trainFlatVarList)
    trueLabelVar = "isTrue"
    extraVarList = []#["r"]
    regressionVar =[] # ["pt", "d0", "z0"]

    varList = trainVarList + trainFlatVarList + [trueLabelVar] + extraVarList + regressionVar + layerVarList
    treeList, entryList = getTreeList()

    print("treeList",treeList)
    print("entryList",entryList)

    # # --=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--
    ########################
    # Create tmp files for later
    ########################
    baseTmpPath = "tmp/"
    os.system("mkdir -vp " + baseTmpPath)

    ## Create Files to append to later...
    with open(baseTmpPath + 'tmp_layerVars.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_trainFlatVars.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_trueLabel.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_weight.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_trkLabel.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_regReg.pkl','wb') as file:
      pass


    ########################
    # Calculate how many iterations and files will be there
    ########################
    index = 0
    m_max = 0
    for e in entryList:
      m_max = max(m_max,entryList[e])

    # overwrite if max events have been specified
    if(args.maxEvents > 0):
        m_max = args.maxEvents
        args.subsample = m_max
        for e in entryList:
            entryList[e] = m_max

    # floor division
    nSampleIter = m_max//args.subsample+1

    print("nSampleIter",nSampleIter)
    print("opened {}".format(args.inFile))

    # iterate over how many batches for longest file
    for i in range(0,nSampleIter):

        print("-----------reading iteration:",i)
        dataset = {}
        if("TrueTracks"               in treeList): 
            dataset["trueTrack"]                = getPandaFrame(treeName = "TrueTracks",               branchList = varList, weight = 1.0, trackType = 1, entries=entryList, t_index=index, subsample=args.subsample)
        if("TrueTruthTracks"          in treeList): 
            dataset["truthTrueTrack"]           = getPandaFrame(treeName = "TrueTruthTracks",          branchList = varList, weight = 1.0, trackType = 1, entries=entryList, t_index=index, subsample=args.subsample)
        if("FullyRandom"              in treeList): 
            dataset["fullyRand"]                = getPandaFrame(treeName = "FullyRandom",              branchList = varList, weight = 0.1, trackType = 2, entries=entryList, t_index=index, subsample=args.subsample)
        if("LayerRandom"              in treeList): 
            dataset["layerRand"]                = getPandaFrame(treeName = "LayerRandom",              branchList = varList, weight = 0.1, trackType = 3, entries=entryList, t_index=index, subsample=args.subsample)
        if("ModuleIterRandom"         in treeList): 
            dataset["moduleIterRand"]           = getPandaFrame(treeName = "ModuleIterRandom",         branchList = varList, weight = 1, trackType = 4, entries=entryList, t_index=index, subsample=args.subsample)
        if("ModuleRandom"             in treeList): 
            dataset["moduleRand"]               = getPandaFrame(treeName = "ModuleRandom",             branchList = varList, weight = 1, trackType = 5, entries=entryList, t_index=index, subsample=args.subsample)
        if("ModuleRotatedIterRandom"  in treeList): 
            dataset["moduleRotatedIterRand"]    = getPandaFrame(treeName = "ModuleRotatedIterRandom",  branchList = varList, weight = 1, trackType = 6, entries=entryList, t_index=index, subsample=args.subsample)
        if("ModuleRotatedRandom"      in treeList): 
            dataset["moduleRotatedRand"]        = getPandaFrame(treeName = "ModuleRotatedRandom",      branchList = varList, weight = 1, trackType = 7, entries=entryList, t_index=index, subsample=args.subsample)
        if("HTTTrueTracks"            in treeList): 
            dataset["HTTTrueTracks"]            = getPandaFrame(treeName = "HTTTrueTracks",            branchList = varList, weight = 1, trackType = 9, entries=entryList, t_index=index, subsample=args.subsample)
        if("HTTFakeTracks"            in treeList): 
            dataset["HTTFakeTracks"]            = getPandaFrame(treeName = "HTTFakeTracks",            branchList = varList, weight = 1, trackType = 8, entries=entryList, t_index=index, subsample=args.subsample)
        if("ExtrapolatedFakeTracks" in treeList): 
            dataset["ExtrapolatedFakeTracks"]   = getPandaFrame(treeName = "ExtrapolatedFakeTracks",   branchList = varList, weight = 1, trackType = 0, entries=entryList, t_index=index, subsample=args.subsample)
        if("ExtrapolatedTrueTracks" in treeList): 
            dataset["ExtrapolatedTrueTracks"]   = getPandaFrame(treeName = "ExtrapolatedTrueTracks",   branchList = varList, weight = 1, trackType = 1, entries=entryList, t_index=index, subsample=args.subsample)
            if tmp is not None:
        
        # Delete the tmp copy to save space
        index += args.subsample
        # Concat all trees read in batch
        dfMain = []        
        for dataName in dataset:
            df = dataset[dataName]
            if(df) dfMain.append(df)
        dfMain = pd.concat(tuple(dfMain), ignore_index=True)

        print("size of dataset of current iteration  dfMain:: ",len(dfMain))

        ## Append batch (of size --subsample) to files
        with open(baseTmpPath + '/tmp_layerVars.pkl','ab') as file:
            _dumpObj(file, dfMain[layerVarList].copy())
        with open(baseTmpPath + '/tmp_trainFlatVars.pkl','ab') as file:
            _dumpObj(file, dfMain[trainFlatVarList].copy().values)
        with open(baseTmpPath + '/tmp_trueLabel.pkl','ab') as file:
            _dumpObj(file, dfMain[trueLabelVar].copy().values)
        with open(baseTmpPath + '/tmp_weight.pkl','ab') as file:
            _dumpObj(file, dfMain["weight"].copy().values)
        with open(baseTmpPath + '/tmp_trkLabel.pkl','ab') as file:
            _dumpObj(file, dfMain["trackType"].copy().values)
        with open(baseTmpPath + '/tmp_regReg.pkl','ab') as file:
            _dumpObj(file, dfMain[regressionVar].copy())
        del dfMain, dataset

    ## --== Finished reading uproot start formatting data ==--
    print("----------- finished reading")
    ## --== Get size of data for shuffling ==--
    print("size of each dataset")
    print(entryList)

    allEntries = 0
    for e in entryList:
        allEntries += entryList[e]

    ix = np.arange(allEntries)
    p = np.random.permutation(ix)
    splt = int(len(ix) * 0.5)

    ## Reads data from tmp pickle file
    def readData(file_name):
        vars = []
        with open(baseTmpPath + "/tmp_"+file_name+".pkl",'rb') as file:
            flag = True
            index = 0
            while(flag):
                try:
                    vars.append(pickle.load(file))
                    # print("batch {}".format(index))
                    index += 1
                except EOFError:
                    flag = False
            try:
                vars = pd.concat(vars)
            except:
                vars = np.concatenate(vars)
            return vars

    ### Best not to use readData() since it would require copying from inside the function to this one, 
    ###  just use it straight
    def readAndWriteData(file_name):
        vars = []
        with open(baseTmpPath + "/tmp_"+file_name+".pkl",'rb') as file:
            flag = True
            index = 0
            while(flag):
                try:
                    vars.append(pickle.load(file))
                    # print("batch {}".format(index))
                    index += 1
                except EOFError:
                    flag = False
        try:
            vars = pd.concat(vars)
        except:
            vars = np.concatenate(vars)

        try:
            vars = vars.iloc[p].reset_index(drop=True)
        except:
            vars = vars[p]

        vars_train, vars_test = vars[:splt], vars[splt:]

        print(file_name, "number of training tracks: ", len(vars_train))
        print(file_name, "number of validation tracks: ", len(vars_test))
        _dumpObj(pFile, vars_train)
        _dumpObj(pFile, vars_test)
        del vars_train, vars_test, vars


    os.system("mkdir -vp pickles")
    pFile = open('./pickles/' + args.outName + '.pkl','wb')

    _dumpObj(pFile, "TrackML")
    _dumpObj(pFile, "v2")

    ############ 
    ## RNN Data - empty for now
    ###########
    # X_train, X_test = splitrNNData(df_trainVars, maxNTrack, ix_train, ix_test)
    X_train = []
    X_test = []
    _dumpObj(pFile, X_train)
    _dumpObj(pFile, X_test)
    del X_train, X_test


    ###############
    ## Flat X data
    ###############
    # Simple Data that can be read and saved
    readAndWriteData("trainFlatVars")

    ###############
    ## Truth Label in cateogrical data
    ###############
    # Requires to_categorial and saving to y_train_old...
    trueLabel = readData("trueLabel")
    trueLabel = trueLabel[p] # randomize
    print(trueLabel)
    y_train, y_test = trueLabel[:splt], trueLabel[splt:] # split

    y_train_old = y_train
    y_test_old  = y_test
    # for categorial data
    y_train = to_categorical(y_train_old)
    y_test = to_categorical(y_test_old)
    _dumpObj(pFile, y_train)
    _dumpObj(pFile, y_test)

    del y_train, y_test, trueLabel
    ###############
    ## Weights
    ###############
    readAndWriteData("weight")

    ###############
    ## Truth labels in simple class format
    ###############
    _dumpObj(pFile, y_train_old)
    _dumpObj(pFile, y_test_old)

    del y_train_old, y_test_old

    ###############
    ## Layer data
    ###############
    readAndWriteData("layerVars")

    ###############
    ## Train var list
    ###############
    _dumpObj(pFile, trainVarList)

    pFile.close()


def _dumpObj(pFile, obj):
    pickle.dump(obj , pFile, protocol = 2)

# get the panda dataframe
def getPandaFrame(treeName, branchList, weight, trackType, entries, t_index=None, subsample=None):
    if t_index > entries[treeName]:
        return None
    # print("Doing: ", treeName)
    frame = None
    # flatten the list
    sampleList = functools.reduce(operator.concat, args.inFile)
    counter = 0

    cut = None
    if (args.doLowPt): cut = "((pt > 0) & (pt < 2))"
    if (args.doHighPt): cut = "(pt > 2)"

    # print(sampleList)

    for index in range(0, len(sampleList)):
        if(subsample is not None and t_index is not None):
            cFrame   = root2pandas(sampleList[index], treeName, branchList, cut, t_index, t_index+subsample)
        else:
            cFrame   = root2pandas(sampleList[index], treeName, branchList, cut, start=None, stop=None)
        # print("finished reading")
        if(counter != 0):
            frame = pd.concat((frame, cFrame), ignore_index=True) 
        else:
            frame = cFrame
            counter += 1

    frame["weight"] = weight
    frame["trackType"] = trackType

    print(treeName, len(frame))

    return frame


########################
## root 2 pandas
########################
def root2pandas(files_path, tree_name, branchList, selectionCut, start, stop, **kwargs):
    import glob
    files = glob.glob(files_path)
    ss = {}
    for fpath in files:
        with uproot.open(fpath) as file:
            print("reading {} to {} {}/{} - done {:.2f}%".format(start,stop,stop-start,file[tree_name].num_entries,float(stop)/file[tree_name].num_entries*100))
            data = file[tree_name].arrays(branchList,selectionCut,library="np",entry_start=start, entry_stop=stop)
            for k in data.keys():
                ss[k] = data[k]
    return pd.DataFrame(ss)


def getTreeList(trueTracks=True):
    sampleList = functools.reduce(operator.concat, args.inFile)
    entries  = {}
    treeName  = []
    trackTypes = args.trackType.split(',')
    print("Print reading trees list")
    print("trackTypes", trackTypes)
    with uproot.open(sampleList[0]) as file:
        for tree in file:
            tree = tree.split(";")[0]
            if(tree in trackTypes):
                print("tree: ", tree)
                treeName.append(tree)
                entries[tree] = file[tree].num_entries
                if not "True" in tree:
                    fakeSize = entries[tree]
                elif "True" in tree:
                    trueSize = entries[tree]
                print("numEntires: ", entries[tree])
    
    # select equal number of true and fake tracks if using extrapolated or fake tracks
    if trueTracks is False:
        for tree in entries:
            if (trueSize > fakeSize):
                entries[tree] = fakeSize
            elif (trueSize < fakeSize): 
                ntries[tree] = trueSize

    return treeName, entries



if __name__ == "__main__":
    start_time = time.time()
    main()
    print("--- %s mins ---" % str((time.time() - start_time)/60.))


