#ifndef _NNEVALUTILS_H
#define _NNEVALUTILS_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNEDM/SeedTrack.h"
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes
#include <TString.h>
#include <core/session/onnxruntime_cxx_api.h>

// Base class 
class NNEvalUtils 
{
   
    public:
        // NNEvalUtils(TString fileName, int hits, std::string trackType, std::string order, bool scaled);
        NNEvalUtils(TString fileName, int hits, std::string trackType="Nominal", std::string order="XX0YY0ZZ0", bool scaled=false);
        virtual ~NNEvalUtils();   

        void initialize();
        float eval(std::vector<float>& input_tensor_value);

        void evalTrack(std::shared_ptr<ExtrapolatedTrack> trk);
        void evalTrack(std::shared_ptr<RecoTrack> trk);
        void evalTrack(std::shared_ptr<FakeTrack> trk);
        void evalTrack(std::shared_ptr<TrackBase> trk);

    private:
        std::vector<const char*> m_input_node_names;
        std::vector<int64_t> m_input_node_dims;
        std::vector<const char*> m_output_node_names;
        std::unique_ptr< Ort::Session > m_session;
        std::unique_ptr< Ort::Env > m_env;
        int m_totalInputs;


        TString m_fileName;
        int n_hits;
        std::string m_trackType;
        std::string m_order;
        bool m_scaled;

        //float eval(std::vector<float>& input_tensor_value);
        float eval(std::vector<std::shared_ptr<Hit>> hitList, bool debug = false);
};


#endif
