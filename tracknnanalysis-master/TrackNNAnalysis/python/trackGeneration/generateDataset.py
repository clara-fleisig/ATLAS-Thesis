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
from sklearn.preprocessing import OneHotEncoder
from sklearn.preprocessing import LabelEncoder


from trackml.dataset import load_event
from trackml.randomize import shuffle_hits
from trackml.score import score_event
from trackml.utils import add_position_quantities, add_momentum_quantities,decode_particle_id

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--outName",        type = str,      required = True,      help = "Outname of the pickle")
parser.add_argument("--nEvents",        type = int,      default = 10,         help = "Number of tracks in the training sample")
parser.add_argument("--traintestSplit", type = float,    default = 0.4,        help = "Fraction of tracks to split into test and train")
parser.add_argument("--fracFake",       type = float,    default = 0.5,        help = "Fraction of tracks that are fake")
parser.add_argument('--fakeType',       type = str,      default = "moduleRotatedRnd", choices=["fullyRnd", "layerRnd", "layerIterRnd", "moduleRnd", "moduleIterRnd", "moduleRotatedRnd", "moduleRotatedIterRnd"], help='Type of fakes wanted')
parser.add_argument('--maxTrackLen',    type = int,      default = 40,         help = "Max length to pad")
parser.add_argument('--filePattern',    type = str,      default = "(.*)",     help = "Pattern to match input file")
parser.add_argument("--rotateToZero",   default = False, action='store_true',  help = "Rotate the hits so the first hit is rotated to zero")


args = parser.parse_args()



