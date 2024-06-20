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

maxhits = 40
#save_dir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN"
load_dir = "/afs/cern.ch/user/a/agekow/trackml/training/RNN"
#file_name = "mixed_data.pkl"
#file_path = os.path.join(load_dir, file_name)
filePath = '/afs/cern.ch/work/s/sabidi/public/ForAlex/TrackML/pickles'
data_folders  = os.listdir(filePath)
data_folders.remove('v1')
for dataType in data_folders:
    print(dataType)
    x_data_train = None
    y_data_train = None
    x_data_test  = None
    y_data_test  = None
    dataSets = os.listdir(os.path.join(filePath, dataType))
    
    for dataSet in dataSets:
        data = os.path.join(filePath,dataType, dataSet)
        
        with open(data,"rb") as fp:
            x_train = pickle.load(fp, encoding = 'latin1')
            y_train = pickle.load(fp, encoding = 'latin1')
            p_train = pickle.load(fp, encoding = 'latin1')
            x_test = pickle.load(fp, encoding = 'latin1')
            y_test = pickle.load(fp, encoding = 'latin1')
            p_test = pickle.load(fp, encoding ='latin1')

        if np.any(x_data_train) == None:
            x_data_train = x_train
            y_data_train = y_train
            x_data_test  = x_test
            y_data_test  = y_test

        else:
            x_data_train = np.append(x_data_train, x_train, axis=0)
            y_data_train = np.append(y_data_train, y_train, axis=0)
            x_data_test = np.append(x_data_test, x_test, axis=0)
            y_data_test = np.append(y_data_test, y_test, axis=0)

    fixed_norm = initializers.glorot_normal(seed=3)
    EPOCHS = 1000
    BATCH = 128
    PATH = os.path.join(load_dir, "{}_weights.hdf5".format(dataType))
    checkpoint_filepath = PATH
    model_checkpoint_callback = tf.keras.callbacks.ModelCheckpoint(
         filepath=checkpoint_filepath,
         save_weights_only=True,
         monitor='val_accuracy',
         mode='max',
         save_best_only=True)
    
    model = Sequential([
        tf.keras.layers.Masking(mask_value=0, input_shape=(maxhits, 3)),
        tf.keras.layers.LSTM(units=64,input_shape = (maxhits,3), activation='tanh', recurrent_activation = 'sigmoid', recurrent_dropout = 0, unroll = False, use_bias = True),
        tf.keras.layers.Dense(64, activation='relu'),
        tf.keras.layers.Dense(1, activation = 'sigmoid')
    ])
    
    
    model.compile(loss=tf.keras.losses.BinaryCrossentropy(from_logits=False), 
                  optimizer=tf.keras.optimizers.Adam(1e-4),
                  metrics=['accuracy'])
    
    history=model.fit(x_data_train, y_data_train, validation_data=(x_data_test, y_data_test), 
                      epochs=EPOCHS,batch_size = BATCH, callbacks=[model_checkpoint_callback])
    
    model.load_weights(checkpoint_filepath)
    model.save(os.path.join(load_dir, '{}_trained.h5'.format(dataType)))
    
    test_loss, test_acc = model.evaluate(x_data_test,  y_data_test, verbose=2)
    fig = plt.figure(figsize =(16,8))
    plt.suptitle('trained on {}'.format(dataType))
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
    
    plt.savefig(os.path.join(load_dir,"accuracy_loss_{}.png".format(dataType)))
    
    predictions = model.predict(x_data_test)
    fpr, tpr, _ = roc_curve(y_data_test, predictions)
    roc_auc = roc_auc_score(y_data_test, predictions)
    
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
    plt.savefig(os.path.join(load_dir,"roc_score_{}.png".format(dataType)))

