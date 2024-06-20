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

// Local includes
#include "TrackNNDataReader/DataReaderITK.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNAnalysis/TrackManager.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
#include "TrackNNEvaluator/NNEvalCombined.h"
#include "TrackNNEvaluator/NNEvalExtrapUtils.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNAnalysis/PlotHolder.h"

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
void extrapolateTracks(vector<shared_ptr<ExtrapolatedTrack>>& completeTracks, vector<shared_ptr<SeedTrack>>& seedTracks, const shared_ptr<HitsManager> hitManager, const shared_ptr<NNEvalCombined> nnEval, int radialError);
void fillVector(vector<shared_ptr<ExtrapolatedTrack>>& storeTracks, vector<shared_ptr<ExtrapolatedTrack>>& completeTracks);

int main(int argc, char *argv[])
{
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    
    // NN module
    auto NNFile   = PathResolver::find_calib_file (opts["onnxFile"]);
    auto NNClassFile   = PathResolver::find_calib_file (opts["NNClassifierOnnxFile"]);
    auto nnEval = make_shared<NNEvalCombined>(NNClassFile,NNFile, atoi(opts["windowSize"].c_str()));
    //auto nnEval = make_shared<NNEvalExtrapUtils>(NNFile, atoi(opts["windowSize"].c_str()));
    nnEval->initialize();
    int trackNumber = atoi(opts["trackNumber"].c_str());
    auto dataReader = make_shared<DataReaderITK>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    dataReader->pileupOnly(true);
    cout<<"initialized datareader"<<endl;
    //dataReader->dropPixHit();
    //cout<<"dropped pixel hits"<<endl;
    // Track plotter for debugging
    auto trackPlotter = make_shared<TrackGeoPlotter>(dataReader->getDetectorModules());
    trackPlotter->initialize();

    //
    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();
    
    int radialError = atoi(opts.at("radialError").c_str());

    int windowSize = atoi(opts["windowSize"].c_str());
    
    // seedManaged to create the seeds
    auto seedManager = make_shared<SeedManager>();
    seedManager->setSeedSize(windowSize);
    seedManager->setWindowSize(windowSize);

    // TrackManager to create single-hit-per-layer tracks
    auto trackManager = make_shared<TrackManager>();

    int nThreads = 1;

    vector<shared_ptr<ExtrapolatedTrack>> goodTracksToPlot;
    vector<shared_ptr<ExtrapolatedTrack>> badTracksToPlot;
    // To store fully found tracks
     std::vector<std::shared_ptr<ExtrapolatedTrack>> completeTracks;
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        //auto recoTracks = dataReader->getRecoTracks();       
        auto recoTracks = dataReader->getOfflineTruthTracks();
        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());

        //second argument true for barrel only tracks
        auto layeredTracks = trackManager->getTrackSingleLayerHitInBarrel(recoTracks, 8);

        std::vector<std::shared_ptr<TrueTrack>> cleanTracks(layeredTracks.begin(), layeredTracks.end());
        auto sTracks = seedManager->getSeedTracks(SeedType::InitialSeeds, cleanTracks);
        if (sTracks.size() < 1) continue;

        // to store threads
        std::vector<std::thread> threads;
        threads.reserve(nThreads);

        std::vector<std::shared_ptr<SeedTrack>> v0 = {sTracks.begin(), sTracks.end()};

        threads.push_back(std::thread(extrapolateTracks, std::ref(completeTracks),  
            std::ref(v0), std::ref(hitManager), std::ref(nnEval), radialError));

        for(std::thread &t:threads)
        {
            if (t.joinable()) t.join();
        }
        cout<<"finished"<<endl;
    }
    
    int nTrueMatched=0;
    int nFakeMatched=0;
    int counter = 0;
    std::cout<<"Number of Extrapolated Tracks: "<< completeTracks.size()<<std::endl;
    std::vector<Index> usedBarcodes;
    int trkSize = completeTracks.size();
    for (size_t idx=0; idx<trkSize; idx++) 
    {
        auto trk = completeTracks.at(idx);
        if (trk->getNHits() < 8) continue;
        auto id = trk->getBaseTrueTrack()->getBarcode();
        //std::cout<<id<<std::endl;

        // if (std::find(usedBarcodes.begin(), usedBarcodes.end(), id) != usedBarcodes.end())
        // {
        //     continue;
        // }
        // bool isMatched = trk->isMatchedToReco(id, 0.8, windowSize);
        // bool isBadMatched = trk->isMatchedToReco(id, 0.25, windowSize);
        bool isMatched = trk->isMatched(0.99);
        bool isBadMatched = trk->isMatched(.45);
        // if (isMatched)
        // {
        //     std::cout<<"is matched. Index "<<idx<<std::endl;
        //     trk->printDebug();
        // }
        // if (!isBadMatched)
        // {
        //     std::cout<<"is not matched. Index "<<idx<<std::endl;
        //     trk->printDebug();
        // }
        

        // if (idx == 10348 || idx == 10306)
        // {
        //     goodTracksToPlot.push_back(trk);
        // }
        // if (idx == 9733 ||  idx == 8249)
        // {
        //     badTracksToPlot.push_back(trk);
        // }

       
        if (isMatched && nTrueMatched < 2)
        {
            goodTracksToPlot.push_back(trk);
            std::cout<<"GOOD TRACK"<<std::endl;
            trk->printDebug();
            nTrueMatched++;
            usedBarcodes.push_back(id);

        }
        // if ((!isBadMatched && nFakeMatched < 2) && (trk->getNHits() > 7))
        // {
        //     badTracksToPlot.push_back(trk);
        //     std::cout<<"BAD TRACK"<<std::endl;
        //     trk->printDebug();
        //     nFakeMatched++;
        //     usedBarcodes.push_back(id);

        // }
        counter ++; 
        if (nTrueMatched >=1 || nFakeMatched >=2) break;
    }
    // std::cout<<"Number of good Tracks: "<< goodTracksToPlot.size()<<std::endl;
    // std::cout<<"Number of bad Tracks: "<< badTracksToPlot.size()<<std::endl;
    std::cout<<"plotting"<<std::endl;
    trackPlotter->plotExtrapolatedTracks(goodTracksToPlot, badTracksToPlot, "ExtrapolatedTracks");
}

