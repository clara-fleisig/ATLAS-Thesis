import tensorflow as tf
from tensorflow import keras
from tensorflow.keras.layers import Dense
from tensorflow.keras import initializers
import os
import sys
import numpy as np
import pickle
import matplotlib.pyplot as plt
from sklearn.metrics import roc_curve, roc_auc_score
import argparse

parser = argparse.ArgumentParser()
#parser.add_argument("--name",             type = str,      required = True,  help = "name of model version"     ) 
parser.add_argument("--numHidden",        type = int,      default = 1,      help = "number of hidden layers")
parser.add_argument("--numNodes",            type = int,   default = 50,      help = "number of nodes per hidden layer")
parser.add_argument("--learningRate",      type = float,   default = .001,      help="number of nodes per hidden layer")
args = parser.parse_args()

save_dir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN_v3"
load_dir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN_v3"

if not os.path.exists(save_dir):
    os.makedirs(save_dir)

filePath = '/afs/cern.ch/work/s/sabidi/public/ForAlex/H4lMachineLearning/DeepLearning/pickles/testTracking.pkl'
with open(filePath, 'rb') as fp:
    pickleType = pickle.load(fp, encoding='latin1')
    pickleVersion = pickle.load(fp, encoding='latin1')
    x_train = pickle.load(fp, encoding='latin1')
    x_test = pickle.load(fp, encoding='latin1') 
    _ = pickle.load(fp, encoding='latin1') #y_train
    _ = pickle.load(fp, encoding='latin1') #y_test
    _ = pickle.load(fp, encoding='latin1') #weight_train
    _ = pickle.load(fp, encoding='latin1') #weight_test
    y_train = pickle.load(fp, encoding='latin1') #y_train_old
    y_test = pickle.load(fp, encoding='latin1') #y_test_old
    
print("succesfully loaded data")
maxhits = len(x_train[0])
numFeatures = 3

fixed_norm = initializers.glorot_normal(seed=3)
EPOCHS = 600
BATCH = 128
checkpoint_filepath = os.path.join(load_dir, 'l{}n{}_weights.h5'.format(str(args.numHidden), str(args.numNodes)))
model_checkpoint_callback = tf.keras.callbacks.ModelCheckpoint(
     filepath=checkpoint_filepath,
     save_weights_only=True,
     monitor='val_accuracy',
     mode='max',
     save_best_only=True)
model_earlystopping_callback = keras.callbacks.EarlyStopping(monitor='val_loss', patience=50)
     
model = keras.models.Sequential()
#model.add(tf.keras.layers.Masking(mask_value=-999, input_shape=(maxhits, numFeatures)))
loop_range = range(args.numHidden)
for i in loop_range:
    if i < max(loop_range):
        model.add(tf.keras.layers.GRU(units=args.numNodes,
                                      input_shape = (maxhits,3), 
                                      activation='tanh', 
                                      recurrent_activation = 'sigmoid', 
                                      recurrent_dropout = 0, unroll = False, 
                                      use_bias = True,
                                      return_sequences=True))
    else:
        model.add(tf.keras.layers.GRU(units=args.numNodes, 
                                      activation='tanh', 
                                      recurrent_activation = 'sigmoid', 
                                      recurrent_dropout = 0, unroll = False, 
                                      use_bias = True,
                                      return_sequences=False))

model.add(Dense(32, activation='relu'))
model.add(Dense(16, activation='relu'))    
model.add(Dense(1,activation='sigmoid'))

lr = args.learningRate
model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False),
                     optimizer=tf.keras.optimizers.Adam(lr),
                      metrics=['accuracy'])

print(model.summary())
history=model.fit(x_train, y_train, validation_data=(x_test, y_test), 
                  epochs=EPOCHS,batch_size = BATCH, callbacks=[model_checkpoint_callback, model_earlystopping_callback])

model.load_weights(checkpoint_filepath)
model.save(os.path.join(load_dir, 'l{}n{}_model.h5'.format(str(args.numHidden), str(args.numNodes))))
json_string = model.to_json()
jsonName = os.path.join(save_dir, 'l{}n{}.json'.format(str(args.numHidden), str(args.numNodes)))
open(jsonName, 'w').write(json_string)

test_loss, test_acc = model.evaluate(x_test,  y_test, verbose=2)
fig = plt.figure(figsize =(16,8))
plt.suptitle('l{}n{}.h5'.format(str(args.numHidden), str(args.numNodes)))
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

plt.savefig(os.path.join(load_dir,"accuracy_loss_l{}n{}.png".format(str(args.numHidden), str(args.numNodes))))

predictions = model.predict(x_test)
fpr, tpr, _ = roc_curve(y_test, predictions)
roc_auc = roc_auc_score(y_test, predictions)

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
plt.savefig(os.path.join(save_dir,"roc_score_l{}n{}.png".format(str(args.numHidden), str(args.numNodes))))

