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

from helperFunctions import get_eta, deltaPhi, get_theta
import random
import pickle



import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--outName",        type = str,   required = True,      help = "Outname of the pickle")
parser.add_argument("--nEvents",        type = int,   default = 10,         help = "Number of tracks in the training sample")
parser.add_argument("--traintestSplit", type = float, default = 0.7,        help = "Fraction of tracks to split into test and train")
parser.add_argument('--maxTrackLen',    type = int,   default = 40,         help = "Max length to pad")
args = parser.parse_args()
maxhits = args.maxTrackLen
numFeatures = 3

def GetHitData(dataframe, HitNum):
    a = dataframe.iloc[HitNum][['x','y','z']]
    
    return a.values
def GetFalseHitData(dataframe):
    a = dataframe.iloc[0][["x", "y", "z"]]

    return a.values

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
    
#    particle_type = [particles.particle_type[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
#    hits_vt.insert(1,"particle_type", particle_type, True)
    
#    pt = [particles.pt[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
#    hits_vt.insert(1,"pt",pt,True)
    
    hits_vt = FixAngles(hits_vt)
    
    return hits_vt

def HitsInLayer(hits, layer, vol = 13):
    hits_v = hits[(hits.volume_id == vol)&(hits.layer_id == layer)]
    hits_vt = hits_v[hits_v.particle_id != 0]
    
#    particle_type = [particles.particle_type[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
#    hits_vt.insert(1,"particle_type", particle_type, True)
    
#    pt = [particles.pt[particles.particle_id == i].values[0] for i in hits_vt.particle_id]
#    hits_vt.insert(1,"pt",pt,True)

    hits_vt = FixAngles(hits_vt)
    return hits_vt

def ApplyCuts(hits_vt, input_hit, phi, theta):
    
    phi_i = input_hit.phi.values[0]
    theta_i = input_hit.theta.values[0]
    rho_i = input_hit.rho.values[0]
    
    h0 = hits_vt.loc[(hits_vt.phi <= phi_i + phi) & (hits_vt.phi >= phi_i - phi) \
            & (hits_vt.theta <= theta_i + theta) & (hits_vt.theta >= theta_i - theta) \
            & (hits_vt.rho >= rho_i)]
    
    h0 = h0.sort_values(by=['r'])
        
    return h0

def computeTrackProb(ids):
    counter = 0
    num = ids[0]
    idsList = ids.tolist()
    for i in idsList: 
        curr_frequency = idsList.count(i) 
        if(curr_frequency> counter): 
            counter = curr_frequency 
            num = i 
    prob = float(idsList.count(num) / len(ids))
    return prob

def PairInLayer(section, layer, initial_hit):
    track = np.array([])
    ids = []
    out = initial_hit
    previous_hit = initial_hit.hit_id.values[0]
    if section == "barrel":
        
        if layer == 0:
            cut_vals = [.007, .06]
        elif layer == 1:
            cut_vals = [.008, .04]
        elif layer == 2:
            cut_vals = [.009,.05]
        elif layer == 3:
            cut_vals = [.008,.04]
            
    if (section == "endcap12") | (section == "endcap14"):
        if layer == 0:
            cut_vals = [.075,.06 ]
        elif layer == 1:
            cut_vals = [.075,.06 ]
        elif layer == 2:
            cut_vals = [.075,.06 ]
        elif layer == 3:
            cut_vals = [.075,.05 ]
        elif layer == 4:
            cut_vals = [.075,.05 ]
        elif layer == 5:
            cut_vals = [.075,.05 ]
    
    loop = random.choice([1,1,1,1,1,2])
    for ii in np.arange(loop):
        if section == "barrel":
            output = ApplyCuts(barrel13[layer], out, cut_vals[0],cut_vals[1])
        elif section == "endcap12":
            output = ApplyCuts(endcap12[layer], out, cut_vals[0],cut_vals[1])
        elif section == "endcap14":
            output = ApplyCuts(endcap14[layer], out, cut_vals[0],cut_vals[1])

        if len(output) > 1:
            
            output = pd.DataFrame(output[output.hit_id != previous_hit])
            output.sort_values(by=["r"], inplace=True)
            track = np.append(track,GetFalseHitData(output))
            out = pd.DataFrame(output.iloc[0]).T
            
            ids.append(out.particle_id.values[0])
            previous_hit = out.hit_id.values[0]
            

        elif len(output) <= 1 :
            out = initial_hit
            break

    return track, out, ids
    
def PairBetweenBarrelLayers(outer_layer, initial_hit):
    
