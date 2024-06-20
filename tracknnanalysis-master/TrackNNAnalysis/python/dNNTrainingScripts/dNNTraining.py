import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import initializers
import os
import sys
import numpy as np
import pickle
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('Agg')
from sklearn.metrics import roc_curve, roc_auc_score
#import qkeras
import argparse
from tensorflow.keras.layers import Dense, BatchNormalization
from tensorflow.keras import initializers
#from qkeras import QDense, QBatchNormalization, QActivation
from sklearn.utils import shuffle
from os.path import join

#maybe remove this and alter data paths in bash scripts
dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(dir_path)
#sys.path.append('/afs/cern.ch/user/a/agekow/trackml/nnScripts/')
from NetworkEvaluationPlots import plotAccLoss
from dataFuncs import *

parser = argparse.ArgumentParser()
parser.add_argument("--epochs", type=int, default=200, help="number of epochs to train for")
parser.add_argument("--nodes", type=int, default=64, help="number of nodes per hidden layer")
parser.add_argument("--batch", type=int, default=2048, help="batch size to trian with")
parser.add_argument("--learning_rate", type=float, default=.001, help="learning rate")
parser.add_argument("--split", default=False, action='store_true', help="split train data 50/50 true/fake")
parser.add_argument("--combined", default=False, action='store_true', help="whether or not to combine multiple datsets")
parser.add_argument("-d", "--dataFiles", nargs="+", default=None, help="List of files to combine if training over combined datasets")
parser.add_argument("--data", type=str, default=None, help="file name of pickle data")

args = parser.parse_args()
if args.combined and args.dataFiles is None:
    parser.error("--combined requires --dataFiles as a list")
elif not args.combined and args.data is None:
    parser.error("input data via --data flag is required")

if args.combined:
    combined = "_combined"
    import re
    # CHANGE if Names of DataFiles Change
    dirName = re.sub(r'^.*?HTTFake_', '', args.dataFiles[0])
    saveDir = '/eos/user/a/agekow/tracking/TrainedNN/dNN/' + 'combined' + dirName
else:
    combined = ""
    saveDir = '/eos/user/a/agekow/tracking/TrainedNN/dNN/' + args.data

if not os.path.exists(saveDir):
    os.makedirs(saveDir)


filePath = '/eos/user/a/agekow/tracking/data/trainingPkl/'
if (not args.combined) & (not args.split):
    combined = ""
    split = ""
    dat = join(filePath, args.data)
    x_train, x_test, y_train, y_test = loadData(dat)

elif (not args.combined) & (args.split):
        combined = ""
        split = "_split"
        dat = join(filePath, args.data)
        x_train, x_test, y_train, y_test = loadData(dat)
        

elif (args.combined) & (not args.split):
    combined = "_combined"
    split = ""
    x_train, x_test, y_train, y_test = combineData(split = args.split, dataFiles = args.dataFiles)

# Need to split data before combining to ensure fake tracks from each region
elif (args.combined) & (args.split):
    combined = "_combined"
    split = "_split"
    x_train, x_test, y_train, y_test = combineData(split = args.split, dataFiles = args.dataFiles)

maxhits = len(x_train[0])

def createModel(nodes):

    input_layer = tf.keras.Input(shape=(maxhits,))

    x = Dense(nodes, activation='relu')(input_layer)
    x = tf.keras.layers.Dropout(.2)(x)
    x = tf.keras.layers.BatchNormalization()(x)
    x = Dense(int(nodes/2), activation='relu')(x)
    x = tf.keras.layers.Dropout(.2)(x)
    x = tf.keras.layers.BatchNormalization()(x)
    output = Dense(1, activation='sigmoid')(x)

    model = tf.keras.Model(inputs=[input_layer], outputs=[output])
    
    return model

if args.split == True:
    callbackPath = join(saveDir, 'DNN_{}{}.h5'.format(args.nodes, split))
else:
    callbackPath = join(saveDir, 'DNN_{}{}.h5'.format(args.nodes, split))

# Check if model already exists and ask user to overwrite if so
# add later. Need input with timeout for cases where it is run on htcondor


model_checkpoint_callback = [tf.keras.callbacks.ModelCheckpoint(
        callbackPath, monitor='val_loss', save_best_only=True), tf.keras.callbacks.EarlyStopping(
            monitor='val_loss', patience=60)]

# ask to overwrite model if it exists. Overwrite after 10 seconds of no input
if os.path.exists(callbackPath):
    import signal
    TIMEOUT = 10 # number of seconds your want for timeout

    def interrupted(signum, frame):
        "called when read times out"
        print('interrupted!')
        raise Exception("Timeout")
    signal.signal(signal.SIGALRM, interrupted)

    def rawinput():
        try:
                print('\n')
                print('You have', TIMEOUT,  'seconds before model is trained and overwrites model saved at:', callbackPath )
                print('y/n')
                foo = input()
                return foo
        except Exception:
                print('\n')
                print("Training will commence and model at", callbackPath,  "will be overwritten")
                return

    # set alarm
    yes = {'Y', 'y', 'yes', 'Yes'}
    no = {'N', 'n', 'no', 'No'}
    signal.alarm(TIMEOUT)
    s = rawinput()
    if s in yes:
        print("Training will commence and model at", callbackPath,  "will be overwritten")
    if s in no:
        print('Aborting Training')
        sys.exit()
    # disable the alarm after success
    signal.alarm(0)
##### End of OverWrite Check ######

model = createModel(args.nodes)
model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False),
		                 optimizer=tf.keras.optimizers.Adam(args.learning_rate),
		                  metrics=['accuracy'])


history=model.fit(x_train, y_train, validation_data=(x_test, y_test), 
		              epochs=args.epochs,batch_size = args.batch, callbacks = model_checkpoint_callback)

model = keras.models.load_model(callbackPath)

plotAccLoss(history)
plt.savefig(os.path.join(saveDir,'DNN_AccLoss_{}{}{}.png'.format(args.nodes, combined, split)), dpi=200)                                                                                                           
plt.close() 