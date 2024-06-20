// Class includes
#include "TrackNNEvaluator/NNEvalUtils.h"

// Local includes
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <math.h>
#include <string.h>

using namespace std;


// Constructors and destructor
// NNEvalUtils::NNEvalUtils(TString fileName, int hits, std::string trackType="Nominal", std::string order="XX0YY0ZZ0", bool scaled=false) // XXYYZZ000
NNEvalUtils::NNEvalUtils(TString fileName, int hits, std::string trackType, std::string order, bool scaled) // XXYYZZ000
{
    m_fileName = fileName;
    n_hits = hits;
    m_trackType = trackType;
    m_order = order;
    m_scaled = scaled;
}

NNEvalUtils::~NNEvalUtils()
{

} 


// big setters
void NNEvalUtils::initialize()
{
    //load the onnx model to memory using the path m_path_to_onnx
    m_env = std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_WARNING, "");

    // initialize session options if needed
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // create session and load model into memory
    m_session = std::make_unique< Ort::Session >(*m_env, m_fileName.Data(), session_options);
    Ort::AllocatorWithDefaultOptions allocator;

    // std::cout << m_session->GetModelMetadata() << std::endl;
    // get the input nodes
    size_t num_input_nodes = m_session->GetInputCount();
    

    m_totalInputs = 1;

    // iterate over all input nodes
    for (std::size_t i = 0; i < num_input_nodes; i++) 
    {
        auto name = m_session->GetInputNameAllocated(i, allocator);

        char* input_name = new char[strlen(name.get()) + 1];
        strcpy(input_name, name.get());
        m_input_node_names.push_back(input_name);
        cout<<"Input node: i "<<i <<" "<<input_name<<endl;

        // print input node types
        Ort::TypeInfo type_info = m_session->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        ONNXTensorElementDataType type = tensor_info.GetElementType();
        cout<<"Input node: i "<<i <<" type = "<<type<<endl;

        // print input shapes/dims
        m_input_node_dims = tensor_info.GetShape();
        cout<<"Input "<<i<<" : num_dims= "<<m_input_node_dims.size()<<endl;
        for (std::size_t j = 0; j < m_input_node_dims.size(); j++)
        {
          if(m_input_node_dims[j]<0) m_input_node_dims[j] =1;
          cout<<"Input "<<i<<" : dim "<<j<<"= "<<m_input_node_dims[j]<<endl;
          m_totalInputs *= m_input_node_dims[j];
        }  
    }

    cout<<"Total input size "<<m_totalInputs<<endl;


    // get the output nodes
    size_t num_output_nodes = m_session->GetOutputCount();
    std::vector<int64_t> output_node_dims;


    // iterate over all output nodes
    for(std::size_t i = 0; i < num_output_nodes; i++) 
    {
        auto name = m_session->GetOutputNameAllocated(i, allocator);
        char* output_name = new char[strlen(name.get()) + 1];
        strcpy(output_name, name.get());
        m_output_node_names.push_back(output_name);
        cout<<"output node: i "<<i <<" "<<output_name<<endl;

        // get output node types
        Ort::TypeInfo type_info = m_session->GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

        ONNXTensorElementDataType type = tensor_info.GetElementType();
        cout<<"Output node: i "<<i <<" type = "<<type<<endl;

        output_node_dims = tensor_info.GetShape();
        cout<<"Output node: i "<<i<<" : num_dims= "<<output_node_dims.size()<<endl;

        for (std::size_t j = 0; j < output_node_dims.size(); j++)
        {
          if(output_node_dims[j]<0) output_node_dims[j] =1;
          cout<<"Output "<<i<<" : dim "<<j<<"= "<<output_node_dims[j]<<endl;
        } 

        // output is of the shape {1, num_jets, num_wp}
        // m_num_wp = output_node_dims.at(2);
    }
}

