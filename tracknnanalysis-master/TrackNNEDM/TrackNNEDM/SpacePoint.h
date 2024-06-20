#ifndef _SPACEPOINT_H
#define _SPACEPOINT_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/Hit.h"

// Local forward includes
class Hit;
class TrueTrack;
class DataReader;
class DataReaderACTS;
class DataReaderITK;
class DataReaderHTT;

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes


// Base class 
class SpacePoint: public Hit
{
    friend class DataReader;
    friend class DataReaderHTT;
    friend class DataReaderACTS;
    friend class DataReaderITK;

 public:
        SpacePoint();
        SpacePoint(Index index, float x, float y, float z);
        SpacePoint(Index index, 
            float x, 
            float y, 
            float z, 
            float r, 
            float rho, 
            int volumeID, 
            int layerID, 
            int moduleID,
            int etaModule, 
            int phiModule, 
            int pixOrSCT, 
            int side,
            Index recoTrackIndex, 
            std::vector<Index> trueTrackBarcode,
            Index FirstHitClusterIndex,
            Index SecondHitClusterIndex);
        virtual ~SpacePoint();   

        Index getFirstHitClusterIndex() {return m_FirstHitClusterIndex;};
        Index getSecondHitClusterIndex() {return m_SecondHitClusterIndex;};

        void setFirstCluster(std::shared_ptr<Hit> cluster) {m_firstCluster = cluster;}; 
        void setSecondCluster(std::shared_ptr<Hit> cluster) {m_secondCluster = cluster;}; 

    private:
        Index m_FirstHitClusterIndex;
        Index m_SecondHitClusterIndex;

        std::shared_ptr<Hit> m_firstCluster = nullptr;
        std::shared_ptr<Hit> m_secondCluster = nullptr;

};

#endif