def main():
    #fileList = getFileList("data/", args.filePattern)
    fileList = getFileList("/eos/user/c/ckaldero/trackML/data/ttbar_pu200_pt500_Viviana_17.01.20/", args.filePattern)
    trueHits = []
    fakeHits = []

    # read detectorInfo
    detectorInfo = pd.read_csv("data/detectors.csv")

    # detectorInfo["oneHotModule"] = detectorInfo["module_id"]
    # detectorInfo["oneHotModule"][(detectorInfo["volume_id"] != 12) & (detectorInfo["volume_id"] != 13) & (detectorInfo["volume_id"] != 14)] += 18000
    # detectorInfo = pd.concat([detectorInfo,pd.get_dummies(detectorInfo['oneHotModule'], prefix='oneHotModule')],axis=1)
    # for name in detectorInfo:
    #     print name

    # print detectorInfo

    index  = 0
    for fileName in fileList:
        index += 1
        if(index > 1): continue
        ## Read the information
        hits, cells, particles, truth = load_event(fileName)

        # decode particle id into vertex id, generation, etc.
        particles = decode_particle_id(particles)
        # add vertex rho, phi, r
        particles = add_position_quantities(particles, prefix='v')
        # add momentum eta, p, pt
        particles = add_momentum_quantities(particles)

        hits = add_position_quantities(hits)
        hits.insert(1, "particle_id", truth.particle_id, True)
        print(hits.head())


        hits = hits[(hits["volume_id"] == 12) | (hits["volume_id"] == 13) | (hits["volume_id"] == 14)]

        ####### truth record
        # hits, cells, particles, truth = decorateExtraInfo(hits = hits, cells = cells, particles = particles, truth = truth)

        hitbyVolumeLayer = {}

        # unique layer elements
        volumeList = hits["volume_id"].unique() 

        for volume in volumeList:
            hitsInVolume = hits[hits["volume_id"] == volume]

            layerList = hitsInVolume["layer_id"].unique() 
            hitsbyLayer = {}

            for layer in layerList:
                hitsbyLayer[layer] = hitsInVolume[hitsInVolume["layer_id"] == layer]

            hitbyVolumeLayer[volume] = hitsbyLayer

        # print detectorInfo

        if(args.nEvents < 0):
            nTruth = -1;
        else:
            nTruth = int(args.nEvents * (1 - args.fracFake))

        print("Generating truth hits")
        ##### 0 - Truth
        cTrueHits = getTrueHits(hits = hits, cells = cells, particles = particles, truth = truth, nTracks = nTruth)
        trueHits += cTrueHits

        nTruth = len(cTrueHits)

        if(args.nEvents < 0):
            nFake  = int(nTruth * args.fracFake /(1 - args.fracFake))
        else:
            nFake  = args.nEvents - nTruth

        print("Got", nTruth, "true hits, getting ", nFake, " fake hits")

        # plotHits(trueHits, [], "Plots/trueHits")

        ############# Generate the fakes

        cFakeHits = None
        print("Generating fake hits")
        if(args.fakeType == "fullyRnd"):
            ##### 1 - Total chaos -- we should be able to fully reject these with an uber small false postive rate
            cFakeHits, _ = getFullyRandomHits(hits = hits, cells = cells, particles = particles, truth = truth, nTracks = nFake)
            #plotHits(cFakeHits, [], "Plots/fullyRndHits")
        elif(args.fakeType == "layerRnd" or args.fakeType == "layerIterRnd"):
            ###### 2 - Total chaos -- we should be able to fully reject these with an uber small false postive rate
            cFakeHits, seedHits = getLayerRandomHits(hits = hits, hVL = hitbyVolumeLayer, cells = cells, particles = particles, truth = truth, nTracks = nFake)
            #plotHits(cFakeHits, seedHits, "Plots/" + args.fakeType + "Hits")
        elif(args.fakeType == "moduleRnd" or args.fakeType == "moduleIterRnd"):
            ####### 3 - This is getting close to what we might get off the detector at trigger level
            ####### Where we idenity Region of intrests (ROI) and consider combinatorical hits from there
            ####### This might be a slightly better than real life, as ROI would be larger
            cFakeHits, seedHits = getModuleRandomHits(hits = hits, hVL = hitbyVolumeLayer, cells = cells, particles = particles, truth = truth, nTracks = nFake)
            # plotHits(cFakeHits, seedHits, "Plots/" + args.fakeType + "Hits")
        elif(args.fakeType == "moduleRotatedIterRnd" or args.fakeType == "moduleRotatedRnd"):
            ##### 5 - Take a Rnd particle and randomly rotate it in xy space/flip it in z space and then repeat the algo 3/4
            ##### if hits don't statisfy a certain distance cut, just skip that layer
            ##### second option is that we iteratively expand the dR cut range till we find a hit
            ##### Probably will reduce the bias that we will have from following true particle trajector
            cFakeHits, seedHits = getModuleRotatedHits(hits = hits, hVL = hitbyVolumeLayer, cells = cells, particles = particles, truth = truth, detector = detectorInfo, nTracks = nFake)
            #plotHits(cFakeHits, seedHits, "Plots/" + args.fakeType + "Hits")
            ####### 6 - Rather than training in the full detector space, identity a ROI in x/y or eta/phi space, and consider all hits from there
            ####### Look at all combinatorical hits from there
            ####### to find the width of the ROI, we will have to compute how much a track will bend for a given pT
            ####### Some optimization ideas, for hits in the strip, make sure that in the up/down layer are close
            ####### Question to answer, how do you create combinatorics for particle that don't traverse the full set of layers
            ####### This idea needs a bit of work
        else:
            print("Fake type not recognized: " + args.fakeType)
            exit(1)

        fakeHits += cFakeHits

        if(args.nEvents > 0):
            if((len(trueHits) + len(fakeHits)) >= args.nEvents):
                break

        pass


    # organize and shuffle the hits

    hitList   = np.array(convertToNumpy(trueHits + fakeHits))
    trueLabel = np.array([1 for hit in trueHits] + [0 for hit in fakeHits])
    trackProb = np.array(computeTrackProb(trueHits) + computeTrackProb(fakeHits))
    indexList = range(len(trueHits) + len(fakeHits))
    random.shuffle(indexList)
    hitList     = hitList[indexList]
    trueLabel   = trueLabel[indexList]
    trackProb   = trackProb[indexList]


    ## Split test and train
    nTrain = int(len(hitList) * args.traintestSplit)

    hitTrain    = hitList[:nTrain]
    labelTrain  = trueLabel[:nTrain]
    probTrain   = trackProb[:nTrain]
    hitTest     = hitList[nTrain:]
    labelTest   = trueLabel[nTrain:]
    probTest    = trackProb[nTrain:]

    ### Save to pickle
    print('writing the pickle')
    os.system("mkdir -vp pickles/" + args.fakeType)
    pFile = open('pickles/' + args.fakeType + "/" + args.outName + '.pkl','wb')

    def _dumpObj(pFile, obj):
        pickle.dump(obj , pFile, protocol = 2)
    _dumpObj(pFile, hitTrain)
    _dumpObj(pFile, labelTrain)
    _dumpObj(pFile, probTrain)
    _dumpObj(pFile, hitTest)
    _dumpObj(pFile, labelTest)
    _dumpObj(pFile, probTest)
    pFile.close()



