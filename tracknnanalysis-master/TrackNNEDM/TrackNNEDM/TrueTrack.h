#ifndef _TRUETRACK_H
#define _TRUETRACK_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"

// Local forward includes
class Hit;
class RecoTrack;
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
class TrueTrack: public TrackBase
{
    friend class DataReader;
    friend class DataReaderHTT;
    friend class DataReaderACTS;
    friend class DataReaderITK;
    friend class TrackManager;

    public:
        TrueTrack(Index barcode, 
            float pt, 
            float eta, 
            float phi, 
            float vertexDistance,
            int pdgid, 
            int status, 
            Index recoTrackIndex,
            std::vector<Index> hitIndex);

        TrueTrack(Index barcode, 
            float pt, 
            float eta, 
            float phi, 
            float vertexDistance,
            int pdgid, 
            int status, 
            Index recoTrackIndex,
            std::vector<Index> hitIndex,
            std::vector<std::shared_ptr<Hit>> hits);
            
        //TrueTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits);
        virtual ~TrueTrack();   

        Index getBarcode() {return m_barcode; };
        Index getRecoTrackIndex(){return m_recoTrackIndex;};

        void printDebug();

        double getTrackProbability() override {return m_trackProb;};
        double getComputedTrackProbability()  {return m_computedTrackProb;};

        void setTrackProbability(double trackProb)            {m_trackProb = trackProb;};
        void setComputedTrackProbability(double trackProb)    {m_computedTrackProb = trackProb;};

        float getPt(){return m_pt;};
        float getEta(){return m_eta;};
        float getPhi(){return m_phi;};
        float getZ0(){return m_z0;};
        float getD0(){return m_d0;};
        int getPDGID(){return m_pdgid;};
        float getVertexDistance(){return m_vertexDistance;};
        int getStatus(){return m_status;};

    protected:
        Index m_barcode; 
        float m_pt; 
        float m_eta; 
        float m_phi; 
        float m_vertexDistance;
        float m_z0;
        float m_d0;
        
        int m_pdgid; 
        int m_status; 
        Index m_recoTrackIndex;
        std::shared_ptr<RecoTrack> m_recoTrack;

        float m_computedTrackProb;
        float m_trackProb;

        void setOrgRecoTrack(std::shared_ptr<RecoTrack> track);

        void setZ0(float z0) {m_z0 = z0;};
        void setD0(float d0) {m_d0 = d0;};


};


#endif
