from qkeras import QDense, QActivation
import tensorflow as tf
import os
import numpy as np

model = tf.keras.models.load_model('trained_models/qKerasTest')

print(model.summary())

X = np.array([2.92328e-09, -0.000177901, -0.00119654, -0.000949157, -0.00210646, 0.290793, 0.385185, 0.545564, 0.55193, 0.747178, 0.0382175, 0.0447846, 0.052826, 0.0532978, 0.0736164])
X = np.array([-1.40211e-08,-0.000230155,-0.00127055,-0.00102403,-0.00220782,0.282919,0.385185,0.545563,0.55193,0.747177,0.0376925,0.0447846,0.052826,0.0532978,0.0736164])
print(X)

print(model.predict(X.reshape(1,15,1)))