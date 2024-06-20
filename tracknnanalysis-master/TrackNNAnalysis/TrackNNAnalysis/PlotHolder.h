#ifndef _PLOTHOLDER_H
#define _PLOTHOLDER_H
// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>
#include <set>

// Root include
#include "TEfficiency.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TFile.h"
#include "TKey.h"
#include "TROOT.h"
#include "TProfile.h"
#include "TObjString.h"

using namespace std;


class PlotHolder
{
    public:
        PlotHolder(){};
        ~PlotHolder(){};

        inline virtual void createHists(double NNCut, int HWThreshold)
        {
            TString key = "_" + getKey(NNCut, HWThreshold);

            m_histList["AllDisc"+key]             = new TH1F("AllDisc"+key,             "AllDisc"+key,             50, 0, 1.1);
            m_histList["AllTrackProb"+key]        = new TH1F("AllTrackProbability"+key, "AllTrackProbability"+key, 55, 0, 1.1);
            m_histList["AllTrackCompProb"+key]    = new TH1F("AllTrackCompProb"+key,    "AllTrackCompProb"+key,    55, 0, 1.1);

            m_histList["SelDisc"+key]             = new TH1F("SelDisc"+key,             "SelDisc"+key,             50, 0, 1.1);
            m_histList["SelTrackProb"+key]        = new TH1F("SelTrackProbability"+key, "SelTrackProbability"+key, 55, 0, 1.1);
            m_histList["SelTrackCompProb"+key]    = new TH1F("SelTrackCompProb"+key,    "SelTrackCompProb"+key,    55, 0, 1.1);

            m_histList["nTracksPerRoad"+key]      = new TH1F("nTracksPerRoad"+key,      "nTracksPerRoad"+key, 100, 0, 100);

            m_histList["nTracksPerEventRaw"+key]                = new TH1F("nTracksPerEventRaw"+key,                "nTracksPerEventRaw"+key,               100010, -10, 100000);
            m_histList["nTracksPerEventNNSel"+key]              = new TH1F("nTracksPerEventNNSel"+key,              "nTracksPerEventNNSel"+key,             100010, -10, 100000);
            m_histList["nTracksPerEventNNSelOverlap"+key]       = new TH1F("nTracksPerEventNNSelOverlap"+key,       "nTracksPerEventNNSelOverlap"+key,      100010, -10, 100000);

            m_histList["nTracksPerEventRawNorm"+key]            = new TH1F("nTracksPerEventRawNorm"+key,            "nTracksPerEventRawNorm"+key,           100010, -10, 100000);
            m_histList["nTracksPerEventNNSelNorm"+key]          = new TH1F("nTracksPerEventNNSelNorm"+key,          "nTracksPerEventNNSelNorm"+key,         100010, -10, 100000);
            m_histList["nTracksPerEventNNSelOverlapNorm"+key]   = new TH1F("nTracksPerEventNNSelOverlapNorm"+key,   "nTracksPerEventNNSelOverlapNorm"+key,  100010, -10, 100000);

            m_histList["nTracksOffline"+key]    = new TH1F("nTracksOffline"+key,    "nTracksOffline"+key,   100, 0, 100);
            m_histList["nRecoPerTruth"+key]     = new TH1F("nRecoPerTruth"+key,     "nRecoPerTruth"+key,    103, -3, 100);
            m_histList["nMaxSharedHits"+key]    = new TH1F("nMaxSharedHits"+key,    "nMaxSharedHits"+key,   10, 0, 10);

            vector <double> bins = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 60, 70, 80, 90, 100, 10000};
            m_histList["RecoAllPt"+key]                 = new TH1F("RecoAllPt"+key,             "RecoAllPt"+key,                bins.size()-1, &bins[0]);
            m_histList["RecoSelectedPt"+key]            = new TH1F("RecoSelectedPt"+key,        "RecoSelectedPt"+key,           bins.size()-1, &bins[0]);
            m_histList["RecoSelectedOverlapPt"+key]     = new TH1F("RecoSelectedOverlapPt"+key, "RecoSelectedOverlapPt"+key,    bins.size()-1, &bins[0]);

            m_histList["TruthAllPt"+key]                = new TH1F("TruthAllPt"+key,             "TruthAllPt"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedPt"+key]    = new TH1F("TruthMatchedSelectedPt"+key, "TruthMatchedSelectedPt"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPt"+key]            = new TH1F("TruthMatchedPt"+key,         "TruthMatchedPt"+key,          bins.size()-1, &bins[0]);

            m_histList["unMatchedDisc"+key]         = new TH1F("unMatchedDisc"+key,         "unMatchedDisc"+key,        50, 0, 1);
            m_histList["unMatchedCompProb"+key]     = new TH1F("unMatchedCompProb"+key,     "unMatchedCompProb"+key,    55, 0, 1.1);
            m_histList["unMatchedPt"+key]           = new TH1F("unMatchedPt"+key,           "unMatchedPt"+key,          bins.size()-1, &bins[0]);

            m_histList["counterHist"+key]   = new TH1F("counterHist"+key, "counterHist"+key, 20, 0, 20);

            bins = getBins(60, -300, 300);
            m_histList["TruthAllZ0"+key]                = new TH1F("TruthAllZ0"+key,             "TruthAllZ0"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedZ0"+key]    = new TH1F("TruthMatchedSelectedZ0"+key, "TruthMatchedSelectedZ0"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedZ0"+key]            = new TH1F("TruthMatchedZ0"+key,         "TruthMatchedZ0"+key,          bins.size()-1, &bins[0]);

            bins = getBins(50, -5, 5);
            m_histList["TruthAllD0"+key]                = new TH1F("TruthAllD0"+key,             "TruthAllD0"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedD0"+key]    = new TH1F("TruthMatchedSelectedD0"+key, "TruthMatchedSelectedD0"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedD0"+key]            = new TH1F("TruthMatchedD0"+key,         "TruthMatchedD0"+key,          bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.14, 3.14);
            m_histList["TruthAllPhi"+key]                = new TH1F("TruthAllPhi"+key,             "TruthAllPhi"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedPhi"+key]    = new TH1F("TruthMatchedSelectedPhi"+key, "TruthMatchedSelectedPhi"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPhi"+key]            = new TH1F("TruthMatchedPhi"+key,         "TruthMatchedPhi"+key,          bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.00, 3.00);
            m_histList["TruthAllEta"+key]                = new TH1F("TruthAllEta"+key,             "TruthAllEta"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedEta"+key]    = new TH1F("TruthMatchedSelectedEta"+key, "TruthMatchedSelectedEta"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedEta"+key]            = new TH1F("TruthMatchedEta"+key,         "TruthMatchedEta"+key,          bins.size()-1, &bins[0]);

