
#include "TrackNNEvaluator/OnnxRuntimeBase.h"

#include <vector>
#include <map>
#include <iterator>
#include <iostream>


// Specialized class that labels tracks as good/duplicate/fake using a
// deep neural network.
class NNDetectorClassifier : public OnnxRuntimeBase {
  using OnnxRuntimeBase::OnnxRuntimeBase;

 public:
  /// @brief Predict the next-hit volume and layer id
  ///
  /// @param inputTensorValues The vector of input features comprising of n hits x,y,z coordinates 
  /// normalized such that x,y,z <=|1|
  /// @return One-hot-encoded volume and layer ID 
  Eigen::MatrixXf PredictVolumeAndLayer(NetworkBatchInput&  inputTensorValues) const;

  // Argmax function to convert predictions to one-hot-encoding
  template <typename T, typename A>
  float arg_max(std::vector<T,A> vec) const {
    return static_cast<float>(std::distance(vec.begin(), max_element(vec.begin(), vec.end())));
  }

  void setVolumeSize(int size) { volumeLength = size; };
  void setLayerSize(int size) { layerLength = size; };

 private:
  float xScale = 1005;
  float yScale = 1005;
  float zScale = 3500;

  int volumeLength = 15;
  int layerLength = 30;

 public:
  constexpr float getXScale() const {return xScale;};
  constexpr float getYScale() const {return yScale;};
  constexpr float getZScale() const {return zScale;};
};

class NNHitPredictor : public OnnxRuntimeBase {
  using OnnxRuntimeBase::OnnxRuntimeBase;

 public:
  /// @brief Predict the next-hit volume and layer id
  ///
  /// @param inputTensorValues The vector of input features comprising of n hits x,y,z coordinates 
  /// normalized such that x,y,z <=|1| as well as the one-hot-encoded volume and layer id prediction
  /// @return The predicted next hit coordinate x,y,z <=|1|
  Eigen::MatrixXf PredictHitCoordinate(NetworkBatchInput& inputTensorValues) const;

 private:
  float xScale = 1005;
  float yScale = 1005;
  float zScale = 3500;

 public:
  constexpr float getXScale() const {return xScale;};
  constexpr float getYScale() const {return yScale;};
  constexpr float getZScale() const {return zScale;};

};
