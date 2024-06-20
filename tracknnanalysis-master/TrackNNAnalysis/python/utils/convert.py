from qkeras.utils import load_qmodel
import onnxmltools

fileList = [
"DNN_ACTS_Region_0p1_0p3_HTTFake_HTTTrue_Nom_v1",
]


for fileName in fileList:
    inputName = "TrainedNN/dNN/"+fileName+"/qDNN.h5"
    outputName = "TrainedNN/dNN/"+fileName+".onnx"

    keras_model = load_qmodel(inputName)
    onnx_model = onnxmltools.convert_keras(keras_model)
    onnxmltools.utils.save_model(onnx_model, outputName)


    weightName = 'TrainedNN/dNN/'+fileName+'.h5'
    keras_model.save_weights(weightName, overwrite=True)

    json_string = keras_model.to_json()
    jsonName = 'TrainedNN/dNN/'+fileName+'.json'
    open(jsonName, 'w').write(json_string)

