// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <thread>
#include <random>
#include <typeinfo>


// Local includes
#include "TrackNNDataReader/DataReaderITK.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNAnalysis/TrackManager.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
#include "TrackNNEvaluator/NNEvalClassUtils.h"
#include "TrackNNEvaluator/NNEvalUtils.h"
#include "TrackNNAnalysis/TrackFitter.h"

#include "TrackNNEDM/Hit.h"
#include "TrackNNAnalysis/PlotHolder.h"
#include "TrackNNAnalysis/OutputManager.h"

#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"

#include "PathResolver/PathResolver.h"

// Root include
#include "TEfficiency.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
std::vector<TString> tokenizeStr(TString str, TString key);
void extrapolateTracks(vector<shared_ptr<ExtrapolatedTrack>>& completeTracks, vector<shared_ptr<SeedTrack>>& seedTracks, 
    const shared_ptr<HitsManager> hitManager, const shared_ptr<NNEvalClassUtils> nnEval, const shared_ptr<TrackFitter> trackFitter,
    const shared_ptr<DataReaderITK> dataReader, int uncertainty, int minHits, std::string algoType);


void fillVector(vector<shared_ptr<ExtrapolatedTrack>>& storeTracks, vector<shared_ptr<ExtrapolatedTrack>>& completeTracks);