            bins = {-2, -1, 0, 1, 2};
            m_histList["TruthAllCharge"+key]             = new TH1F("TruthAllCharge"+key,             "TruthAllCharge"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedCharge"+key] = new TH1F("TruthMatchedSelectedCharge"+key, "TruthMatchedSelectedCharge"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedCharge"+key]         = new TH1F("TruthMatchedCharge"+key,         "TruthMatchedCharge"+key,          bins.size()-1, &bins[0]);

            bins = getBins(20, 0, 20);
            m_histList["TruthAllNMeas"+key]             = new TH1F("TruthAllNMeas"+key,             "TruthAllNMeas"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedNMeas"+key] = new TH1F("TruthMatchedSelectedNMeas"+key, "TruthMatchedSelectedNMeas"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNMeas"+key]         = new TH1F("TruthMatchedNMeas"+key,         "TruthMatchedNMeas"+key,          bins.size()-1, &bins[0]);

            bins = getBins(20, 0, 20);
            m_histList["TruthAllNOutlier"+key]             = new TH1F("TruthAllNOutlier"+key,             "TruthAllNOutlier"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedNOutlier"+key] = new TH1F("TruthMatchedSelectedNOutlier"+key, "TruthMatchedSelectedNOutlier"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNOutlier"+key]         = new TH1F("TruthMatchedNOutlier"+key,         "TruthMatchedNOutlier"+key,          bins.size()-1, &bins[0]);

            bins = getBins(20, 0, 20);
            m_histList["TruthAllNOther"+key]             = new TH1F("TruthAllNOther"+key,             "TruthAllNOther"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedNOther"+key] = new TH1F("TruthMatchedSelectedNOther"+key, "TruthMatchedSelectedNOther"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNOther"+key]         = new TH1F("TruthMatchedNOther"+key,         "TruthMatchedNOther"+key,          bins.size()-1, &bins[0]);


            bins = getBins(100, 0, 1);
            m_histList["TruthAllTruthProb"+key]             = new TH1F("TruthAllTruthProb"+key,             "TruthAllTruthProb"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedSelectedTruthProb"+key] = new TH1F("TruthMatchedSelectedTruthProb"+key, "TruthMatchedSelectedTruthProb"+key,  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedTruthProb"+key]         = new TH1F("TruthMatchedTruthProb"+key,         "TruthMatchedTruthProb"+key,          bins.size()-1, &bins[0]);

        };


        inline TString getKey(double NNCut, int HWThreshold)
        {
            return TString(getNNKey(NNCut) + "_" + getHWKey(HWThreshold));
        };


        inline void setKey(double NNCut, int HWThreshold)
        {
            m_defaultKey = getKey(NNCut, HWThreshold);
        };

        inline void Fill(TString histName, float val, float weight = 1)
        {
            getHist(histName)->Fill(val, weight);
        };

        inline void Fill2d(TString histName, float val, float weight = 1)
        {
            get2dHist(histName)->Fill(val, weight);
        };

        inline TH1F* getHist(TString histName)
        {
            //cout<<histName + "_" + m_defaultKey << endl;
            //for (auto& h: m_histList) cout<<h.first<<endl;
            return m_histList.at(histName + "_" + m_defaultKey);
        };

        inline TH2F* get2dHist(TString histName)
        {
            //cout<<histName + "_" + m_defaultKey << endl;
            //for (auto& h: m_histList) cout<<h.first<<endl;
            return m_2dhistList.at(histName + "_" + m_defaultKey);
        };

        inline vector<TH1F*> getAllHist(double NNCut, int HWThreshold)
        {
            TString key = getKey(NNCut, HWThreshold);
            vector<TH1F*> histList;
            for(const auto& hist: m_histList)
            {
                if(hist.first.Contains(key)) histList.push_back(hist.second);
            }
            return histList;
        };

        inline vector<TH2F*> getAll2dHist(double NNCut, int HWThreshold)
        {
            TString key = getKey(NNCut, HWThreshold);
            vector<TH2F*> histList;
            for(const auto& hist: m_2dhistList)
            {
                if(hist.first.Contains(key)) histList.push_back(hist.second);
            }
            return histList;
        };

        inline map<TString, TH1F*> getHistMap(double NNCut, int HWThreshold)
        {
            TString key = getKey(NNCut, HWThreshold);
            //cout<<"Looking for key: "<<key<<endl;
            map<TString, TH1F*> histList;
            for(const auto& hist: m_histList)
            {
                if(hist.first.Contains(key))
                {
                    TString histName = hist.first;
                    histName = histName.ReplaceAll("_" + key, "");
                    histList[histName] = hist.second;
                } 
            }
            return histList;
        };

        inline map<TString, TH2F*> get2dHistMap(double NNCut, int HWThreshold)
        {
            TString key = getKey(NNCut, HWThreshold);
            // cout<<"Looking for key: "<<key<<endl;
            map<TString, TH2F*> histList;
            for(const auto& hist: m_2dhistList)
            {
                if(hist.first.Contains(key))
                {
                    TString histName = hist.first;
                    histName = histName.ReplaceAll("_" + key, "");
                    // cout<<histName<<endl;
                    histList[histName] = hist.second;
                } 
            }
            return histList;
        };

        inline void addToCounter(TString key, float val)
        {
            m_counter[key + "_" + m_defaultKey] += val;
        };

        inline float getCounter(TString key)
        {
            if(m_counter.find(key + "_" + m_defaultKey) == m_counter.end())
            {
                std::cout<<"can't find key "<<key + "_" + m_defaultKey<<" in the counter map"<<std::endl;

            }
            return m_counter.at(key + "_" + m_defaultKey);
        };

        inline void readHist(TString fileName)
        {
            TFile* cFile = TFile::Open(fileName);

            TIter next(cFile->GetListOfKeys());
            TKey *key;
            while ((key = (TKey*)next())) 
            {
                TString keyName(key->GetName());
                TClass *cl = gROOT->GetClass(key->GetClassName());
                if (!cl->InheritsFrom("TDirectoryFile")) continue;
                // cout<<keyName<<endl;
                // Read hist and values from the folder
                readFromFolder((TDirectoryFile*) cFile->Get(key->GetName()));
            }
            delete cFile;

        };

        inline void readFromFolder(TDirectoryFile* obj)
        {
            TIter next(obj->GetListOfKeys());
            TKey *key;
            // Reading the hist
            while ((key = (TKey*)next())) 
            {
                TClass *cl = gROOT->GetClass(key->GetClassName());
                if (!(cl->InheritsFrom("TH1F")) || (cl->InheritsFrom("TH2F"))) continue;
                // cout<<key->GetName()<<endl;

                // Store the hist
                m_histList[key->GetName()] = (TH1F*) obj->Get(key->GetName());
                m_histList[key->GetName()]->SetDirectory(0);
            }
        };

        inline std::set<float> getNNList(int HWThreshold)
        {
            TString HWKey = getHWKey(HWThreshold);

            std::set<float> nnCutVal;

            for(const auto& hist: m_histList)
            {
                if(!hist.first.Contains(HWKey)) continue;
                TObjArray* Varparts = hist.first.Tokenize("_");
                vector<TString> varNameVec;
                if(Varparts->GetEntriesFast()) {
                    TIter iString(Varparts);
                    TObjString* os=0;
                    while ((os=(TObjString*)iString())) {
                        varNameVec.push_back(os->GetString());
                    }
                }

                TString nnStr = varNameVec[varNameVec.size() - 3];
                nnCutVal.insert(atof(nnStr.Data()));
            }

            return nnCutVal;
        };

        inline vector<double> getBins(int nBins, double lowerLim, double upperLim)
        {
            vector<double> bins;
            double delta = (upperLim - lowerLim)/nBins;
            for(int i = 0; i < nBins+1; i++)
            {
                bins.push_back(lowerLim + i * delta);
            }
            return bins;
        }




    protected:
        map<TString, TH1F*>         m_histList;
        map<TString, TH2F*>         m_2dhistList;
        map<TString, float>         m_counter;

        TString m_defaultKey;

        inline virtual TString getHWKey(int HWThreshold) 
        {
            return Form("HW_%i", HWThreshold);
        };
        inline virtual TString getNNKey(double NNCut) 
        {
            return Form("NN_%f", NNCut);
        };


};

