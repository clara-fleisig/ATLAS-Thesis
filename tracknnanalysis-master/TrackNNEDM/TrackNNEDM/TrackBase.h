#ifndef _TRACKBASE_H
#define _TRACKBASE_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"


// Local forward includes
class Hit;
class DataReader;
class DataReaderACTS;
class DataReaderITK;
class DataReaderHTT;
// c++ includes
#include <map>
#include <vector>
#include <memory>
#include <iostream>

// Root includes
#include <TString.h>


// Base class 
class TrackBase 
{
    friend class DataReader;
    friend class DataReaderHTT;
	friend class DataReaderACTS;
    friend class DataReaderITk;
    friend class TrackManager;

    public:
        enum class DetectorRegion
        {
            full,
            barrel,
            endcap
        };

    public:
        TrackBase(std::vector<Index> hitIndex);
        virtual ~TrackBase();   

        std::vector<Index> getHitIndex() {return m_hitIndex;};
        std::vector<std::shared_ptr<Hit>> getHitsList();
        std::vector<std::shared_ptr<Hit>> getHitsRotatedToZeroList();
        std::vector<std::shared_ptr<Hit>> getHitsRoughRotatedToZeroList();
        // std::vector<std::shared_ptr<Hit>> getBarrelLayeredHits();
        std::vector<std::shared_ptr<Hit>> getLayeredHitsList(DetectorRegion detectorRegion);
        std::vector<std::shared_ptr<Hit>> getSPHitsList();
        std::vector<std::shared_ptr<Hit>> getSPHitsRotatedToZeroList();
        std::vector<std::shared_ptr<Hit>> getSPHitsRoughRotatedToZeroList();
        std::vector<std::shared_ptr<Hit>> getSPHitsUnscaledRotatedToZeroList();

        std::shared_ptr<Hit> getHit(int i);

        int getNHits();
        virtual void printDebug(){};
        void printArray();

        void setNNScore(float _x)   {m_nnScore = _x;};
        float getNNScore()          {return m_nnScore; };

        //void setPredictedHit(std::shared_ptr<Hit> predHit)   {m_predictedHit = predHit;}
        //std::shared_ptr<Hit> getPredictedHit()  {return m_predictedHit;}

        bool hasHit(Index index);

        virtual double getTrackProbability() {return -999;};

        void addIntMetadata(TString key, int val){m_intMetadata[key] = val; };
        int getIntMetadata(TString key) 
        {
            if(m_intMetadata.find(key) == m_intMetadata.end())
            {
                std::cout<<"Can't find int metadata in the track with the key: "<<key<<std::endl;
                exit(1);
            }
            return m_intMetadata.at(key);
        };

        void addFloatMetadata(TString key, float val){m_floatMetadata[key] = val; };
        float getFloatMetadata(TString key) 
        {
            if(m_floatMetadata.find(key) == m_floatMetadata.end())
            {
                std::cout<<"Can't find float metadata in the track with the key: "<<key<<std::endl;
                exit(1);
            }
            return m_floatMetadata.at(key);
        };


        double calcOverlapProbability(std::shared_ptr<TrackBase> track);
        int getNHitOverlap(std::shared_ptr<TrackBase> track);

    protected:
        std::vector<Index> m_hitIndex;
        std::map<Index, std::shared_ptr<Hit>> m_hitList;

        int m_nHits;

        std::vector<Index> getOrgHitIndex() {return m_hitIndex;};
        void setOrgHits(std::vector<std::shared_ptr<Hit>> hits);

        std::vector<std::shared_ptr<Hit>> m_outList;
        std::vector<std::shared_ptr<Hit>> m_outRotatedList;
        std::vector<std::shared_ptr<Hit>> m_outRoughRotatedList;


        void setSPHits(std::vector<std::shared_ptr<Hit>> hits) {m_inSPList = hits;};

        std::vector<std::shared_ptr<Hit>> m_inSPList; // Orginal space point hit list
        std::vector<std::shared_ptr<Hit>> m_outSPList; // Ordered SP hit list for caching
        std::vector<std::shared_ptr<Hit>> m_outRotatedSPList; // rotated SP for caching
        std::vector<std::shared_ptr<Hit>> m_outRoughRotatedSPList; // rotated SP for caching, with rough rotation
        std::vector<std::shared_ptr<Hit>> m_outUnscaledRotatedSPList; // rotated SP for caching, with unscaled rotation
        std::vector<std::shared_ptr<Hit>> m_outLayeredList; //  hits in consecutive layers for caching


        float m_nnScore;

        virtual void resetCaches();

        std::map<TString, int> m_intMetadata;
        std::map<TString, float> m_floatMetadata;

        void fillRotatedHits(std::vector<std::shared_ptr<Hit>> hits, std::vector<std::shared_ptr<Hit>>& rotatedHits);
        void fillRoughRotatedHits(std::vector<std::shared_ptr<Hit>> hits, std::vector<std::shared_ptr<Hit>>& rotatedHits);
        void fillUnscaledRotatedHits(std::vector<std::shared_ptr<Hit>> hits, std::vector<std::shared_ptr<Hit>>& rotatedHits);
        void fillLayeredHits(std::vector<std::shared_ptr<Hit>> hits, std::vector<std::shared_ptr<Hit>>& layeredHits, DetectorRegion detectorRegion);



};


#endif