int main(int argc, char *argv[])
{
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;

     // NN Classifier module
    auto NNClassFile   = PathResolver::find_calib_file (opts["NNClassifierOnnxFile"]);
    auto nnClassEval = make_shared<NNEvalClassUtils>(NNClassFile, atoi(opts["windowSize"].c_str()));
    nnClassEval->initialize();


    // NN module
    auto NNOverlapFile = PathResolver::find_calib_file(opts["NNOverlapOnnxFile"]);
    auto nnOverlapEval = make_shared<NNEvalUtils>(NNOverlapFile, 10, "RotatedToZero", "XX0YY0ZZ0", false);
    nnOverlapEval->initialize();

    auto dataReader = make_shared<DataReaderITK>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    dataReader->setDeltaROverlap(atof(opts["deltaROverlap"].c_str()));

    auto trackPlotter = make_shared<TrackGeoPlotter>(dataReader->getDetectorModules()); 
    trackPlotter->initialize();

    if ((opts["pileupOnly"]=="true") && (opts["muonOnly"]=="true") ) 
    {
        std::cout<<"pileupOnly and muonOnly cannot both be true. Set one to false"<<std::endl;
        exit(1);
    }
    if (opts["pileupOnly"]=="true") 
        {
            dataReader->pileupOnly(true);
            dataReader->muonOnly(false);
        }
    else if (opts["muonOnly"]=="true") 
    {
        dataReader->muonOnly(true);
        dataReader->pileupOnly(false);
    }

    auto trackFitter = make_shared<TrackFitter>(atoi(opts["windowSize"].c_str()));

    cout<<"initialized datareader"<<endl;


    auto outputManager = make_shared<OutputManager>(opts.at("outputManagerFile"), 10);
    if(opts["hitType"].find("Nominal")              != std::string::npos) outputManager->setHitType(OutHitType::Nominal);
    if(opts["hitType"].find("RotatedToZero")        != std::string::npos) outputManager->setHitType(OutHitType::RotatedToZero);
    if(opts["hitType"].find("SP")                   != std::string::npos) outputManager->setHitType(OutHitType::SP);
    if(opts["hitType"].find("SPWithRotatedToZero")  != std::string::npos) outputManager->setHitType(OutHitType::SPWithRotatedToZero);
  
    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();
    
    int uncertainty = atoi(opts.at("uncertainty").c_str());
    int windowSize = atoi(opts["windowSize"].c_str());
    std::string LayerRegion = opts["LayerRegion"];
    std::string algoType = opts["algoType"];
    std::string saveOutputTracks = opts["saveOutputTracks"];
    dataReader->setLayerRegion(LayerRegion);

    vector<std::shared_ptr<TrackBase>> trkToPlot;
    vector<std::shared_ptr<RecoTrack>> trueToPlot;
    vector<double> uncertaintyList, nnCutList;
    vector<int> minNHitsList, nHWThresholdList;


    auto uncertaintyStr = tokenizeStr(opts["uncertainty"], ",");
    auto minHitsStr = tokenizeStr(opts["minHits"], ",");
    for(const auto& val: uncertaintyStr) uncertaintyList.push_back(atof(val));
    for(const auto& val: minHitsStr) minNHitsList.push_back(atof(val));
    auto minHits = minNHitsList.at(0);
    auto nnCutStr = tokenizeStr(opts["NNCutVal"], ",");
    auto HWThresStr = tokenizeStr(opts["nHitOverlapThres"], ",");

    for(const auto& val: nnCutStr) nnCutList.push_back(atof(val));
    for(const auto& val: HWThresStr) nHWThresholdList.push_back(atoi(val));

    // seedManaged to create the seeds
    auto seedManager = make_shared<SeedManager>();
    seedManager->setSeedSize(windowSize);
    seedManager->setWindowSize(windowSize);

    plotHolderACTSExtrapolation* plots = new plotHolderACTSExtrapolation();
    for (const auto& err: uncertaintyList)
    {
        for (const auto& minHit: minNHitsList)
        {
            for (const auto& nnCut: nnCutList)
            {
                for (const auto& HWThreshold: nHWThresholdList)
                {
                    plots->createHists(err, minHit, nnCut, HWThreshold);
                }
            }
        }
    }

    // TrackManager to create single-hit-per-layer tracks
    auto trackManager = make_shared<TrackManager>();

    float efficiencyNumerator = 0;
    float efficiencyDenom = 0;
    int nRoads = 0;
    int nThreads = atoi(opts["nThreads"].c_str());

    map<TString, shared_ptr<TrackBase>> tracksToPlot;
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        //auto recoTracks = dataReader->getRecoTracks();       
        auto recoTracks = dataReader->getOfflineTruthTracks();
        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());
        std::vector<std::shared_ptr<TrueTrack>> cleanTracks;
        if (LayerRegion =="barrel") cleanTracks = trackManager->getTrackSingleLayerHitInBarrel(recoTracks, minHits);
        else if(LayerRegion =="endcaps")  cleanTracks = trackManager->getTrackSingleLayerHitInEndcap(recoTracks, minHits);
        else  cleanTracks = trackManager->getTrackSingleLayerHit(recoTracks, minHits);
        //std::cout<<"number of tracks in event: "<<cleanTracks.size()<<std::endl;

        // for(auto trk: cleanTracks) trk->printDebug();
        auto seedTracks = seedManager->getSeedTracks(SeedType::InitialSeeds, cleanTracks);

        if (seedTracks.size() < 1) continue;

        std::vector<std::shared_ptr<ExtrapolatedTrack>> outputFakeTracks;
        std::vector<std::shared_ptr<ExtrapolatedTrack>> outputTrueTracks;

        for (const auto& err: uncertaintyList)
        {
            for (const auto& minHit: minNHitsList)
            {
                // To store fully found tracks
                std::vector<std::vector<std::shared_ptr<ExtrapolatedTrack>>> completeTracks;

                // For threading
                std::map<int, std::vector<std::shared_ptr<ExtrapolatedTrack>>> completedTracksMap;
                std::map<int, std::vector<std::shared_ptr<SeedTrack>>> subSeedMap;
                for(int i = 0; i < nThreads; i++)  completedTracksMap[i] = {};

                int nSeedsPerThread = floor(seedTracks.size() / nThreads);

                // to store threads
                std::vector<std::thread> threads;
                threads.reserve(nThreads);

                for(int i = 0; i < nThreads; i++)
                {
                    auto ptrStart = seedTracks.begin() + i * nSeedsPerThread;
                    auto ptrEnd   = seedTracks.begin() + (i+1) * nSeedsPerThread;
                    if(i + 1 == nThreads) ptrEnd = seedTracks.end();

                    std::vector<std::shared_ptr<SeedTrack>> v0 = {ptrStart, ptrEnd};
                    subSeedMap[i] = v0;
                    threads.push_back(std::thread(extrapolateTracks, std::ref(completedTracksMap[i]), std::ref(subSeedMap[i]), 
                        std::ref(hitManager), std::ref(nnClassEval), std::ref(trackFitter),std::ref(dataReader), err, minHit, algoType));
                }
                
                for(std::thread &t:threads)
                {
                    if (t.joinable()) t.join();
                }

                // move all the completed tracks to a single vector
                std::vector<std::shared_ptr<ExtrapolatedTrack>> allCompleteTracks;
                for(int i = 0; i < nThreads; i++)  fillVector(allCompleteTracks, completedTracksMap[i]);

                //////////////////////////////////////////////////////////////////////
                // Eval the NN score
                //////////////////////////////////////////////////////////////////////
                // for(auto& trk: allCompleteTracks)
                // {
                //     nnOverlapEval->evalTrack(trk);
                // }
                int nDuplicates = 0;
                //calculate the efficiency

                for (const auto& nnCut: nnCutList)
                {
                    for (const auto& HWThreshold: nHWThresholdList)
                    {
                        plots->setKey(err, minHit, nnCut, HWThreshold);

                        std::vector<std::shared_ptr<ExtrapolatedTrack>> NNSelectedTracks;
                        std::vector<std::shared_ptr<ExtrapolatedTrack>> NNOverlapTracks;

                        std::map<std::shared_ptr<SeedTrack>, std::shared_ptr<ExtrapolatedTrack>> selectedNNTracks;


                        //////////////////////////////////////////////////////////////////////
                        // HW only alog
                        //////////////////////////////////////////////////////////////////////

                        // for(const auto& trk: allCompleteTracks)
                        // {
                        //     plots->Fill("AllDisc", trk->getNNScore());
                        //     if(trk->getNNScore() < nnCut) continue; 
                        //     NNSelectedTracks.push_back (trk);
                        // }

                        // Order the track by their NN score
                        // sort(NNSelectedTracks.begin(), NNSelectedTracks.end(), 
                        // [](const std::shared_ptr<ExtrapolatedTrack> & a, const std::shared_ptr<ExtrapolatedTrack> & b) -> bool
                        // { 
                        //     return a->getNNScore() > b->getNNScore(); 
                        // });

                        // // HW algorithm
                        // std::set<int> indexToRemove;

                        // for(int i = 0; i < NNSelectedTracks.size(); i++)
                        // {
                        //     // This has already been removed, don't worry about it
                        //     if(indexToRemove.find(i) != indexToRemove.end()) continue;

                        //     for(int j = i+1; j < NNSelectedTracks.size(); j++)
                        //     {

                        //         auto nOverlapHits = NNSelectedTracks[j]->getNHitOverlap(NNSelectedTracks[i]);
                        //         if(nOverlapHits >= HWThreshold)
                        //         {
                        //             // Since it is already sorted, just reject jth track
                        //             indexToRemove.insert(j);
                        //         }
                        //     }
                        // }


                        // for(int i = 0; i < NNSelectedTracks.size(); i++)
                        // {
                        //     if(indexToRemove.find(i) != indexToRemove.end()) continue;
                        //     NNOverlapTracks.push_back (NNSelectedTracks[i]);

                        // }




                        /////////////////////////////////////
                        // This is like the TDR algo
                        /////////////////////////////////////

                        // For each seed, select the track with the highest NN score and above the cut
                        // for(auto& trk: allCompleteTracks)
                        // {
                        //     plots->Fill("AllDisc", trk->getNNScore());
                        //     // NN cut
                        //     if(trk->getNNScore() < nnCut) continue;
                            
                        //     auto seed = trk->getBaseSeedTrack();
                        //     if(selectedNNTracks.find(seed) == selectedNNTracks.end())
                        //     {
                        //         selectedNNTracks[seed] = trk;
                        //     }
                        //     else
                        //     {
                        //         if(trk->getNNScore() > selectedNNTracks[seed]->getNNScore())
                        //         {
                        //             selectedNNTracks[seed] = trk;
                        //         }
                        //     }
                        // }

                        // for(const auto& trk: selectedNNTracks) NNSelectedTracks.push_back (trk.second);

                        // // HW algorithm
                        // std::set<int> indexToRemove;

                        // for(int i = 0; i < NNSelectedTracks.size(); i++)
                        // {
                        //     // This has already been removed, don't worry about it
                        //     if(indexToRemove.find(i) != indexToRemove.end()) continue;

                        //     for(int j = i+1; j < NNSelectedTracks.size(); j++)
                        //     {

                        //         auto nOverlapHits = NNSelectedTracks[j]->getNHitOverlap(NNSelectedTracks[i]);
                        //         if(nOverlapHits >= HWThreshold)
                        //         {
                        //             // Since it is already sorted, just reject jth track
                        //             indexToRemove.insert(j);
                        //         }
                        //     }
                        // }


                        // for(int i = 0; i < NNSelectedTracks.size(); i++)
                        // {
                        //     if(indexToRemove.find(i) != indexToRemove.end()) continue;
                        //     NNOverlapTracks.push_back (NNSelectedTracks[i]);

                        // }


                        // To init the counters
                        plots->addToCounter("nTracks", 0);
                        plots->addToCounter("nMatchedTracks", 0);
                        plots->addToCounter("nMatchedNNSelectedTracks", 0);
                        plots->addToCounter("nMatchedNNOverlapTracks", 0);
                        plots->addToCounter("nRoads", 0);

                        //calculate the efficiency. Loop over seed tracks because not all layer tracks were used 
                        int nMatchedTracks = 0;
                        int nMatchedNNSelectedTracks = 0;
                        int nMatchedNNOverlapTracks = 0;
                        
                        for (auto& track: cleanTracks)
                        {
                            //cut on track length
                            if (track->getNHits() < minHit) continue;
                            //plots->Fill("AllDisc", track->getNNScore());
                            plots->Fill("TruthAllZ0", track->getZ0());
                            plots->Fill("TruthAllD0", track->getD0());
                            plots->Fill("TruthAllPt", track->getPt());
                            plots->Fill("TruthAllPhi", track->getPhi());
                            plots->Fill("TruthAllEta", track->getEta());
                            plots->Fill("TruthAllNHitsPerTrack", track->getNHits());

                            plots->addToCounter("nTracks", 1);
                            efficiencyDenom += 1;
                            Index recoId = track->getBarcode();

                            // find if any extrapolated tracks have at least 50% hits matching this track
                            
                            //////////////////////////////////////////////////////////////////////
                            // Inclusive perf
                            //////////////////////////////////////////////////////////////////////
                            bool hasAtleastOneTrack = false;
                            for (auto& extrapTrk: allCompleteTracks)
                            {
                                // consider only the hits after the seed
                                if (extrapTrk->getNHits() < minHit) continue;
                                bool isMatched = extrapTrk->isMatchedToReco(recoId, 0.5, 3);
                                if (isMatched)
                                {
                                    // std::cout<<recoId<<std::endl;
                    
                                    plots->addToCounter("nMatchedTracks", 1);
                                    hasAtleastOneTrack = true;
                                    
                                    nMatchedTracks++;
                                    efficiencyNumerator += 1;
                                    plots->Fill("TruthMatchedZ0",               track->getZ0());
                                    plots->Fill("TruthMatchedD0",               track->getD0());
                                    plots->Fill("TruthMatchedPt",               track->getPt());
                                    plots->Fill("TruthMatchedPhi",              track->getPhi());
                                    plots->Fill("TruthMatchedEta",              track->getEta());
                                    plots->Fill("TruthMatchedNHitsPerTrack",    track->getNHits());
                                    break;
                                }
                                else
                                {
                                    auto badTrack = extrapTrk->getBaseSeedTrack()->getBaseTrueTrack();
                                }
                            }

                            //////////////////////////////////////////////////////////////////////
                            // After NN Cut perf
                            //////////////////////////////////////////////////////////////////////

                            // bool matched=false;

                            // for (auto& extrapTrk: NNSelectedTracks)
                            // {
                            //     // consider only the hits after the seed
                            //     if (extrapTrk->getNHits() < minHit) continue;
                            //     bool isMatched = extrapTrk->isMatchedToReco(recoId, 0.5, 3);
                            //     if (isMatched)
                            //     {
                            //         plots->addToCounter("nMatchedNNSelectedTracks", 1);
                            //         nMatchedNNSelectedTracks++;
                            //         efficiencyNumerator += 1;
                            //         plots->Fill("TruthMatchedNNSelectedZ0",               track->getZ0());
                            //         plots->Fill("TruthMatchedNNSelectedD0",               track->getD0());
                            //         plots->Fill("TruthMatchedNNSelectedPt",               track->getPt());
                            //         plots->Fill("TruthMatchedNNSelectedPhi",              track->getPhi());
                            //         plots->Fill("TruthMatchedNNSelectedEta",              track->getEta());
                            //         matched=true;
                            //         break;
                            //     }
                            // }
                            // static int counter = 0;
                            // tracksToPlot.clear();
                            // if (!matched && hasAtleastOneTrack)
                            // {
                            //     std::cout<<"Missing -------------: "<<std::endl;
                            //     tracksToPlot["Atruth"]=dynamic_pointer_cast<TrackBase> (track);
                            //     track->printDebug();
                            //     //std::cout<<std::endl;

                            //     std::cout<<"All extrapolated: "<<std::endl;
                            //     int nTracs = 0;
                            //     for(const auto& trk: allCompleteTracks)
                            //     {
                            //         if(trk->isMatchedToReco(recoId, 0.5, 3))
                            //         {
                            //             auto t = trk;
                            //             tracksToPlot["Extrapolated " + TString::Itoa(nTracs, 10)] = t;
                            //             nTracs++;
                            //             t->printDebug();
                            //             // auto misMatchedBarcode = trk.second->getHitsList().at(3)->getTruthTrackBarcode().at(0);
                            //             // //std::cout<<"Mismatched Track barcode: "<<misMatchedBarcode<<" of type "<<typeid(misMatchedBarcode).name()<<std::endl;
                            //             // for(const auto& trueTrack: recoTracks)
                            //             // {
                            //             //     if(misMatchedBarcode == trueTrack->getBarcode())
                            //             //     {
                            //             //         //std::cout<<"true track barcode: "<<trueTrack->getBarcode()<<" of type "<<typeid(trueTrack->getBarcode()).name()<<std::endl;
                            //             //         //trueTrack->printDebug();
                            //             //         //tracksToPlot.push_back(dynamic_pointer_cast<TrackBase> (trueTrack));
                            //             //     }
                            //             // }                                        
                            //         }
                            //     }



                            //     // std::cout<<"Selected: "<<std::endl;
                            //     // for(const auto& trk: selectedNNTracks)
                            //     // {
                            //     //     if(trk.second->isMatchedToReco(recoId, 0.5, 3))
                            //     //     {
                            //     //         auto t = trk.second;
                            //     //         // tracksToPlot["selected"]=dynamic_pointer_cast<TrackBase> (t);
                            //     //         t->printDebug();
                            //     //         // auto misMatchedBarcode = trk.second->getHitsList().at(3)->getTruthTrackBarcode().at(0);
                            //     //         // //std::cout<<"Mismatched Track barcode: "<<misMatchedBarcode<<" of type "<<typeid(misMatchedBarcode).name()<<std::endl;
                            //     //         // for(const auto& trueTrack: recoTracks)
                            //     //         // {
                            //     //         //     if(misMatchedBarcode == trueTrack->getBarcode())
                            //     //         //     {
                            //     //         //         //std::cout<<"true track barcode: "<<trueTrack->getBarcode()<<" of type "<<typeid(trueTrack->getBarcode()).name()<<std::endl;
                            //     //         //         //trueTrack->printDebug();
                            //     //         //         //tracksToPlot.push_back(dynamic_pointer_cast<TrackBase> (trueTrack));
                            //     //         //     }
                            //     //         // }                                        
                            //     //     }
                            //     // }


                            //     trackPlotter->plotTracks(tracksToPlot, "NNSelectedTracks_" + TString::Itoa(counter, 10));
                            //     counter++;
                            //     // exit(1);    
                            // }

                            //////////////////////////////////////////////////////////////////////
                            // After NN Cut perf
                            //////////////////////////////////////////////////////////////////////
                            // for (auto& extrapTrk: NNOverlapTracks)
                            // {
                            //     // consider only the hits after the seed
                            //     if (extrapTrk->getNHits() < minHit) continue;
                            //     bool isMatched = extrapTrk->isMatchedToReco(recoId, 0.5, 3);
                            //     if (isMatched)
                            //     {
                            //         plots->addToCounter("nMatchedNNOverlapTracks", 1);
                            //         nMatchedNNOverlapTracks++;
                            //         efficiencyNumerator += 1;
                            //         plots->Fill("TruthMatchedNNOverlapZ0",               track->getZ0());
                            //         plots->Fill("TruthMatchedNNOverlapD0",               track->getD0());
                            //         plots->Fill("TruthMatchedNNOverlapPt",               track->getPt());
                            //         plots->Fill("TruthMatchedNNOverlapPhi",              track->getPhi());
                            //         plots->Fill("TruthMatchedNNOverlapEta",              track->getEta());
                            //         break;
                            //     }
                            // }
                        }

                        plots->Fill("nTracksPerEvent", allCompleteTracks.size());
                        //plots->Fill("nTracksPerEventNNSelected", NNSelectedTracks.size());
                        //plots->Fill("nTracksPerEventNNOverlap", NNOverlapTracks.size());
                        plots->Fill("nTracksPerEventMatched", nMatchedTracks);
                        //plots->Fill("nTracksPerEventMatchedNNSelected", nMatchedNNSelectedTracks);
                        //plots->Fill("nTracksPerEventMatchedNNOverlap", nMatchedNNOverlapTracks);
                        plots->Fill("nTrueTracksPerEvent", seedTracks.size());

                        plots->addToCounter("nRoads", allCompleteTracks.size());
                        //nRoads += allCompleteTracks.size();
                    }
                }
                
                if (saveOutputTracks == "true")
                {
                    std::vector<std::shared_ptr<SeedTrack>> usedFakeSeeds;
                    std::vector<std::shared_ptr<SeedTrack>> usedTrueSeeds;
                    for (auto& extrapTrk: allCompleteTracks)
                    {
                        // only save one fake track and one true track per seed for fake track classification

                        double truthProb = extrapTrk->getComputedTrackProbability();
                        if (truthProb >= 0.9) 
                        {
                            if (std::find(usedTrueSeeds.begin(), usedTrueSeeds.end(), extrapTrk->getBaseSeedTrack()) == usedTrueSeeds.end())
                            {
                                outputTrueTracks.push_back(extrapTrk);
                                usedTrueSeeds.push_back(extrapTrk->getBaseSeedTrack());
                            }
                        }
                        else if (truthProb <= 0.5) 
                        {
                            if (std::find(usedFakeSeeds.begin(), usedFakeSeeds.end(), extrapTrk->getBaseSeedTrack()) == usedFakeSeeds.end())
                            {
                                outputFakeTracks.push_back(extrapTrk);
                                usedFakeSeeds.push_back(extrapTrk->getBaseSeedTrack());
                            }   
                        }
                    }
                }

                /*
                plots->Fill("diff_nExtrap_nDuplicate", allCompleteTracks.size() - nDuplicates);
                plots->Fill("nDuplicateTracks", nDuplicates);
                //std::cout<<"nDuplicates: "<<nDuplicates<<std::endl;

                plots->addToCounter("nRoads", allCompleteTracks.size()-nDuplicates);
                nRoads += allCompleteTracks.size()-nDuplicates;
                */
                //tracksToPlot = allCompleteTracks;
                //std::vector<std::shared_ptr<ExtrapolatedTrack>> sampledOutputFakeTracks;
                //std::sample(outputTracks.begin(), outputTracks.end(), std::back_inserter(sampledOutputFakeTracks), nSamples, std::mt19937{std::random_device{}()});
                if (saveOutputTracks == "true")
                {
                    outputManager->saveExtrapolatedFakeTracks(outputFakeTracks);
                    outputManager->saveExtrapolatedTrueTracks(outputTrueTracks);
                    //std::cout<<outputTrueTracks.size()<<"   "<<outputFakeTracks.size()<<std::endl;
                    //std::cout<<"nSavedTrueTracks: "<<outputTrueTracks.size()<<" nSavedFakeTracks: "<<outputFakeTracks.size()<<std::endl;
                }
                // std::cout << "number of tracks in event: " << efficiencyDenom  << std::endl;
                // std::cout << "number of roads: " << nRoads  << std::endl;
                // std::cout << "number of matched tracks: " << efficiencyNumerator << std::endl;
                // std::cout << "efficiency is: " << efficiencyNumerator / efficiencyDenom << std::endl;
            }
        }
    } // event loop
    
    
    for (const auto& err: uncertaintyList)
    {
        for (const auto& mH: minNHitsList)
        { 
            for (const auto& nnCut: nnCutList)
            {
                for (const auto& HWThreshold: nHWThresholdList)
                {
                    plots->setKey(err, mH, nnCut, HWThreshold);
            
                    plots->getHist("counterHist")->SetBinContent(1, plots->getCounter("nTracks"));
                    plots->getHist("counterHist")->SetBinContent(2, plots->getCounter("nMatchedTracks"));
                    plots->getHist("counterHist")->SetBinContent(3, plots->getCounter("nMatchedNNSelectedTracks"));
                    plots->getHist("counterHist")->SetBinContent(4, plots->getCounter("nMatchedNNOverlapTracks"));
                    plots->getHist("counterHist")->SetBinContent(5, plots->getCounter("nRoads"));
                    
                    plots->getHist("counterHist")->GetXaxis()->SetBinLabel(1, "nTracks");
                    plots->getHist("counterHist")->GetXaxis()->SetBinLabel(2, "nMatchedTracks");
                    plots->getHist("counterHist")->GetXaxis()->SetBinLabel(3, "nMatchedNNSelectedTracks");
                    plots->getHist("counterHist")->GetXaxis()->SetBinLabel(4, "nMatchedNNOverlapTracks");
                    plots->getHist("counterHist")->GetXaxis()->SetBinLabel(5, "nRoads");
                }
            }
        }
    }
    
    // trackPlotter->plotExtrapolatedTracks(tracksToPlot, "track");

    
    
    TFile* outFile = new TFile(TString(opts["outputFile"]), "recreate");
    for(const auto& errorSize: uncertaintyList)
    {
        for(const auto& minNHits: minNHitsList)
        {
            //plots->setKey(errorSize, minNHits);
            //TString key = plots->getKey(errorSize, minNHits);
            //std::cout<<key<<std::endl;
            //outFile->mkdir(key);
            //outFile->cd(key);
            //auto histList = plots->getAllHist(errorSize, minNHits);
            // for(auto& hist: histList) 
            // {
            //     //std::cout<<hist->GetName()<<std::endl;
            //     hist->Write();
            // }
            for (const auto& nnCut: nnCutList)
            {
                for (const auto& HWThreshold: nHWThresholdList)
                {
                    plots->setKey(errorSize, minNHits, nnCut, HWThreshold);
                    TString key = plots->getKey(errorSize, minNHits, nnCut, HWThreshold);
                    std::cout<<key<<std::endl;
                    outFile->mkdir(key);
                    outFile->cd(key);
                    auto histList = plots->getAllHist(errorSize, minNHits, nnCut, HWThreshold);
                    for(auto& hist: histList) 
                    {
                        std::cout<<hist->GetName()<<std::endl;
                        hist->Write();
                    }
                    outFile->cd("");
                }
            }
        }
    }
    outFile->Close();
    

    if (saveOutputTracks == "true")
    {
        std::cout<<"finalizing output manager"<<std::endl;
        outputManager->finalize();
    }
}

