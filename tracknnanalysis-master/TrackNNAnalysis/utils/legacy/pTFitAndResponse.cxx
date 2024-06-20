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
#include "TrackNNDataReader/DataReaderACTS.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNAnalysis/TrackManager.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
#include "TrackNNEvaluator/NNEvalExtrapUtils.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNAnalysis/PlotHolder.h"
#include "TrackNNAnalysis/HitGeoPlotter.h"
#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"
#include "TrackNNAnalysis/TrackFitter.h"
#include "PathResolver/PathResolver.h"

// Root include
#include "TEfficiency.h"
#include "TH1F.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
std::vector<TString> tokenizeStr(TString str, TString key);
int main(int argc, char *argv[])
{
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    
    // NN module
    auto NNFile   = PathResolver::find_calib_file (opts["onnxFile"]);
    auto nnEval = make_shared<NNEvalExtrapUtils>(NNFile, atoi(opts["windowSize"].c_str()));
    nnEval->initialize();

    std::string LayerRegion = opts["LayerRegion"];
    auto dataReader = make_shared<DataReaderACTS>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    dataReader->setLayerRegion(LayerRegion);

    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();

    auto hitPlotter = make_shared<HitGeoPlotter>(dataReader->getDetectorModules());
    hitPlotter->initialize();

    // TrackManager to create single-hit-per-layer tracks
    auto trackManager = make_shared<TrackManager>();

    auto trackFitter = make_shared<TrackFitter>(3);
    
    vector<double> response;
    vector<double> pt_spectrum;

    auto pTResponse_hist = new TProfile("pTResponse_hist", "fit p_{T} / true p_{T}", 100,0,1000,0,10);
    auto pTRecoResponse_hist = new TProfile("pTRecoResponse_hist", "fit p_{T} / true p_{T}", 100,0,1000,0,10);
    auto etaResponse_hist = new TProfile("etaResponse_hist", "fit eta / true eta", 20,-1,1,0,10);

    //auto pTCalibration  = PathResolver::find_calib_file("TrackNNAnalysis/calibration/pTCalibration.root");
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        //auto recoTracks = dataReader->getRecoTracks(); 
        auto recoTracks = dataReader-> getOfflineTruthTracks();
        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());
        // use layered tracks instead of pure reco tracks. ACTS does not require layer tracks. already layerec
        auto layeredTracks = trackManager->getTrackSingleLayerHitInBarrel(recoTracks, 7);
        //cout<<"number of tracks: "<<layeredTracks.size()<<endl;
        for(auto& track:layeredTracks)
        {
            cout<<"fitting circle..."<<endl;
            Circle circ;
            auto c =  trackFitter->CircleFitByTaubin(track->getHitsList());
            auto r = c.r;
            c.print();
            double fitpT = 2.*1.60217663*r/1000./5.36;
            //c.r += 20;
            //c.a *= 2;
            //c.b *=1.5;
            //auto code = CircleFitByLevenbergMarquardtFull(track, c,.00001,circ);
            //cout<<code<<endl;
            //circ.print();
            //auto radius = circ.r;
            
            cout<<"alg. fit pT: "<<fitpT<<" true pT: "<<track->getPt()<<endl;
            //cout<<"geo. fit pT: "<<2.*1.60217663*radius/1000./5.36<<" true pT: "<<track->getPt()<<endl;
            double res = abs(2.*1.60217663*r/1000./5.36 / track->getPt());
            response.push_back(res);
            pt_spectrum.push_back(track->getPt());
            pTResponse_hist->Fill(track->getPt(), res, 1);
            pTRecoResponse_hist->Fill(fitpT, res, 1);
            etaResponse_hist->Fill(track->getEta(), res, 1);

        }
        
    }

    TFile* outFile = new TFile(TString(opts["outputFile"]), "recreate");        
    //TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);
    //H1F* frame = (TH1F*) (response_hist->Clone("frame"));
    //frame->Reset();
    pTResponse_hist->GetYaxis()->SetTitle("p_{T, fit}/p_{T, truth}");
    pTResponse_hist->GetXaxis()->SetTitle("p_{T} [GeV]"); 
    pTRecoResponse_hist->GetYaxis()->SetTitle("p_{T, fit}/p_{T, truth}");
    pTRecoResponse_hist->GetXaxis()->SetTitle("p_{T} [GeV]"); 
    etaResponse_hist->GetYaxis()->SetTitle("p_{T, fit}/p_{T, truth}");
    etaResponse_hist->GetXaxis()->SetTitle("#eta"); 
    //response_hist->SetMarkerStyle(2);
    //response_hist->Draw();
    //c1->SaveAs("pTResponse.eps");
    pTResponse_hist->Write();
    pTRecoResponse_hist->Write();
    etaResponse_hist->Write();
    outFile->Close();
}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ACTS_ODD.root";
    opts["outputFile"]      = "pTResponse.root";
    opts["nEvents"]         = "-1";
    opts["onnxFile"]        = "TrackNNAnalysis/ExtrapolationModels/Extrap_RNN_ACTS_ws3_barrel_layerInfo.onnx";
    opts["LayerRegion"]  = "barrel";


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

        if (opt == "dropOverlay") 
        {
            opts["dropOverlay"] = "true";
            continue;
        }
        if (opt == "overlayOnly") 
        {
            opts["overlayOnly"] = "true";
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