// inputs are: 
// a vector to store completely extrapolated tracks in
// a vector of track seeds to extrapolate from (vector<shared_ptr<Seed>>)
// the hitmanager
// the nneval object
void extrapolateTracks(vector<shared_ptr<ExtrapolatedTrack>>& completeTracks, vector<shared_ptr<SeedTrack>>& seedTracks, const shared_ptr<HitsManager> hitManager, const shared_ptr<NNEvalCombined> nnEval, int radialError)
{    
    cout<<"seed tracks size in function: "<<seedTracks.size()<<endl;
    for(auto& seed: seedTracks)
    {   
        //cout<<"memory address of the first seed in thread: "<<seed.get()<<endl;
        //cout<<"------ true reco Index: "<<seed->getHitsList().at(0)->getRecoTrackIndex()<<" ------"<<endl;
        // create vector to hold all roads
        std::vector<std::shared_ptr<ExtrapolatedTrack>> roads;
        std::vector<std::shared_ptr<ExtrapolatedTrack>> new_roads;
        //Make an initial extrapolated track
        //cout<<"getting seed hit index"<<endl;
        auto qb = seed->getHitIndex();
        //cout<<"making seed hit list"<<endl;
        auto q = seed->getHitsList();
        //cout<<"making initial extrap track"<<endl;
        auto initialExtrapTrk = make_shared<ExtrapolatedTrack>(seed->getHitIndex(), seed->getHitsList(), seed);
        //cout<<"pushing extrap track into roads"<<endl;
        roads.push_back(initialExtrapTrk);
        std::cout<<"number of initial roads: " << roads.size() << std::endl;
        while (roads.size() > 0) // after a road is complete, it should be stored in another container and removed from roads
        {
            //road_size = roads.size()
            for(auto& trk: roads)
            {
                //cout << "---- New Roads Being Built ----" << std::endl;
                auto hitList = trk->getHitsList();
                std::vector<std::pair<std::shared_ptr<Hit>, std::vector<int>>> predictedHitPairs; 
                auto predictedHitPair = nnEval->Extrap(hitList, false);
                predictedHitPairs.push_back(predictedHitPair);
                //predictedHit->printDebug();

                // this can consider only hits within layers of interest.  use absolute distance if not doing it by layer
                //auto recoveredHits = hitManager->getClosestHitsListFromPredictedHit(predictedHit, hitList, radialError);
                for (auto& pr: predictedHitPairs)
                {
                    int layerId;
                    int volumeId;
                    auto det = hitManager->getVolumesITK();
                    for (size_t i=0; i<15; i++)
                    {
                        if (pr.second[i]==1) {volumeId = det[i];}
                    }
                    for (size_t i=0; i<30; i++)
                    {
                        if (pr.second[i+15]==1) {layerId = i*2;}
                    }
                    auto recoveredHits = hitManager->getClosestLayerHitsList(pr.first, hitList, volumeId, layerId, 15);
                    if (recoveredHits.size() < 1)
                    {
                        recoveredHits = hitManager->getKNearestNeighborHits(pr.first, hitList, volumeId, layerId, 30, 1);
                        if (recoveredHits.size()<1)
                        {
                            completeTracks.push_back(trk);
                            seed->incrimentNExtrapolatedTracks();
                            continue;
                        } 
                    }
                    if(recoveredHits.size() == 0) 
                    {
                        //std::cout<<"no hits found" << std::endl;
                        trk->addPredictedHit(pr.first);
                        completeTracks.push_back(trk);
                        continue; //store this as a finished track
                    }
                    for(auto foundHit: recoveredHits)
                    {
                        // Make the extrapolated track
                        //cout << "found hit reco index: " << foundHit->getRecoTrackIndex() << std::endl;
                        //cout << "X: " << foundHit->getX() << " Y: " << foundHit->getY() << " Z: " << foundHit->getZ() << std::endl;
                        auto extrapTrk = make_shared<ExtrapolatedTrack>(trk->getHitIndex(), trk->getHitsList(), seed);
                        //cout<<"making extrapolated track"<<endl;
                        //cout<<foundHit->getRecoTrackIndex()<<endl;
                        extrapTrk->addHit(pr.first, foundHit);
                        //cout<<"added hit to track"<<endl;
                        //extrapTrk->printDebug();
                        if( (abs(foundHit->getZ()) > 3000) || (foundHit->getRho() > 1050) || (extrapTrk->getNHits() >= 10) )
                            completeTracks.push_back(extrapTrk);
                        else
                            new_roads.push_back(extrapTrk);
                    }
                }
            }
            roads = new_roads;
            //std::cout<<"number of roads: " << roads.size() << std::endl;
            //cout<<"clearing new_roads"<<endl;
            new_roads.clear();
            
        }
        // break after one iteration for testing
        // cout<<"----- True Track -----" << endl;
        // layeredTracks.at(1)->printDebug();

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
    opts["outputFile"]      = "outputFile.root";
    opts["onnxFile"]        = "TrackNNAnalysis/ExtrapolationModelsITK/Extrap_biRNN_ITK_ws3_barrel_layerInfo.onnx";
    opts["windowSize"]      = "3";
    opts["radialError"]     = "25";
    opts["trackNumber"]     = "0";
    opts["NNClassifierOnnxFile"]        = "TrackNNAnalysis/ONNXModels/ACTS_OverlapModel/Extrap_model_overlapRemoval_NN_10hits_Rotated_v2.onnx";



    // For the future, when we add nn and overlap
    opts["NNCutVal"]        = "1";
    opts["nHitOverlapThres"]= "1";

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
            cout<<"--radialError        : The radial size to search for nearby hits when predicting"<<endl;
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

