#ifndef _NNEVALLWTNNUTILS_H
#define _NNEVALLWTNNUTILS_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes
#include <TString.h>

#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"
#include "lwtnn/Exceptions.hh"
#include "lwtnn/lightweight_nn_streamers.hh"
#include "lwtnn/NanReplacer.hh"



#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/Hit.h"

// Base class 
class NNEvalLwtnnUtils
{
   
    public:
        NNEvalLwtnnUtils(TString fileName);
        virtual ~NNEvalLwtnnUtils();   

        void initialize();

        void evalTrack(std::shared_ptr<RecoTrack> trk);
        void evalTrack(std::shared_ptr<FakeTrack> trk);
        void evalTrack(std::shared_ptr<TrackBase> trk);

    private:
        TString m_fileName;
        std::shared_ptr<lwt::LightweightGraph> m_lwnn;


};


#endif
