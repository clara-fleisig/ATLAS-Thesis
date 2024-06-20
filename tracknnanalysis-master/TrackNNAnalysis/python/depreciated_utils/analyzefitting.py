import tensorflow as tf
from tensorflow.keras.models import Sequential
import pickle 
import os
import numpy as np
import matplotlib.pyplot as plt

maxhits = 40
numFeatures = 3

path_to_pickles = '/afs/cern.ch/work/s/sabidi/public/ForAlex/TrackML/pickles/v1'
data_folders  = ['fullyRnd', 'layerRnd', 'moduleIterRnd', 'moduleRnd', 'moduleRotatedIterRnd', 'moduleRotatedRnd','alexAlgo' ]

Xtrain = {}
Ytrain = {}
Xtest = {}
Ytest = {}

for dataType in data_folders:
    print(dataType)
    x_data_train = None
    y_data_train = None
    p_data_train = None
    x_data_test  = None
    y_data_test  = None
    p_data_test = None
    dataSets = os.listdir(os.path.join(path_to_pickles, dataType))
    
    for dataSet in dataSets:
        data = os.path.join(path_to_pickles,dataType, dataSet)
        
        with open(data,"rb") as fp:
    	    x_train = pickle.load(fp, encoding = 'latin1')
    	    y_train = pickle.load(fp, encoding = 'latin1')
    	    p_train = pickle.load(fp, encoding = 'latin1')
            x_test = pickle.load(fp, encoding = 'latin1')
    	    y_test = pickle.load(fp, encoding = 'latin1')
            p_test = pickle.load(fp, encoding = 'latin1')

        if np.any(x_data_train) == None:
            x_data_train = x_train
            y_data_train = y_train
            p_data_train = p_train
            x_data_test  = x_test
            y_data_test  = y_test
            p_data_test = p_test

        else:
            x_data_train = np.append(x_data_train, x_train, axis=0)
            y_data_train = np.append(y_data_train, y_train, axis=0)
            x_data_test = np.append(x_data_test, x_test, axis=0)
            y_data_test = np.append(y_data_test, y_test, axis=0)
            
    Xtest['{}'.format(dataType)] = x_data_test
    Ytest['{}'.format(dataType)] = y_data_test
    Xtrain['{}'.format(dataType)] = x_data_train
    Ytrain['{}'.format(dataType)] = y_data_train

def ComparePrediction(xdata, ydata, model, boolean):
    predictions = model.predict(xdata)
    index = np.where(ydata == boolean)
    tracks = np.array([])
    for i in index[0]:
        tracks = np.append(tracks, predictions[i])
    return tracks
    
PATH = '/afs/cern.ch/user/a/agekow/trackml/training/RNN'

moduleIterRnd = os.path.join(PATH, 'moduleIterRnd_weights.hdf5')
layerRnd = os.path.join(PATH, 'layerRnd_weights.hdf5')
fullyRnd = os.path.join(PATH, 'fullyRnd_weights.hdf5')
moduleRnd = os.path.join(PATH, 'moduleRnd_weights.hdf5')
moduleRotatedIterRnd = os.path.join(PATH, 'moduleRotatedIterRnd_weights.hdf5')
moduleRotatedRnd = os.path.join(PATH, 'moduleRotatedRnd_weights.hdf5')
we = os.path.join(PATH, 'alexAlgo_weights.hdf5')

weights_str = ['alexAlgo']
weights = [we]
#weights = [fullyRnd, layerRnd, moduleIterRnd, moduleRnd, moduleRotatedIterRnd, moduleRotatedRnd]
#weights_str = ['moduleIterRnd', 'layerRnd', 'fullyRnd', 'moduleRnd', 'moduleRotatedIterRnd', 'moduleRotatedRnd']
for weight, weight_name in zip(weights, weights_str):

    model = Sequential([
        tf.keras.layers.Masking(mask_value=0, input_shape=(maxhits, numFeatures)),
        tf.keras.layers.LSTM(units=64,input_shape = (maxhits,numFeatures), activation='tanh', recurrent_activation = 'sigmoid', recurrent_dropout = 0, unroll = False, use_bias = True),
        tf.keras.layers.Dense(64, activation='relu'),
        tf.keras.layers.Dense(1, activation = 'sigmoid')
    ])
    
    model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False), 
                  optimizer=tf.keras.optimizers.Adam(1e-4),
                  metrics=['accuracy'])
    
    model.load_weights(weight)
    
    for dataType in data_folders:
        
        xtest = Xtest['{}'.format(dataType)]
        ytest = Ytest['{}'.format(dataType)]
        xtrain = Xtrain['{}'.format(dataType)]
        ytrain = Ytrain['{}'.format(dataType)]
        
        true_train = ComparePrediction(xtrain, ytrain, model, 1)
        fake_train = ComparePrediction(xtrain, ytrain, model, 0)
        true_test = ComparePrediction(xtest, ytest, model, 1)
        fake_test = ComparePrediction(xtest, ytest, model, 0)
        
        fig= plt.figure()
        plt.hist(true_train, bins=100, density=True, alpha = .4, color = 'blue', label = 'true train')
        plt.hist(fake_train, bins=100, density=True, alpha = .4, color = 'red', label='fake train')
        plt.hist(true_test, bins=100, density=True, alpha = .4, color = 'green', label='true test')
        plt.hist(fake_test, bins=100, density=True, alpha = .4, color = 'orange', label='fake test')
        plt.xlabel('Prediction')
        plt.ylabel('Number of Events')
        title = '{} Trained, {} Tested'.format(weight_name, dataType)
        plt.title(title)
        plt.legend()
        plt.savefig(os.path.join(PATH, title + '.png'))
