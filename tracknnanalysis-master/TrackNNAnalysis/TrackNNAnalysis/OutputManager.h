#ifndef _OUTPUTMANAGER_H
#define _OUTPUTMANAGER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TRandom3.h>

enum class OutHitType 
{ 
    Nominal,
    RotatedToZero,
    RoughRotatedToZero,
    SP,
    SPWithRotatedToZero,
    SPWithRoughRotatedToZero,
    SPWithUnscaledRotatedToZero,
    SPCylindrical,
    SPCylindricalRotatedToZero
    
};


// Base class 
class OutputManager 
{
    public:
        OutputManager(TString outFileName, int trackSize);
        virtual ~OutputManager();   

        void finalize();


        void setDoComputedTrackProb(bool set = true){m_doComputedTrackProb = set;};
        void setHitType(OutHitType outHitType)
        {
            m_hitType = outHitType;
                 if(m_hitType == OutHitType::SPCylindrical) m_outputCylindrical = true;
            else if(m_hitType == OutHitType::SPCylindricalRotatedToZero) m_outputCylindrical = true;
            else m_outputCylindrical = false;
        };

        void saveTrueTruthTracks(std::vector<std::shared_ptr<TrueTrack>> trackList);
        void saveTrueTracks(std::vector<std::shared_ptr<RecoTrack>> trackList);
        void saveExtrapolatedFakeTracks(std::vector<std::shared_ptr<ExtrapolatedTrack>> trackList);
        void saveExtrapolatedTrueTracks(std::vector<std::shared_ptr<ExtrapolatedTrack>> trackList);
        void saveFakeTracks(std::vector<std::shared_ptr<FakeTrack>> trackList, TString fakeName);
        void saveHTTFakeTracks(std::vector<std::shared_ptr<RecoTrack>> trackList);
        void saveHTTTrueTracks(std::vector<std::shared_ptr<RecoTrack>> trackList);
        void saveTruthTrainingData(std::vector<std::shared_ptr<TrueTrack>> trackList, int window_size, bool barrelOnly, bool endcapOnly, bool cylindrical=false);



    protected:
        // Variables to store the info
        TFile* m_inFile;
        TString m_fileName;
        bool m_outputCylindrical;

        std::map<TString, TTree*> m_treeList;

        TTree* getTree(TString name);

        std::vector<float>* m_hitX;
        std::vector<float>* m_hitY;
        std::vector<float>* m_hitZ;
        std::vector<float>* m_hitR;
        std::vector<int>*   m_hitPixorSct;
        std::vector<int>*   m_oneHotEncodedDetectorID;

        std::map<TString, float> m_floatVarList;
        std::map<TString, int>   m_intVarList;
        std::map<TString, std::vector<int>> m_vectorVarList;

        std::map<TString, float> m_targetFloatVarList;
        std::map<TString, int>   m_targetIntVarList;
        std::map<TString, std::vector<int>> m_targetVectorVarList;

        std::vector<int> m_barrelVolumes_ITK = {9,16,23};
        std::vector<int> m_endcapVolumes_ITK = {8,10,13,14,15,18,19,20,22,24,2,25};
        std::vector<int> m_volumes_ITK = {2,8,9,10,13,14,15,16,18,19,20,22,23,24,25};
        

        
        int m_isTrue;
        int m_PixOrSCT;
        double m_pt;
        double m_d0;
        double m_z0;
        double m_eta;
        double m_phi;
        double m_truthProb;
        bool m_doComputedTrackProb;
        OutHitType m_hitType;


        std::vector<std::shared_ptr<Hit>> getHits(std::shared_ptr<TrackBase> trk);
        void fillBaseHitInfo(std::vector<std::shared_ptr<Hit>> hits);
        void fillBaseHitInfo(std::vector<std::shared_ptr<Hit>> inputHits, std::shared_ptr<Hit> targetHit, bool cylindrical=false);
        void fillHitsWithSubsample(std::vector<std::shared_ptr<Hit>> hitList, TTree* currTree);
        TRandom3* m_randGen;

};


#endif