def computeTrackProb(hitsList):
    probList = []
    for hit in hitsList:
        nHits = len(hit)

        cprobList = hit['particle_id'].value_counts()

        prob = 0
        val = 0
        for cVal in cprobList.index:

            # dont want fakes
            if(cVal == 0): continue

            if(cprobList[cVal] > prob):
                val = cVal
                prob = cprobList[cVal]

        probList.append(prob/float(nHits))

    return probList



def convertToNumpy(inList):
    varList = ["x", "y", "z"]
    # row of zeros
    defaultRow = pd.DataFrame([[int(0)] * len(varList)], columns=varList)

    outList = []
    for hit in inList:
        limHit = hit[varList]
        while(len(limHit) < args.maxTrackLen):
            limHit = limHit.append(defaultRow)
        limHit = limHit.reset_index()
        arr = limHit[varList].to_numpy()
        outList.append(arr)
        # print limHit
    return outList 


def getTrueHits(hits, cells, particles, truth, nTracks = 10):
    hitsList = []
    ## remove these just in case
    hits = hits[hits.particle_id !=0]
    for index, particle in particles.iterrows():
        trackHits = hits[hits["particle_id"] == particle["particle_id"]]
        if(len(trackHits) <= 1): continue
        hitsList.append(cleanHitList(trackHits))

        if(len(hitsList) >= nTracks and nTracks >= 0): break

    return hitsList


# totally random hits in the detector
# but effectively follows the nHit distribution of reconstructed particles
def getFullyRandomHits(hits, cells, particles, truth, nTracks = 10):
    hitList = []
    while len(hitList) < nTracks:
        # get a nHits from a random particle
        part =  particles.sample(n = 1).reset_index()
        nHits = part["nhits"][0]
        if(nHits <= 1): continue

        # sample that many random hits
        randHits = hits.sample(n = nHits)

        hitList.append(cleanHitList(randHits))

    return hitList, None

# totally random hits in the layer
# This function randomly picks a track and find random hits in the layers traversed
#  hVL = hitbyVolumeLayer
def getLayerRandomHits(hits, hVL, cells, particles, truth, nTracks = 10):
    hitList = []
    seedList = []
    while len(hitList) < nTracks:
        # get a nHits from a random particle
        part =  particles.sample(n = 1).reset_index()
        trackHits = hits[hits["particle_id"] == part["particle_id"][0]]
        if(len(trackHits) <= 1): continue

        randHits = None

        # find a hit in the layer traversed by the particle 
        for index, hit in trackHits.iterrows():
            randHit = histSampler(hit["x"], hit["y"], hit["z"], hVL[hit["volume_id"]][hit["layer_id"]])

            if(randHits is None): randHits = randHit
            else: randHits = randHits.append(randHit)


        hitList.append(cleanHitList(randHits))
        seedList.append(cleanHitList(trackHits))

    return hitList, seedList

# totally random hits in the module
# This function randomly picks a track and find random hits in the module traversed
#  hVL = hitbyVolumeLayer
def getModuleRandomHits(hits, hVL, cells, particles, truth, nTracks = 10):
    hitList = []
    seedList = []
    while len(hitList) < nTracks:
        # get a nHits from a random particle
        part =  particles.sample(n = 1).reset_index()
        trackHits = hits[hits["particle_id"] == part["particle_id"][0]]
        if(len(trackHits) <= 1): continue

        randHits = None

        # find a hit in the layer traversed by the particle 
        for index, hit in trackHits.iterrows():
            hitsinLayer = hVL[hit["volume_id"]][hit["layer_id"]]
            randHit = histSampler(hit["x"], hit["y"], hit["z"], hitsinLayer[hitsinLayer["module_id"] == hit["module_id"]])

            if(randHits is None): randHits = randHit
            else: randHits = randHits.append(randHit)


        hitList.append(cleanHitList(randHits))
        seedList.append(cleanHitList(trackHits))

    return hitList, seedList

