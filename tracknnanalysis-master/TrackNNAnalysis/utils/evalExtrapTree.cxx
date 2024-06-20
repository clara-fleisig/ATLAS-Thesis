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
#include "TObjString.h"
#include <Eigen/Core>
#include <nlohmann/json.hpp>
#include "PathResolver/PathResolver.h"

// Local includes
#include "TrackNNDataReader/DataReaderITK.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNEvaluator/NNEvalExtrapUtils.h"
#include "TrackNNEvaluator/NNEvalUtils.h"
#include "TrackNNEvaluator/NNEvalClassUtils.h"
#include "TrackNNEvaluator/NNHitPredictor.h"
#include "TrackNNEDM/TrackTree.h"
#include "TrackNNAnalysis/TrackManager.h"

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
std::vector<TString> tokenizeStr(TString str, TString key);
NetworkBatchInput BatchTracksForGeoPrediction(std::vector<TrackTree*> trackTrees, int batch_size=128, int window_size=3);
NetworkBatchInput BatchTracksForClassification(std::vector<TrackTree*> trackTrees, int batch_size, std::pair<int, int>& startTreeIndex);



int main(int argc, char *argv[])
{
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    
    // NN Extrap module
    auto NNExtrapFile   = PathResolver::find_calib_file (opts["onnxFile"]);

    // NN Classifier module
    auto NNClassFile   = PathResolver::find_calib_file (opts["NNClassifierOnnxFile"]);

    int windowSize = atoi(opts["windowSize"].c_str());

    auto HitCoordinatePredictor = std::make_shared<NNHitPredictor>(NNExtrapFile);
    HitCoordinatePredictor->initialize();
    auto HitDetectorPredictor = std::make_shared<NNDetectorClassifier>(NNClassFile);
    HitDetectorPredictor->initialize();
    // // NN module
    // Implimenet Overlap Removal Network
    // auto NNOverlapFile = PathResolver::find_calib_file(opts["NNOverlapOnnxFile"]);
    // auto nnOverlapEval = make_shared<NNEvalUtils>(NNOverlapFile, 10, "RotatedToZero", "XX0YY0ZZ0", false);
    // nnOverlapEval->initialize();

    auto dataReader = make_shared<DataReaderITK>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    dataReader->setDeltaROverlap(atof(opts["deltaROverlap"].c_str()));

    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();
    
    float uncertainty = atof(opts.at("uncertainty").c_str());
    std::string LayerRegion = opts["LayerRegion"];
    dataReader->setLayerRegion(LayerRegion);

    auto seedManager = make_shared<SeedManager>();
    seedManager->setSeedSize(windowSize);
    seedManager->setWindowSize(windowSize);

    auto trackManager = make_shared<TrackManager>();

    int minHits = atoi(opts["minHits"].c_str());;
    int maxBranches = atoi(opts["maxBranches"].c_str());

    int batchSize = atoi(opts["batchSize"].c_str());
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);
        //auto recoTracks = dataReader->getRecoTracks();       
        auto recoTracks = dataReader->getOfflineTruthTracks();
        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());
        auto detectorLabels = hitManager->getVolumesITK();
        std::vector<std::shared_ptr<TrueTrack>> cleanTracks;
        if (LayerRegion =="barrel") cleanTracks = trackManager->getTrackSingleLayerHitInBarrel(recoTracks, minHits);
        else if(LayerRegion =="endcaps")  cleanTracks = trackManager->getTrackSingleLayerHitInEndcap(recoTracks, minHits);
        else  cleanTracks = trackManager->getTrackSingleLayerHit(recoTracks, minHits);
        //std::cout<<"number of tracks in event: "<<cleanTracks.size()<<std::endl;
        // for(auto trk: cleanTracks) trk->printDebug();
        auto seedTracks = seedManager->getSeedTracks(SeedType::InitialSeeds, cleanTracks);
        int totalActiveHits=0;
        std::vector<TrackTree*> trackTrees;
        int tot = 0;
        for(auto& seed:seedTracks)
        {
            if (tot == 1) break; // Use only n seeds for testing
            seed->printDebug();
            seed->getBaseTrueTrack()->printDebug();
            TrackTree* tree = new TrackTree(seed);
            tree->setMaxBranches(maxBranches);
            trackTrees.push_back(tree);
            totalActiveHits += tree->getActiveHits().size();
            tot++;
        }

        std::cout<<"number of initial seeds: "<<trackTrees.size()<<std::endl;
        for(auto tree:trackTrees)
        {
            for(auto h:tree->getActiveHits()){
                tree->printDebug(h);
            }
        }

        // Main Hit Search loop
        while (totalActiveHits > 0)
        {
            auto networkInput = BatchTracksForGeoPrediction(trackTrees, batchSize);
            auto predictedDet = HitDetectorPredictor->PredictVolumeAndLayer(networkInput);
            NetworkBatchInput coordinateNetworkInput(networkInput.rows(), networkInput.cols() + predictedDet.cols());
            coordinateNetworkInput << networkInput, predictedDet;
            auto predictedHitMatrix = HitCoordinatePredictor->PredictHitCoordinate(coordinateNetworkInput);
            std::cout<<predictedHitMatrix<<"\n"<<std::endl;
            int idx=0;
            bool breakLoop = false;
            for (auto& tree:trackTrees)
            {
                if (tree->getActiveHits().size() == 0) { continue; }
                auto activeHitNodes = tree->getActiveHits();
                std::reverse(activeHitNodes.begin(), activeHitNodes.end()); // reverse so we can use pop_back to remove items from tree's active hits
                for(auto& hitNode:activeHitNodes)
                {
                    auto currentHit = hitNode->getHit();
                    Eigen::VectorXf predictedHitCoordinate = predictedHitMatrix.row(idx);
                    int layerId; 
                    int volumeId; 
                    int layerIdIdx; 
                    int volumeIdIdx;

                    // Get first 15 elements of this row corresponding to the volume one hot encoding
                    Eigen::VectorXf volumeRow = predictedDet.block<1,15>(idx,0);
                    volumeRow.maxCoeff(&volumeIdIdx);

                    // Get last 30 elements of this row corresponding to the layer one hot encoding
                    Eigen::VectorXf layerRow = predictedDet.block<1,30>(idx,15);
                    layerRow.maxCoeff(&layerIdIdx);

                    volumeId = detectorLabels[volumeIdIdx];
                    layerId = layerIdIdx*2;

                    std::vector<std::shared_ptr<Hit>> hitsList;
                    tree->getHitsList(hitNode, hitsList, windowSize);
                    std::reverse(hitsList.begin(), hitsList.end());

                    auto recoveredHits = hitManager->getClosestLayerHitsList(predictedHitCoordinate, hitsList, volumeId, layerId, uncertainty, true);

                    std::cout<<"n recovered hits: "<<recoveredHits.size()<<std::endl;
                    std::vector<std::shared_ptr<Hit>> FinalHits;
                    std::vector<std::shared_ptr<Hit>> activeHits;

                    // add recovered hits to node.
                    // Need to determine better track finish condition
                    int nAddedHits = 0;
                    for(const auto& hit: recoveredHits)
                    {
                        //std::cout<<nAddedHits<<std::endl;
                        if (hit->getRho() > 975 || std::abs(hit->getZ() > 3400))
                        {
                            std::cout<<"added final hit"<<std::endl;
                            FinalHits.push_back(hit);
                        }
                        else
                        {
                            
                            // std::cout<<"recovered "<<recoveredHits.size()<<" hits"<<std::endl;
                            // only need to add as many active hits as there are allowed active branches
                            if (nAddedHits < maxBranches) activeHits.push_back(hit);
                            std::cout<<"n active hits: "<<activeHits.size()<<std::endl;
                            nAddedHits++;
                        }
                    }
                    
                    // If there were no hits found, move the active hit to final hits as it is the end of a track
                    if (recoveredHits.size() == 0)
                    {
                        // FinalHits.push_back(hitNode->getHit());
                        tree->activeHitToFinalHits(hitNode);
                        std::cout<<"no hits found. Ending branch "<<std::endl;
                    }

                    // Want to clear active hits after they are used. They are no longer active!
                    // Cannot clear the whole active hits vector yet as we are iterating over it!
                    if (FinalHits.size() > 0)
                    {
                        // returns the number of final hits added. If number would bring tree > max size those hits will be skipped
                        int tmpVar = tree->addFinalHitNodes(hitNode, FinalHits);
                    }
  
                    tree->eraseActiveHits(); // erases the active hit already used. Pop_back from the reversed list
                    int nActiveHits = tree->addActiveHitNodes(hitNode, activeHits);

                    // Only add the number of hits that were used
                    // Some may not be because the tree is overgrown
                    // totalActive hits  = number of newly added hits minus the hit that was just used
                    totalActiveHits += nActiveHits - 1;
                    idx++;
                    if(idx >= batchSize)
                    {
                        breakLoop = true;
                        break;
                        
                    }
                }
                if(breakLoop) break;
            }
            // int r = static_cast<int>(coordinateNetworkInput.rows());
            // totalActiveHits -= r;
            // std::cout<<"total active hits "<<totalActiveHits<<std::endl;
        }
        for (auto& tree:trackTrees)
        {
            std::cout<<"final hits size "<<tree->getFinalHits().size()<<std::endl;
            for(auto h:tree->getFinalHits()){
                std::cout<<"printing"<<std::endl;
                tree->printDebug(h);
            }
        }

        //// NNEval Classification Loop for OverlapRemoval
        //// indices get updated when BatchTracksForClassification is called
        //
        //std::pair<int,int> indices(0,0);
        //bool breakWhile = false;
        //int scoreIdx = 0;
        //while (true)
        //{   
        //    // Preserve start indices for easy looping when assigning scores
        //    std::pair<int, int> startIndices = indices;
        //    auto NetworkInput = BatchTracksForClassification(trackTrees, batchSize, startIndices);
        //    auto classifierScores = NNEVal->eval(NetworkInput);

        //    for(int i = startIndices.first; i<indices.first; i++)
        //    {
        //        for(int j = startIndices.second; j<indices.second; j++)
        //        {
        //            float score = classifierScores.coeff(scoreIdx);
        //            trackTrees.at(i).getFinalHits().at(j)->setNNEvalScore(score);
        //            scoreIdx++;
        //            if (scoreIdx == static_cast<int>(classifierScores.rows()) - 1;)
        //            {
        //                breakWhile = true;
        //                break;
        //            }
        //        }
        //    }
        //    if(breakWhile)
        //    {
        //        // Break after all scores have been used
        //        break;
        //    }
        //}
            
    } // event loop
} // main

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ACTS_ITK.root";
    opts["nEvents"]         = "-1";
    opts["onnxFile"]        = "TrackNNAnalysis/ExtrapolationModels/Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx";
    opts["NNClassifierOnnxFile"]        = "TrackNNAnalysis/ONNXModels/ACTS_OverlapModel/Extrap_model_overlapRemoval_NN_10hits_Rotated_v2.onnx";

    opts["windowSize"]      = "3";
    opts["uncertainty"]     = "10";
    opts["minHits"]         = "6";
    opts["LayerRegion"]  = "barrel";
    opts["muonOnly"]        = "";
    opts["deltaROverlap"]   = "0";
    opts["batchSize"]       = "128";
    opts["maxBranches"]       = "10";


    // For the future, when we add nn and overlap
    opts["NNCutVal"]        = "0.5";
    opts["nHitOverlapThres"]= "5";
    opts["NNOverlapOnnxFile"]        = "TrackNNAnalysis/ONNXModels/ACTS_OverlapModel/Extrap_model_overlapRemoval_NN_10hits_Rotated_v2.onnx";


    for(int i = 1; i < argc; ++i)
    {
        string opt=argv[i];
        if(opt=="--help" || opt == "--options" || opt =="--h")
        {
            cout<< "Options menu \n ============================================== \n" << endl;
            cout<<"--inputFile          : Path to the input file to process"<< endl;
            cout<<"--detectorGeoFile    : Path to the file that contains the detector geometry information"<<endl;
            cout<<"--nEvents            : Number of events to run over (-1 is all)"<<endl;
            cout<<"--uncertainty        : The radial size to search for nearby hits when predicting"<<endl;
            cout<<"--nThreads           : The number of treads to spawn for seed finding"<<endl;
            return false;
 
        }

        if(0!=opt.find("--")) {
            cout<<"ERROR: options start with '--'!"<<endl;
            cout<<"ERROR: options is: "<<opt<<endl;
            return false;
        }
        opt.erase(0,2);


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

NetworkBatchInput BatchTracksForGeoPrediction(std::vector<TrackTree*> trackTrees, int batch_size, int window_size)
{
    // Check if there are enough active hits to fill the matrix
    int nActiveHits = 0;
    for(auto& tree: trackTrees)
    {
        nActiveHits += tree->getActiveHits().size();
    }
    if (nActiveHits < batch_size) { batch_size = nActiveHits; }
    NetworkBatchInput networkInput(batch_size, 9); // 9 hard coded as the number of inputs. Change depending on NN model used
    int trkIndex = 0;
    bool breakWhile = false;
    while(true) 
    {
        for(auto tree = trackTrees.begin(); tree != trackTrees.end(); tree++)
        {
            if((*tree)->getActiveHits().size() == 0) 
            {
                if(tree == (trackTrees.end()-1)) 
                {
                    breakWhile = true;
                    break;
                } 
                continue; 
            }
            
            for(auto& hitNode: (*tree)->getActiveHits())
            {
                std::vector<std::shared_ptr<Hit>> hitsList;
                (*tree)->getHitsList(hitNode, hitsList, window_size);

                // Fill in reverse order b.c hitsList is in reverse order (descending in r)
                int counter = window_size-1;
                while(counter >= 0 )
                {
                    networkInput(trkIndex, (window_size - 1- counter)*3    ) =  hitsList.at(counter)->getX() / hitsList.at(counter)->getXScale();
                    networkInput(trkIndex, (window_size - 1- counter)*3 + 1) =  hitsList.at(counter)->getY() / hitsList.at(counter)->getYScale();
                    networkInput(trkIndex, (window_size - 1- counter)*3 + 2) =  hitsList.at(counter)->getZ() / hitsList.at(counter)->getZScale();
                    counter--;
                }
                trkIndex++;
                if(trkIndex >= batch_size)
                {  
                    breakWhile = true;
                    break;
                }
            }
            if(breakWhile) break;
        }
        if(breakWhile) break;
    }
  
    return networkInput;
}

NetworkBatchInput BatchTracksForClassification(std::vector<TrackTree*> trackTrees, int batch_size, std::pair<int, int>& startTreeIndex)
{
    int maxLength = 10; // Max length of input track
    
    // Check if there are enough active hits to fill the matrix
    int nFinalHits = 0;
    for(auto& tree: trackTrees)
    {
        nFinalHits += tree->getFinalHits().size();
    }
    if (nFinalHits < batch_size) { batch_size = nFinalHits; }
    NetworkBatchInput networkInput(batch_size, 30); // 30 hard coded as the number of inputs (max length 10 hits 3 features each)


    int treeIndex = startTreeIndex.first;
    int trkIndex = startTreeIndex.second;

    // Check if we have used all the hits in this tree
    if(trkIndex == (trackTrees.at(treeIndex)->getFinalHits().size()-1) )
    {
        trkIndex = 0;
        treeIndex++;
    }

    std::vector<TrackTree*>::iterator tree;
    std::vector<TrackTree*>::iterator begin = trackTrees.begin() + startTreeIndex.first;
    for(tree = begin; tree != trackTrees.end(); tree++)
    {   
        for(auto hitNode_itr = (*tree)->getFinalHits().begin() + startTreeIndex.second; hitNode_itr != (*tree)->getFinalHits().end(); hitNode_itr++)
        // for(auto& hitNode: (*tree)->getFinalHits())
        {

            std::vector<std::shared_ptr<Hit>> hitsList;
            (*tree)->getHitsList((*hitNode_itr), hitsList, -1);

            // Fill in reverse order b.c hitsList is in reverse order (descending in r)
            int counter = hitsList.size() - 1;
            for(int i=0; i<hitsList.size(); i++)
            {
                networkInput(trkIndex, i*3    ) =  hitsList.at(counter-i)->getX() / hitsList.at(counter-i)->getXScale();
                networkInput(trkIndex, i*3 + 1) =  hitsList.at(counter-i)->getY() / hitsList.at(counter-i)->getYScale();
                networkInput(trkIndex, i*3 + 2) =  hitsList.at(counter-i)->getZ() / hitsList.at(counter-i)->getZScale();
                counter--;
            }
            // Pad the rest with zeros
            for (size_t i = hitsList.size() ; i < maxLength ; i++)
            {
                networkInput(trkIndex, i*3    ) =  0;
                networkInput(trkIndex, i*3 + 1) =  0;
                networkInput(trkIndex, i*3 + 2) =  0;
            }

            trkIndex++;
            if(trkIndex >= batch_size)
            {  
                startTreeIndex.first = treeIndex;
                startTreeIndex.second = trkIndex;
                break;
            }
        }
        treeIndex++;
    }
    
  
    return networkInput;
}
