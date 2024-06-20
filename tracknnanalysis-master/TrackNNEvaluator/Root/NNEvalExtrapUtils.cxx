// Class includes
#include "TrackNNEvaluator/NNEvalCombined.h"

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
NNEvalExtrapUtils::NNEvalExtrapUtils(TString fileName, int hits)
{
    m_fileName = fileName;
    n_hits = hits;
}

NNEvalExtrapUtils::~NNEvalExtrapUtils()
{

} 


// big setters
void NNEvalExtrapUtils::initialize()
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
    

    //m_totalInputs = 48*3; //3 hits, x,y,z, volume and layer id one-hot-encoded
    m_totalInputs = 45+9; // 45 detector ohe 3 hits with 3 features each
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
          //m_totalInputs *= m_input_node_dims[j];
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


std::vector<float> NNEvalExtrapUtils::extrapSeed(std::vector<float>& input_tensor_value)
{
    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    // problem line leading to memory corruption:
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_value.data(), input_tensor_value.size(), m_input_node_dims.data(), m_input_node_dims.size());
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());
    // Get pointer to output tensor float values
    float* output_score_array = output_tensors.front().GetTensorMutableData<float>();
    // What is this output? 
    float output_x = output_score_array[0];
    float output_y = output_score_array[1];
    float output_z = output_score_array[2];
    std::vector<float> outputs = {output_x, output_y, output_z};

    return outputs;
}


std::shared_ptr<Hit>  NNEvalExtrapUtils::extrapSeed(std::vector<std::shared_ptr<Hit>> hitList, bool debug)
{
    static int hitIndex = -1;
    std::vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    unsigned int nHits = hitList.size();
    int index = 0;
    for (size_t i = nHits - n_hits; i < nHits; i++)
    {
        if(i >= hitList.size()) continue;
        // if(i < 0) continue;
        // // if ordering is x0, x1, x2, y0, y1, y2, ...
        // input_tensor_value[i + 0]        = hitList.at(i)->getX() / xScale;
        // input_tensor_value[i + n_hits]   = hitList.at(i)->getY() / yScale;
        // input_tensor_value[i + 2*n_hits] = hitList.at(i)->getZ() / zScale;

        // if ordering is x0, y0, z0, x1, y1, z1, ...
        input_tensor_value[index * nFeatures + 0] = hitList.at(i)->getX()/xScale;
        input_tensor_value[index * nFeatures + 1] = hitList.at(i)->getY()/yScale;
        input_tensor_value[index * nFeatures + 2] = hitList.at(i)->getZ()/zScale;

        
        // auto volumeVec = hitList.at(i)->OneHotEncodeVolumeID(volumesITK);
        // auto layerVec = hitList.at(i)->OneHotEncodeLayerID(30);
        // volumeVec.insert(volumeVec.end(), layerVec.begin(), layerVec.end()); //full ohe vector

        auto ohe = hitList.at(i)->OneHotEncoding(volumesITK, 30);
        vector<float>::iterator it =  input_tensor_value.begin() + index*nFeatures + 3;
        std::copy(ohe.begin(), ohe.end(),it );
        index++;
        // auto vol = hitList.at(i)->getVolume();
        // // TODO
        // bool isBarrel = (vol == 28) || (vol == 30) || (vol == 23) || (vol == 25) || (vol == 16) || (vol == 18);

        //input_tensor_value[index * nFeatures + 3] = isBarrel;
        //input_tensor_value[index*nFeatures + 4] = pixOrSCT;
        /*
        if (volID == 17)
        {
            input_tensor_value[index*nFeatures+3] = 1;
            input_tensor_value[index*nFeatures+4] = 0;
            input_tensor_value[index*nFeatures+5] = 0;
        }
        else if (volID == 24)
        {
            input_tensor_value[index*nFeatures+3] = 0;
            input_tensor_value[index*nFeatures+4] = 1;
            input_tensor_value[index*nFeatures+5] = 0;
        }
        else if (volID == 29)
        {
            input_tensor_value[index*nFeatures+3] = 0;
            input_tensor_value[index*nFeatures+4] = 0;
            input_tensor_value[index*nFeatures+5] = 1;
        }
        */
        // one-hot-encoded layer id for 3 input hits and 48 features
        // for (size_t j = 1; j<=8; j++)
        // {
        //     if ( layerID == j) 
        //     {
        //         input_tensor_value[index * nFeatures + j+11] = 1;
        //     }
        //     else input_tensor_value[index * nFeatures + j+11] = 0;
        // }
        //index++;
        
        
        // old ACTS ODD formatting
        // for (size_t j = 1; j<=8; j++)
        // {
        //     if ( layerID == j) 
        //     {
        //         input_tensor_value[index * nFeatures + j+11] = 1;
        //     }
        //     else input_tensor_value[index * nFeatures + j+11] = 0;
        // }
        // index++;
    }
    
    std::vector<float> val = extrapSeed(input_tensor_value);
    val[0] *= xScale;
    val[1] *= yScale;
    val[2] *= zScale;
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
            std::cout<<r<<" ";
        }
        std::cout<<endl;

    }

    auto predictedHit = std::make_shared<Hit> (hitIndex, val[0], val[1], val[2]);
    hitIndex--;

    return predictedHit;
}

