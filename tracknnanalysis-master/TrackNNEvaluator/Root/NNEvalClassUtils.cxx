// Class includes
#include "TrackNNEvaluator/NNEvalClassUtils.h"

// Local includes
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <math.h>

using namespace std;


// Constructors and destructor
NNEvalClassUtils::NNEvalClassUtils(TString fileName, int hits)
{
    m_fileName = fileName;
    n_hits = hits;
}

NNEvalClassUtils::~NNEvalClassUtils()
{

} 


// big setters
void NNEvalClassUtils::initialize()
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

    // get the input nodes
    size_t num_input_nodes = m_session->GetInputCount();
    

    //m_totalInputs = 48; //3 hits, x,y,z, volume and layer id one-hot-encoded
    //m_totalInputs = 48*3;
    m_totalInputs = 9;
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
          // *= m_input_node_dims[j];
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

std::vector<int> NNEvalClassUtils::ArgMaxDetectorEncoding(float* predVolumeID, float* predLayerID)
{
    //find the max value of predicted volume and layer
    float max=0;
    int volIdx=-1;
    for (size_t i=0; i<15;i++)
    {
        if (predVolumeID[i]>max)
        {
            max = predVolumeID[i];
            volIdx = i;
        }
    }
    max = 0;
    int layerIdx = -1;
    for (size_t i=0; i<30;i++)
    {
        if (predLayerID[i]>max)
        {
            max = predLayerID[i];
            layerIdx = i;
        }
    }
    std::vector<int> outputs(45,0);
    outputs[volIdx]=1;
    outputs[layerIdx+15]=1;

    return outputs;
}

std::vector<std::vector<int>> NNEvalClassUtils::ArgMaxDetectorEncoding(float* predVolumeID, float* predLayerID, float threshold)
{
    //find the values of predicted volume and layer over a given threshold. Create an individual OHE for each
    // There may be correltations between volume and layer predictions we can exploit. For now do every combination
    // full study of classification errors can help reduce combinatorics
    std::vector<std::vector<int>> outputs;
    std::vector<float> volIndices;
    for (size_t i=0; i<15;i++)
    {
        if (predVolumeID[i]>threshold)
        {
            volIndices.push_back(i);
        }
    }
    std::vector<float> layerIndices;
    for (size_t i=0; i<30;i++)
    {
        if (predLayerID[i]>threshold)
        {
            layerIndices.push_back(i);
        }
    }
    for (auto& v: volIndices)
    {
        for(auto&l: layerIndices)
        {
            std::vector<int> output(45,0);
            output[v]=1;
            output[l+15]=1;
            outputs.push_back(output);
        }
    }
    
    return outputs;
}


std::vector<int> NNEvalClassUtils::classify(std::vector<float>& input_tensor_value)
{
    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_value.data(), input_tensor_value.size(), m_input_node_dims.data(), m_input_node_dims.size());
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());

    // Get pointer to output tensor float values
    float* predLayerID = output_tensors.front().GetTensorMutableData<float>();
    float* predVolumeID =  output_tensors.back().GetTensorMutableData<float>();
    
    auto outputs = ArgMaxDetectorEncoding(predVolumeID, predLayerID);
    return outputs;
}

std::vector<std::vector<int>> NNEvalClassUtils::classify(std::vector<float>& input_tensor_value, float threshold)
{
    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_value.data(), input_tensor_value.size(), m_input_node_dims.data(), m_input_node_dims.size());
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());

    // Get pointer to output tensor float values
    float* predLayerID = output_tensors.front().GetTensorMutableData<float>();
    float* predVolumeID =  output_tensors.back().GetTensorMutableData<float>();
    
    auto outputs = ArgMaxDetectorEncoding(predVolumeID, predLayerID, threshold);
    return outputs;
}



std::vector<int>  NNEvalClassUtils::classify(std::vector<std::shared_ptr<Hit>> hitList, bool debug)
{
    std::vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    unsigned int nHits = hitList.size();
    int index = 0;
    for (size_t i = nHits - n_hits; i < nHits; i++)
    {
        if(i >= hitList.size()) continue;
        // // if ordering is x0, x1, x2, y0, y1, y2, ...
        // input_tensor_value[i + 0]        = hitList.at(i)->getX() / xScale;
        // input_tensor_value[i + n_hits]   = hitList.at(i)->getY() / yScale;
        // input_tensor_value[i + 2*n_hits] = hitList.at(i)->getZ() / zScale;

        // if ordering is x0, y0, z0, x1, y1, z1, ...
        input_tensor_value[index * nFeatures + 0] = hitList.at(i)->getX()/xScale;
        input_tensor_value[index * nFeatures + 1] = hitList.at(i)->getY()/yScale;
        input_tensor_value[index * nFeatures + 2] = hitList.at(i)->getZ()/zScale;
        // auto detectorID = hitList.at(i)->OneHotEncoding(volumesITK, 30);
        // vector<float>::iterator it =  input_tensor_value.begin() + index*nFeatures + 3;
        // std::copy(detectorID.begin(), detectorID.end(),it );
        index++;
        
        
    }
    
    std::vector<int> val = classify(input_tensor_value);
    if(debug)
    {
        std::cout<<"input: ";
        for(auto& r: input_tensor_value)
        {
            std::cout<<r<<", ";
        }
        std::cout<<endl;

        std::cout<<"output: ";
        for(auto& r: val)
        {
            std::cout<<r<<", ";
        }
        std::cout<<endl;

    }
    return val;
}

std::vector<std::vector<int>>  NNEvalClassUtils::classify(std::vector<std::shared_ptr<Hit>> hitList, float threshold, bool /*debug*/)
{
    std::vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    unsigned int nHits = hitList.size();
    int index = 0;
    for (size_t i = nHits - n_hits; i < nHits; i++)
    {
        if(i >= hitList.size()) continue;
        // // if ordering is x0, x1, x2, y0, y1, y2, ...
        // input_tensor_value[i + 0]        = hitList.at(i)->getX() / xScale;
        // input_tensor_value[i + n_hits]   = hitList.at(i)->getY() / yScale;
        // input_tensor_value[i + 2*n_hits] = hitList.at(i)->getZ() / zScale;

        // if ordering is x0, y0, z0, x1, y1, z1, ...
        input_tensor_value[index * nFeatures + 0] = hitList.at(i)->getX()/xScale;
        input_tensor_value[index * nFeatures + 1] = hitList.at(i)->getY()/yScale;
        input_tensor_value[index * nFeatures + 2] = hitList.at(i)->getZ()/zScale;
        // auto detectorID = hitList.at(i)->OneHotEncoding(volumesITK, 30);
        // vector<float>::iterator it =  input_tensor_value.begin() + index*nFeatures + 3;
        // std::copy(detectorID.begin(), detectorID.end(),it );
        index++;
        
        
    }
    
    auto val = classify(input_tensor_value, threshold);
    // if(debug)
    // {
    //     std::cout<<"input: ";
    //     for(auto& r: input_tensor_value)
    //     {
    //         std::cout<<r<<", ";
    //     }
    //     std::cout<<endl;

    //     std::cout<<"output: ";
    //     for(auto& r: val)
    //     {
    //         std::cout<<r<<", ";
    //     }
    //     std::cout<<endl;

    // }
    return val;
}





