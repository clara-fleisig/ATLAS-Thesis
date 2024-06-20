
import os
import sys
import numpy as np
import pandas as pd

from trackml.dataset import load_event
from trackml.randomize import shuffle_hits
from trackml.score import score_event
from trackml.utils import add_position_quantities, add_momentum_quantities

import matplotlib.pyplot as plt
import math
from sklearn.model_selection import train_test_split


import tensorflow as tf
from tensorflow.keras.layers import Dense, GRU, Input, Masking
from tensorflow.keras import optimizers
sys.path.append("TrackML/trackmlchallenge-studies")

from helperFunctions import get_eta, deltaPhi, get_theta


## Target is a sequence of hit coordinates ###

maxhits = 40
numFeatures = 3
x_data = None
y_data = None
#Run for training data
file_path = 'ttbar_pu200_pt500_Viviana_17.01.20'
#file_path = '/eos/user/c/ckaldero/trackML/data/ttbar_pu200_pt500_Viviana_17.01.20'
for i_e in np.arange(51, 52):
    event = 'event1' + format((i_e), '08d')
    #print(event)
    hits, cells, particles, truth = load_event(os.path.join(file_path, event))
    hits = add_position_quantities(hits)
    hits.insert(1, "particle_id", truth.particle_id, True)
    hits = hits[hits.particle_id < 1e5]
    particles = add_momentum_quantities(particles)
    
    hitsV = hits[hits.volume_id == 13]
    hitsV = hitsV[hitsV.particle_id !=0]
    hitsV.sort_values(by=['particle_id'], inplace=True)
    
    hitsV.x = hitsV.x / 1000
    hitsV.y = hitsV.y / 1000
    hitsV.z = hitsV.z / 3000
    for ii in np.unique(hitsV.particle_id):
        track = None
        target = None
        h = hitsV[hitsV.particle_id==ii]
        layers = np.unique(h.layer_id)
        if len(layers) < 3:
            continue

        # In this loop, build track and target hit
        for jj in layers:
            h0 = h[h.layer_id == jj]
            coordinates = h0.iloc[0][['x','y','z']].values
            
            # If this is the first hit of the track, add it to the inputs
            if jj == layers[0]:
                track = coordinates
                
            #If this is the last hit, add it only to targets
            elif jj == layers[-1]:
                target = np.append(target,coordinates, axis=0)
                
            else:
                track = np.append(track, coordinates, axis=0)
                
                if np.any(target) == None:
                    target = coordinates
                else:
                    target = np.append(target, coordinates, axis=0)
                    

        
        z = np.zeros((int(7-len(track)/3),3))
        track = np.append(track, z)
        target = np.append(target, z)
        
        if np.any(x_data) == None:
            x_data = track
        else:
            x_data = np.append(x_data, track, axis=0)
        
        if np.any(y_data) == None:
            y_data = target
        else:
            y_data = np.append(y_data, target, axis=0)
            

x_data = x_data.reshape(int(len(x_data)/21), 7,3)
y_data = y_data.reshape(int(len(y_data)/21), 7,3)

x_train, x_test, y_train, y_test = train_test_split(x_data, y_data)

class lstm_bottleneck(tf.keras.layers.Layer):
    def __init__(self, lstm_units, time_steps, **kwargs):
        self.lstm_units = lstm_units
        self.time_steps = time_steps
        self.lstm_layer = tf.keras.layers.GRU(lstm_units, return_sequences=False)
        self.repeat_layer = tf.keras.layers.RepeatVector(time_steps)
        super(lstm_bottleneck, self).__init__(**kwargs)
    
    def call(self, inputs):
        # just call the two initialized layers
        return self.repeat_layer(self.lstm_layer(inputs))
    
    def compute_mask(self, inputs, mask=None):
        # return the input_mask directly
        return mask


maxhits = 3
n_features = 3

inputs = Input(shape=(maxhits, n_features))
masked_input = Masking(mask_value=0, input_shape=(maxhits,n_features))(inputs)
encoded = GRU(100, return_sequences=True)(masked_input)
encoded = lstm_bottleneck(lstm_units=100, time_steps=3)(encoded)
#decoded = RepeatVector(maxhits)(encoded)

decoded = GRU(100, return_sequences=True)(encoded)
decoded = GRU(100, return_sequences=True)(decoded)
decoded = Dense(3)(decoded)

gru_autoencoder = tf.keras.Model(inputs, decoded)

adam = optimizers.Adam(1e-3)
gru_autoencoder.compile(loss='mse', optimizer=adam)

gru_autoencoder.summary()

epochs = 20
batch = 64

history = lstm_autoencoder.fit(x_train, y_train, 
                                                epochs=epochs, 
                                                batch_size=batch, 
                                                validation_data=(x_test, y_test),
                                                verbose=1)

