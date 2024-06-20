#include "TrackNNEvaluator/NNHitPredictor.h"
#include <cassert>
#include <stdexcept>
#include <array>

// prediction function
Eigen::MatrixXf NNDetectorClassifier::PredictVolumeAndLayer(NetworkBatchInput& inputTensorValues) const 
{
  int totalSize = volumeLength + layerLength;

  const int batchSize = inputTensorValues.rows();
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> outputs(batchSize, totalSize);
  // run the model over the input
  std::map<int, Eigen::MatrixXf> outputTensorValuesMap = runONNXInferenceMultilayerOutput(inputTensorValues);
  // The first layer should be (batch, 15) volume OHE
  // The second layer should be (batch, 30) layer OHE
  for (int i = 0; i < batchSize; i++)
  {
    // cast eigen vectors to std::vectors to use argmax
    // more efficient to write an argmax function to operate on Eigen::Vectors?
    auto volEigenVec = outputTensorValuesMap[1].row(i);
    auto layEigenVec = outputTensorValuesMap[0].row(i);

    Eigen::MatrixXf::Index predVolume;
    Eigen::MatrixXf::Index predLayer;
    volEigenVec.maxCoeff(&predVolume);
    layEigenVec.maxCoeff(&predLayer);

    Eigen::VectorXf onehotencoding = Eigen::VectorXf::Zero(totalSize); 
    onehotencoding[ static_cast<int>(predVolume)] = 1;
    onehotencoding[ static_cast<int>(volumeLength + predLayer)] = 1;
    outputs.row(i) = onehotencoding;
  }
  return outputs;
}

// prediction function
// covnert to eigen functions
Eigen::MatrixXf NNHitPredictor::PredictHitCoordinate(NetworkBatchInput& inputTensorValues) const
{
  // run the model over the input
  std::map<int, Eigen::MatrixXf> outputTensorValuesMap = runONNXInferenceMultilayerOutput(inputTensorValues);

  auto output = outputTensorValuesMap[0];//Outputs for this model should have only one output layer
  output.col(0) *= getXScale();
  output.col(1) *= getYScale();
  output.col(2) *= getZScale();
  
  return output;
}