// inputs are: 
// a vector to store completely extrapolated tracks in
// a vector of track seeds to extrapolate from (vector<shared_ptr<Seed>>)
// the hitmanager
// the nneval object
void extrapolateTracks(vector<shared_ptr<ExtrapolatedTrack>>& completeTracks, vector<shared_ptr<SeedTrack>>& seedTracks, 
    const shared_ptr<HitsManager> hitManager,  const shared_ptr<NNEvalClassUtils> nnEval, const shared_ptr<TrackFitter> trackFitter,
    const shared_ptr<DataReaderITK> dataReader, int uncertainty, int minHits, std::string algoType)
{   
    auto det = hitManager->getVolumesITK();
    for(auto& seed: seedTracks)
    {   
        // create vector to hold all roads
        std::vector<std::shared_ptr<ExtrapolatedTrack>> roads;
        std::vector<std::shared_ptr<ExtrapolatedTrack>> new_roads;
        //Make an initial extrapolated track
        auto initialExtrapTrk = make_shared<ExtrapolatedTrack>(seed->getHitIndex(), seed->getHitsList(), seed);
        roads.push_back(initialExtrapTrk);

        int counter = 0;
        while (roads.size() > 0) // after a road is complete, it should be stored in another container and removed from roads
        {
            counter++;
            for(auto& trk: roads)
            {
                //cout<<"------------------------------------------------------------------------ Extrapolating: "<<endl;
                //trk->printDebug();

                //cout << "---- New Roads Being Built ----" << std::endl;
                auto hitList = trk->getHitsList();
                // if (!(trk->getHitsList().at(0)->getTruthTrackBarcode().at(0) == 36028799300665344)) continue;
                //     trk->printDebug();
                
                auto predDectector = nnEval->classify(hitList, false);
                int layerId;
                int volumeId;
                for (size_t i=0; i<15; i++)
                {
                    if (predDectector[i]==1) {volumeId = det[i];}
                }
                for (size_t i=0; i<30; i++)
                {
                    if (predDectector[i+15]==1) {layerId = i*2;}
                }
                 // calculate the predicted hit
                //  trk->printDebug();
                //  trk->printArray();
                auto predZ = trackFitter->predictZBarrelHit(hitList,volumeId,layerId,false);
                std::pair<float,float> predPointXY;
                auto intersects = trackFitter->get_circle_intersection_with_barrelLayer(predPointXY, hitList, volumeId, layerId, false);
                if(!intersects){
                    completeTracks.push_back(trk);
                    seed->incrimentNExtrapolatedTracks();
                    continue;
                }
                auto predHit = make_shared<Hit>(-999, predPointXY.first, predPointXY.second, predZ);

                auto recoveredHits = hitManager->getClosestLayerHitsList(predHit, hitList, volumeId, layerId, uncertainty);
                if (recoveredHits.size() < 1)
                {
                    recoveredHits = hitManager->getKNearestNeighborHits(predHit, hitList, volumeId, layerId, 30, 1);
                    if (recoveredHits.size()<1)
                    {
                        completeTracks.push_back(trk);
                        seed->incrimentNExtrapolatedTracks();
                        continue;
                    } 
                }

                for(auto& foundHit: recoveredHits)
                {
                    // Make the extrapolated track
                    auto extrapTrk = make_shared<ExtrapolatedTrack>(trk->getHitIndex(), trk->getHitsList(), seed);
                    extrapTrk->addHit(predHit, foundHit);
                    // extrapTrk->printDebug();

                    if( (abs(foundHit->getZ()) > 3450) || (foundHit->getRho() > 1000) || (extrapTrk->getNHits() >=15) )
                    {
                        completeTracks.push_back(extrapTrk);
                        seed->incrimentNExtrapolatedTracks();
                    } 
                    else new_roads.push_back(extrapTrk);
                }
            }
            roads = new_roads;
            new_roads.clear();
        }
    }
}

