#ifndef _FAKEMANAGER_H
#define _FAKEMANAGER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>
#include <random>

// Root includes
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1F.h>
#include <TRandom3.h>

enum class FakeType 
{ 
	FullyRandom,
    LayerRandom,
    ModuleRandom,
    ModuleRotatedRandom,
    ModuleIterRandom,
    ModuleRotatedIterRandom,
    FullCombinatoricsinRegions,
    TrackHitChangeRandom,
    TrackHitChangeClosest,

};



// Base class 
class FakeManager 
{
    public:
        FakeManager();
        virtual ~FakeManager();   

        void setHitManager(std::shared_ptr<HitsManager> manager){ m_hitManager = manager; };

        std::vector<std::shared_ptr<FakeTrack>> getFakeTracks(FakeType fakeType, std::vector<std::shared_ptr<RecoTrack>> recoTracks, unsigned int nTracks, int phiSlice = -1);

    protected:
        // Variables to store the info
        std::shared_ptr<HitsManager> m_hitManager;

        std::shared_ptr<FakeTrack> getFullyRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getLayerRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getModuleRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getModuleRotatedRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getModuleIterRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getModuleRotatedIterRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getHitChangeRandomTrack(std::shared_ptr<TrackBase> track);
        std::shared_ptr<FakeTrack> getHitChangeClosestTrack(std::shared_ptr<TrackBase> track);


        std::vector<std::shared_ptr<FakeTrack>> getFullCombinatoricsFakeinRegions(int phiSlice);
        std::vector<std::shared_ptr<FakeTrack>> getFullCombinatoricsFakeinRegion(double lwrPhi, double uprPhi, double lwrEta, double uprEta, bool doOverlapEdges);

        TH1F* m_inverseRandAngleDistribution;
        TRandom3* m_randGen;
        std::default_random_engine m_generator;

};


#endif
