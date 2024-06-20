#ifndef _DATAREADERITK_H
#define _DATAREADERITK_H

// Local includes
#include "TrackNNDataReader/DataReader.h"
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNDataReader/ACTSReaders/HitReaderClass.h"
#include "TrackNNDataReader/ACTSReaders/RecoTracksReaderClass.h"
#include "TrackNNDataReader/ACTSReaders/RoadReaderClass.h"
#include "TrackNNDataReader/ACTSReaders/TrackReaderClass.h"
#include "TrackNNDataReader/ACTSReaders/TruthTracksReaderClass.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes

// Base class 
class DataReaderITK: public DataReader 
{
    public:
        DataReaderITK(TString fileName, TString geoFileName): DataReader(fileName, geoFileName){ m_pileupOnly = false; m_lwrPtLim = 1; m_uprPtLim=100000; m_muonOnly = false; m_LayerRegion = "noCut", m_deltaROverlap = 0;};
        virtual ~DataReaderITK() {};

        void initialize() override;
        void loadEntry(int iEvent) override;


        void pileupOnly(bool tpileupOnly = true) {m_pileupOnly = tpileupOnly;} ;
        void muonOnly(bool tmuonOnly = true) {m_muonOnly = tmuonOnly;} ;
        void setLwrPtCut(float lim){ m_lwrPtLim = lim; };
        void setUprPtCut(float lim){ m_uprPtLim = lim; };
        void setDeltaROverlap(float deltaROverlap) {m_deltaROverlap = deltaROverlap;} ;

        void predictNextVolumeAndLayer(std::shared_ptr<Hit> hit, std::vector<std::pair<int, int>>& nextID) override;

        std::vector<int> getBarrelVolumes() const {return m_barrelVolumes;}
        

    protected:
        int m_maxEntries;
        bool m_pileupOnly;
        bool m_muonOnly;
        float m_deltaROverlap = 0;

        float m_lwrPtLim;
        float m_uprPtLim;
        float m_etaLim;
        const std::vector<int> m_barrelVolumes = {9,16,23};

        void readGeo() override;

        std::shared_ptr<HitReaderClass> m_hits;
        std::shared_ptr<RecoTracksReaderClass> m_recoTracks;
        std::shared_ptr<RoadReaderClass> m_roads;
        std::shared_ptr<TrackReaderClass> m_tracks;
        std::shared_ptr<TruthTracksReaderClass> m_truthTracks;
};


#endif
