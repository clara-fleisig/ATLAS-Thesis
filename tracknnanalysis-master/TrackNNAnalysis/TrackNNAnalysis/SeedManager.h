#ifndef _SEEDMANAGER_H
#define _SEEDMANAGER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/TrueTrack.h"
// #include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/SeedTrack.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>
#include <random>


enum class SeedType 
{ 
    InitialSeeds,
    SlidingWindow,
    //SlidingWindowBarrelLayered
};


class SeedManager
{
    public:
        SeedManager();
        virtual ~SeedManager();
        // void setHitManager(std::shared_ptr<HitsManager> manager){ m_hitManager = manager; };


        void setSeedSize(int size) {m_seedSize = size; };
        void setWindowSize(int size) {m_windowSize = size; };

        // Get the seeds from the track
        std::vector<std::shared_ptr<SeedTrack>> getSeedTracks(SeedType seedType, std::vector<std::shared_ptr<TrueTrack>> recoTracks);
        std::vector<std::pair<std::shared_ptr<SeedTrack>, std::shared_ptr<Hit>>> getSlidingWindowSeedPairs(std::vector<std::shared_ptr<TrueTrack>> recoTracks);
    protected:
        std::vector<std::shared_ptr<SeedTrack>> getSlidingWindowSeeds(std::vector<std::shared_ptr<TrueTrack>> recoTracks);
        std::vector<std::shared_ptr<SeedTrack>> getInitialSeeds(std::vector<std::shared_ptr<TrueTrack>> recoTracks);
        //std::vector<std::shared_ptr<SeedTrack>> getSlidingWindowSeedsBarrelLayered(std::vector<std::shared_ptr<TrueTrack>> recoTracks); 
        int m_seedSize;
        int m_windowSize;
       
};

#endif