std::shared_ptr<Hit>  NNEvalExtrapUtils::extrapSeed(std::vector<std::shared_ptr<Hit>> hitList, std::vector<int> oneHotEncoding, bool debug)
{    
    static int hitIndex = -1;
    std::vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    unsigned int nHits = hitList.size();
    //nFeatures = 45+nHits*3; //45 detector ohe + 3 features per hits
    int index = 0;
    for (size_t i = nHits - n_hits; i < nHits; i++)
    {
        if(i >= hitList.size()) continue;

        // if ordering is x0, y0, z0, x1, y1, z1, ...
        input_tensor_value[index * nFeatures + 0] = hitList.at(i)->getX()/xScale;
        input_tensor_value[index * nFeatures + 1] = hitList.at(i)->getY()/yScale;
        input_tensor_value[index * nFeatures + 2] = hitList.at(i)->getZ()/zScale;
        index++;
    }
    // vector<float>::iterator it =  input_tensor_value.begin() + index*nFeatures + 3;
    
    //std::copy(oneHotEncoding.begin(), oneHotEncoding.end(), it );
    // so stupid but trying to format the input another way breaks it
    // inp.reserve(54);
    // for (auto& v:input_tensor_value) inp.push_back(v);
    // for (auto v: oneHotEncoding)
    // {
    //     float a = static_cast<float> (v);
    //     inp.push_back(a);
    // }
    for (size_t i=0; i<45; i++)
    {
        float a = static_cast<float> (oneHotEncoding[i]);
        input_tensor_value[i+9] = a;
    }
    // std::cout<<"=============="<<std::endl;
    // std::cout<<"input size "<<inp.size()<<std::endl;
    // for (auto& v: inp) std::cout<<v<<", ";
    // std::cout<<std::endl;
    // std::cout<<"=============="<<std::endl;

    std::vector<float> val = extrapSeed(input_tensor_value);
    val[0] *= xScale;
    val[1] *= yScale;
    val[2] *= zScale;
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
            std::cout<<r<<" ";
        }
        std::cout<<endl;

    }

    auto predictedHit = std::make_shared<Hit> (hitIndex, val[0], val[1], val[2]);
    hitIndex--;

    return predictedHit;
}

std::shared_ptr<Hit>  NNEvalExtrapUtils::ClassifyAndExtrapolate(std::vector<std::shared_ptr<Hit>> hitList, bool debug)
{
    /*Use for model that was trained as a combined classifier and extrapolator. Classification is built into model call*/
    static int hitIndex = -1;

    std::vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    unsigned int nHits = hitList.size();
    //nFeatures = 45+nHits*3; //45 detector ohe + 3 features per hits
    int index = 0;
    for (size_t i = nHits - n_hits; i < nHits; i++)
    {
        if(i >= hitList.size()) continue;

        // if ordering is x0, y0, z0, x1, y1, z1, ...
        input_tensor_value[index * nFeatures + 0] = hitList.at(i)->getX()/xScale;
        input_tensor_value[index * nFeatures + 1] = hitList.at(i)->getY()/yScale;
        input_tensor_value[index * nFeatures + 2] = hitList.at(i)->getZ()/zScale;
        index++;
    }


    std::vector<float> val = extrapSeed(input_tensor_value);
    val[0] *= xScale;
    val[1] *= yScale;
    val[2] *= zScale;
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
            std::cout<<r<<" ";
        }
        std::cout<<endl;

    }

    auto predictedHit = std::make_shared<Hit> (hitIndex, val[0], val[1], val[2]);
    hitIndex--;

    return predictedHit;
}

