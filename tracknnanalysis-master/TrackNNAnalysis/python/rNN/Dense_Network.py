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

import random
import pickle

sys.path.append("/afs/cern.ch/user/a/agekow/TrackNNAnalysis/source/TrackNNAnalysis/python/utils")

from NetworkEvaluationPlots import plotAccLoss, plotROC

##### Generate data from ttbar sample #######
"""
def get_eta(r, z):
    # eta = -ln(tan(theta/2))
    if z == 0:
        return 0

    sign = 1
    if z < 0:
        sign = -1
        z = abs(z)
    theta = math.atan(r/float(z))
    tan_theta_over_two = math.tan(theta/2.)
    try:
        eta = -math.log( tan_theta_over_two )
    except ValueError:
        print ("can't take log of", tan_theta_over_two, 'from theta =', theta)
        sys.exit()
    return eta * sign

def deltaPhi(a, b):

    dphi = a - b
    if dphi > math.pi:
        dphi -= 2*math.pi
    if dphi < - math.pi:
        dphi += 2*math.pi
        
    return dphi

def get_theta(eta):
    # eta = - ln (tan(theta/2))
    # theta = 2*arctan(exp(-eta))
    theta = 2 * math.atan(math.exp(-eta))
    if theta > 0.5*math.pi:
        theta = theta - math.pi
    return theta

def GetHitData(dataframe, cells, HitNum):
    a = dataframe[dataframe.particle_id == i].iloc[HitNum][[ 'particle_type','pt','x','y','z','module_id']]
    hit_a = dataframe[dataframe.particle_id == i].iloc[HitNum].hit_id
    temp_arr_a = np.array([-1,-1,-1,-1]);
    cells_a = cells[cells.hit_id==hit_a].sort_values(by=['ch0']).iloc[:,1].values
    temp_arr_a[:len(cells_a[:4])] = cells_a[:4];
    a.x = a.x / maxx
    a.y = a.y / maxy
    a.z = a.z / maxz
    
    return a.values, temp_arr_a

def GetFalseHitData(dataframe, cells):
    a = dataframe[["particle_type","pt", "x", "y", "z", "module_id"]]
    hit_a = dataframe.hit_id.values[0]
    temp_arr_a = np.array([-1,-1,-1,-1]);
    cells_a = cells[cells.hit_id==hit_a].sort_values(by=['ch0']).iloc[:,1].values
    temp_arr_a[:len(cells_a[:4])] = cells_a[:4];
    
    a.x = a.x / maxx
    a.y = a.y / maxy
    a.z = a.z / maxz
    
    return a.values, temp_arr_a

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

def ApplyCuts(hits_vt, phi_i, theta_i, phi, theta):
    h0 = hits_vt[(hits_vt.phi <= phi_i + phi) & (hits_vt.phi >= phi_i - phi)                          & (hits_vt.theta <= theta_i + theta) & (hits_vt.theta >= theta_i - theta)]
    
    return h0

fair = [[.006, .02], [.1,.05], [.006,.02], [.15, .05], [.0075, .02], [.2,.05],[.0075, .02]]

#Run for training data
file_path = 'ttbar_pu200_pt500_Viviana_17.01.20'
#file_path = '/eos/user/c/ckaldero/trackML/data/ttbar_pu200_pt500_Viviana_17.01.20'
save_dir = '/afs/cern.ch/user/a/agekow/trackml/training/4layer'
pair = []
for i_e in np.arange(51,56):
    event = 'event1' + format((i_e), '08d')
    hits, cells, particles, truth = load_event(os.path.join(file_path, event))
    hits = add_position_quantities(hits)
    hits.insert(1, "particle_id", truth.particle_id, True)
    particles = add_momentum_quantities(particles)

    cells_s = cells[cells.ch1 == -1]
    

    hits_in_volume = HitsInVolume(hits)
    hits_v13_0t = HitsInLayer(hits, 0)
    hits_v13_1t = HitsInLayer(hits, 1)
    hits_v13_2t = HitsInLayer(hits, 2)
    hits_v13_3t = HitsInLayer(hits, 3)
    
    maxx = 1000
    maxy = 1000
    maxz = 3000
    
    counter1 = 0  
    for i in np.arange(1000):#np.unique(particles.particle_id):
        if len(hits_in_volume.loc[hits_in_volume["particle_id"] == i])  >= 8:
            hits_V = hits_in_volume[hits_in_volume.particle_id == i].sample(n=8)
            hits_V = hits_V.sort_values(by=["r"])
            
            a, cells_a = GetHitData(hits_V, cells_s, 0)
            b, cells_b = GetHitData(hits_V,cells_s, 1)
            c, cells_c = GetHitData(hits_V,cells_s, 2)
            d, cells_d = GetHitData(hits_V,cells_s, 3)
            e, cells_e = GetHitData(hits_V, cells_s, 4)
            f, cells_f = GetHitData(hits_V,cells_s, 5)
            g, cells_g = GetHitData(hits_V,cells_s, 6)
            h, cells_h = GetHitData(hits_V,cells_s, 7)
            
            pair.append((np.concatenate((i, a, cells_a, b, cells_b, c, cells_c, d, cells_d,
                                        e, cells_e, f, cells_f, g, cells_g, h, cells_h), axis = None)))
            

            counter1 += 1
            
    counter2 = 0
    while counter2 < counter1:    
        in0 = hits_v13_0t.sample(n=1)
        phi_i = in0.phi.values[0]
        theta_i = in0.theta.values[0]
        
        try:
            out0 = ApplyCuts(hits_v13_0t, phi_i, theta_i, fair[0][0], fair[0][1]).sample(n = 1)

            in1 = ApplyCuts(hits_v13_1t, out0.phi.values[0], out0.theta.values[0],fair[1][0] , fair[1][1]).sample(n = 1)
            out1 = ApplyCuts(hits_v13_1t, in1.phi.values[0], in1.theta.values[0], fair[2][0], fair[2][1]).sample(n = 1)

            in2 = ApplyCuts(hits_v13_2t, out1.phi.values[0], out1.theta.values[0], fair[3][0] , fair[3][1]).sample(n = 1)
            out2 = ApplyCuts(hits_v13_2t, in2.phi.values[0], in2.theta.values[0], fair[4][0], fair[4][1]).sample(n = 1)

            in3 = ApplyCuts(hits_v13_3t, out2.phi.values[0], out2.theta.values[0], fair[5][0], fair[5][1]).sample(n = 1)
            out3 = ApplyCuts(hits_v13_3t, in3.phi.values[0], in3.theta.values[0], fair[6][0], fair[6][1]).sample(n = 1)
            
            rows = [in0, out0, in1, out1, in2, out2, in3, out3]
            ids = [i.particle_id.values[0] for i in rows]
            
        except:
            continue
        if len(np.unique(ids)) == 1:
            continue
        

        a, cells_a = GetFalseHitData(in0,cells_s)
        b, cells_b = GetFalseHitData(out0,cells_s)
        c, cells_c = GetFalseHitData(in1,cells_s)
        d, cells_d = GetFalseHitData(out1,cells_s)
        e, cells_e = GetFalseHitData(in2,cells_s)
        f, cells_f = GetFalseHitData(out2,cells_s)
        g, cells_g = GetFalseHitData(in3,cells_s)
        h, cells_h = GetFalseHitData(out3,cells_s)


        pair.append((np.concatenate((0, a, cells_a, b, cells_b, c, cells_c, d, cells_d,
                                        e, cells_e, f, cells_f, g, cells_g, h, cells_h), axis = None)))
        
        counter2 += 1

pair_ar = np.array(pair)
np.random.shuffle(pair_ar)

# with open(os.path.join(save_dir,"8hit_pair_HTCondor.txt"), "wb") as fp:
#      pickle.dump(pair_ar, fp)
        
pair_df = pd.DataFrame(pair_ar)
lst = list(range(0, 81))
type_to_drop = lst[1::10]
pt_to_drop = lst[2::10]
cols_to_drop = type_to_drop + pt_to_drop
pair_ar = np.delete(pair_ar,cols_to_drop, axis=1) #Delete particle type information

y = (pair_ar.T[0] != 0).T # True if row is a track, False if row isn't a track
X = pair_ar.T[1:].T # each row represents a unique particle id


frac_train = round(len(X)*0.8)
frac_test = round(len(X)*0.2)

x_train = np.around(X[0: frac_train], decimals=5)
x_test = np.around(X[frac_train:], decimals=5)
#y_train = y[0:frac_train]
#y_test = y[frac_train:]

y_init = pair_ar.T[0]
y = np.array([])
for i in y_init:
    if i == 0.0:
        y = np.append(y, 0)
    else:
        y = np.append(y, 1)

y_train = y[0:frac_train]
y_test = y[frac_train:]


with open('Dense_data.pkl',"wb") as fp:
    pickle.dump(x_train, fp, protocol=2)
    pickle.dump(x_test, fp, protocol=2)
    pickle.dump(y_train, fp,  protocol=2)
    pickle.dump(y_test, fp,  protocol=2)

"""

