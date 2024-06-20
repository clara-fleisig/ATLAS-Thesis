
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
from sklearn.preprocessing import normalize
from sklearn.preprocessing import scale
from sklearn.metrics import roc_curve, roc_auc_score

import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, Activation, BatchNormalization
from tensorflow.keras import regularizers, initializers
from tensorflow.keras import optimizers
from tensorflow.keras import metrics
#sys.path.append("trackmlchallenge-studies")

from helperFunctions import get_eta, deltaPhi, get_theta
import random
import pickle


# In[7]:


def GetHitData(dataframe, HitNum):
    a = dataframe[dataframe.particle_id == i].iloc[HitNum][['x','y','z','module_id']]
    a.x = a.x / maxx
    a.y = a.y / maxy
    a.z = a.z / maxz
    a.module_id  = a.module_id / 100000
#     hit_a = dataframe[dataframe.particle_id == i].iloc[HitNum].hit_id
#     temp_arr_a = np.array([-1,-1,-1,-1]);
#     cells_a = cells[cells.hit_id==hit_a].sort_values(by=['ch0']).iloc[:,1].values
#     temp_arr_a[:len(cells_a[:4])] = cells_a[:4];
    return a.values
    #return np.concatenate((a.values, temp_arr_a / 10000)).tolist()

def GetHitData2(dataframe, HitNum):
    a = dataframe.iloc[HitNum][['x','y','z','module_id']]
    a.x = a.x / maxx
    a.y = a.y / maxy
    a.z = a.z / maxz
    a.module_id  = a.module_id / 100000
#     hit_a = dataframe[dataframe.particle_id == i].iloc[HitNum].hit_id
#     temp_arr_a = np.array([-1,-1,-1,-1]);
#     cells_a = cells[cells.hit_id==hit_a].sort_values(by=['ch0']).iloc[:,1].values
#     temp_arr_a[:len(cells_a[:4])] = cells_a[:4];
    return a.values
    #return np.concatenate((a.values, temp_arr_a / 10000)).tolist()
    
def GetFalseHitData(dataframe):
    a = dataframe.iloc[0][["x", "y", "z",'module_id']]
    a.x = a.x / maxx
    a.y = a.y / maxy
    a.z = a.z / maxz
    a.module_id  = a.module_id / 100000
#     hit_a = dataframe[dataframe.particle_id == i].iloc[HitNum].hit_id
#     temp_arr_a = np.array([-1,-1,-1,-1]);
#     cells_a = cells[cells.hit_id==hit_a].sort_values(by=['ch0']).iloc[:,1].values
#     temp_arr_a[:len(cells_a[:4])] = cells_a[:4];
    return a.values
    #return np.concatenate((a.values, temp_arr_a / 10000)).tolist()

def FixAngles(hits_vt):
    
    phi_vals = hits_vt.phi.values
    for j in np.arange(len(hits_vt)):
        if phi_vals[j] > math.pi:
             hits_vt.iloc[j].phi = phi_vals[j] - 2*math.pi
                
    eta0 = [get_eta(hits_vt.iloc[i].r, hits_vt.iloc[i].z ) for i in np.arange(len(hits_vt))]
    theta_vals0 = [get_theta(i) for i in eta0]
    hits_vt.insert(10, "theta", theta_vals0, True)
                
    return hits_vt