#format of cut_vals is [barrel_phi, barrel_theta, endcap_phi, endcap_theta]
    if (outer_layer == 1):
        cut_vals = [.15,.1,.3,.02]
        zcut = 600
    elif outer_layer == 2:
        cut_vals = [.13, .3, .3, .015]
        zcut = 800
    elif outer_layer == 3:
        cut_vals = [.2,.1,.15,.01]
        zcut = 1100
        
    z = initial_hit.z.values[0]
    if (z > zcut) | (z < -zcut):
        output = ApplyCuts(barrel13ec[outer_layer], initial_hit, cut_vals[2], cut_vals[3])
        if len(output) >= 1:
            output = output.sort_values(by='r')
            out = pd.DataFrame(output.iloc[0]).T
        else:
            out = initial_hit
    else:
        output = ApplyCuts(barrel13[outer_layer], initial_hit, cut_vals[0], cut_vals[1])
        if len(output) > 1:
            output = output.sort_values(by='r')
            out = pd.DataFrame(output.iloc[0]).T
        else:
            out = initial_hit

    return out

def PairBetweenECLayers(volume, outer_layer, initial_hit):
    global phi_dir, theta_dir, track
#format of cut_vals is [barrel_phi, barrel_theta, endcap_phi, endcap_theta]
    if (outer_layer == 1):
        cut_vals = [.0075,.006]
    elif outer_layer == 2:
        cut_vals = [.0075,.006]
    elif outer_layer == 3:
        cut_vals = [.0075,.004]
    elif outer_layer == 4:
        cut_vals = [.075,.004]
    elif outer_layer == 5:
        cut_vals = [.075,.004]
        
    
    z = initial_hit.z.values[0]
    
    if volume == 12:
        output = ApplyCuts(endcap12[outer_layer], initial_hit, cut_vals[0], cut_vals[1])
        if len(output) > 1:
            output = output.sort_values(by=['r'])
            out = pd.DataFrame(output.iloc[0]).T
        else:
            out = initial_hit
            
    if volume == 14:
        output = ApplyCuts(endcap14[outer_layer], initial_hit, cut_vals[0], cut_vals[1])
        if len(output) > 1:
            output = output.sort_values(by=['r'])
            out = pd.DataFrame(output.iloc[0]).T
        else:
            out = initial_hit

    return out

def PairHits():
    
    particle_ids = np.array([])
    part_tracks = np.array([])
    
    out = bec.sample(n=1)
    previous_hit = out.hit_id.values[0]
    z = out.z.values[0]
    current_id = out.particle_id.values[0]
    particle_ids = np.append(particle_ids, current_id)
    part_tracks = GetFalseHitData(out)
    
    current_layer = out.layer_id.values[0]
        
    for II in np.arange(4):
        if out.volume_id.values[0]  == 13:
            track, out, ids = PairInLayer("barrel",II, out)
            EC = False
        elif out.volume_id.values[0] == 14:
            track, out, ids = PairInLayer("endcap14",current_layer, out) 
            EC = True
            break
        elif out.volume_id.values[0] == 12:
            track, out, ids = PairInLayer("endcap12",current_layer, out)
            EC = True
            break
        part_tracks = np.append(part_tracks, track)    
        previous_hit = out.hit_id.values[0]
        particle_ids = np.append(particle_ids,ids)
        
        if II < 3:
            out = PairBetweenBarrelLayers(II+1, out)
            if previous_hit != out.hit_id.values[0]:
                vals = GetFalseHitData(out)
                ids = out.particle_id.values[0]
                part_tracks = np.append(part_tracks, vals)
                particle_ids = np.append(particle_ids, ids)
            else:
                continue
                
    if EC == True:                    
        for JJ in np.arange(current_layer, 6):
            
            if JJ < 5:
                out = PairBetweenECLayers(out.volume_id.values[0], JJ+1, out)
                if previous_hit != out.hit_id.values[0]:
                    vals = GetFalseHitData(out)
                    ids = out.particle_id.values[0]
                    particle_ids = np.append(particle_ids,ids)
                    part_tracks = np.append(part_tracks, vals)
                else:
                    continue
            
            if out.volume_id.values[0] == 14:
                track, out, ids = PairInLayer("endcap14",JJ, out) 
            elif out.volume_id.values[0] == 12:
                track, out, ids = PairInLayer("endcap12",JJ, out)
            
            part_tracks = np.append(part_tracks, track)
            previous_hit = out.hit_id.values[0]
            particle_ids = np.append(particle_ids,ids)
            
    l = int((len(part_tracks) / 3)%maxhits)
    if l != 0:
        z = np.zeros((maxhits-l,3))
        part_tracks = np.append(part_tracks,z)
    
    
    return part_tracks, particle_ids
    
