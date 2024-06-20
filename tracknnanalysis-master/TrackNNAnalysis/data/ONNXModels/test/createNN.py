from qkeras import QDense, QActivation
import tensorflow as tf
import os
import numpy as np

def qmlp(window_size=3, n_features=3, bits=8):

    input_layer = tf.keras.Input(shape=(15,))

    x = input_layer
    x = QDense(64,
               kernel_quantizer = 'quantized_bits({},0,alpha=1)'.format(bits),
               bias_quantizer = 'quantized_bits({},0,alpha=1)'.format(bits))(x)
    x = QActivation('quantized_relu({},0)'.format(bits))(x)
    x = QDense(32,
               kernel_quantizer = 'quantized_bits({},0,alpha=1)'.format(bits),
               bias_quantizer = 'quantized_bits({},0,alpha=1)'.format(bits))(x)
    x = QActivation('quantized_relu({},0)'.format(bits))(x)
    x = QDense(1,
               kernel_quantizer = 'quantized_bits({},0,alpha=1)'.format(bits),
               bias_quantizer = 'quantized_bits({},0,alpha=1)'.format(bits))(x)

    output = tf.keras.layers.Activation('tanh')(x)

    model = tf.keras.Model(inputs=[input_layer], outputs=[output])

    return model

model = qmlp()
model.compile(loss = tf.keras.losses.BinaryCrossentropy(from_logits=False), optimizer = tf.keras.optimizers.Adam(learning_rate=0.01), metrics=['accuracy'])

X = np.array([2.92328e-09, -0.000177901, -0.00119654, -0.000949157, -0.00210646, 0.290793, 0.385185, 0.545564, 0.55193, 0.747178, 0.0382175, 0.0447846, 0.052826, 0.0532978, 0.0736164])
print(X)

print(model.predict(X.reshape(1,15,1)))

tf.keras.models.save_model(qmlp(), 'trained_models/qKerasTest')

os.system("python3 -m tf2onnx.convert --saved-model trained_models/qKerasTest/" + " --output qKerasTest.onnx")