float NNEvalUtils::eval(vector<float>& input_tensor_value)
{
    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_value.data(), input_tensor_value.size(), m_input_node_dims.data(), m_input_node_dims.size());
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());


    // Get pointer to output tensor float values
    float* output_score_array = output_tensors.front().GetTensorMutableData<float>();

    // Binary classification - the score is just the first element of the output tensor
    float output_score = output_score_array[0];

    return output_score;
}

void NNEvalUtils::evalTrack(std::shared_ptr<RecoTrack> trk)
{
    evalTrack(dynamic_pointer_cast<TrackBase, RecoTrack>(trk));
}
void NNEvalUtils::evalTrack(std::shared_ptr<FakeTrack> trk)
{
    evalTrack(dynamic_pointer_cast<TrackBase, FakeTrack>(trk));
}

void NNEvalUtils::evalTrack(std::shared_ptr<ExtrapolatedTrack> trk)
{
    evalTrack(dynamic_pointer_cast<TrackBase, ExtrapolatedTrack>(trk));
}

void NNEvalUtils::evalTrack(std::shared_ptr<TrackBase> trk)
{
    bool debug = false;
    // if(trk->getTrackProbability() > 0.95) debug = true;
    // if(debug) trk->printDebug();
    float NNVal;
    if(m_trackType == "Nominal")
    {
      NNVal = eval(trk->getHitsList(), debug);
    }
    else if(m_trackType == "RotatedToZero")
    {
      NNVal = eval(trk->getHitsRotatedToZeroList(), debug);
    }
    else if(m_trackType == "RoughRotatedToZero")
    {
      NNVal = eval(trk->getHitsRoughRotatedToZeroList(), debug);
    }
    else if(m_trackType == "SP")
    {    
      NNVal = eval(trk->getSPHitsList(), debug);
    }
    else if(m_trackType == "SPWithRotatedToZero")
    {    
      NNVal = eval(trk->getSPHitsRotatedToZeroList(), debug);
    }
    else if(m_trackType == "SPWithRoughRotatedToZero")
    {    
      NNVal = eval(trk->getSPHitsRoughRotatedToZeroList(), debug);
    }
    else if(m_trackType == "SPWithUnscaledRotatedToZero")
    {    
      NNVal = eval(trk->getSPHitsUnscaledRotatedToZeroList(), debug);
    }
    else
    {
       std::cout<<"Cannot recognize the hit type '"<<m_trackType<<"'"<<endl;
       exit(1);
    }
      
    trk->setNNScore(NNVal);
}

float NNEvalUtils::eval(std::vector<std::shared_ptr<Hit>> hitList, bool debug)
{
    vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    if(!m_scaled)
    {
      xScale = 1;
      yScale = 1;
      zScale = 1;
    }

    int nHits = n_hits;
    for (int i = 0; i < nHits; i++)
    {
        if(i >= (int) hitList.size()) continue;
        if(i + 2*nHits >= m_totalInputs) continue;

        if(m_order == "XXYYZZ000")
        {
          input_tensor_value[i + 0]                = hitList.at(i)->getX() / xScale;
          input_tensor_value[i + hitList.size()]   = hitList.at(i)->getY() / yScale;
          input_tensor_value[i + 2*hitList.size()] = hitList.at(i)->getZ() / zScale;
        } 
        else if(m_order == "XX0YY0ZZ0")
        {
          input_tensor_value[i +  0] = hitList.at(i)->getX() / xScale;
          input_tensor_value[i + nHits] = hitList.at(i)->getY() / yScale;
          input_tensor_value[i + 2*nHits] = hitList.at(i)->getZ() / zScale;
        }
    }
    float val = eval(input_tensor_value);
    // float val = eval(input);
    if(debug)
    {

        for(size_t i = 0; i < input_tensor_value.size(); i++)
        {
            std::cout<<input_tensor_value.at(i)<<" ";
        }
        // for(int i = 0; i < input.size(); i++)
        // {
        //     std::cout<<input.at(i)<<" ";
        // }
        std::cout<<std::endl;

        std::cout<<"eval: "<<val<<endl;
    }
    return val;
}



