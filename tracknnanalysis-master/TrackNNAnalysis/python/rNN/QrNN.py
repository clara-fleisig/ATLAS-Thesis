




import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, Activation, BatchNormalization
from tensorflow.keras import regularizers, initializers
from tensorflow.keras import optimizers
from tensorflow.keras import metrics
import os
import sys
import numpy as np
import pickle
import matplotlib.pyplot as plt
from sklearn.metrics import roc_curve, roc_auc_score
import qkeras
import argparse

sys.path.append("/afs/cern.ch/user/a/agekow/TrackNNAnalysis/source/TrackNNAnalysis/python/utils")
from NetworkEvaluationPlots import plotAccLoss, plotROC

parser = argparse.ArgumentParser()
parser.add_argument("--nodes", type=int, default=10, help="number of nodes per GRU layer")
parser.add_argument("--bits", type=int, default=8, help="number of bits in quantized network components")
parser.add_argument("--lr", type=float, default = .0001, help="learning rate")
args = parser.parse_args()

saveDir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN_Quantized"
if not os.path.exists(saveDir):
    os.makedirs(saveDir)

data = '/afs/cern.ch/work/s/sabidi/public/ForAlex/H4lMachineLearning/DeepLearning/pickles/1event_ttbar_scaled_MRIR.pkl'
#data = "testTracking.pkl"
with open(data,"rb") as fp:
    title = pickle.load(fp, encoding = 'latin1')
    version = pickle.load(fp, encoding = 'latin1')
    x_train = pickle.load(fp, encoding = 'latin1')
    x_test = pickle.load(fp, encoding = 'latin1')
    y_train = pickle.load(fp, encoding = 'latin1')
    y_test = pickle.load(fp, encoding = 'latin1')
    w_train = pickle.load(fp, encoding = 'latin1')
    w_test = pickle.load(fp, encoding ='latin1')
    y_train_old = pickle.load(fp, encoding = 'latin1')
    y_test_old = pickle.load(fp, encoding = 'latin1')
    #track_label_train = pickle.load(fp, encoding = 'latin1')
    #track_label_test = pickle.load(fp, encoding = 'latin1')

maxhits = len(x_train[0])
n_features = 3

def createQModel(nodes, bits):
    
    inputs = tf.keras.layers.Input(shape=(maxhits,n_features))
    masking = tf.keras.layers.Masking(mask_value=-999, input_shape=(maxhits,n_features))(inputs)
    x = qkeras.QGRU(
        nodes,
        return_sequences=True,
        activation='quantized_relu({},0,1)'.format(bits),
        recurrent_activation='quantized_tanh({})'.format(bits),
        kernel_quantizer='stochastic_ternary("auto")',
        recurrent_quantizer='quantized_bits(2,1,1,alpha=1.0)',
        bias_quantizer='quantized_bits({},0,1)'.format(bits))(masking)
    
    x = qkeras.QGRU(
        nodes,
        activation='quantized_relu({},0,1)'.format(bits),
        recurrent_activation='quantized_tanh({})'.format(bits),
        kernel_quantizer='stochastic_ternary("auto")',
        recurrent_quantizer='quantized_bits(2,1,1,alpha=1.0)',
        bias_quantizer='quantized_bits({},0,1)'.format(bits))(x)
    
    x = qkeras.QDense(
        1, 
        kernel_quantizer="quantized_bits({},0,1)".format(bits),
        bias_quantizer='quantized_bits({},0,1)'.format(bits))(x)
    
    output = qkeras.QActivation('sigmoid')(x)
    
    model = tf.keras.Model(inputs=[inputs], outputs=[output])
        
    return model

model = createQModel(args.nodes,args.bits)
history = fitModel(model, epochs=100, batch_size = 500, learning_rate=args.lr)

plotAccLoss(history, x_test, y_test)
plt.savefig(os.path.join(saveDir, "l2n{}Q{}_AccLoss.png".format(args.nodes, args.bits)))

plotROC(model, x_test, y_test)
plt.savefig(os.path.join(saveDir, "l2n{}Q{}_ROC.png".format(args.nodes, args.bits)))
