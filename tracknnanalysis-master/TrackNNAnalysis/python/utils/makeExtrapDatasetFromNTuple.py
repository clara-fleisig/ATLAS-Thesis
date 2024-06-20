import os
import sys
import numpy as np
import pandas as pd
import math
import time
import random
import argparse
#import matplotlib  
import pickle
# import ROOT
#matplotlib.use('Agg')
#import matplotlib.pyplot as plt  
#from mpl_toolkits.mplot3d import Axes3D
import copy
#from sklearn.model_selection import train_test_split
#from tensorflow.keras.utils import to_categorical

import functools
import operator
import uproot



import argparse
parser = argparse.ArgumentParser()
parser.add_argument("-i", "--inFile",     type = str, required = True, action="append", nargs="+",    help = "InFile")
parser.add_argument("-o", "--outName",    type = str, required = True,                                help = "Out name without pkl")
parser.add_argument("-t", "--trackType",  type = str, default="TrainingInputs",             help = "Type of track wanted")
parser.add_argument("-s", "--subsample",  type = int, default=10000000,                                     help = "Type of track wanted")
parser.add_argument("-lpT", "--doLowPt",              default = False, action='store_true',           help = "lowPt < 2")
parser.add_argument("-hpT", "--doHighPt",             default = False, action='store_true',           help = "high pT > 2")

args = parser.parse_args()

def main():

    # define the variable
    trainVarList = ["x", "y", "z"]
    layerVarList = []
    print (args.inFile[0][0])
    #Look for the number of hits in sliding window specified in filename
    windowSize = args.inFile[0][0].index("windowSize")
    windowSize = int(args.inFile[0][0][windowSize+len("windowSize")])

    inputVarList, targetVarList, inputDetList, targetDetList = [], [], [], []
    for i in range(windowSize):
      inputVarList.extend([f"hitX{i}", f"hitY{i}", f"hitZ{i}"])
      inputDetList.extend([f"hitDet{i}"])
    targetVarList.extend([f"tarHitX", f"tarHitY", f"tarHitZ"])
    targetDetList.extend([f"tarHitDet"])

    varList =  inputVarList + targetVarList + inputDetList + targetDetList
    treeList, entryList = getTreeList()

    print("treeList",treeList)
    print("entryList",entryList)

    # # --=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--
    baseTmpPath = "tmp/"
    os.system("mkdir -vp " + baseTmpPath)

    ## Create Files to append to later...
    with open(baseTmpPath + 'tmp_inputVars.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_targetVars.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_inputDets.pkl','wb') as file:
      pass
    with open(baseTmpPath + 'tmp_targetDets.pkl','wb') as file:
      pass


    # Calculate how many iterations and files will be there
    index = 0
    m_max = 0
    for e in entryList:
      m_max = max(m_max,entryList[e])
    final = m_max//args.subsample+1
    print("max",m_max)
    print("final",final)
    print("opened {}".format(args.inFile))

    # iterate over how many batches for longest file
    for i in range(0,final):
      # if index > m_max:
      #   break
      dataset = {}
      if("TrainingInputs" in treeList): 
        tmp = getPandaFrame(treeName = "TrainingInputs", branchList = varList, weight = 1, trackType = 1, entries=entryList, t_index=index, subsample=args.subsample);
        if tmp is not None:
          dataset["TrainingInputs"] = tmp
          print("found dataset")
      
      del tmp
      index += args.subsample
      # Concat all trees read in batch
      ## TODO: might be better to read in subsample from a single tree, not --subsamples from all trees...
      dfMain = []
      # select an equal number of true and fake tracks
      
      for dataName in dataset:
          df = dataset[dataName]
          dfMain.append(df)
      dfMain = pd.concat(tuple(dfMain), ignore_index=True)

      # print("dfMain:: ",len(dfMain))

      ## Append batch (of size --subsample) to files
      with open(baseTmpPath + 'tmp_inputVars.pkl','ab') as file:
        _dumpObj(file, dfMain[inputVarList].copy())

      with open(baseTmpPath + 'tmp_targetVars.pkl','wb') as file:
        _dumpObj(file, dfMain[targetVarList].copy())

      with open(baseTmpPath + 'tmp_inputDets.pkl','wb') as file:
        _dumpObj(file, dfMain[inputDetList].copy())

      with open(baseTmpPath + 'tmp_targetDets.pkl','wb') as file:
        _dumpObj(file, dfMain[targetDetList].copy())


    ## --== Finished reading uproot; start formatting data ==--


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

      print("read " + file_name)
      
      _dumpObj(pFile, vars)
      del vars
      print("dumped " + file_name)


    os.system("mkdir -vp pickles")
    pFile = open( args.outName + '.pkl','wb')


    # Simple Data that can be read and saved
    readAndWriteData("inputVars")
    readAndWriteData("targetVars")
    readAndWriteData("inputDets")
    readAndWriteData("targetDets")

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

    print(treeName, len(frame),"\n")

    return frame


