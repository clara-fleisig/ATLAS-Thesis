#ifndef _TRACKMANAGER_H
#define _TRACKMANAGER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/TrueTrack.h"
// #include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/SeedTrack.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

class TrackManager
{
    public:
        TrackManager();
        virtual ~TrackManager();
        
        // Make a track with only a single hit per layer
        std::vector<std::shared_ptr<TrueTrack>> getTrackSingleLayerHitInBarrel(std::vector<std::shared_ptr<TrueTrack>> recoTracks, int minHits) {return getTrackSingleLayerHit(recoTracks, minHits, TrackBase::DetectorRegion::barrel);};
        std::vector<std::shared_ptr<TrueTrack>> getTrackSingleLayerHitInEndcap(std::vector<std::shared_ptr<TrueTrack>> recoTracks, int minHits) {return getTrackSingleLayerHit(recoTracks, minHits, TrackBase::DetectorRegion::endcap);};
        std::vector<std::shared_ptr<TrueTrack>> getTrackSingleLayerHit(std::vector<std::shared_ptr<TrueTrack>> recoTracks, int minHits) {return getTrackSingleLayerHit(recoTracks, minHits, TrackBase::DetectorRegion::full);};
        std::vector<std::pair<std::shared_ptr<TrueTrack>, std::shared_ptr<Hit>>> getTrackSegmentPairs(std::vector<std::shared_ptr<TrueTrack>> recoTracks);

    private:
        // Keep this private to reduce number of options needed to be specified by the user
        std::vector<std::shared_ptr<TrueTrack>> getTrackSingleLayerHit(std::vector<std::shared_ptr<TrueTrack>> recoTracks, int minHits, TrackBase::DetectorRegion detectorRegion);
        int m_initialSeedSize = 3;
};

#endif