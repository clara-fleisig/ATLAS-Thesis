import matplotlib.pyplot as plt
import tensorflow as tf
from sklearn.metrics import roc_curve, roc_auc_score
import numpy as np
def plotAccLoss(history, epochs):

	fig = plt.figure(figsize =(16,8))
	plt.suptitle('Accuracy & Loss')
	plt.subplot(1,2,1)
	plt.plot(history.history['accuracy'], label='accuracy')
	plt.plot(history.history['val_accuracy'], label = 'val_accuracy')
	plt.xlabel('Epoch')
	plt.ylabel('Accuracy')
	plt.ylim([0, 1])
	plt.xlim([0, epochs])
	plt.yticks(np.arange(0,1,.05))
	plt.legend(loc='lower right')

	plt.subplot(1,2,2)
	plt.plot(history.history['loss'], label='loss')
	plt.plot(history.history['val_loss'], label = 'val_loss')
	plt.xlabel('Epoch')
	plt.ylabel('loss')
	plt.legend(loc='upper right')

def plotROC(model, x_test, y_test):
	predictions = model.predict(x_test)
	predictions = tf.math.sigmoid(predictions)
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