with open("Dense_data.pkl", "rb") as fp:
    x_train = pickle.load(fp, encoding='latin1')
    x_test = pickle.load(fp, encoding='latin1')
    y_train = pickle.load(fp, encoding='latin1')
    y_test = pickle.load(fp, encoding='latin1')

EPOCHS = 100
BATCH = 128
checkpoint_filepath = "Dense_model_weights.h5"
model_checkpoint_callback = tf.keras.callbacks.ModelCheckpoint(
    filepath=checkpoint_filepath,
    save_weights_only=True,
    monitor='val_accuracy',
    mode='max',
    save_best_only=True)


fixed_norm = initializers.glorot_normal(seed=3)


model=Sequential()
model.add(Dense(50, input_dim=64, activation='relu', name='Dense_1', use_bias=True, kernel_initializer=fixed_norm))
model.add(Dense(1, activation='sigmoid', name='Output',use_bias=True, kernel_initializer=fixed_norm))

model.summary()

EPOCHS=850
BATCH=128
opt = tf.keras.optimizers.Adam(learning_rate=1e-4)
model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False), 
              optimizer=opt, 
              metrics=['accuracy'])

history=model.fit(x_train, y_train, validation_data=(x_test, y_test), 
                  epochs=EPOCHS,batch_size = BATCH, callbacks=model_checkpoint_callback)

model.load_weights(checkpoint_filepath)
model.save('Dense_model.h5')
#model.save_weights('weights_Dense_64input_dropout_sigmoid_biasOff.h5')
config = model.to_json()
with open("Dense_model.json","w") as f:
    f.write(config)


model.load_weights(checkpoint_filepath)

plotAccLoss(model, x_test, y_test)
#plt.savefig(os.path.join(save_dir,"accuracy_loss_8hit.png"))


plotROC(model, x_test, y_test)
