#ifndef _SEEDTRACK_H
#define _SEEDTRACK_H

//local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/TrueTrack.h"

// Local forward includes
class Hit;
class SeedManager;
class RecoTrack;

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes


// Base class 
class SeedTrack: public TrackBase
{
    friend class SeedManager;

    public:
        //SeedTrack(std::vector<Index> hitIndex, std::pair <std::vector<std::shared_ptr<Hit>> hits, std::shared_ptr<Hit>>);
        //store a pair of the seed, and corresponding next hit
        SeedTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits);
        virtual ~SeedTrack();

        void printDebug();
        std::shared_ptr<RecoTrack> getBaseRecoTrack() {return m_baseRecoTrack; };
        std::shared_ptr<TrueTrack> getBaseTrueTrack() {return m_baseTrueTrack; };
        void setNExtrapolatedTracks(int nExtrapolatedTracks) {m_nExtrapolatedTracks = nExtrapolatedTracks;}
        void incrimentNExtrapolatedTracks() {m_nExtrapolatedTracks++;}
        int getNExtrapolatedTracks() {return m_nExtrapolatedTracks;}


    protected:
        std::shared_ptr<RecoTrack> m_baseRecoTrack;
        std::shared_ptr<TrueTrack> m_baseTrueTrack;
        // store how many tracks are extrapolated from each seed
        int m_nExtrapolatedTracks=0;
        void setBaseRecoTrack(std::shared_ptr<RecoTrack> trk) {m_baseRecoTrack = trk; };
        void setBaseTrueTrack(std::shared_ptr<TrueTrack> trk) {m_baseTrueTrack = trk; };



};

#endif