file_path = '/eos/user/c/ckaldero/trackML/data/ttbar_pu200_pt500_Viviana_17.01.20'
save_dir = '/afs/cern.ch/work/s/sabidi/public/ForAlex/TrackML/pickles/alexAlgo'

x_data = np.array([])
y_data = []
track_probs = []
for i_e in np.arange(51, 51+args.nEvents):
    event = 'event1' + format((i_e), '08d')
    hits, cells, particles, truth = load_event(os.path.join(file_path, event))
    
    ##### Format Data ######

    cells_s = cells[cells.ch1 == -1]
    hits = add_position_quantities(hits)
    hits.insert(1, "particle_id", truth.particle_id, True)
    particles = add_momentum_quantities(particles)
    
    hitsV = hits.loc[(hits.volume_id == 12) | (hits.volume_id == 13) | (hits.volume_id == 14)]
    hitsV = hitsV[hitsV.particle_id!=0]
    hitsv13 = HitsInVolume(hits)
    hitsv14 = HitsInVolume(hits, 14)
    hitsv12 = HitsInVolume(hits, 12)
    
    nhits13 = [len(hitsv13[hitsv13.particle_id == n]) for n in hitsv13.particle_id]
    hitsv13.insert(3, "nhits", nhits13, True)
    
    nhits14 = [len(hitsv14[hitsv14.particle_id == n]) for n in hitsv14.particle_id]
    hitsv14.insert(3, "nhits", nhits14, True)
    
    nhits12 = [len(hitsv12[hitsv12.particle_id == n]) for n in hitsv12.particle_id]
    hitsv12.insert(3, "nhits", nhits12, True)

    barrel13 = {}
    for i in np.arange(4):
        barrel13[i] = hitsv13.loc[hitsv13.layer_id == i]
        
    endcap12 = {}
    for i in np.arange(6):
        endcap12[i] = hitsv12.loc[hitsv12.layer_id == i]
        
    endcap14 = {}
    for i in np.arange(6):
        endcap14[i] = hitsv14.loc[hitsv14.layer_id == i]

    barrel13ec = {}
    for i in np.arange(4):
        barrel13ec[i] = pd.concat([barrel13[i], endcap12[0], endcap14[0]])
    
    bec = pd.concat([barrel13[0], endcap12[0], endcap12[1], endcap12[2], endcap12[3], endcap12[4], endcap12[5],
               endcap14[0], endcap14[1], endcap14[2], endcap14[3], endcap14[4], endcap14[5]])
    
    #### Find all Real Tracks #####
    
    counter1 = 0  
    loop =np.unique(hitsV.particle_id)
    for i in loop:
        hits_V = hitsV[hitsV.particle_id == i]
        hits_V = hits_V.sort_values(by=["rho"])
        stack = np.array([])
        for m in np.arange(len(hits_V)):
            a = GetHitData(hits_V, m)
            stack = np.append(stack,a)
        for n in np.arange(maxhits - len(hits_V)):
            stack = np.append(stack, np.zeros(3))

        x_data = np.append(x_data, stack)
        y_data.append(1)
        track_probs.append(1)
        counter1 += 1
    
    # Generate Fake Tracks
    counter2 = 0
    fakes = np.array([])
    while counter2 < counter1:
        track, ids = PairHits()
        if len(np.unique(ids)) == 1:
            continue
        elif len(track) != (maxhits * 3):
            continue
        else:
            fakes = np.append(fakes, track)
            y_data.append(0)
            track_probs.append(computeTrackProb(ids))
            counter2+=1
        
    x_data = np.append(x_data,fakes)

l = int(len(x_data) / (maxhits*numFeatures))
x_data = x_data.reshape(l,maxhits,numFeatures)

#Shuffle Data
c = list(zip(x_data, y_data, track_probs))
random.shuffle(c)
c = list(zip(*c))
x_data = np.array(list(c[0]))
y_data = list(c[1])
trackProb = list(c[2])
  
nTrain = int(len(x_data) * args.traintestSplit)
hitTrain    = x_data[:nTrain]
labelTrain  = y_data[:nTrain]
probTrain = trackProb[:nTrain]
hitTest     = x_data[nTrain:]
labelTest   = y_data[nTrain:]
probTest = trackProb[nTrain:]

### Save to pickle
pFile = open(os.path.join(save_dir, args.outName + '.pkl'),'wb')

def _dumpObj(pFile, obj):
    pickle.dump(obj , pFile, protocol = 2)
    
_dumpObj(pFile, hitTrain)
_dumpObj(pFile, labelTrain)
_dumpObj(pFile, probTrain)
_dumpObj(pFile, hitTest)
_dumpObj(pFile, labelTest)
_dumpObj(pFile, probTest)
pFile.close()