########################
## root 2 pandas
########################
def root2pandas(files_path, tree_name, branchList, selectionCut, start, stop, **kwargs):
    import glob
    # from root_numpy import root2array
    # from numpy.lib.recfunctions import stack_arrays

    # print ('Loading: ', files_path)
    '''
    Args:
    -----
        files_path: a string like './data/*.root', for example
        tree_name: a string like 'Collection_Tree' corresponding to the name of the folder inside the root 
                   file that we want to open
        kwargs: arguments taken by root2array, such as branches to consider, start, stop, step, etc
    Returns:
    --------    
        output_panda: a pandas dataframe like allbkg_df in which all the info from the root file will be stored
    
    Note:
    -----
        if you are working with .root files that contain different branches, you might have to mask your data
        in that case, return pd.DataFrame(ss.data)
    '''
    # -- create list of .root files to process
    files = glob.glob(files_path)
    ss = {}
    for fpath in files:
      with uproot.open(fpath) as file:
        # print("opened {}".format(fpath))
        # print(selectionCut)
        # print(file[tree_name].num_entries)
        print("reading {} to {}; {}/{} - done {:.2f}%".format(start,stop,stop-start,file[tree_name].num_entries,float(stop)/file[tree_name].num_entries*100))
        data = file[tree_name].arrays(branchList,selectionCut,library="np",entry_start=start, entry_stop=stop)
        for k in data.keys():
          if type(data[k]) == list: data[k] = np.array(data[k])
          ss[k] = data[k]
        # print("append")
    # print("returning")
    return pd.DataFrame(ss)



    # ss = {}
    # for fpath in files:
    #   with uproot.open(fpath) as file:
    #     print("opened {}".format(fpath))
    #     print(selectionCut)
    #     maximum_reading = 1e6
    #     print(file[tree_name].num_entries)
    #     if file[tree_name].num_entries < maximum_reading:
    #       data = file[tree_name].arrays(branchList,selectionCut,library="np")
    #       for k in data.keys():
    #         ss[k] = data[k]
    #     else:
    #       print("reading 1e6 values...")
    #       cEntry = 0
    #       while cEntry < file[tree_name].num_entries:
    #         print("reading next {}/{}".format(cEntry,file[tree_name].num_entries))
    #         stop = min(cEntry + maximum_reading,file[tree_name].num_entries)
    #         data = file[tree_name].arrays(branchList,selectionCut,library="np",entry_start=cEntry, entry_stop=stop)
    #         cEntry = stop
    #         if len(ss) == 0:
    #           for k in data.keys():
    #             ss[k] = data[k]
    #         else:
    #           for k in ss.keys():
    #             ss[k] = np.concatenate((ss[k],data[k]),axis=0)
    #         del data
    #     print("append")
    # return pd.DataFrame(ss)
    # try:
    #     # return pd.DataFrame(ss)
    #     return pd.DataFrame.from_dict(ss)
    # except Exception:
    #     return pd.DataFrame(ss.data)

def getTreeList(trueTracks=True):
    sampleList = functools.reduce(operator.concat, args.inFile)
    entries  = {}
    treeName  = []
    trackTypes = args.trackType.split(',')
    print("trackTypes", trackTypes)
    with uproot.open(sampleList[0]) as file:
      for tree in file:
        #tree = tree[:-2]
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
          entries[tree] = trueSize

    return treeName, entries



