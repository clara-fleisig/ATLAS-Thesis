#ifndef _EXTRAPOLATEDTRACK_H
#define _EXTRAPOLATEDTRACK_H

//local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/TrueTrack.h"

// Local forward includes
class Hit;
class SeedTrack;
class RecoTrack;

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes


// Base class 
class ExtrapolatedTrack: public TrackBase
{
    public:
        ExtrapolatedTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits, std::shared_ptr<SeedTrack> seed);
        virtual ~ExtrapolatedTrack();

        void printDebug();
        std::shared_ptr<RecoTrack> getBaseRecoTrack();
        std::shared_ptr<TrueTrack> getBaseTrueTrack();
        std::shared_ptr<SeedTrack> getBaseSeedTrack() {return m_baseSeedTrack; };

        void addHit(std::shared_ptr<Hit> predHit, std::shared_ptr<Hit> recovHit){ addPredictedHit(predHit); addRecoveredHit(recovHit);};

        void addPredictedHit(std::shared_ptr<Hit> hit);
        void addRecoveredHit(std::shared_ptr<Hit> hit);
        
        std::vector<std::shared_ptr<Hit>> getPredictedHitsList();
        std::vector<std::shared_ptr<Hit>> getPredictedHitsRotatedToZeroList();

        float fracTrueHits();
        bool isMatchedToReco(Index recoIndex, float threshold, int seedSize);
        bool isMatchedToRecoHits(Index recoIndex, int numMatchingHits);
        Index isMatchedToTrack(float threshold);
        bool isMatched(float fraction);
        void setDuplicate(){m_isDuplicate = true;}
        bool isDuplicate(){return m_isDuplicate;}
        double getComputedTrackProbability();


    protected:
        std::shared_ptr<SeedTrack> m_baseSeedTrack;

        std::vector<Index> m_hitPredIndex;
        std::map<Index, std::shared_ptr<Hit>> m_hitPredList;

        std::vector<std::shared_ptr<Hit>> m_outPredList;
        std::vector<std::shared_ptr<Hit>> m_outPredRotatedList;
        bool m_isDuplicate = false;
        
        std::map<Index, float> m_truthBarcodeCountMap;
        double m_computedTrackProb = 0;
        double m_totalWeight=0;

        void resetCaches() override;


};

#endif
