import os
import numpy as np
import pickle
import copy
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--Save",        type=bool,        default = False,        help = "Save the output of the combined data? True/False")
parser.add_argument("--saveDir",     type = str,       required = False,       help = "Directory to save combined data files")
args = parser.parse_args()

def _dumpObj(pFile, obj):
    pickle.dump(obj , pFile, protocol = 2)
    
#Either copy and paste this code into the script you want to run, or save combined data structure as a new file.

maxhits = 40

if args.saveDir == True:
    save_dir = args.saveDir
    
if not os.path.exists(save_dir):
    os.makedirs(save_dir)
# Change v2 in filePath to v1 to access non-rotated data
filePath = '/afs/cern.ch/work/s/sabidi/public/ForAlex/TrackML/pickles/v2'
data_folders  = os.listdir(filePath)


for dataType in data_folders:
    #Only loop over directories, this is to exclude readme files
    if not os.path.isdir(os.path.join(filePath,dataType)):
        continue
    print(dataType)
    x_data_train = None
    y_data_train = None
    p_data_train = None
    x_data_test  = None
    y_data_test  = None
    p_data_test = None
    dataSets = os.listdir(os.path.join(filePath, dataType))
    
    #Note, p_train, p_test should be commented out if using v1 data as they do not contain track probability information
    for dataSet in dataSets:
        data = os.path.join(filePath,dataType, dataSet)
        with open(data,"rb") as fp:
            x_train = pickle.load(fp, encoding = 'latin1')
            y_train = pickle.load(fp, encoding = 'latin1')
            p_train = pickle.load(fp, encoding = 'latin1')
            x_test = pickle.load(fp, encoding = 'latin1')
            y_test = pickle.load(fp, encoding = 'latin1')
            p_test = pickle.load(fp, encoding ='latin1')
            
        # If using for classification task, we do not need the probabilities. These are only used in regression.
        if np.any(x_data_train) == None:
            x_data_train = x_train
            y_data_train = y_train
            #p_data_train = p_train
            x_data_test  = x_test
            y_data_test  = y_test
            #p_data_test = p_test

        else:
            x_data_train = np.append(x_data_train, x_train, axis=0)
            y_data_train = np.append(y_data_train, y_train, axis=0)
            #p_data_train = np.append(p_data_train, p_train, axis=0)
            x_data_test = np.append(x_data_test, x_test, axis=0)
            y_data_test = np.append(y_data_test, y_test, axis=0)
            #p_data_test = np.append(p_data_test, p_test, axis=0)
    print("succesfully loaded data")
    
    ########################### Cut here if copy and pasting into another script ###############################
    if args.Save = True:
        # Directory to save output files to if saving data
        save_dir = args.saveDir
        #File name is the data type_combined.pkl
        saveFileName = "{}_combined.pkl".format(dataType)
        saveFilePath = os.path.join(save_dir, saveFileName)
        
        with open(saveFilePath, 'wb') as fp:
            _dumpObj(fp, x_data_train)
            _dumpObj(fp, y_data_train)
            _dumpObj(fp, p_data_train)
            _dumpObj(fp, x_data_test)
            _dumpObj(fp, y_data_test)
            _dumpObj(fp, p_data_test)
            fp.close()
            
        
