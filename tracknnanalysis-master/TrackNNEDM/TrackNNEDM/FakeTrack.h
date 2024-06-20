#ifndef _FAKETRACK_H
#define _FAKETRACK_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"

// Local forward includes
class Hit;
class FakeManager;

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes


// Base class 
class FakeTrack: public TrackBase
{
    public:
        FakeTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits);
        virtual ~FakeTrack();   

        void printDebug();

        double getTrackProbability();

    protected:
    	double m_prob;
    
};


#endif