if __name__ == "__main__":
    start_time = time.time()
    main()
    print("--- %s mins ---" % str((time.time() - start_time)/60.))




# # main call for padding
# def padDataset(dataset, varList, _size):
#     # clean varList
#     cList = list(varList)

#     if(len(cList) == 0):
#         return dataset

#     ## subset dataset
#     df = dataset[cList].copy()
#     n_variables = df.shape[1]
#     var_names = df.keys()

#     data = np.zeros((df.shape[0], _size, n_variables), dtype='float32')

#     _createNumpyArray(df, data, _size)

#     for index, row in dataset.iterrows():
#         for v, name in enumerate(var_names):
#             lepLen = len(row[name])
#             row[name] = data[index, :, v]
#             dataset.at[index, name] = row[name]
#     return dataset

# def splitrNNData(df, num_obj, ix_train, ix_test):
   
#     n_variables = df.shape[1]
#     var_names = df.keys()

#     data = np.zeros((df.shape[0], num_obj, n_variables), dtype='float32')

#     # -- call functions to build X (a.k.a. data)                                                                                                                                                                      
#     _createNumpyArray(df, data, num_obj)
    
#     # -- ix_{train, test} from above or from previously stored ordering
#     Xobj_train = data[ix_train]
#     Xobj_test = data[ix_test]

#     #print 'Scaling features ...'
#     # _scale(Xobj_train, var_names, savevars=True,  VAR_FILE_NAME=scalingfile, scalingType = scalingType) # scale training sample and save scaling
#     # _scale(Xobj_test,  var_names, savevars=False, VAR_FILE_NAME=scalingfile, scalingType = scalingType) # apply scaling to test set
#     return Xobj_train, Xobj_test


## Actual function that pads
# def _createNumpyArray(df, data, max_nobj=None):
#     ''' 
#     sort objects using your preferred variable
    
#     Args:
#     -----
#         df: a dataframe with event-level structure where each event is described by a sequence of jets, muons, etc.
#         data: an array of shape (nb_events, nb_particles, nb_features)
#         max_nobj: number of particles to cut off at. if >, truncate, else, -999 pad
    
#     Returns:
#     --------
#         modifies @a data in place. Pads with -999
    
#     '''
#     # import tqdm
#     # i = event number, event = all the variables for that event 
#     for i, event in df.iterrows(): 
#         varArray = [v.tolist() for v in event.get_values()]

#         # clean up
#         maxCurrsize = -1
#         doPadding = False
#         for var in varArray:
#             if(len(var) > maxCurrsize): maxCurrsize = len(var)
#             if(len(var) != maxCurrsize): doPadding = True


#         if doPadding:
#             for var in varArray:
#                 if(len(var) != maxCurrsize): 
#                     while len(var) != maxCurrsize: var.append(-999)
            
#         objs = np.array(varArray, dtype='float32')

#         # total number of tracks per jet      
#         nobjs = objs.shape[1] 
#         cMaxObj = max_nobj
#         if(max_nobj == None): cMaxObj = nobjs

#         # take all tracks unless there are more than n_tracks 
#         data[i, :(min(nobjs, cMaxObj)), :] = objs.T[:(min(nobjs, cMaxObj)), :] 

#         # default value for missing tracks 
#         data[i, (min(nobjs, cMaxObj)):, :  ] = 0


# simple plotting script
# def plotHits(df, baseName, nTracks):
#     def _plotHist(xVar, yVar, name):
#         # for hit in hitList:
#         #     plt.plot(hit[xVar], hit[yVar], 'o-', linewidth=1, markersize=2)

#         for index, row in df.iterrows():
#             if(index > nTracks and nTracks > 0): break
#             plt.plot(row[xVar], row[yVar], 'o-', linewidth=1, markersize=2)

#         plt.gca().set_prop_cycle(None)
#         # for hit in seeds:
#         #     plt.plot(hit[xVar], hit[yVar], '+--', linewidth=1, markersize=4)

#         plt.savefig(baseName + '_' + name + '.pdf',  bbox_inches='tight')
#         plt.clf()

#     _plotHist("x", "y", "xy");
#     _plotHist("x", "z", "xz");
#     _plotHist("y", "z", "yz");
#     _plotHist("z", "r", "rhoz");

