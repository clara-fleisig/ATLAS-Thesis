// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>

// Local includes
#include "TrackNNDataReader/DataReader.h"
#include "TrackNNDataReader/DataReaderHTT.h"
#include "TrackNNDataReader/DataReaderACTS.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/FakeManager.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
#include "TrackNNEvaluator/NNEvalUtils.h"
#include "TrackNNAnalysis/OutputManager.h"
#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"
#include "TrackNNAnalysis/PlotHolder.h"

#include "PathResolver/PathResolver.h"

// Root include
#include "TEfficiency.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"

using namespace std;


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
void setPrettyStuff();
std::vector<TString> tokenizeStr(TString str, TString key);
double getPtDependantCut(TString fileName, std::shared_ptr<RecoTrack> selectedTrk);
int main(int argc, char *argv[])
{   
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;


    // NN module
    auto NNFile = PathResolver::find_calib_file(opts["onnxFile"]);
    auto nnEval = make_shared<NNEvalUtils>(NNFile, atoi(opts["nHits"].c_str()), opts["hitType"] , opts["order"] , opts["scaled"] == "true");
    nnEval->initialize();

    std::shared_ptr<DataReader> dataReader;

    if(opts["doACTS"] == "true")
    {
        std::cout<<"Doing ACTS"<<std::endl;
        // ACTS reader module
        auto dataReaderInt = make_shared<DataReaderACTS>(opts.at("inputFile"), PathResolver::find_calib_file ("TrackNNAnalysis/DetectorGeo/DetectorGeo_ACTS_ODD.root"));
        dataReaderInt->setLayerRegion(opts["LayerRegion"]);
        dataReaderInt->setLwrPtCut(atof(opts["lwrPtLim"].c_str()));
        dataReaderInt->setUprPtCut(atof(opts["uprPtLim"].c_str()));
        dataReaderInt->initialize();
        if(opts["pileupOnly"] == "true") dataReaderInt->pileupOnly();
        if(opts["muonOnly"] == "true") dataReaderInt->muonOnly();
        dataReader = dynamic_pointer_cast<DataReader, DataReaderACTS>(dataReaderInt);
    }
    else
    {
        std::cout<<"Doing HTT"<<std::endl;
        // HTT reader module
        auto dataReaderInt = make_shared<DataReaderHTT>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
        dataReaderInt->setLwrPtCut(atof(opts["lwrPtLim"].c_str()));
        dataReaderInt->setUprPtCut(atof(opts["uprPtLim"].c_str()));
        dataReaderInt->setLayerRegion(opts["LayerRegion"]);
        dataReaderInt->initialize();
        if(opts["pileupOnly"] == "true") dataReaderInt->pileupOnly();
        if(opts["muonOnly"] == "true") dataReaderInt->muonOnly();
        dataReader = dynamic_pointer_cast<DataReader, DataReaderHTT>(dataReaderInt);
    }
    // Track plotter for debugging
    auto trackPlotter = make_shared<TrackGeoPlotter>(dataReader->getDetectorModules());
    trackPlotter->initialize();

    // Nevents
    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();

    vector<std::shared_ptr<TrackBase>> trkToPlot;
    vector<std::shared_ptr<RecoTrack>> trueToPlot;

    vector<double> nnCutList, nHWThresholdList;

    auto nnCutStr = tokenizeStr(opts["NNCutVal"], ",");
    auto HWThresStr = tokenizeStr(opts["nHitOverlapThres"], ",");

    for(const auto& val: nnCutStr) nnCutList.push_back(atof(val));
    for(const auto& val: HWThresStr) nHWThresholdList.push_back(atof(val));

    // Helper class to holder all the plots
    PlotHolder* plots = new PlotHolder();
    
    for(const auto& nnCut: nnCutList)
    {
        cout<<"NN: "<<nnCut<<endl;
        for(const auto& nHitOverlap: nHWThresholdList)
        {
            cout<<"nHitOverlap: "<<nHitOverlap<<endl;
            plots->createHists(nnCut, nHitOverlap);
        }  
    }

    // Match threshild
    double matchThreshold = 0.5;

    for(int i = 0; i < nEvents; i++)
    {
        if(i%100 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        auto recoTrack = dataReader->getRecoTracks();


        std::vector<std::shared_ptr<TrueTrack>> truthTracks;
             if(opts["trackType"] == "truth")   truthTracks = dataReader->getTrueTracks();
        else if(opts["trackType"] == "offline") truthTracks = dataReader->getOfflineTruthTracks();
        else
        {
            cout<<"Track type not recognized"<<endl;
            exit(1);
        }

        //////////////////////////////////////////////////////////////////////
        // Eval the NN score
        //////////////////////////////////////////////////////////////////////
        for(auto& trk: recoTrack) nnEval->evalTrack(trk);

        for(const auto& nnCut: nnCutList)
        {
            for(const auto& nHitOverlap: nHWThresholdList)
            {
                plots->setKey(nnCut, nHitOverlap);


                // To init the counters
                plots->addToCounter("TotalReco", 0);
                plots->addToCounter("SelectedReco", 0);
                plots->addToCounter("UnmatchedReco", 0);
                plots->addToCounter("OverlapRemovedReco", 0);
                plots->addToCounter("UnmatchedOverlapRemovedReco", 0);
                plots->addToCounter("TotalTruth", 0);
                plots->addToCounter("TruthMatched", 0);
                plots->addToCounter("TruthMatchedSelected", 0);




                // Total numbers of tracks
                map<int, std::vector<std::shared_ptr<RecoTrack>>> tracksInRoads;

                //////////////////////////////////////////////////////////////////////
                // Performance: inclusive
                //////////////////////////////////////////////////////////////////////
                // Compute the score for all the track and organize them into their roads
                int nRawTracks = 0;
                int nSelTracks = 0;
                int nOverlapTracks = 0;
                for(auto& trk: recoTrack)
                { 
                    
                    // cout<<"------------------------------------- raw track "<<endl;
                    // trk->printDebug();

                    plots->addToCounter("TotalReco", 1);
                    nRawTracks += 1;

                    plots->Fill("AllDisc",          trk->getNNScore());
                    plots->Fill("AllTrackProb",     trk->getTrackProbability());
                    plots->Fill("AllTrackCompProb", trk->getComputedTrackProbability());
                    plots->Fill("RecoAllPt",        trk->getPt());
                    tracksInRoads[trk->getIntMetadata("roadnumber")].push_back(trk);

                    // Check if the current selected track is truth matched to any tracks
                    bool matched = false;
                    for(auto& truthTrack: truthTracks)
                    {
                        auto barcode = truthTrack->getBarcode();
                        double cScore = trk->getMatchedProbability(barcode);
                        if(cScore > matchThreshold)
                        {
                            matched = true;
                            break;
                        }
                    }
                    if(!matched) plots->addToCounter("UnmatchedTotal", 1);
                }

                //////////////////////////////////////////////////////////////////////
                // NN Cut - pick the best NN track in each road, and apply a min NN cut
                //////////////////////////////////////////////////////////////////////
                // The selected tracks
                map<int, std::shared_ptr<RecoTrack>> selectedTrack;
                map<std::shared_ptr<RecoTrack>, int> selectedTrackReverseMap;
                vector<std::shared_ptr<RecoTrack>> selectedTrackVec;
                
                for(const auto& road: tracksInRoads)
                {
                    std::shared_ptr<RecoTrack> selectedTrk;
                    double NNScore = -1;
                    bool hasTrack = false;

                    // Select the track with the best score
                    for(const auto& trk: road.second)
                    {
                        if(trk->getNNScore() > NNScore)
                        {
                            selectedTrk = trk;
                            NNScore = trk->getNNScore();
                        }
                    }

                    double cNN = nnCut;
                    if(cNN < 0) cNN = getPtDependantCut(TString(opts["outputFile"]), selectedTrk);

                    // Apply the cut on the NN Score
                    if(NNScore < cNN) continue;
                    
                    // cout<<"------------------------------------- selectedTrk track "<<endl;
                    // selectedTrk->printDebug();

                    selectedTrack[road.first] = selectedTrk;
                    selectedTrackReverseMap[selectedTrk] = road.first;
                    selectedTrackVec.push_back(selectedTrk);

                    plots->Fill("nTracksPerRoad", road.second.size());
                }


                //////////////////////////////////////////////////////////////////////
                // Performance: After NN Cut 
                //////////////////////////////////////////////////////////////////////
                for(const auto& trk: selectedTrack)
                {
                    plots->addToCounter("SelectedReco", 1);
                    nSelTracks += 1;

                    plots->Fill("SelTrackProb",     trk.second->getTrackProbability());
                    plots->Fill("SelTrackCompProb", trk.second->getComputedTrackProbability());
                    plots->Fill("SelDisc",          trk.second->getNNScore());


                    plots->Fill("RecoSelectedPt",   trk.second->getPt());

                    // Truth Matching and see if there is something
                    bool matched = false;
                    for(auto& truthTrack: truthTracks)
                    {
                        auto barcode = truthTrack->getBarcode();
                        double cScore = trk.second->getMatchedProbability(barcode);
                        if(cScore > matchThreshold)
                        {
                            matched = true;
                            break;
                        }
                    }
                    if(!matched) plots->addToCounter("UnmatchedReco", 1);
                }


                //////////////////////////////////////////////////////////////////////
                // HW track
                //////////////////////////////////////////////////////////////////////
                // Order the track by their NN score
                sort(selectedTrackVec.begin(), selectedTrackVec.end(), 
                [](const std::shared_ptr<RecoTrack> & a, const std::shared_ptr<RecoTrack> & b) -> bool
                { 
                    return a->getNNScore() > b->getNNScore(); 
                });

                // cout<<"Selected tracks" <<endl;

                std::set<int> indexToRemove;

                for(int i = 0; i < selectedTrackVec.size(); i++)
                {
                    // This has already been removed, don't worry about it
                    if(indexToRemove.find(i) != indexToRemove.end()) continue;

                    for(int j = i+1; j < selectedTrackVec.size(); j++)
                    {
                        if(nHitOverlap >= 0)
                        {
                            auto nOverlapHits = selectedTrackVec[j]->getNHitOverlap(selectedTrackVec[i]);
                            if(nOverlapHits >= nHitOverlap)
                            {
                                // Since it is already sorted, just reject jth track
                                indexToRemove.insert(j);
                            }
                        }
                        else
                        {
                            auto nOverlapHits = selectedTrackVec[j]->getNHits() - selectedTrackVec[j]->getNHitOverlap(selectedTrackVec[i]);
                            if(nOverlapHits <= fabs(nHitOverlap))
                            {
                                // Since it is already sorted, just reject jth track
                                indexToRemove.insert(j);
                            }
                        }
                    }
                }

                // Reset the selected tracks and add all that haven't been rejected
                selectedTrack.clear();
                for(int i = 0; i < selectedTrackVec.size(); i++)
                {
                    if(indexToRemove.find(i) != indexToRemove.end()) continue;
                    selectedTrack[selectedTrackReverseMap[selectedTrackVec[i]]] = selectedTrackVec[i];
                }

                //////////////////////////////////////////////////////////////////////
                // Performance: After HW step
                //////////////////////////////////////////////////////////////////////
                map<Index, int> nRecoPerTruth;
                for(auto& truthTrack: truthTracks) nRecoPerTruth[truthTrack->getBarcode()] = 0;

                for(const auto& trk: selectedTrack)
                {
                    plots->addToCounter("OverlapRemovedReco", 1);
                    nOverlapTracks += 1;
                    plots->Fill("RecoSelectedOverlapPt", trk.second->getPt());

                    // trk.second->printDebug();

                    // Check if the current selected track is truth matched to any tracks
                    bool matched  = false;
                    double mScore = -1;
                    int mBarcode  = -2;
                    for(auto& truthTrack: truthTracks)
                    {
                        auto barcode = truthTrack->getBarcode();
                        double cScore = trk.second->getMatchedProbability(barcode);
                        if((cScore > matchThreshold) && (cScore > mScore) )
                        {
                            matched = true;
                            mScore = cScore;
                            mBarcode = barcode;
                        }
                    }
                    nRecoPerTruth[mBarcode]++;

                    if(!matched)
                    {
                        plots->addToCounter("UnmatchedOverlapRemovedReco", 1);
                        plots->Fill("unMatchedDisc",        trk.second->getNNScore());
                        plots->Fill("unMatchedCompProb",    trk.second->getComputedTrackProbability());
                        plots->Fill("unMatchedPt",          trk.second->getPt());
                    }
                }


                // Number of reco tracks for each track
                for(const auto& var: nRecoPerTruth)
                {
                    if(var.first >= 0) plots->Fill("nRecoPerTruth", var.second+0.01); 
                    else               plots->Fill("nRecoPerTruth", var.first, var.second); 
                }

                // Max number of hits
                for(const auto& trk1: selectedTrack)
                {
                    int maxShare = 0;
                    for(const auto& trk2: selectedTrack)
                    {
                        if(trk1.second == trk2.second) continue;
                        int cShare = trk1.second->getNHitOverlap(trk2.second);
                        if(cShare > maxShare)
                        {
                            maxShare = cShare;
                        }
                    }
                   plots->Fill("nMaxSharedHits", maxShare); 
                }

                //////////////////////////////////////////////////////////////////////
                // Performance: Truth tracks being compared to the reco one
                //////////////////////////////////////////////////////////////////////
                // Calculate stuff for true efficiency
                for(auto& truthTrack: truthTracks)
                {            
                    auto barcode = truthTrack->getBarcode();
                    // truthTrack->printDebug();

                    // Reject particles from Geant but not the overlay
                    if((opts["doACTS"] != "true") && (barcode > 200000) && (barcode < 4000000)) continue;
                    // truthTrack->printDebug();

                    plots->addToCounter("TotalTruth", 1);
                    plots->Fill("TruthAllPt",       truthTrack->getPt());
                    plots->Fill("TruthAllZ0",       truthTrack->getZ0());
                    plots->Fill("TruthAllD0",       truthTrack->getD0());
                    plots->Fill("TruthAllEta",      truthTrack->getEta());
                    plots->Fill("TruthAllPhi",      truthTrack->getPhi());
                    plots->Fill("TruthAllCharge",   truthTrack->getIntMetadata("charge"));
                    // plots->Fill("TruthAllNMeas",    truthTrack->getIntMetadata("nMeas"));
                    // plots->Fill("TruthAllNOutlier", truthTrack->getIntMetadata("nOutlier"));
                    // plots->Fill("TruthAllNOther",   truthTrack->getIntMetadata("nOther"));
                    plots->Fill("TruthAllTruthProb",truthTrack->getComputedTrackProbability());

                    //////////////////////////////////////////
                    // Best matched track with selected tracks
                    //////////////////////////////////////////
                    double probMatch = 0;
                    std::shared_ptr<RecoTrack> matchTrk;

                    // Do the reco match which selected tracks
                    for(auto& recoTrk: selectedTrack)
                    {
                        // Don't match 
                        if(!recoTrk.second->isMatched(barcode, matchThreshold)) continue;

                        double cScore = recoTrk.second->getMatchedProbability(barcode);
                        if(cScore == 0) continue;
                        if(cScore > probMatch)
                        {
                            probMatch = cScore;
                            matchTrk  = recoTrk.second;
                        }
                        else if((cScore == probMatch) && (recoTrk.second->getNNScore() > matchTrk->getNNScore()) && (probMatch > 0))
                        {
                            probMatch = cScore;
                            matchTrk  = recoTrk.second;
                        }
                    }


                    if(probMatch > matchThreshold)
                    {
                        plots->addToCounter("TruthMatchedSelected", 1);
                        plots->Fill("TruthMatchedSelectedPt",       truthTrack->getPt());
                        plots->Fill("TruthMatchedSelectedZ0",       truthTrack->getZ0());
                        plots->Fill("TruthMatchedSelectedD0",       truthTrack->getD0());
                        plots->Fill("TruthMatchedSelectedEta",      truthTrack->getEta());
                        plots->Fill("TruthMatchedSelectedPhi",      truthTrack->getPhi());
                        plots->Fill("TruthMatchedSelectedCharge",   truthTrack->getIntMetadata("charge"));
                        // plots->Fill("TruthMatchedSelectedNMeas",    truthTrack->getIntMetadata("nMeas"));
                        // plots->Fill("TruthMatchedSelectedNOutlier", truthTrack->getIntMetadata("nOutlier"));
                        // plots->Fill("TruthMatchedSelectedNOther",   truthTrack->getIntMetadata("nOther"));
                        plots->Fill("TruthMatchedSelectedTruthProb",truthTrack->getComputedTrackProbability());
                    }



                    //////////////////////////////////////////
                    // Best matched track with all the track
                    //////////////////////////////////////////
                    double allProbMatch = -1;
                    std::shared_ptr<RecoTrack> matchTrkAll;
                    for(auto& recoTrk: recoTrack)
                    {
                        // Don't match 
                        if(!recoTrk->isMatched(barcode, matchThreshold)) continue;
                        double cScore = recoTrk->getMatchedProbability(barcode);
                        if(cScore == 0) continue;
                        if(cScore > allProbMatch)
                        {
                            allProbMatch = cScore;
                            matchTrkAll  = recoTrk;
                        }
                        else if((cScore == allProbMatch) && (recoTrk->getNNScore() > matchTrkAll->getNNScore()) && (allProbMatch > 0))
                        {
                            allProbMatch = cScore;
                            matchTrkAll  = recoTrk;
                        }
                    }


                    if(allProbMatch > matchThreshold)
                    {
                        plots->addToCounter("TruthMatched", 1);
                        plots->Fill("TruthMatchedPt",       truthTrack->getPt());
                        plots->Fill("TruthMatchedZ0",       truthTrack->getZ0());
                        plots->Fill("TruthMatchedD0",       truthTrack->getD0());
                        plots->Fill("TruthMatchedEta",      truthTrack->getEta());
                        plots->Fill("TruthMatchedPhi",      truthTrack->getPhi());
                        plots->Fill("TruthMatchedCharge",   truthTrack->getIntMetadata("charge"));
                        // plots->Fill("TruthMatchedNMeas",    truthTrack->getIntMetadata("nMeas"));
                        // plots->Fill("TruthMatchedNOutlier", truthTrack->getIntMetadata("nOutlier"));
                        // plots->Fill("TruthMatchedNOther",   truthTrack->getIntMetadata("nOther"));
                        plots->Fill("TruthMatchedTruthProb",truthTrack->getComputedTrackProbability());
                    }
                    else if(!matchTrkAll)
                    {
                        // cout<<"Can't find reco track for"<<endl;
                        // truthTrack->printDebug();
                    }
                    else
                    {
                        // cout<<"didn't match due to probability score: "<<allProbMatch<<endl;
                        // truthTrack->printDebug();
                        // if(matchTrkAll) matchTrkAll->printDebug();
                    }



                    if(matchTrkAll && !matchTrk)
                    {
                        //cout<<"----------------------------------------------------------"<<endl;
                        //cout<<"For truth track found match with NNScore - "<<endl;
                        //if(matchTrk) cout<<matchTrk->getNNScore()<<" and match prob: "<<probMatch<<endl;
                        //truthTrack->printDebug();
                        //if(matchTrk) matchTrk->printDebug();
                        //if(matchTrkAll) cout<<"However, the best matched track - "<<matchTrkAll->getNNScore()<<" and match prob: "<<allProbMatch<<endl;
                        //if(matchTrkAll) matchTrkAll->printDebug();

                        //if(allProbMatch > 0.99)
                        //{
                        //    // trkToPlot.push_back(dynamic_pointer_cast<TrackBase, RecoTrack>(matchTrkAll));
                        //    // trueToPlot.push_back(matchTrkAll);
                        //}
                        //cout<<"----------------------------------------------------------"<<endl;       
                    // if(matchTrkAll) trkToPlot.push_back(dynamic_pointer_cast<TrackBase, RecoTrack>(matchTrkAll));
                    // if(matchTrkAll) trueToPlot.push_back(matchTrkAll);
                    // if(matchTrk) trueToPlot.push_back(matchTrk);

                    }
                    // if(matchTrk) trkToPlot.push_back(dynamic_pointer_cast<TrackBase, RecoTrack>(matchTrk));
                    // if(matchTrk) trueToPlot.push_back(matchTrk);;

                }

                plots->Fill("nTracksPerEventRaw",               nRawTracks);
                plots->Fill("nTracksPerEventNNSel",             nSelTracks);
                plots->Fill("nTracksPerEventNNSelOverlap",      nOverlapTracks);

                plots->Fill("nTracksPerEventRawNorm",           nRawTracks - truthTracks.size());
                plots->Fill("nTracksPerEventNNSelNorm",         nSelTracks - truthTracks.size());
                plots->Fill("nTracksPerEventNNSelOverlapNorm",  nOverlapTracks - truthTracks.size());
                plots->Fill("nTracksOffline",                   truthTracks.size());
            }
        }

    }


    for(const auto& nnCut: nnCutList)
    {
        for(const auto& nHitOverlap: nHWThresholdList)
        {
            plots->setKey(nnCut, nHitOverlap);
            cout<<"---------------------------------------------"<<endl;
            cout<<"nnCut: "<<nnCut<<" nHitOverlap: "<<nHitOverlap<<endl;
            cout<<"Total Reco:                                    "<<plots->getCounter("TotalReco")<<endl;
            cout<<"Selected Reco:                                 "<<plots->getCounter("SelectedReco")<<endl;
            cout<<"Unmatched Reco:                                "<<plots->getCounter("UnmatchedReco")<<endl;
            cout<<"Selected & Overlap Reco:                       "<<plots->getCounter("OverlapRemovedReco")<<endl;
            cout<<"Unmatched & Overlap Reco:                      "<<plots->getCounter("UnmatchedOverlapRemovedReco")<<endl;
            cout<<"Total Truth:                                   "<<plots->getCounter("TotalTruth")<<endl;
            cout<<"Matched Truth:                                 "<<plots->getCounter("TruthMatched")<<endl;
            cout<<"Matched & SelectedTruth:                       "<<plots->getCounter("TruthMatchedSelected")<<endl;

            plots->getHist("counterHist")->SetBinContent(1, plots->getCounter("TotalReco"));
            plots->getHist("counterHist")->SetBinContent(2, plots->getCounter("SelectedReco"));
            plots->getHist("counterHist")->SetBinContent(3, plots->getCounter("UnmatchedReco"));
            plots->getHist("counterHist")->SetBinContent(4, plots->getCounter("OverlapRemovedReco"));
            plots->getHist("counterHist")->SetBinContent(5, plots->getCounter("UnmatchedOverlapRemovedReco"));
            plots->getHist("counterHist")->SetBinContent(6, plots->getCounter("TotalTruth"));
            plots->getHist("counterHist")->SetBinContent(7, plots->getCounter("TruthMatched"));
            plots->getHist("counterHist")->SetBinContent(8, plots->getCounter("TruthMatchedSelected"));
            plots->getHist("counterHist")->SetBinContent(10, nEvents);

            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(1, "TotalReco");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(2, "SelectedReco");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(3, "UnmatchedReco");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(4, "OverlapRemovedReco");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(5, "UnmatchedOverlapRemovedReco");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(6, "TotalTruth");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(7, "TruthMatched");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(8, "TruthMatchedSelected");
            plots->getHist("counterHist")->GetXaxis()->SetBinLabel(10, "NEvents");
        }
    }

    TFile* outFile = new TFile(TString(opts["outputFile"]), "recreate");
    for(const auto& nnCut: nnCutList)
    {
        for(const auto& nHitOverlap: nHWThresholdList)
        {
            plots->setKey(nnCut, nHitOverlap);

            TString key = plots->getKey(nnCut, nHitOverlap);
            outFile->mkdir(key);
            outFile->cd(key);
            auto histList = plots->getAllHist(nnCut, nHitOverlap);
            for(auto& hist: histList) hist->Write();
            outFile->cd("");
        }
    }
    outFile->Close();


}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";

    // NN information
    opts["onnxFile"]        = "TrackNNAnalysis/ONNXModels/DNN_HTTfakes_SingleP_RecoTruth_v1.onnx";
    opts["NNCutVal"]        = "0.2";

    // Overlap settings
    opts["nHitOverlapThres"]= "5";


    // Which tracks to use
    opts["pileupOnly"]        = "false";
    opts["muonOnly"]        = "false";
    opts["trackType"]       = "truth";
    
    opts["nHits"]           = "8";    
    opts["lwrPtLim"]        = "1";
    opts["uprPtLim"]        = "10000";

    opts["scaled"]          = "false";
    opts["hitType"]         = "Nominal";
    opts["order"]           = "XX0YY0ZZ0";

    
    // Region info    
    opts["LayerRegion"]  = "r1";

    // Overwrite to do ACTS reader
    opts["doACTS"]  = "false";

    for(int i = 1; i < argc; ++i)
    {
        string opt=argv[i];
        if(opt=="--help" || opt == "--options" || opt =="--h")
        {
            cout<< "Options menu \n ============================================== \n" << endl;
            cout<<"--inputFile          : Path to the input file to process"<< endl;
            cout<<"--outputFile         : Path to the output file"<<endl;
            cout<<"--detectorGeoFile    : Path to the file that contains the detector geometry information"<<endl;
            cout<<"--nEvents            : Number of events to run over (-1 is all)"<<endl;
            cout<<"--outputFile         : Name of the output file"<<endl;
            cout<<"--hitType            : Track type (Nominal vs RotatedToZero)"<<endl;
            cout<<"--scaled             : use scaling"<<endl;
            cout<<"--order              : order or input data (XX0YY0ZZ0 vs XXYYZZ000)"<<endl;
            return false;
 
        }

        if(0!=opt.find("--")) {
            cout<<"ERROR: options start with '--'!"<<endl;
            cout<<"ERROR: options is: "<<opt<<endl;
            return false;
        }
        opt.erase(0,2);

        if (opt == "pileupOnly") 
        {
            opts["pileupOnly"] = "true";
            continue;
        }
        if (opt == "muonOnly") 
        {
            opts["muonOnly"] = "true";
            continue;
        }
        if (opt == "doACTS") 
        {
            opts["doACTS"] = "true";
            continue;
        }
        if (opt == "scaled") 
        {
            opts["scaled"] = "true";
            continue;
        }

        string nxtopt=argv[i+1];
        if(0==nxtopt.find("--")||i+1>=argc) {
            cout<<"ERROR: option '"<<opt<<"' requires value!"<<endl;
            return false;
        }
        if(opts.find(opt)!=opts.end())  opts[opt]=nxtopt;
        else
        {
            cout<<"ERROR: invalid option '"<<opt<<"'!"<<endl;
            return false;
        }

        i++;
    }

    return true;
}
void setPrettyStuff()
{
    SetAtlasStyle();
    Int_t ci = 1754; // color index
    vector<TColor*> TColors;
    new TColor(ci, 62/255.,    153/255.,    247/255.); //54
    ci++;

    new TColor(ci, 0.8500,    0.3250,    0.0980); //57
    ci++;          
    new TColor(ci, 0.4940,    0.1840,    0.5560); //58
    ci++;
    new TColor(ci, 0.9290,    0.6940,    0.1250); //59
    ci++; 
    new TColor(ci, 0.4660,    0.6740,    0.1880); //60
    ci++;
    new TColor(ci, 0.3010,    0.7450,    0.9330); //61
    ci++;
    new TColor(ci, 0,    0.4470,    0.7410); //56
    ci++;
    new TColor(ci, 254/255., 139/255., 113/255.); //55
    ci++;
    new TColor(ci, 28.0/255 , 0.0/255 , 19.0/255);
    ci++;
    new TColor(ci, 255.0/255 , 102.0/255 , 165.0/255);
    ci++;
    new TColor(ci, 50.0/255 , 134.0/255 , 255.0/255);
    ci++;
    new TColor(ci, 112.0/255 , 0.0/255 , 88.0/255);
    ci++;
    new TColor(ci, 1.0/255 , 237.0/255 , 171.0/255);
    ci++;
}