def HitsInVolume(hits, vol = 13):
    hits_v = hits[(hits.volume_id == vol)]
    hits_vt = hits_v[hits_v.particle_id != 0]
    
    particle_type = [particles.particle_type[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
    hits_vt.insert(1,"particle_type", particle_type, True)
    
    pt = [particles.pt[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
    hits_vt.insert(1,"pt",pt,True)
    
    hits_vt = FixAngles(hits_vt)
    return hits_vt

def HitsInLayer(hits, layer, vol = 13):
    hits_v = hits[(hits.volume_id == vol)&(hits.layer_id == layer)]
    hits_vt = hits_v[hits_v.particle_id != 0]
    
    particle_type = [particles.particle_type[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
    hits_vt.insert(1,"particle_type", particle_type, True)
    
    pt = [particles.pt[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
    hits_vt.insert(1,"pt",pt,True)

    hits_vt = FixAngles(hits_vt)
    return hits_vt

def ApplyCuts(hits_vt, phi_i, theta_i, rho_i, phi, theta, phi_dir, theta_dir):
    h0 = hits_vt[(hits_vt.phi <= phi_i + phi) & (hits_vt.phi >= phi_i - phi)             & (hits_vt.theta <= theta_i + theta) & (hits_vt.theta >= theta_i - theta)             & (hits_vt.rho > rho_i)]
    
    if phi_dir == 'pos':
        h1 = h0[(h0.phi - phi_i) > 0]
    elif phi_dir == 'neg':
        h1 = h0[(h0.phi - phi_i) < 0]
    
    if theta_dir == 'pos':
        h1 = h0[(h0.theta - theta_i) > 0]
    elif theta_dir == 'neg':
        h1 = h0[(h0.theta - theta_i) < 0]
        
    return h1


#fair = [[.006, .02], [.1,.05], [.006,.02], [.15, .05], [.0075, .02], [.2,.05],[.0075, .02]]
#fair = [[.0075, .01], [.12,.08], [.0075,.01], [.17, .08], [.0075, .01], [.22,.08],[.0075, .01]]
fair = [[.005, .01], [.1,.03], [.005,.01], [.13, .03], [.005, .01], [.1,.03],[.005, .01]]

def PairHitsInLayer():
    global maxhits
    ids = []
    stack = []
    out = hits_v13_0t.sample(n=1)
    phi = out.phi.values[0]
    theta = out.theta.values[0]
    rho = out.rho.values[0]
    current_id = out.particle_id.values[0]
    ids.append(current_id)
    q = GetFalseHitData(out)
    stack.append(q.tolist())
    previous_hit = out.hit_id.values[0]

    for j, i in zip(list(range(7)),[0,1,1,2,2,3,3]):

        for k in np.arange(10):#random.choice([1,2,3,4])):
            curve_dir = ['pos', 'neg']
            output = ApplyCuts(layers[i], phi, theta, rho, fair[j][0], fair[j][1], phi_dir = random.choice(curve_dir), theta_dir = random.choice(curve_dir))
            if len(output) > 1:
                out = output.sample(n=1)
                if previous_hit == out.hit_id.values[0]:
                    continue
                else:
                    ids.append(out.particle_id.values[0])
                    w = GetFalseHitData(out)
                    stack.append(w.tolist())
                    phi = out.phi.values[0]
                    theta = out.theta.values[0]
                    rho = out.rho.values[0]
                    previous_hit = out.hit_id.values[0]

            elif len(output) <= 1:
                break

        if len(np.unique(ids)) == 1:
            continue
    length = len(stack)
    for l in np.arange(maxhits - length):
        stack.append(np.zeros(4).tolist())
    if len(stack) > 15:
        stack = np.delete(stack,np.s_[15:], 0)
        
    return stack


# In[10]:


#Real Run
#file_path = 'ttbar_pu200_pt500_Viviana_17.01.20'
file_path = '/eos/user/c/ckaldero/trackML/data/ttbar_pu200_pt500_Viviana_17.01.20'
save_dir = '/afs/cern.ch/user/a/agekow/trackml/training/RNN'
x_data = np.zeros((15,4))
y_data = []
for i_e in np.arange(51,250):
    event = 'event1' + format((i_e), '08d')
    hits, cells, particles, truth = load_event(os.path.join(file_path, event))
    cells_s = cells[cells.ch1 == -1]
    hits = add_position_quantities(hits)
    hits.insert(1, "particle_id", truth.particle_id, True)
    particles = add_momentum_quantities(particles)

    hitsv13 = HitsInVolume(hits)
    nhits = [len(hitsv13[hitsv13.particle_id == n]) for n in hitsv13.particle_id]
    hitsv13.insert(3, "nhits", nhits, True)
    maxx = max(hitsv13.x)
    maxy = max(hitsv13.y)
    maxz = max(hitsv13.z)

    hits_v13_0t = HitsInLayer(hits, 0)
    hits_v13_1t = HitsInLayer(hits, 1)
    hits_v13_2t = HitsInLayer(hits, 2)
    hits_v13_3t = HitsInLayer(hits, 3)
    
    layers = [hits_v13_0t, hits_v13_1t, hits_v13_2t, hits_v13_3t]
    maxhits = max(hitsv13.nhits)
    counter1 = 0  
    loop = np.unique(hitsv13.particle_id)
    for i in loop:
            hits_V = hitsv13[hitsv13.particle_id == i]
            hits_V = hits_V.sort_values(by=["rho"])
            stack = []
            for m in np.arange(len(hits_V)):
                a = GetHitData2(hits_V, m)
                stack.append(a.tolist())
            for n in np.arange(maxhits - len(hits_V)):
                stack.append(np.zeros(4).tolist())
            
            x_data = np.concatenate((x_data, stack), axis = 0)
            #x_data.append(np.array(stack))
            y_data.append(1)

            counter1 += 1
    counter2 = 0
    while counter2 < counter1:
        x_data = np.concatenate((x_data, PairHitsInLayer()), axis = 0)
        #x_data.append(np.array(PairHitsInLayer()))
        y_data.append(0)
        counter2 += 1


# In[11]:


x_dat = np.delete(x_data, np.s_[0:15],axis=0)

X_data = x_dat.reshape(int(len(x_dat)/15), 15, 4)
frac_train = round(len(X_data) * .8)
c = list(zip(X_data, y_data))
random.shuffle(c)
X, Y = zip(*c)

X = np.array(X)
Y = np.array(Y)

x_train = X[0:frac_train]
y_train = Y[0:frac_train]

x_val = X[frac_train:]
y_val = Y[frac_train:]


# In[22]:


fixed_norm = initializers.glorot_normal(seed=3)
EPOCHS = 500
BATCH = 128
PATH = os.path.join(save_dir, "weights.hdf5")
checkpoint_filepath = PATH
model_checkpoint_callback = tf.keras.callbacks.ModelCheckpoint(
     filepath=checkpoint_filepath,
     save_weights_only=True,
     monitor='val_accuracy',
     mode='max',
     save_best_only=True)
#LSTM formatted for LSTM_CuDNN. Parameters can be changed for use in GPU w/o LSTM CuDNN capabilities
model = Sequential([
    tf.keras.layers.Masking(mask_value=0, input_shape=(15, 4)),
    tf.keras.layers.LSTM(units=64,input_shape = (15,4), activation='tanh', recurrent_activation = 'sigmoid', recurrent_dropout = 0, unroll = False, use_bias = True),
    tf.keras.layers.Dense(64, activation='relu'),
    tf.keras.layers.Dense(1, activation = 'sigmoid')
])


model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False), 
              optimizer=tf.keras.optimizers.Adam(1e-4),
              metrics=['accuracy'])

history=model.fit(x_train, y_train, validation_data=(x_val, y_val), 
                  epochs=EPOCHS,batch_size = BATCH), callbacks=[model_checkpoint_callback]))


# In[24]:


model.load_weights(checkpoint_filepath)

test_loss, test_acc = model.evaluate(x_val,  y_val, verbose=2)
fig = plt.figure(figsize =(16,8))
plt.suptitle('track finding 80/20 layers 0-3')
plt.subplot(1,2,1)
plt.plot(history.history['accuracy'], label='accuracy')
plt.plot(history.history['val_accuracy'], label = 'val_accuracy')
plt.plot([0,EPOCHS],[test_acc, test_acc],linestyle='--',color="k",label="Final Accuracy")
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.ylim([0, 1])
plt.yticks(np.arange(0,1,.05))
plt.legend(loc='lower right')

plt.subplot(1,2,2)
plt.plot(history.history['loss'], label='loss')
plt.plot(history.history['val_loss'], label = 'val_loss')
plt.xlabel('Epoch')
plt.ylabel('loss')
plt.legend(loc='upper right')

plt.savefig(os.path.join(save_dir,"accuracy_loss_RNN.png"))




