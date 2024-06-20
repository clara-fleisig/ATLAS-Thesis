#ifndef _RECOTRACK_H
#define _RECOTRACK_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"

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
class RecoTrack: public TrackBase
{
    friend class DataReader;
    friend class DataReaderHTT;
    friend class DataReaderACTS;
    friend class DataReaderITK;

    public:
        RecoTrack(Index index, 
            float pt, 
            float eta, 
            float phi, 
            float truthProb,
            float track_d0, 
            float track_z0,
            float track_phi,
            float track_theta, 
            float track_qoverp,
            int charge, 
            int nPixHits, 
            int nSCTHits, 
            Index truthBarcode,
            std::vector<Index> hitIndex);

        RecoTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits);
        virtual ~RecoTrack();   

        Index getIndex() {return m_index; };

        void printDebug();

        Index getTrueTrackBarcode(){return m_truthBarcode;};

        float getPt(){return m_pt;};
        float getEta(){return m_eta;};
        float getPhi(){return m_phi;};
        float getD0(){return m_track_d0;};
        float getZ0(){return m_track_z0;};
        int   getCharge(){return m_charge;};

        void setPt(float in){m_pt = in;};
        void setEta(float in){m_eta = in;};
        void setPhi(float in){m_phi = in;};
        void setD0(float in){m_track_d0 = in;};
        void setZ0(float in){m_track_z0 = in;};
        void setCharge(int in){m_charge = in;};

        double getTrackProbability() override;
        double getComputedTrackProbability();
        double getMatchedProbability(Index barcode);
        bool isMatched(Index barcode, double threshold);

    protected:
        Index m_index; 
        float m_pt; 
        float m_eta; 
        float m_phi; 
        float m_truthProb;
        float m_track_d0; 
        float m_track_z0;
        float m_track_phi;
        float m_track_theta; 
        float m_track_qoverp;
        int m_charge; 
        int m_nPixHits; 
        int m_nSCTHits; 
        Index m_truthBarcode;

        std::shared_ptr<TrueTrack> m_trueTrack;

        void setOrgTrueTrack(std::shared_ptr<TrueTrack> track);

        float m_computedTrackProb;
        std::map<Index, float> m_truthBarcodeCountMap;
        float m_totalWeight;


};


#endif
