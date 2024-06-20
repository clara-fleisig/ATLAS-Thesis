// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>
#include <cmath>

// Local includes
#include "TrackNNDataReader/DataReaderITK.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNAnalysis/TrackManager.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
#include "TrackNNEvaluator/NNEvalExtrapUtils.h"
#include "TrackNNEvaluator/NNEvalClassUtils.h"
#include "TrackNNAnalysis/TrackFitter.h"

#include "TrackNNEDM/Hit.h"
#include "TrackNNAnalysis/PlotHolder.h"
#include "TrackNNAnalysis/HitGeoPlotter.h"
#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"

#include "PathResolver/PathResolver.h"

// Root include
#include "TEfficiency.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TProfile.h"

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
std::vector<TString> tokenizeStr(TString str, TString key);
int main(int argc, char *argv[])
{

    std::vector<int> volumesITK = {2, 8, 9, 10, 13, 14, 15, 16, 18, 19, 20, 22, 23, 24, 25};
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    
    // NN module
    auto NNFile   = PathResolver::find_calib_file (opts["onnxFile"]);
    auto nnEval = make_shared<NNEvalExtrapUtils>(NNFile, atoi(opts["windowSize"].c_str()));
    nnEval->initialize();

    auto NNClassFile = PathResolver::find_calib_file (opts["NNClassifierOnnxFile"]);
    auto nnClassEval = make_shared<NNEvalClassUtils>(NNClassFile, atoi(opts["windowSize"].c_str()));
    nnClassEval->initialize();

    auto trackFitter = make_shared<TrackFitter>(atoi(opts["windowSize"].c_str()));

    std::string LayerRegion = opts["LayerRegion"];
    auto dataReader = make_shared<DataReaderITK>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    dataReader->setLayerRegion(LayerRegion);

    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();

    auto hitPlotter = make_shared<HitGeoPlotter>(dataReader->getDetectorModules());
    hitPlotter->initialize();
    

    int windowSize = atoi(opts["windowSize"].c_str());
    
    // seedManaged to create the seeds
    auto seedManager = make_shared<SeedManager>();
    seedManager->setSeedSize(windowSize);
    seedManager->setWindowSize(windowSize);
    // TrackManager to create single-hit-per-layer tracks
    auto trackManager = make_shared<TrackManager>();

    vector<double> nnCutList, nHWThresholdList;
    auto nnCutStr = tokenizeStr(opts["NNCutVal"], ",");
    auto HWThresStr = tokenizeStr(opts["nHitOverlapThres"], ",");
    std::string plotOverlay = opts["plotOverlay"];
    

    for(const auto& val: nnCutStr)   nnCutList.push_back(atof(val));
    for(const auto& val: HWThresStr) nHWThresholdList.push_back(atof(val));

    // Helper class to holder all the plots
    PlotHolderExtrap* plots = new PlotHolderExtrap();
    HistogramBookkeeper* bookkeeper = new HistogramBookkeeper();
    
    for(const auto& nnCut: nnCutList)
    {
        for(const auto& nHitOverlap: nHWThresholdList)
        {
            plots->createHists("");
        }  
    }
    plots->setKey(1, 1);


    std::vector<std::shared_ptr<Hit>> predictedHits; 
    std::vector<std::shared_ptr<Hit>> trueHits; 
    
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        //auto recoTracks = dataReader->getRecoTracks(); 
        auto recoTracks = dataReader-> getOfflineTruthTracks();

        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());
        // use layered tracks instead of pure reco tracks. ACTS does not require layer tracks. already layerec
        //auto layeredTracks = trackManager->getTrackSingleLayerHitInBarrel(recoTracks, windowSize+1);
        auto layeredTracks = trackManager->getTrackSingleLayerHit(recoTracks, windowSize+1);
        //for(auto& trk: layeredTracks) trk->printDebug();
        //for(auto& trk: layeredTracks) plots->Fill2d("Eta_vs_Z0", trk->getZ0(), trk->getEta());

        auto seedTracks = seedManager->getSlidingWindowSeedPairs(layeredTracks);
        auto det = hitManager->getVolumesITK();
        for(auto& seedPair: seedTracks)
        {
            
            auto trueTrack = seedPair.first->getBaseTrueTrack();

            //seedPair.first->printDebug();
            //truthHit = seedPair.second
            auto hitList = seedPair.first->getHitsList();
            // std::cout<<"---HITS---"<<std::endl;
            //auto trkIndex = seedPair.second->getRecoTrackIndex();
            auto trkIndex = seedPair.second->getTruthTrackBarcode().at(0);
            auto ohe = seedPair.second->OneHotEncoding(volumesITK,30);
            // skip if the true next hit is outside the barrel
            //if (seedPair.second->getIntMetadata("isBarrel") == 0) continue;
            //std::cout<<"barrel: "<<seedPair.second->getIntMetadata("isBarrel")<<" layer-id: "<<seedPair.second->getLayer()<<std::endl;
            //std::cout << "True track index:" << trkIndex << std::endl;
            //std::cout<<"Truth OHE   ";
            //for (auto& o: ohe) std::cout<<o<<", ";
            //std::cout<<"\n"<<std::endl;

            //std::cout<<"Pred OHE    \n";
            auto predOHE = nnClassEval->classify(hitList, false);
            int layerId;
            int volumeId;
            for (size_t i=0; i<15; i++)
            {
                if (predOHE[i]==1) {volumeId = det[i];}
            }
            for (size_t i=0; i<30; i++)
            {
                if (predOHE[i+15]==1) {layerId = i*2;}
            }
            // second arugment debug=true to print input hits
            // auto predZ = trackFitter->predictZBarrelHit(hitList,volumeId,layerId,false);
            // std::pair<float,float> predPointXY;
            // auto intersects = trackFitter->get_circle_intersection_with_barrelLayer(predPointXY,hitList, volumeId, layerId, false);
            // if (!intersects){
            //     plots->Fill("hitDistance", -999); 
            //     continue;
            // } // store to count how many failed
            // auto predictedHit = make_shared<Hit>(-999, predPointXY.first, predPointXY.second, predZ);
            auto predictedHit = nnEval->extrapSeed(hitList, ohe, false);
            //std::cout<<"---TRACK---"<<std::endl;
            //trueTrack->printDebug();
            
            // match the prediction to a hit in the event
            auto recoveredHit = hitManager->getClosestHitFromPredictedHit(predictedHit, hitList, 1000);
            auto recoveredHits = hitManager->getClosestHitsListFromPredictedHit(predictedHit, hitList, seedPair.second->distanceTo(predictedHit));
            //std::cout<<"recovered hit"<<std::endl;

            // store all predicted hits to plot their distribution overlayed with the detector
            if (i==0) // do for only one event so as not to clutter the plot
            {
                predictedHits.push_back(predictedHit);
                trueHits.push_back(seedPair.second);
            }
            // fill information about the distance between the predicted and recoveredHit
            plots->Fill("hitDistance",  seedPair.second->distanceTo(predictedHit));
            plots->Fill("hitDistance_phi",  seedPair.second->distanceToPhi(predictedHit));
            plots->Fill("hitDistance_rho",  seedPair.second->distanceToRho(predictedHit));
            plots->Fill("hitDistance_z",  seedPair.second->distanceToZ(predictedHit));
            // Fill errors by volume and layer
            string volAndlayer ("_vol" + to_string(seedPair.second->getVolume()) + "_layer" + to_string(seedPair.second->getLayer()));
            bookkeeper->Fill("residuals_phi" + volAndlayer , seedPair.second->distanceToPhi(predictedHit, true), 50, -.1, .1);
            bookkeeper->Fill("residuals_rho" + volAndlayer , seedPair.second->distanceToRho(predictedHit, true), 100, -100, 100);
            bookkeeper->Fill("residuals_z" + volAndlayer , seedPair.second->distanceToZ(predictedHit, true), 100, -100, 100);
            bookkeeper->Fill("HitsWithinRange"+volAndlayer, seedPair.second->distanceTo(predictedHit), recoveredHits.size(), 100,0,100,30,0,30);
            plots->Fill("residuals_phi",  seedPair.second->distanceToPhi(predictedHit, true));
            plots->Fill("residuals_rho",  seedPair.second->distanceToRho(predictedHit, true));
            plots->Fill("residuals_z",  seedPair.second->distanceToZ(predictedHit, true));
            plots->Fill("predicted_z", predictedHit->getZ());
            plots->Fill("predicted_phi", predictedHit->getPhi());
            plots->Fill("predicted_rho", predictedHit->getRho());
            plots->Fill("true_z",   seedPair.second->getZ());
            plots->Fill("true_phi", seedPair.second->getPhi());
            plots->Fill("true_rho", seedPair.second->getRho());
            plots->Fill2d("HitsWithinRange", seedPair.second->distanceTo(predictedHit), recoveredHits.size());
            
            plots->Fill2d("residualsPhi_vs_pT",  trueTrack->getPt(),   seedPair.second->distanceToPhi(predictedHit, true));
            plots->Fill2d("residualsPhi_vs_eta", trueTrack->getEta(), seedPair.second->distanceToPhi(predictedHit, true));
            plots->Fill2d("residualsPhi_vs_d0",  trueTrack->getD0(),   seedPair.second->distanceToPhi(predictedHit, true));
            plots->Fill2d("residualsPhi_vs_z0",  trueTrack->getZ0(),   seedPair.second->distanceToPhi(predictedHit, true));
            plots->Fill2d("residualsPhi_vs_phi", trueTrack->getPhi(), seedPair.second->distanceToPhi(predictedHit, true));

            plots->Fill2d("residualsZ_vs_pT",  trueTrack->getPt(),   seedPair.second->distanceToZ(predictedHit, true));
            plots->Fill2d("residualsZ_vs_eta", trueTrack->getEta(), seedPair.second->distanceToZ(predictedHit, true));
            plots->Fill2d("residualsZ_vs_d0",  trueTrack->getD0(),   seedPair.second->distanceToZ(predictedHit, true));
            plots->Fill2d("residualsZ_vs_z0",  trueTrack->getZ0(),   seedPair.second->distanceToZ(predictedHit, true));
            plots->Fill2d("residualsZ_vs_phi", trueTrack->getPhi(), seedPair.second->distanceToZ(predictedHit, true));

            plots->Fill2d("residualsRho_vs_pT",  trueTrack->getPt(),   seedPair.second->distanceToRho(predictedHit, true));
            plots->Fill2d("residualsRho_vs_eta", trueTrack->getEta(), seedPair.second->distanceToRho(predictedHit, true));
            plots->Fill2d("residualsRho_vs_d0",  trueTrack->getD0(),   seedPair.second->distanceToRho(predictedHit, true));
            plots->Fill2d("residualsRho_vs_z0",  trueTrack->getZ0(),   seedPair.second->distanceToRho(predictedHit, true));
            plots->Fill2d("residualsRho_vs_phi", trueTrack->getPhi(), seedPair.second->distanceToRho(predictedHit, true));

            if (recoveredHit == nullptr) continue;
            // plots->FillProfile("residualsPhi_vs_pT",  trueTrack->getPt(),   seedPair.second->distanceToPhi(predictedHit, true));
            // plots->FillProfile("residualsPhi_vs_eta", trueTrack->getEta(), seedPair.second->distanceToPhi(predictedHit, true));
            // plots->FillProfile("residualsPhi_vs_d0",  trueTrack->getD0(),   seedPair.second->distanceToPhi(predictedHit, true));
            // plots->FillProfile("residualsPhi_vs_z0",  trueTrack->getZ0(),   seedPair.second->distanceToPhi(predictedHit, true));
            // plots->FillProfile("residualsPhi_vs_phi", trueTrack->getPhi(), seedPair.second->distanceToPhi(predictedHit, true));

            // plots->FillProfile("residualsZ_vs_pT",  trueTrack->getPt(),   seedPair.second->distanceToZ(predictedHit, true));
            // plots->FillProfile("residualsZ_vs_eta", trueTrack->getEta(), seedPair.second->distanceToZ(predictedHit, true));
            // plots->FillProfile("residualsZ_vs_d0",  trueTrack->getD0(),   seedPair.second->distanceToZ(predictedHit, true));
            // plots->FillProfile("residualsZ_vs_z0",  trueTrack->getZ0(),   seedPair.second->distanceToZ(predictedHit, true));
            // plots->FillProfile("residualsZ_vs_phi", trueTrack->getPhi(), seedPair.second->distanceToZ(predictedHit, true));

            // plots->FillProfile("residualsRho_vs_pT",  trueTrack->getPt(),   seedPair.second->distanceToRho(predictedHit, true));
            // plots->FillProfile("residualsRho_vs_eta", trueTrack->getEta(), seedPair.second->distanceToRho(predictedHit, true));
            // plots->FillProfile("residualsRho_vs_d0",  trueTrack->getD0(),   seedPair.second->distanceToRho(predictedHit, true));
            // plots->FillProfile("residualsRho_vs_z0",  trueTrack->getZ0(),   seedPair.second->distanceToRho(predictedHit, true));
            // plots->FillProfile("residualsRho_vs_phi", trueTrack->getPhi(), seedPair.second->distanceToRho(predictedHit, true));
           
            if (recoveredHit->getTruthTrackBarcode().at(0) == trkIndex){
                plots->Fill("matchedHitFrac", 1);
                plots->Fill("matchedHit_resPhi", seedPair.second->distanceToPhi(predictedHit, true));
                plots->Fill("matchedHit_resRho", seedPair.second->distanceToPhi(predictedHit, true));
                plots->Fill("matchedHit_resZ", seedPair.second->distanceToZ(predictedHit, true));
                plots->Fill("matchedHit_deltaR", seedPair.second->distanceTo(predictedHit));
                plots->Fill("matchedHit_true_z",   seedPair.second->getZ());
                plots->Fill("matchedHit_true_phi", seedPair.second->getPhi());
                plots->Fill("matchedHit_true_rho", seedPair.second->getRho());
                bookkeeper->Fill("matchedHitFrac" + volAndlayer, 1, 2, 0, 2);
            }
            else{
                plots->Fill("matchedHitFrac", 0);
                plots->Fill("unmatchedHit_resPhi", seedPair.second->distanceToPhi(predictedHit, true));
                plots->Fill("unmatchedHit_resRho", seedPair.second->distanceToPhi(predictedHit, true));
                plots->Fill("unmatchedHit_resZ", seedPair.second->distanceToZ(predictedHit, true));
                plots->Fill("unmatchedHit_deltaR", seedPair.second->distanceTo(predictedHit));
                plots->Fill("unmatchedHit_true_z",   seedPair.second->getZ());
                plots->Fill("unmatchedHit_true_phi", seedPair.second->getPhi());
                plots->Fill("unmatchedHit_true_rho", seedPair.second->getRho());
                bookkeeper->Fill("matchedHitFrac" + volAndlayer, 0, 2, 0, 2);
            }
        } //loop over seeds
    }      

   
    TFile* outFile = new TFile(TString(opts["outputFile"]), "recreate");
    TDirectory* subdir = outFile->mkdir("perDetector");
    

    outFile->mkdir("fullDetector");
    outFile->cd("fullDetector");
    auto histList = plots->getHistMap();
    for(auto& hist: histList) hist.second->Write();
    for(auto& hist: plots->getProfileMap()) hist.second->Write();
    auto histList2d = plots->get2dHistMap();
    for(auto& hist: histList2d) hist.second->Write();
    subdir->cd();
    auto histMap = bookkeeper->getHistograms();
    for(auto& it: histMap) it.second->Write();
    outFile->cd("");
    if (plotOverlay == "true")
        hitPlotter->plotTrueAndPredictedHitsList(trueHits, predictedHits, "predictedHitsOverlay", false);

    outFile->Close();


}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ACTS_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["onnxFile"]        = "TrackNNAnalysis/data/ExtrapolationModelsITK/extrap_barrelOnly_ws3_biRNN.onnx";
    opts["windowSize"]      = "3";
    opts["plotOverlay"]     = "false";
    opts["LayerRegion"]  = "barrel";
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

