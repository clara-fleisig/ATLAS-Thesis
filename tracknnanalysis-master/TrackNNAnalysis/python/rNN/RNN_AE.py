
import tensorflow as tf
from tensorflow.keras import optimizers, Sequential
from tensorflow.keras.models import Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras.layers import Dense, LSTM, RepeatVector, TimeDistributed, Input, Masking, Lambda
import numpy as np
import pickle
import matplotlib.pyplot as plt
import tensorflow.keras.backend as K
from trackml.dataset import load_event
import os
from trackml.utils import add_position_quantities, add_momentum_quantities
import copy
import math
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import OneHotEncoder

def cleanHitList(inHitList):
    hitList = inHitList.sort_values(by=["rho"]).reset_index()

    # rotate the hits to zero

    hitList = copy.deepcopy(hitList)

    xVal = hitList.at[0, 'x']
    yVal = hitList.at[0, 'y']
    zVal = hitList.at[0, 'z']

    # rotate it to 0 on the xAxis
    angle = math.atan(xVal/yVal)
    if(yVal < 0): angle += math.pi

    signZ = 1
    # make all z positive
    if(zVal < 0): signZ = -1 

    for i in hitList.index:
        xVal = hitList.at[i, 'x']
        yVal = hitList.at[i, 'y']
        zVal = hitList.at[i, 'z']
        hitList.at[i, 'x'] = xVal * math.cos(angle) - yVal * math.sin(angle)
        hitList.at[i, 'y'] = xVal * math.sin(angle) + yVal * math.cos(angle)
        hitList.at[i, 'z'] = signZ * zVal

    return hitList

maxhits = 40
n_features = 9
enc = OneHotEncoder()
layer_oneHot = None
#file_path = 'ttbar_pu200_pt500_Viviana_17.01.20'
file_path = '/eos/user/c/ckaldero/trackML/data/ttbar_pu200_pt500_Viviana_17.01.20'
save_dir = '/afs/cern.ch/user/a/agekow/trackml/training/RNN_AE'
# If save_dir does not exist, create it
if not os.path.exists(save_dir):
    os.makedirs(save_dir)

#Load in Data
x_data_true = None
for i_e in np.arange(51,100):
    event = 'event1' + format((i_e), '08d')
    print(event)
    hits, cells, particles, truth = load_event(os.path.join(file_path, event))
    cells_s = cells[cells.ch1 == -1]
    hits = add_position_quantities(hits)
    hits.insert(1, "particle_id", truth.particle_id, True)
    
    # Look at all hits in volumes 12,13,14 and scale x,y,z by factor of 1000
    h = hits[hits.particle_id != 0]
    h = h[(h.volume_id == 12) | (h.volume_id == 13) | (h.volume_id == 14)]

    h.sort_values(by=['particle_id'], inplace=True)
    h.x = h.x / 1000
    h.y = h.y / 1000
    h.z = h.z / 1000
    
    # fit layer oneHot Encoder to layers present in dataset, keep this fit for all consecutive datasets loaded
    if layer_oneHot == None:
        layer_oneHot = enc.fit(h.layer_id.values.reshape(-1,1))
    
    # Create a 40 hit track for each particle ID, append with -2's which are used to mask in training
    tracks = None
    count = 0
    for II in np.unique(h.particle_id.values):
        t = cleanHitList(h[h.particle_id == II])
        track = t[['x', 'y', 'z']].values
        
        layer = enc.transform(t.layer_id.values.reshape(-1,1)).toarray()
        track = np.append(track, layer, axis=1)
        count += 1
        pad_len = 40 - len(track)
        zeros = np.zeros((pad_len, n_features))
        zeros = zeros + -2
        track = np.append(track, zeros, axis=0)
        
        if np.any(tracks) == None:
            tracks = track
        else:
            tracks = np.append(tracks, track, axis=0)
            
    tracks = tracks.reshape(int(tracks.shape[0] / maxhits), maxhits, n_features)        
    if np.any(x_data_true) == None:
        x_data_true = tracks
    else:
        x_data_true = np.append(x_data_true, tracks, axis=0)
        
x_train, x_test, _, _ = train_test_split(x_data_true, x_data_true, train_size=0.9)

#Necessary to keep the output of the NN the same length as the input, otherwise all 40 hits would be filled
def cropOutputs(x):

    #x[0] is decoded at the end
    #x[1] is inputs
    #both have the same shape

    #padding = 1 for actual data in inputs, -2 for mask
    padding =  K.cast( K.not_equal(x[1],-2), dtype=K.floatx())
        #if you have zeros for non-padded data, they will lose their backpropagation

    return x[0]*padding

inputs = Input(shape=(maxhits, n_features))
masked_input = Masking(mask_value=-2.0, input_shape=(maxhits,n_features))(inputs)
encoded = LSTM(100)(masked_input)
decoded = RepeatVector(maxhits)(encoded)
decoded = LSTM(n_features, return_sequences=True)(decoded)
decoded = Lambda(cropOutputs,output_shape=(maxhits,n_features))([decoded,inputs])
lstm_autoencoder = Model(inputs, decoded)

print(lstm_autoencoder.summary())

adam = optimizers.Adam(5e-4)
lstm_autoencoder.compile(loss='mse', optimizer=adam, metrics=['accuracy'])
epochs = 100
batch = 128

callback = tf.keras.callbacks.ModelCheckpoint(
         filepath=save_dir + '/Best_model.h5',
         save_weights_only=False,
         monitor='val_loss',
         mode='max',
         save_best_only=True)

history = lstm_autoencoder.fit(x_train, x_train, 
                                                epochs=epochs, 
                                                batch_size=batch, 
                                                validation_data=(x_test, x_test),
                                                verbose=0, callbacks=[callback])


