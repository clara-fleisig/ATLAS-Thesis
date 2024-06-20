#ifndef _NNEVALEXTRAPUTILS_H
#define _NNEVALEXTRAPUTILS_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes
#include <TString.h>
#include <core/session/onnxruntime_cxx_api.h>

#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/SeedTrack.h"
#include "TrackNNEDM/Hit.h"

// Base class 
class NNEvalExtrapUtils 
{
    public:
        NNEvalExtrapUtils(TString fileName, int hits);
        virtual ~NNEvalExtrapUtils();   

        void initialize();

        std::shared_ptr<Hit> extrapSeed(std::vector<std::shared_ptr<Hit>> hitList, bool debug = false);
        std::shared_ptr<Hit>  extrapSeed(std::vector<std::shared_ptr<Hit>> hitList, std::vector<int> oneHotEncoding, bool debug=false);
       std::shared_ptr<Hit>  ClassifyAndExtrapolate(std::vector<std::shared_ptr<Hit>> hitList, bool debug);

        std::vector<float> extrapSeed(std::vector<float>& input_tensor_value);
        std::shared_ptr<Hit> extrapolate(std::vector<std::shared_ptr<Hit>> hitList, int idx, bool debug = false);
        std::vector<float> extrapolate(std::vector<float>& input_tensor_value);
        

        // setNFeatures(int N){nFeatures = N;}
        
    private:
        std::vector<const char*>    m_input_node_names;
        std::vector<int64_t>        m_input_node_dims;
        std::vector<const char*>    m_output_node_names;
        std::unique_ptr< Ort::Session > m_session;
        std::unique_ptr< Ort::Env > m_env;
        int m_totalInputs;
        int nFeatures = 3;
        //int nFeatures = 20; //ACTS ODD training
        int trackLength = 9; // This specifies the length of tracks used for training in many-to-many models
        std::vector<int> volumesACTS = {17,24,29,16,23,28,18,25,30}; //same format as used in training
        std::vector<int> volumesITK = {2, 8, 9, 10, 13, 14, 15, 16, 18, 19, 20, 22, 23, 24, 25};
        int nVolumesITK = volumesITK.size();
        int nVolumesACTS = volumesACTS.size();

        TString m_fileName;
        int n_hits;

        //std::vector<float> extrapSeed(std::vector<float>& input_tensor_value);
};


#endif
