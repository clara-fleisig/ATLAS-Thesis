#ifndef _DATAREADER_H
#define _DATAREADER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/SpacePoint.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/DetectorModule.h"

// Reader include
#include "TrackNNDataReader/ATLASReaders/HitInfoATLAS.h"
#include "TrackNNDataReader/ATLASReaders/SPInfoATLAS.h"
#include "TrackNNDataReader/ATLASReaders/RecoTracksATLAS.h"
#include "TrackNNDataReader/ATLASReaders/TruthTracksATLAS.h"

// c++ includes
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <set>
// Root includes
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TChain.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Base class 
class DataReader 
{
    public:
        DataReader(TString fileName, TString geoFileName);
        virtual ~DataReader();   

        virtual void initialize();

        int getEntries() { return m_nEvents; };   

        virtual void loadEntry(int iEvent);

        void dropPixHit(bool dropHits = true)        {m_dropPixHit = dropHits; };
        void setLayerRegion(std::string HTTLayer) {m_LayerRegion = TString(HTTLayer); };
        void setLayerThreshold(int thershold)        {m_nLayerThreshold = thershold;};

        virtual std::vector<std::shared_ptr<RecoTrack>> getRecoTracks() {return m_recoTrackList;}; 
        virtual std::vector<std::shared_ptr<TrueTrack>> getTrueTracks() {return m_trueTrackList;}; 
        virtual std::vector<std::shared_ptr<Hit>>       getHits()       {return m_hitList;}; 
        virtual std::vector<std::shared_ptr<TrueTrack>> getOfflineTruthTracks() {return m_offlineTrackList;}; 

        void setLwrEtaLimit(float etaLim) { m_lwrEtaLim = etaLim;};
        void setUprEtaLimit(float etaLim) { m_uprEtaLim = etaLim;};

        std::vector<std::shared_ptr<DetectorModule>> getDetectorModules() {return m_detectorModule;}; 

        virtual void predictNextVolumeAndLayer(std::shared_ptr<Hit> /*hit*/, std::vector<std::pair<int, int>>& /*nextID*/) {};

    protected:
        // Variables to store the info
        TFile* m_inFile;
        TString m_fileName;

        TString m_inDir;
        TFile* m_hitsFile;
        TFile* m_particlesFile;
        TFile* m_tracksFile;

        TFile* m_inGeoFile;
        TString m_geofileName;

        int m_nEvents;
        bool m_dropPixHit;
        TString m_LayerRegion;
        int m_nLayerThreshold;

        float m_lwrEtaLim;
        float m_uprEtaLim;
        

        // Reader for the tree
        std::map<TString, TTreeReader*> m_readerList;

        virtual void readGeo();


        // Maps to dynamically read the events
        std::map<TString, TTreeReaderArray<bool>*>   m_boolVectorVar;
        std::map<TString, TTreeReaderArray<int>*>    m_intVectorVar;
        std::map<TString, TTreeReaderArray<unsigned int>*> m_UintVectorVar;
        std::map<TString, TTreeReaderArray<unsigned long>*> m_UlongVectorVar;
        std::map<TString, TTreeReaderArray<unsigned long long>*> m_UlonglongVectorVar;
        std::map<TString, TTreeReaderArray<float>*>  m_floatVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<int>>*>     m_intVectorVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<double>>*>  m_doubleVectorVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<unsigned int>>*>  m_UintVectorVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<std::vector<int>>>*>  m_intVectorVectorVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<std::vector<unsigned int>>>*>  m_UintVectorVectorVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<std::vector<float>>>*>  m_floatVectorVectorVectorVar;
        std::map<TString, TTreeReaderArray<std::vector<std::vector<bool>>>*>  m_boolVectorVectorVectorVar;

        std::map<TString, TTreeReaderValue<int>*>    m_intVar;
        std::map<TString, TTreeReaderValue<float>*>  m_floatVar;
        std::map<TString, TTreeReaderValue<double>*> m_doubleVar;
        std::map<TString, TTreeReaderValue<unsigned int>*> m_UintVar;
        std::map<TString, TTreeReaderValue<unsigned long long>*> m_UlonglongVar;

        // Fill the maps above for a given reader
        void fillVariableList(TTreeReader* reader, TString prefix = "");

        // Readers
        std::shared_ptr<HitInfoATLAS> m_hitsReader;
        std::shared_ptr<RecoTracksATLAS> m_recoTracksReader;
        std::shared_ptr<TruthTracksATLAS> m_truthTracksReader;
        std::shared_ptr<SPInfoATLAS> m_SPReader;


        // store events for one event
        std::vector<std::shared_ptr<Hit>>       m_hitList;
        std::vector<std::shared_ptr<SpacePoint>>m_SPList;
        std::vector<std::shared_ptr<RecoTrack>> m_recoTrackList;
        std::vector<std::shared_ptr<TrueTrack>> m_trueTrackList;
        std::vector<std::shared_ptr<DetectorModule>> m_detectorModule;
        std::vector<std::shared_ptr<TrueTrack>> m_offlineTrackList;

        // Caching for easier transferring info
        std::map<Index, std::shared_ptr<Hit>>          m_hitCache;
        std::map<Index, std::shared_ptr<Hit>>          m_SPCache;
        std::map<Index, std::shared_ptr<Hit>>          m_HitIndexToSPCache;
        std::map<Index, std::vector<std::shared_ptr<Hit>>>          m_RecoTrackIndexToSPCache;
        std::map<Index, std::vector<std::shared_ptr<Hit>>>          m_TruthTrackIndexToSPCache;
        std::map<Index, std::shared_ptr<RecoTrack>>    m_recoTrackCache;
        std::map<Index, std::shared_ptr<TrueTrack>>    m_trueTrackCache;
        std::map<Index, std::shared_ptr<DetectorModule>>   m_detectorModuleCache;
        std::map<Index, std::shared_ptr<TrueTrack>> m_offlineTrackCache;

        void cleanTrackHits();

    private:
        void createSP(std::shared_ptr<TrackBase> trk);



};


#endif
