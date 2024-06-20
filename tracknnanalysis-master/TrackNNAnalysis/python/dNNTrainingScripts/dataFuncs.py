import pickle
import numpy as np
from sklearn.utils import shuffle

def loadData(data):
    ## UNCOMMENT THESE FOR NEW DATA ON LXPLUS
    with open(data, 'rb') as fp:
        _ = pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        x_train = pickle.load(fp, encoding='latin1')
        x_test =pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        _ = pickle.load(fp, encoding='latin1')
        y_train = pickle.load(fp, encoding='latin1')
        y_test = pickle.load(fp, encoding='latin1')
        trackLabel_train = pickle.load(fp, encoding='latin1')
        trackLabel_test = pickle.load(fp, encoding='latin1')
        pt_train = pickle.load(fp, encoding='latin1') 
        pt_test = pickle.load(fp, encoding='latin1')
    return x_train, x_test, y_train, y_test

def splitTrainingData(x_train, y_train):
    train_neg_idx = np.where(y_train == 0)[0]
    train_pos_idx = np.where(y_train == 1)[0]
    y_train_pos = y_train[train_pos_idx]
    y_train_neg = y_train[train_neg_idx][0:len(train_pos_idx)]
    y_train = np.append(y_train_pos, y_train_neg)

    x_train_pos = x_train[train_pos_idx]
    x_train_neg = x_train[train_neg_idx][0:len(train_pos_idx)]
    x_train = np.append(x_train_pos, x_train_neg, axis=0)

    x_train, y_train = shuffle(x_train, y_train)

    return x_train, y_train

def combineData(split, dataFiles):
    xtrain = None
    ytrain = None
    xtest = None
    ytest = None
    for f in dataFiles:
        print(f)
        #filePath = os.path.join(filePath, f)
        x_train, x_test, y_train, y_test = loadData(f)
        if split:
            x_train, y_train = splitTrainingData(x_train, y_train)
        if np.any(xtest) == None:
            xtrain = x_train
            xtest = x_test
            ytrain = y_train
            ytest = y_test

        else:
            xtrain = np.vstack([xtrain, x_train])
            ytrain = np.append(ytrain, y_train)
            xtest = np.vstack([xtest, x_test])
            ytest = np.append(ytest, y_test)
    
    x_train, y_train = shuffle(xtrain, ytrain)
    x_test, y_test = shuffle(xtest, ytest)

    return x_train, x_test, y_train, y_test