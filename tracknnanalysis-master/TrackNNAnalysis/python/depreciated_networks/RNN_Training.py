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
import math
from sklearn.preprocessing import normalize
from sklearn.preprocessing import scale
from sklearn.metrics import roc_curve, roc_auc_score
import random

maxhits = 40
numFeatures = 3

save_dir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN"
load_dir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN"
file_name = "data.txt"
file_path = os.path.join(load_dir, file_name)
with open(file_path,"rb") as fp:
    data = pickle.load(fp)
    
x_data = data[0]
y_data = data[1]
frac_train = round(len(x_data) * .8)
c = list(zip(x_data, y_data))
random.shuffle(c)
X, Y = zip(*c)

X = np.array(X)
Y = np.array(Y)

x_train = X[0:frac_train]
y_train = Y[0:frac_train]

x_val = X[frac_train:]
y_val = Y[frac_train:]

fixed_norm = initializers.glorot_normal(seed=3)
EPOCHS = 500
BATCH = 128
PATH = os.path.join(load_dir, "weights.hdf5")
checkpoint_filepath = PATH
model_checkpoint_callback = tf.keras.callbacks.ModelCheckpoint(
     filepath=checkpoint_filepath,
     save_weights_only=True,
     monitor='val_accuracy',
     mode='max',
     save_best_only=True)

model = Sequential([
    tf.keras.layers.Masking(mask_value=0, input_shape=(maxhits, numFeatures)),
    tf.keras.layers.LSTM(units=64,input_shape = (maxhits,numFeatures), activation='tanh', recurrent_activation = 'sigmoid', recurrent_dropout = 0, unroll = False, use_bias = True),
    tf.keras.layers.Dense(64, activation='relu'),
    tf.keras.layers.Dense(1, activation = 'sigmoid')
])


model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False), 
              optimizer=tf.keras.optimizers.Adam(1e-4),
              metrics=['accuracy'])

history=model.fit(x_train, y_train, validation_data=(x_val, y_val), 
                  epochs=EPOCHS,batch_size = BATCH, callbacks=[model_checkpoint_callback])


model.load_weights(checkpoint_filepath)

test_loss, test_acc = model.evaluate(x_val,  y_val, verbose=2)
fig = plt.figure(figsize =(16,8))
plt.suptitle('track finding 80/20')
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

predictions = model.predict(x_val)
predictions = tf.math.sigmoid(predictions)
fpr, tpr, _ = roc_curve(y_val, predictions)
roc_auc = roc_auc_score(y_val, predictions)

plt.figure(figsize=(10,9))
plt.clf()
ax = plt.axes()
ax.set_aspect('equal')
ax.set_xticks(np.arange(0, 1., 0.1))

plt.plot(tpr,1-fpr,color='darkorange',label='ROC(DNN_test)(area = %0.3f)' % roc_auc)
plt.plot([1,0],[0,1],linestyle='--',color="#C0C0C0",label="Random")
plt.xlabel('TPR (Sensitivity)', fontsize=16)
plt.ylabel('TNR (Specificity)', fontsize=16)
plt.title('ROC Curve', fontsize=16)
plt.axis([0, 1, 0, 1])
plt.legend(loc='lower right', fontsize=14)
plt.grid(True)
plt.savefig(os.path.join(save_dir,"roc_score.png"))