////// This is for many-to-many extrapolation ///////
std::vector<float> NNEvalExtrapUtils::extrapolate(std::vector<float>& input_tensor_value)
{
    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_value.data(), input_tensor_value.size(), m_input_node_dims.data(), m_input_node_dims.size());
    auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());
    // Get pointer to output tensor float values
    float* output_score_array = output_tensors.front().GetTensorMutableData<float>();
    //float output_x = output_score_array[0];
    //float output_y = output_score_array[1];
    //float output_z = output_score_array[2];
    //std::vector<float> outputs = {output_x, output_y, output_z};

    //hardcode the number of outupts. Better to do so dynamically. How to get size of output_score_array?
    std::vector<float> outputs;
    for (size_t i=0; i<3; i++) 
    {
        outputs.push_back(output_score_array[i]);
    }
    return outputs;
}


std::shared_ptr<Hit>  NNEvalExtrapUtils::extrapolate(std::vector<std::shared_ptr<Hit>> hitList, int idx, bool debug)
{
    // idx refers to the idx of the hit that we want to output
    static int hitIndex = -1;

    std::vector<float> input_tensor_value(m_totalInputs, 0);

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    unsigned int nHits = hitList.size();
    //int nFeatures = 3; //12 if including layer info
    //int trackLength = 9; // change if trained on longer tracks or to pad length
    for (size_t i = 0; i < nHits; i++)
    {
        if(i >= hitList.size()) continue;
        // // if ordering is x0, x1, x2, y0, y1, y2, ...
        // input_tensor_value[i + 0]        = hitList.at(i)->getX() / xScale;
        // input_tensor_value[i + n_hits]   = hitList.at(i)->getY() / yScale;
        // input_tensor_value[i + 2*n_hits] = hitList.at(i)->getZ() / zScale;

        // if ordering is x0, y0, z0, x1, y1, z1, ...
        input_tensor_value[i * nFeatures + 0] = hitList.at(i)->getX()/xScale;
        input_tensor_value[i * nFeatures + 1] = hitList.at(i)->getY()/yScale;
        input_tensor_value[i * nFeatures + 2] = hitList.at(i)->getZ()/zScale;
        
        // if using one hot encoded layer id
        //first index is 0/1 for barrel/endcap
        // int layerID = hitList.at(index)->getLayer() / 2;
        // auto vol = hitList.at(i)->getVolume();
        // bool isBarrel = (vol == 28) || (vol == 30) || (vol == 23) || (vol == 25) || (vol == 16) || (vol == 18);
        // input_tensor_value[index * nFeatures + 3] = isBarrel;
        // for (size_t j = 1; j<=8; j++)
        // {
        //     if ( layerID == j) input_tensor_value[index * nFeatures + j+3] = 1;
        //     else input_tensor_value[index * nFeatures + j+3] = 0;
        // }
        // index++;
    }
    //pad track
    // for (size_t i = nHits; i<trackLength; i++)
    // {
    //     input_tensor_value[i * nFeatures + 0] = 0;
    //     input_tensor_value[i * nFeatures + 1] = 0;
    //     input_tensor_value[i * nFeatures + 2] = 0;
    // }
    
    std::vector<float> val = extrapolate(input_tensor_value);
    // val[idx*nFeatures+0] *= xScale;
    // val[idx*nFeatures+1] *= yScale;
    // val[idx*nFeatures+2] *= zScale;
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
            std::cout<<r<<" ";
        }
        std::cout<<endl;

    }
    
    //Only need to output a prediction at a single index
    auto predictedHit = std::make_shared<Hit> (hitIndex, val[idx*nFeatures+0], val[idx*nFeatures+1], val[idx*nFeatures+2]);
    hitIndex--;

    return predictedHit;
}

////////  end many-to-many methods /////////





