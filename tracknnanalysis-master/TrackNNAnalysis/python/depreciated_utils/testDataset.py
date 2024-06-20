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
matplotlib.use('TkAgg')   
import matplotlib.pyplot as plt  
from mpl_toolkits.mplot3d import Axes3D

from trackml.dataset import load_event
from trackml.randomize import shuffle_hits
from trackml.score import score_event
from trackml.utils import add_position_quantities, add_momentum_quantities,decode_particle_id

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--inName",        type = str,   required = True,      help = "Outname of the pickle")
args = parser.parse_args()



def main():
    f = open(args.inName, 'rb')
    print("here")
    hitTrain    = pickle.load(f)
    labelTrain  = pickle.load(f)
    hitTest     = pickle.load(f)
    labelTest   = pickle.load(f)


    print hitTrain[0]

if __name__ == "__main__":
    start_time = time.time()
    main()
    print("--- %s hours ---" % str((time.time() - start_time)/3600.))