std::vector<TString> tokenizeStr(TString str, TString key)
{
    TObjArray *Varparts = str.Tokenize(key);
    vector<TString> varNameVec;
    if(Varparts->GetEntriesFast()) {
        TIter iString(Varparts);
        TObjString* os=0;
        while ((os=(TObjString*)iString())) {
            varNameVec.push_back(os->GetString());
        }
    }
    return varNameVec;
}

double getPtDependantCut(TString name, std::shared_ptr<RecoTrack> selectedTrk)
{
    static int errCount = 0;
    double cNN = 0;
    if(name.Contains("DNN_Region_0p1_0p3") && name.Contains("8L") && name.Contains("Nom"))
    {
             if(selectedTrk->getPt() > 20)  cNN = 0.75;
        else if(selectedTrk->getPt() > 10)  cNN = 0.001;
        else if(selectedTrk->getPt() >  5)  cNN = 0.025;
        else if(selectedTrk->getPt() >  2)  cNN = 0.2;
        else if(selectedTrk->getPt() > 1.5) cNN = 0.025;
        else                                cNN = 0.75;
    }
    else if(name.Contains("DNN_Region_0p1_0p3") && name.Contains("8L") && name.Contains("Stubs"))
    {
             if(selectedTrk->getPt() > 20)  cNN = 0.75;
        else if(selectedTrk->getPt() > 10)  cNN = 0.10;
        else if(selectedTrk->getPt() >  5)  cNN = 0.60;
        else if(selectedTrk->getPt() >  2)  cNN = 0.60;
        else if(selectedTrk->getPt() > 1.5) cNN = 0.75;
        else                                cNN = 0.75;                    
    }
    else if(name.Contains("DNN_Region_0p7_0p9") && name.Contains("8L") && name.Contains("Nom"))
    {
             if(selectedTrk->getPt() > 20)  cNN = 0.00001;
        else if(selectedTrk->getPt() > 10)  cNN = 0.0025;
        else if(selectedTrk->getPt() >  5)  cNN = 0.025;
        else if(selectedTrk->getPt() >  2)  cNN = 0.025;
        else if(selectedTrk->getPt() > 1.5) cNN = 0.001;
        else                                cNN = 0.75;  
    }
    else if(name.Contains("DNN_Region_0p7_0p9") && name.Contains("8L") && name.Contains("Stubs"))
    {
             if(selectedTrk->getPt() > 20)  cNN = 0.005;
        else if(selectedTrk->getPt() > 10)  cNN = 0.01;
        else if(selectedTrk->getPt() >  5)  cNN = 0.2;
        else if(selectedTrk->getPt() >  2)  cNN = 0.4;
        else if(selectedTrk->getPt() > 1.5) cNN = 0.75;
        else                                cNN = 0.5;                    
    }
    else if(name.Contains("DNN_Region_2p0_2p2") && name.Contains("8L") && name.Contains("Nom"))
    {
             if(selectedTrk->getPt() > 20)  cNN = 0.01;
        else if(selectedTrk->getPt() > 10)  cNN = 0.75;
        else if(selectedTrk->getPt() >  5)  cNN = 0.3;
        else if(selectedTrk->getPt() >  2)  cNN = 0.4;
        else if(selectedTrk->getPt() > 1.5) cNN = 0.4;
        else                                cNN = 0.1;  
    }
    else if(name.Contains("DNN_Region_2p0_2p2") && name.Contains("8L") && name.Contains("Stubs"))
    {
             if(selectedTrk->getPt() > 20)  cNN = 0.01;
        else if(selectedTrk->getPt() > 10)  cNN = 0.6;
        else if(selectedTrk->getPt() >  5)  cNN = 0.1;
        else if(selectedTrk->getPt() >  2)  cNN = 0.1;
        else if(selectedTrk->getPt() > 1.5) cNN = 0.2;
        else                                cNN = 0.2;                      
    }
    else
    {
        errCount++;
        if(errCount < 100) cout<<"PT dependant cut asked, but it is not supported, returning 0"<<endl;
    }
    return cNN;
}