class plotHolderACTSExtrapolation
{
    public:
        plotHolderACTSExtrapolation(){};
        ~plotHolderACTSExtrapolation(){};

        inline void createHists(double error, int minHits, double nnCut, int nnOverlapHits) {

            TString key = "_" + getKey(error, minHits, nnCut, nnOverlapHits);
            //std::cout<<key<<std::endl;
            //TString key = "";
            auto bins = getBins(20, 0, 20);
            m_histList["TruthAllNHitsPerTrack"+key]       = new TH1F("TruthAllNHitsPerTrack"+key,   "TruthAllNHitsPerTrack"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNHitsPerTrack"+key]   = new TH1F("TruthMatchedNHitsPerTrack"+key,   "TruthMatchedNHitsPerTrack"+key,    bins.size()-1, &bins[0]);

            bins = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 60, 70, 80, 90, 100, 10000};
            m_histList["TruthAllPt"+key]                 = new TH1F("TruthAllPt"+key,                   "TruthAllPt"+key,                   bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPt"+key]             = new TH1F("TruthMatchedPt"+key,               "TruthMatchedPt"+key,               bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedPt"+key]   = new TH1F("TruthMatchedNNSelectedPt"+key,     "TruthMatchedNNSelectedPt"+key,     bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapPt"+key]    = new TH1F("TruthMatchedNNOverlapPt"+key,      "TruthMatchedNNOverlapPt"+key,      bins.size()-1, &bins[0]);

            bins = getBins(60, -300, 300);
            m_histList["TruthAllZ0"+key]                = new TH1F("TruthAllZ0"+key,                "TruthAllZ0"+key,                   bins.size()-1, &bins[0]);
            m_histList["TruthMatchedZ0"+key]            = new TH1F("TruthMatchedZ0"+key,            "TruthMatchedZ0"+key,               bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedZ0"+key]  = new TH1F("TruthMatchedNNSelectedZ0"+key,  "TruthMatchedNNSelectedZ0"+key,     bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapZ0"+key]   = new TH1F("TruthMatchedNNOverlapZ0"+key,   "TruthMatchedNNOverlapZ0"+key,      bins.size()-1, &bins[0]);

            bins = getBins(50, -5, 5);
            m_histList["TruthAllD0"+key]                = new TH1F("TruthAllD0"+key,                "TruthAllD0"+key,                   bins.size()-1, &bins[0]);
            m_histList["TruthMatchedD0"+key]            = new TH1F("TruthMatchedD0"+key,            "TruthMatchedD0"+key,               bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedD0"+key]  = new TH1F("TruthMatchedNNSelectedD0"+key,  "TruthMatchedNNSelectedD0"+key,     bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapD0"+key]   = new TH1F("TruthMatchedNNOverlapD0"+key,   "TruthMatchedNNOverlapD0"+key,      bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.14, 3.14);
            m_histList["TruthAllPhi"+key]                = new TH1F("TruthAllPhi"+key,                  "TruthAllPhi"+key,                  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPhi"+key]            = new TH1F("TruthMatchedPhi"+key,              "TruthMatchedPhi"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedPhi"+key]  = new TH1F("TruthMatchedNNSelectedPhi"+key,    "TruthMatchedNNSelectedPhi"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapPhi"+key]   = new TH1F("TruthMatchedNNOverlapPhi"+key,     "TruthMatchedNNOverlapPhi"+key,     bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.00, 3.00);
            m_histList["TruthAllEta"+key]                = new TH1F("TruthAllEta"+key,                  "TruthAllEta"+key,                  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedEta"+key]            = new TH1F("TruthMatchedEta"+key,              "TruthMatchedEta"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedEta"+key]  = new TH1F("TruthMatchedNNSelectedEta"+key,    "TruthMatchedNNSelectedEta"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapEta"+key]   = new TH1F("TruthMatchedNNOverlapEta"+key,     "TruthMatchedNNOverlapEta"+key,     bins.size()-1, &bins[0]);

            m_histList["counterHist"+key]   = new TH1F("counterHist"+key, "counterHist"+key, 20, 0, 20);

            m_histList["nExtrapolatedTracks"+key]       = new TH1F("nExtrapolatedTracks"+key,      "nExtrapolatedTracks"+key, 10000, 0, 25000);
            m_histList["nTrueTracks"+key]               = new TH1F("nTrueTracks"+key,      "nTrueTracks"+key, 500, 0, 500);
            m_histList["diff_nExtrap_nDuplicate"+key]   = new TH1F("diff_nExtrap_nDuplicate"+key,      "diff_nExtrap_nDuplicate"+key, 500, 0, 500);
            m_histList["nDuplicateTracks"+key]          = new TH1F("nDuplicateTracks"+key,      "nDuplicateTracks"+key, 10000, 0, 10000);
            m_histList["nExtrapPerSeed"+key]            = new TH1F("nExtrapPerSeed"+key,      "nExtrapPerSeed"+key, 3000, 0, 3000);
            m_histList["AllDisc"+key]             = new TH1F("AllDisc"+key,             "AllDisc"+key,             50, 0, 1.1);

            m_histList["nTrueTracksPerEvent"+key]               = new TH1F("nTrueTracksPerEvent"+key,                   "nTrueTracksPerEvent"+key,                  100010, -10, 100000);
            m_histList["nTracksPerEvent"+key]                   = new TH1F("nTracksPerEvent"+key,                   "nTracksPerEvent"+key,                  100010, -10, 100000);
            m_histList["nTracksPerEventMatched"+key]            = new TH1F("nTracksPerEventMatched"+key,            "nTracksPerEventMatched"+key,           100010, -10, 100000);
            m_histList["nTracksPerEventMatchedNNSelected"+key]  = new TH1F("nTracksPerEventMatchedNNSelected"+key,  "nTracksPerEventMatchedNNSelected"+key, 100010, -10, 100000);
            m_histList["nTracksPerEventMatchedNNOverlap"+key]   = new TH1F("nTracksPerEventMatchedNNOverlap"+key,   "nTracksPerEventMatchedNNOverlap"+key,  100010, -10, 100000);
            m_histList["nTracksPerEventNNSelected"+key]         = new TH1F("nTracksPerEventNNSelected"+key,         "nTracksPerEventNNSelected"+key,        100010, -10, 100000);
            m_histList["nTracksPerEventNNOverlap"+key]          = new TH1F("nTracksPerEventNNOverlap"+key,          "nTracksPerEventNNOverlap"+key,         100010, -10, 100000);
        }

        inline void createHists(double error, int minHits) {

            TString key = "_" + getKey(error, minHits);
            //std::cout<<key<<std::endl;
            //TString key = "";
            auto bins = getBins(20, 0, 20);
            m_histList["TruthAllNHitsPerTrack"+key]       = new TH1F("TruthAllNHitsPerTrack"+key,   "TruthAllNHitsPerTrack"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNHitsPerTrack"+key]   = new TH1F("TruthMatchedNHitsPerTrack"+key,   "TruthMatchedNHitsPerTrack"+key,    bins.size()-1, &bins[0]);

            bins = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 60, 70, 80, 90, 100, 10000};
            m_histList["TruthAllPt"+key]                 = new TH1F("TruthAllPt"+key,                   "TruthAllPt"+key,                   bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPt"+key]             = new TH1F("TruthMatchedPt"+key,               "TruthMatchedPt"+key,               bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedPt"+key]   = new TH1F("TruthMatchedNNSelectedPt"+key,     "TruthMatchedNNSelectedPt"+key,     bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapPt"+key]    = new TH1F("TruthMatchedNNOverlapPt"+key,      "TruthMatchedNNOverlapPt"+key,      bins.size()-1, &bins[0]);

            bins = getBins(60, -300, 300);
            m_histList["TruthAllZ0"+key]                = new TH1F("TruthAllZ0"+key,                "TruthAllZ0"+key,                   bins.size()-1, &bins[0]);
            m_histList["TruthMatchedZ0"+key]            = new TH1F("TruthMatchedZ0"+key,            "TruthMatchedZ0"+key,               bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedZ0"+key]  = new TH1F("TruthMatchedNNSelectedZ0"+key,  "TruthMatchedNNSelectedZ0"+key,     bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapZ0"+key]   = new TH1F("TruthMatchedNNOverlapZ0"+key,   "TruthMatchedNNOverlapZ0"+key,      bins.size()-1, &bins[0]);

            bins = getBins(50, -5, 5);
            m_histList["TruthAllD0"+key]                = new TH1F("TruthAllD0"+key,                "TruthAllD0"+key,                   bins.size()-1, &bins[0]);
            m_histList["TruthMatchedD0"+key]            = new TH1F("TruthMatchedD0"+key,            "TruthMatchedD0"+key,               bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedD0"+key]  = new TH1F("TruthMatchedNNSelectedD0"+key,  "TruthMatchedNNSelectedD0"+key,     bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapD0"+key]   = new TH1F("TruthMatchedNNOverlapD0"+key,   "TruthMatchedNNOverlapD0"+key,      bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.14, 3.14);
            m_histList["TruthAllPhi"+key]                = new TH1F("TruthAllPhi"+key,                  "TruthAllPhi"+key,                  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPhi"+key]            = new TH1F("TruthMatchedPhi"+key,              "TruthMatchedPhi"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedPhi"+key]  = new TH1F("TruthMatchedNNSelectedPhi"+key,    "TruthMatchedNNSelectedPhi"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapPhi"+key]   = new TH1F("TruthMatchedNNOverlapPhi"+key,     "TruthMatchedNNOverlapPhi"+key,     bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.00, 3.00);
            m_histList["TruthAllEta"+key]                = new TH1F("TruthAllEta"+key,                  "TruthAllEta"+key,                  bins.size()-1, &bins[0]);
            m_histList["TruthMatchedEta"+key]            = new TH1F("TruthMatchedEta"+key,              "TruthMatchedEta"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNSelectedEta"+key]  = new TH1F("TruthMatchedNNSelectedEta"+key,    "TruthMatchedNNSelectedEta"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNNOverlapEta"+key]   = new TH1F("TruthMatchedNNOverlapEta"+key,     "TruthMatchedNNOverlapEta"+key,     bins.size()-1, &bins[0]);

            m_histList["counterHist"+key]   = new TH1F("counterHist"+key, "counterHist"+key, 20, 0, 20);

            m_histList["nExtrapolatedTracks"+key]       = new TH1F("nExtrapolatedTracks"+key,      "nExtrapolatedTracks"+key, 10000, 0, 25000);
            m_histList["nTrueTracks"+key]               = new TH1F("nTrueTracks"+key,      "nTrueTracks"+key, 500, 0, 500);
            m_histList["diff_nExtrap_nDuplicate"+key]   = new TH1F("diff_nExtrap_nDuplicate"+key,      "diff_nExtrap_nDuplicate"+key, 500, 0, 500);
            m_histList["nDuplicateTracks"+key]          = new TH1F("nDuplicateTracks"+key,      "nDuplicateTracks"+key, 10000, 0, 10000);
            m_histList["nExtrapPerSeed"+key]            = new TH1F("nExtrapPerSeed"+key,      "nExtrapPerSeed"+key, 3000, 0, 3000);
            m_histList["AllDisc"+key]             = new TH1F("AllDisc"+key,             "AllDisc"+key,             50, 0, 1);


            m_histList["nTracksPerEvent"+key]                   = new TH1F("nTracksPerEvent"+key,                   "nTracksPerEvent"+key,                  100010, -10, 100000);
            m_histList["nTracksPerEventMatched"+key]            = new TH1F("nTracksPerEventMatched"+key,            "nTracksPerEventMatched"+key,           100010, -10, 100000);
            m_histList["nTracksPerEventMatchedNNSelected"+key]  = new TH1F("nTracksPerEventMatchedNNSelected"+key,  "nTracksPerEventMatchedNNSelected"+key, 100010, -10, 100000);
            m_histList["nTracksPerEventMatchedNNOverlap"+key]   = new TH1F("nTracksPerEventMatchedNNOverlap"+key,   "nTracksPerEventMatchedNNOverlap"+key,  100010, -10, 100000);
            m_histList["nTracksPerEventNNSelected"+key]         = new TH1F("nTracksPerEventNNSelected"+key,         "nTracksPerEventNNSelected"+key,        100010, -10, 100000);
            m_histList["nTracksPerEventNNOverlap"+key]          = new TH1F("nTracksPerEventNNOverlap"+key,          "nTracksPerEventNNOverlap"+key,         100010, -10, 100000);
        }


        inline void createHists(float zUncertainty, float phiUncertainty, int minHits) {

            TString key = "_" + getKey(zUncertainty, phiUncertainty, minHits);
            //TString key = "";
            auto bins = getBins(20, 0, 20);
            m_histList["TruthAllNHitsPerTrack"+key]   = new TH1F("TruthAllNHitsPerTrack"+key,   "TruthAllNHitsPerTrack"+key,    bins.size()-1, &bins[0]);
            m_histList["TruthMatchedNHitsPerTrack"+key]   = new TH1F("TruthMatchedNHitsPerTrack"+key,   "TruthMatchedNHitsPerTrack"+key,    bins.size()-1, &bins[0]);

            bins = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 60, 70, 80, 90, 100, 10000};
            m_histList["TruthAllPt"+key]                 = new TH1F("TruthAllPt"+key,             "TruthAllPt"+key,                bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPt"+key]                 = new TH1F("TruthMatchedPt"+key,             "TruthMatchedPt"+key,                bins.size()-1, &bins[0]);

            bins = getBins(60, -300, 300);
            m_histList["TruthAllZ0"+key]                = new TH1F("TruthAllZ0"+key,             "TruthAllZ0"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedZ0"+key]            = new TH1F("TruthMatchedZ0"+key,         "TruthMatchedZ0"+key,          bins.size()-1, &bins[0]);

            bins = getBins(50, -5, 5);
            m_histList["TruthAllD0"+key]                = new TH1F("TruthAllD0"+key,             "TruthAllD0"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedD0"+key]            = new TH1F("TruthMatchedD0"+key,         "TruthMatchedD0"+key,          bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.14, 3.14);
            m_histList["TruthAllPhi"+key]                = new TH1F("TruthAllPhi"+key,             "TruthAllPhi"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedPhi"+key]            = new TH1F("TruthMatchedPhi"+key,         "TruthMatchedPhi"+key,          bins.size()-1, &bins[0]);

            bins = getBins(5000, -3.00, 3.00);
            m_histList["TruthAllEta"+key]                = new TH1F("TruthAllEta"+key,             "TruthAllEta"+key,              bins.size()-1, &bins[0]);
            m_histList["TruthMatchedEta"+key]            = new TH1F("TruthMatchedEta"+key,         "TruthMatchedEta"+key,          bins.size()-1, &bins[0]);

            m_histList["counterHist"+key]   = new TH1F("counterHist"+key, "counterHist"+key, 20, 0, 20);

        }

    // protected:

    //      Override to use different names/titles //

        inline TString getMinHits(int HWThreshold)
        {
            return Form("minNHits_%i", HWThreshold);
        };
        inline TString getUncertKey(double NNCut)
        {
            return Form("Uncertainty_%f", NNCut);
        };

        inline TString getZPhiUncertaintyKey(float zUncertainty, float phiUncertainty)
        {
            return Form("z_phi_Uncertainty_%f_%f", zUncertainty, phiUncertainty);
        };

        inline TString getKey(double uncertainty, int minNHits)
        {
            return TString(getUncertKey(uncertainty) + "_" + getMinHits(minNHits));
        };

        inline TString getKey_NN_HW(double NNCut, int HWThreshold)
        {
            return TString(getNNKey(NNCut) + "_" + getHWKey(HWThreshold));
        };

        inline TString getKey(double uncertainty, int minNHits, double nnOverlapCut, int nnOverlapHits)
        {
            return TString(getUncertKey(uncertainty) + "_" + getMinHits(minNHits) + "_" +getNNKey(nnOverlapCut) + "_" + getHWKey(nnOverlapHits));
        };
        
        inline TString getKey(float zUncertainty, float phiUncertainty, int minNHits)
        {
            return TString(getZPhiUncertaintyKey(zUncertainty, phiUncertainty) + "_" + getMinHits(minNHits));
        };

        inline void setKey(double uncertainty, int minNHits)
        {
            m_defaultKey = getKey(uncertainty, minNHits);
        };

        inline void setKey(double uncertainty, int minNHits, double nnOverlapCut, int nnOverlapHits)
        {
            m_defaultKey = getKey(uncertainty, minNHits, nnOverlapCut, nnOverlapHits);
        };

        inline void setKey(float zUncertainty, float phiUncertainty, int minNHits)
        {
            m_defaultKey = getKey(zUncertainty, phiUncertainty, minNHits);
        };

        inline map<TString, TH1F*> getHistMap(float zUncertainty, float phiUncertainty, int minNHits)
        {
            TString key = getKey(zUncertainty, phiUncertainty, minNHits);
            //cout<<"Looking for key: "<<key<<endl;
            map<TString, TH1F*> histList;
            for(const auto& hist: m_histList)
            {
                if(hist.first.Contains(key))
                {
                    TString histName = hist.first;
                    histName = histName.ReplaceAll("_" + key, "");
                    // cout<<histName<<endl;
                    histList[histName] = hist.second;
                } 
            }
            return histList;
        };

        inline map<TString, TH1F*> getHistMap(double NNCut, int HWThreshold)
        {
            TString key = getKey_NN_HW(NNCut, HWThreshold);
            //cout<<"Looking for key: "<<key<<endl;
            map<TString, TH1F*> histList;
            for(const auto& hist: m_histList)
            {   
                if(hist.first.Contains(key))
                {
                    TString histName = hist.first;
                    histName = histName.ReplaceAll("_" + key, "");
                    // cout<<histName<<endl;
                    histList[histName] = hist.second;
                } 
            }
            return histList;
        }

        inline map<TString, TH1F*> getHistMap(double uncertainty, int minNHits, double nnOverlapCut, int nnOverlapHits)
        {
            TString key = getKey(uncertainty, minNHits, nnOverlapCut, nnOverlapHits);
            // cout<<"Looking for key: "<<key<<endl;
            map<TString, TH1F*> histList;
            for(const auto& hist: m_histList)
            {
                if(hist.first.Contains(key))
                {
                    TString histName = hist.first;
                    histName = histName.ReplaceAll("_" + key, "");
                    // cout<<histName<<endl;
                    histList[histName] = hist.second;
                } 
            }
            return histList;
        }

        inline vector<TH1F*> getAllHist(double uncertainty, int minNHits)
        {
            TString key = getKey(uncertainty, minNHits);
            vector<TH1F*> histList;

            for(const auto& hist: m_histList)
            {
                //std::cout<<hist.first<<std::endl;
                if(hist.first.Contains(key)) histList.push_back(hist.second);
            }
            //for (auto& h: histList) std::cout<<h->GetName()<<std::endl;
            return histList;
        };

        inline vector<TH1F*> getAllHist(float zUncertainty, float phiUncertainty, int minNHits)
        {
            TString key = getKey(zUncertainty, phiUncertainty, minNHits);
            vector<TH1F*> histList;
            for(const auto& hist: m_histList)
            {
                if(hist.first.Contains(key)) histList.push_back(hist.second);
            }
            return histList;
        };

        inline vector<TH1F*> getAllHist(double uncertainty, int minNHits, double nnOverlapCut, int nnOverlapHits)
        {
            TString key = getKey(uncertainty, minNHits, nnOverlapCut, nnOverlapHits);
            vector<TH1F*> histList;

            for(const auto& hist: m_histList)
            {
                //std::cout<<hist.first<<std::endl;
                if(hist.first.Contains(key)) histList.push_back(hist.second);
            }
            //for (auto& h: histList) std::cout<<h->GetName()<<std::endl;
            return histList;
        };

        inline void Fill(TString histName, float val, float weight = 1)
        {
            getHist(histName)->Fill(val, weight);
        };
        inline vector<double> getBins(int nBins, double lowerLim, double upperLim)
        {
            vector<double> bins;
            double delta = (upperLim - lowerLim)/nBins;
            for(int i = 0; i < nBins+1; i++)
            {
                bins.push_back(lowerLim + i * delta);
            }
            return bins;
        }

        inline void addToCounter(TString key, float val)
        {
            m_counter[key + "_" + m_defaultKey] += val;
        };

        inline float getCounter(TString key)
        {
            if(m_counter.find(key + "_" + m_defaultKey) == m_counter.end())
            {
                std::cout<<"can't find key "<<key + "_" + m_defaultKey<<" in the counter map"<<std::endl;

            }
            return m_counter.at(key + "_" + m_defaultKey);
        };

        inline void readHist(TString fileName)
        {
            TFile* cFile = TFile::Open(fileName);

            TIter next(cFile->GetListOfKeys());
            TKey *key;
            while ((key = (TKey*)next())) 
            {
                TString keyName(key->GetName());
                TClass *cl = gROOT->GetClass(key->GetClassName());
                if (!cl->InheritsFrom("TDirectoryFile")) continue;
                // cout<<keyName<<endl;
                // Read hist and values from the folder
                readFromFolder((TDirectoryFile*) cFile->Get(key->GetName()));
            }
            delete cFile;

        };

        inline void readFromFolder(TDirectoryFile* obj)
        {
            TIter next(obj->GetListOfKeys());
            TKey *key;
            // Reading the hist
            while ((key = (TKey*)next())) 
            {
                TClass *cl = gROOT->GetClass(key->GetClassName());
                if (!(cl->InheritsFrom("TH1F")) || (cl->InheritsFrom("TH2F"))) continue;
                // cout<<key->GetName()<<endl;

                // Store the hist
                m_histList[key->GetName()] = (TH1F*) obj->Get(key->GetName());
                m_histList[key->GetName()]->SetDirectory(0);
            }
        };

        inline TH1F* getHist(TString histName)
        {
            //cout<<histName + "_" + m_defaultKey << endl;
            //for (auto& h: m_histList) cout<<h.first<<endl;
            return m_histList.at(histName + "_" + m_defaultKey);
        };

    protected:
        map<TString, TH1F*>         m_histList;
        map<TString, TH2F*>         m_2dhistList;
        map<TString, float>         m_counter;

        TString m_defaultKey;

        inline virtual TString getHWKey(int HWThreshold) 
        {
            return Form("HW_%i", HWThreshold);
        };
        inline virtual TString getNNKey(double NNCut) 
        {
            return Form("NN_%f", NNCut);
        };

};

class PlotHolderExtrap: public PlotHolder
{
    
    public:
        PlotHolderExtrap(){};
        ~PlotHolderExtrap(){};

        inline void createHists(TString modelType)
        {
            //TString key = "_" + getKey(NNCut, HWThreshold);
            TString key = "";
            m_histList["hitDistance"] = new TH1F("hitDistance", "Distance between true and predicted hit", 100, 0, 200);
            m_histList["hitDistance_rho"] = new TH1F("hitDistance_rho", "|#Delta #rho| between true and predicted hit", 100, 0, 100);
            m_histList["hitDistance_z"] = new TH1F("hitDistance_z", "|#Delta z| between true and predicted hit", 100, 0, 100);
            m_histList["hitDistance_phi"] = new TH1F("hitDistance_phi", "|#Delta #phi | between true and predicted hit", 50, 0, .1);

            m_histList["residuals_rho"] = new TH1F("residuals_rho", "#rho residuals", 100, -100, 100);
            m_histList["residuals_z"] = new TH1F("residuals_z", "z residuals", 100, -100, 100);
            m_histList["residuals_phi"] = new TH1F("residuals_phi", "#phi residuals", 50, -.1, .1);

            // TProfiles
            std::vector<double> bins = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 60, 70, 80, 90, 100, 10000};
            //std::vector<double> resphiBins = getBins(50, -.1, .1);
            //std::vector<double> reszBins = getBins(100, -100, 100);
            //std::vector<double> resrhoBins = getBins(100,-100,100);
            
            m_2dhistList["HitsWithinRange"] = new TH2F("HitsWithinRange","N Hits within Range;distance;nHits",200,0,100,30,0,30);
            
            // m_profileList["residualsPhi_vs_pT"]= new TProfile("residualsPhi_vs_pT", "#phi residuals vs. p_{T}",bins.size()-1, &bins[0], -.1, .1);
            // m_profileList["residualsZ_vs_pT"]= new TProfile("residualsZ_vs_pT", "z residuals vs. p_{T}",bins.size()-1, &bins[0], -100,100);
            // m_profileList["residualsRho_vs_pT"]= new TProfile("residualsRho_vs_pT", "#rho residuals vs. p_{T}",bins.size()-1, &bins[0], -100,100);

            // bins = getBins(50, -3,3);
            // m_profileList["residualsPhi_vs_eta"]= new TProfile("residualsPhi_vs_eta", "#phi residuals vs. #eta",bins.size()-1, &bins[0], -.1, .1);
            // m_profileList["residualsZ_vs_eta"]= new TProfile("residualsZ_vs_eta", "z residuals vs. #eta",bins.size()-1, &bins[0], -100,100);
            // m_profileList["residualsRho_vs_eta"]= new TProfile("residualsRho_vs_eta", "#rho residuals vs. #eta",bins.size()-1, &bins[0], -100,100);

            // bins = getBins(50, -5,5);
            // m_profileList["residualsPhi_vs_d0"]= new TProfile("residualsPhi_vs_d0", "#phi residuals vs. d_{0}",bins.size()-1, &bins[0], -.1, .1);
            // m_profileList["residualsZ_vs_d0"]= new TProfile("residualsZ_vs_d0", "z residuals vs. d_{0}",bins.size()-1, &bins[0], -100,100);
            // m_profileList["residualsRho_vs_d0"]= new TProfile("residualsRho_vs_d0", "#rho residuals vs. d_{0}",bins.size()-1, &bins[0], -100,100);

            // bins = getBins(200, -200,200);
            // m_profileList["residualsPhi_vs_z0"]= new TProfile("residualsPhi_vs_z0", "#phi residuals vs. z_{0}",bins.size()-1, &bins[0], -.1, .1);
            // m_profileList["residualsZ_vs_z0"]= new TProfile("residualsZ_vs_z0", "z residuals vs. z_{0}",bins.size()-1, &bins[0], -100,100);
            // m_profileList["residualsRho_vs_z0"]= new TProfile("residualsRho_vs_z0", "#rho residuals vs. z_{0}",bins.size()-1, &bins[0], -100,100);

            // bins = getBins(50, -3.14,3.14);
            // m_profileList["residualsPhi_vs_phi"]= new TProfile("residualsPhi_vs_phi", "#phi residuals vs. #phi",bins.size()-1, &bins[0], -.1, .1);
            // m_profileList["residualsZ_vs_phi"]= new TProfile("residualsZ_vs_phi", "z residuals vs. #phi",bins.size()-1, &bins[0], -100,100);
            // m_profileList["residualsRho_vs_phi"]= new TProfile("residualsRho_vs_phi", "#rho residuals vs. #phi",bins.size()-1, &bins[0], -100,100);

            ////////////// 
            // 2d hists to later make resolution vs var histograms
            m_2dhistList["residualsPhi_vs_pT"]= new TH2F("residualsPhi_vs_pT", "#phi residuals vs. p_{T}",bins.size()-1, &bins[0], 100,-0.1,0.1 );
            m_2dhistList["residualsZ_vs_pT"]=   new TH2F("residualsZ_vs_pT", "z residuals vs. p_{T}",     bins.size()-1, &bins[0], 100,-100,100 );
            m_2dhistList["residualsRho_vs_pT"]= new TH2F("residualsRho_vs_pT", "#rho residuals vs. p_{T}",bins.size()-1, &bins[0], 50,-50,50   );

            bins = getBins(50, -3,3);
            m_2dhistList["residualsPhi_vs_eta"]= new TH2F("residualsPhi_vs_eta", "#phi residuals vs. #eta",bins.size()-1, &bins[0], 100,-0.1,0.1);
            m_2dhistList["residualsZ_vs_eta"]=   new TH2F("residualsZ_vs_eta", "z residuals vs. #eta",     bins.size()-1, &bins[0], 100,-100,100);
            m_2dhistList["residualsRho_vs_eta"]= new TH2F("residualsRho_vs_eta", "#rho residuals vs. #eta",bins.size()-1, &bins[0], 50,-50,50 );

            bins = getBins(50, -5,5);
            m_2dhistList["residualsPhi_vs_d0"]= new TH2F("residualsPhi_vs_d0", "#phi residuals vs. d_{0}",bins.size()-1, &bins[0], 100,-0.1,0.1);
            m_2dhistList["residualsZ_vs_d0"]=   new TH2F("residualsZ_vs_d0", "z residuals vs. d_{0}",     bins.size()-1, &bins[0], 100,-100,100);
            m_2dhistList["residualsRho_vs_d0"]= new TH2F("residualsRho_vs_d0", "#rho residuals vs. d_{0}",bins.size()-1, &bins[0], 50,-50,50   );

            bins = getBins(100, -200,200);
            m_2dhistList["residualsPhi_vs_z0"]= new TH2F("residualsPhi_vs_z0", "#phi residuals vs. z_{0}",bins.size()-1, &bins[0], 100,-0.1,0.1);
            m_2dhistList["residualsZ_vs_z0"]=   new TH2F("residualsZ_vs_z0", "z residuals vs. z_{0}",     bins.size()-1, &bins[0], 100,-100,100);
            m_2dhistList["residualsRho_vs_z0"]= new TH2F("residualsRho_vs_z0", "#rho residuals vs. z_{0}",bins.size()-1, &bins[0], 50,-50,50   );

            bins = getBins(100, -3.14,3.14);
            m_2dhistList["residualsPhi_vs_phi"]= new TH2F("residualsPhi_vs_phi", "#phi residuals vs. #phi",bins.size()-1, &bins[0], 100,-0.1,0.1);
            m_2dhistList["residualsZ_vs_phi"]=   new TH2F("residualsZ_vs_phi", "z residuals vs. #phi",     bins.size()-1, &bins[0], 100,-100,100);
            m_2dhistList["residualsRho_vs_phi"]= new TH2F("residualsRho_vs_phi", "#rho residuals vs. #phi",bins.size()-1, &bins[0], 50,-50,50   );
            
            /////////////
            m_histList["predicted_z"] = new TH1F("predicted_z", "predicted z distribution", 100, -3000, 3000);
            m_histList["predicted_phi" ] = new TH1F("predicted_phi", "predicted #phi distribution", 100, -4, 4);
            m_histList["predicted_rho"] = new TH1F("predicted_rho", "predicted #rho distribution", 50, 0, 1200);

            m_histList["true_z"] = new TH1F("true_z", "true z distribution", 100, -3000, 3000);
            m_histList["true_phi" ] = new TH1F("true_phi", "true #phi distribution", 100, -4, 4);
            m_histList["true_rho"] = new TH1F("true_rho", "true #rho distribution", 50, 0, 1100);
            m_2dhistList["z0_vs_eta"] = new TH2F("eta_vs_z0", "z0_vs_eta", 50,-5,5,50,-3,3);
            
            m_histList["matchedHit_true_z"] = new TH1F("matchedHit_true_z", "true z distribution", 100, -2000, 2000);
            m_histList["matchedHit_true_phi" ] = new TH1F("matchedHit_true_phi", "true #phi distribution", 100, -4, 4);
            m_histList["matchedHit_true_rho"] = new TH1F("matchedHit_true_rho", "true #rho distribution", 50, 0, 1100);  
            m_histList["unmatchedHit_true_z"] =     new TH1F("unmatchedHit_true_z", "true z distribution", 100, -2000, 2000);
            m_histList["unmatchedHit_true_phi" ] =  new TH1F("unmatchedHit_true_phi", "true #phi distribution", 100, -4, 4);
            m_histList["unmatchedHit_true_rho"] =   new TH1F("unmatchedHit_true_rho", "true #rho distribution", 50, 0, 1100);     

            m_histList["matchedHitFrac"] = new TH1F("matchedHitFrac", "Fraction of predicted hits whos closest hit is the true hit"+ key, 2, 0, 2);

            m_histList["matchedHit_resZ"] = new TH1F("matchedHit_resZ", "z_{true}- z{pred} for matched hits", 100, -100, 100);
            m_histList["matchedHit_resPhi" ] = new TH1F("matchedHit_resPhi", "#phi_{true}- #phi_{pred} for matched hits", 50, -.1, .1);
            m_histList["matchedHit_resRho"] = new TH1F("matchedHit_resRho", "#rho_{true}- #rho_{pred} for matched hits", 100, -100, 100);
            m_histList["matchedHit_deltaR"] = new TH1F("matchedHit_deltaR", "#Delta #vec{r} for matched hits", 100, 0, 200);

            m_histList["unmatchedHit_resZ"] =       new TH1F("unmatchedHit_resZ", "z_{true}- z{pred} for matched hits", 100, -100, 100);
            m_histList["unmatchedHit_resPhi" ] =    new TH1F("unmatchedHit_resPhi", "#phi_{true}- #phi_{pred} for matched hits", 50, -.1, .1);
            m_histList["unmatchedHit_resRho"] =     new TH1F("unmatchedHit_resRho", "#rho_{true}- #rho_{pred} for matched hits", 100, -100, 100);
            m_histList["unmatchedHit_deltaR"] =     new TH1F("unmatchedHit_deltaR", "#Delta #vec{r} for matched hits", 100, 0, 200);
            m_2dhistList["Eta_vs_Z0"]   = new TH2F("Eta_vs_Z0", "Eta_vs_Z0", 100, -1000, 1000, 100, -3, 3);

            // m_histList["truthFraction"+key]      = new TH1F("truthFraction"+key,      "truthFraction"+key, 20, 0, 2);
            // m_histList["nTracksPerRoad"+key]      = new TH1F("nTracksPerRoad"+key,      "nTracksPerRoad"+key, 100, 0, 100);
            // m_histList["efficiency"+key]         = new TH1F("efficiency"+key,           "truth efficiency"+key, 20, 0, 2);
        };

        //override b.c extrapolation doesn't have an NNCut or HWThreshold
        inline map<TString, TH1F*> getHistMap()
        {
            return m_histList;
        };

        inline TH1F* getHist(TString histName)
        {
            return m_histList.at(histName);
        };

        inline void Fill(TString histName, float val1, float weight = 1)
        {
            getHist(histName)->Fill(val1, weight);
        };

        inline TProfile* getProfile(TString histName)
        {
            return m_profileList.at(histName);
        };

        inline std::map<TString, TProfile*> getProfileMap()
        {
            return m_profileList;
        };

        inline void FillProfile(TString histName, float val1, float val2, float weight = 1)
        {
            getProfile(histName)->Fill(val1, val2, weight);
        };

        inline TH2F* get2dHist(TString histName)
        {
            return m_2dhistList.at(histName);
        };

        inline void Fill2d(TString histName, float val, float weight = 1)
        {
            get2dHist(histName)->Fill(val, weight);
        };

        inline map<TString, TH2F*> get2dHistMap()
        {
            return m_2dhistList;
        };

    protected:
        map<TString, TProfile*>         m_profileList;


};

class HistogramBookkeeper
{
    private:
        typedef std::map<std::string,TH1*> coll_type;
        typedef std::map<std::string,TH2*> coll_type_2d;
        coll_type _histograms;
        coll_type_2d _histograms2d;
        
        void cleanup(){
            _histograms.clear();
        }

    public:

        HistogramBookkeeper(){}
        virtual ~HistogramBookkeeper() { cleanup(); }
        coll_type getHistograms() { return _histograms; }

        void Fill( const std::string name , const float& x,
            const UInt_t& nbins_x , const float& minx , const float& maxx
            ) 
        {
            coll_type::iterator i = _histograms.find(name);
            TH1F* h = 0;
            if( i==_histograms.end() ) {
                h = new TH1F( name.c_str() , name.c_str() , nbins_x , minx , maxx );
                assert(h);
                _histograms.insert( std::make_pair(name,h) );
            } 
            else {
                h = static_cast<TH1F*>(i->second);
                assert(h);
                assert(h->IsA()==TH1F::Class());
            }
            
            h->Fill(x);
        }

        void Fill( const std::string name , const float& x, const float& y,
            const UInt_t& nbins_x , const float& minx , const float& maxx,
            const UInt_t& nbins_y, const float& miny, const float& maxy
            ) 
        {
            coll_type::iterator i = _histograms.find(name);
            TH2F* h = 0;
            if( i==_histograms.end() ) {
                h = new TH2F( name.c_str() , name.c_str() , nbins_x , minx , maxx, nbins_y, miny, maxy);
                assert(h);
                _histograms.insert( std::make_pair(name,h) );
            } 
            else {
                h = static_cast<TH2F*>(i->second);
                assert(h);
                assert(h->IsA()==TH2F::Class());
            }
            
            h->Fill(x,y);
        }

        

};

#endif