void fillVector(vector<shared_ptr<ExtrapolatedTrack>>& storeTracks, vector<shared_ptr<ExtrapolatedTrack>>& completeTracks)
{
    for (auto& track: completeTracks) storeTracks.push_back(track);
}

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ACTS_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["outputManagerFile"] = "outputTracks.root";
    opts["onnxFile"]        = "TrackNNAnalysis/ExtrapolationModels/Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx";
    opts["windowSize"]      = "3";
    opts["uncertainty"]     = "10";
    opts["nThreads"]        = "6";
    opts["minHits"]         = "6";
    opts["LayerRegion"]  = "barrel";
    opts["algoType"]        = "slidingWindow";
    opts["saveOutputTracks"] = "";
    opts["hitType"]         = "RotatedToZero";
    opts["muonOnly"]        = "";
    opts["pileupOnly"]        = "";
    opts["deltaROverlap"]   = "0";


    // For the future, when we add nn and overlap
    opts["NNCutVal"]        = "0.5";
    opts["nHitOverlapThres"]= "5";
    opts["NNOverlapOnnxFile"]        = "TrackNNAnalysis/ONNXModels/ACTS_OverlapModel/Extrap_model_overlapRemoval_NN_10hits_Rotated_v2.onnx";
    opts["NNClassifierOnnxFile"]        = "TrackNNAnalysis/ONNXModels/ACTS_OverlapModel/Extrap_model_overlapRemoval_NN_10hits_Rotated_v2.onnx";


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
            cout<<"--uncertainty        : The radial size to search for nearby hits when predicting"<<endl;
            cout<<"--nThreads           : The number of treads to spawn for seed finding"<<endl;
            cout<<"--algoType           : slidingWindow or fullTrack algorithms"<<endl;
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
        if (opt == "saveOutputTracks")
        {
            opts["saveOutputTracks"]="true";
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

