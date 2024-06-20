#ifndef _DATAREADERHTT_H
#define _DATAREADERHTT_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNDataReader/DataReader.h"

#include "TrackNNDataReader/HTTReaders/HTTHoughRootOutput.h"
#include "TrackNNDataReader/HTTReaders/HTTOfflineTree.h"
#include "TrackNNDataReader/HTTReaders/HTTTruthTree.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes

// Base class 
class DataReaderHTT: public DataReader 
{
    public:
        DataReaderHTT(TString fileName, TString geoFileName): DataReader(fileName, geoFileName){ m_pileupOnly = false; m_lwrPtLim = 1; m_uprPtLim=100000; m_muonOnly = false;};
        virtual ~DataReaderHTT() {};

        void initialize() override;
        void loadEntry(int iEvent) override;
        void pileupOnly(bool tpileupOnly = true) {m_pileupOnly = tpileupOnly;} ;
        void muonOnly(bool tmuonOnly = true) {m_muonOnly = tmuonOnly;} ;
        void setLwrPtCut(float lim){ m_lwrPtLim = lim; };
        void setUprPtCut(float lim){ m_uprPtLim = lim; };
    protected:
    	int m_currIndex;
    	int m_maxEntries;
        bool m_pileupOnly;
        bool m_muonOnly;

        float m_lwrPtLim;
        float m_uprPtLim;

    private:
        void createSP(std::shared_ptr<TrackBase> trk);

        std::shared_ptr<HTTHoughRootOutput> m_HTReader;
        std::shared_ptr<HTTOfflineTree> m_offlineReader;
        std::shared_ptr<HTTTruthTree> m_truthReader;

};


#endif