# 5 - Take a random particle and randomly rotate it in xy space/flip it in z space and then repeat the algo 3/4
# if hits don't statisfy a certain distance cut, just skip that layer
# second option is that we iteratively expand the dR cut range till we find a hit
# Probably will reduce the bias that we will have from following true particle trajector
def getModuleRotatedHits(hits, hVL, cells, particles, truth, detector, nTracks = 10):
    def _findClosestModule(x, y, z, volume_id, layer_id, debug = False):
        moduleList = detector[(detector["volume_id"] == volume_id) & (detector["layer_id"] == layer_id)]
        distance = float('inf') 
        module_id = -1

        for index, module in moduleList.iterrows():
            cdist = math.sqrt((x - module["cx"])**2 + (y - module["cy"])**2 + (z - module["cz"])**2)
            if(cdist < distance):
                distance = cdist
                module_id = module["module_id"]

        return module_id

    hitList = []
    seedHitList = []
    while len(hitList) < nTracks:
        print("Fake done:", len(hitList), "need:", nTracks)
        # get a nHits from a random particle
        part =  particles.sample(n = 1).reset_index()
        trackHits = hits[hits["particle_id"] == part["particle_id"][0]]
        if(len(trackHits) == 0): continue

        randHits = None

        randAngle = random.random() * 2 * math.pi

        # find a hit in the layer traversed by the particle 
        for index, hit in trackHits.iterrows():
            xprime = hit["x"] * math.cos(randAngle) - hit["y"] * math.sin(randAngle)
            yprime = hit["x"] * math.sin(randAngle) + hit["y"] * math.cos(randAngle)

            closestModule = _findClosestModule(xprime, yprime, hit["z"], hit["volume_id"], hit["layer_id"])

            hitsinLayer = hVL[hit["volume_id"]][hit["layer_id"]]
            randHit = histSampler(xprime, yprime, hit["z"], hitsinLayer[hitsinLayer["module_id"] == closestModule])


            if (randHit is None): 
                print("Found an empty hit - continue")
                continue

            # print("rotated x: ", xprime, " y:", yprime)
            # print hit
            # print randHit
            # exit(1)



            if(randHits is None): randHits = randHit
            else: randHits = randHits.append(randHit)


        hitList.append(cleanHitList(randHits))
        seedHitList.append(cleanHitList(trackHits))

    return hitList, seedHitList


def histSampler(oX, oY, oZ, hitList):
    if (len(hitList) == 0): return None

    def _randSampler():
        return hitList.sample(n = 1)

    def __distanceSqr(hit):
        # forget the sqrt to save time
        return (oX - hit["x"])**2 + (oY - hit["y"])**2 + (oZ - hit["z"])**2

    def __decorateDistance():
        with pd.option_context('mode.chained_assignment', None):
            hitList['distanceSqr'] = hitList.apply (lambda row: __distanceSqr (row), axis = 1)
        chitList = hitList.sort_values(by=["distanceSqr"])
        return chitList

    def _closestPoint():
        hitList = __decorateDistance()
        hit = hitList[hitList["distanceSqr"] > 0].head(1)
        return hit

    def _iterativeDistancePoint():
        hitList = __decorateDistance()

        hit = None
        incr = 30
        distance = 30 ## increments of 30mm
        lwrCut = 0 ## increments of 30mm

        # if(args.fakeType == "layerRandom"):


        ###### TODO: This needs to be tuned
        while (distance < 100): 

            if(distance > 50):
                distance = 100
                lwrCut = 0

            chitList = hitList[(hitList["distanceSqr"] > lwrCut) & (hitList["distanceSqr"] < distance*distance)]

            if(len(chitList) == 0): 
                distance += 5
                continue

            return chitList.sample(n = 1)

        # return just the hit itself
        hitList.sample(n = 1)
        # print("Not other hit found for")
        # print oX, oY, oZ
        # print hitList
    if("Iter" in args.fakeType): return _iterativeDistancePoint() 
    return _randSampler()

def cleanHitList(inHitList):
    hitList = inHitList.sort_values(by=["rho"]).reset_index()

    # rotate the hits to zero
    if(args.rotateToZero):
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


def plotHits(hitList, seeds, baseName):
    def _plotHist(xVar, yVar, name):
        for hit in hitList:
            plt.plot(hit[xVar], hit[yVar], 'o-', linewidth=1, markersize=2)

        plt.gca().set_prop_cycle(None)
        for hit in seeds:
            plt.plot(hit[xVar], hit[yVar], '+--', linewidth=1, markersize=4)

        plt.savefig(baseName + '_' + name + '.pdf',  bbox_inches='tight')
        plt.clf()

    _plotHist("x", "y", "xy");
    _plotHist("x", "z", "xz");
    _plotHist("y", "z", "yz");
    _plotHist("z", "rho", "rhoz");


def getFileList(indir, pattern):
    import re
    entries = os.listdir(indir + '/')
    fileList = []
    for entry in entries:
        if('event' not in entry): continue
        if('-' not in entry): continue

        if(not re.match(pattern, entry)): continue

        baseName = entry.split("-")[0]
        fileList.append(indir + "/" + baseName)

    fileList = list(set(fileList))
    fileList.sort()

    return fileList


if __name__ == "__main__":
    start_time = time.time()
    main()
    print("--- %s hours ---" % str((time.time() - start_time)/3600.))




