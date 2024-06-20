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
#include "TrackNNDataReader/DataReader.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNAnalysis/TrackManager.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
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
#include "TString.h"

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
std::vector<TString> tokenizeStr(TString str, TString key);

int main(int argc, char *argv[])
{
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    
    // NN module
    int trackNumber = atoi(opts["trackNumber"].c_str());
    auto dataReader = make_shared<DataReader>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
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
    Index barcode = atoll(opts["barcode"].c_str());
    
    // seedManaged to create the seeds
    auto seedManager = make_shared<SeedManager>();
    seedManager->setSeedSize(windowSize);
    seedManager->setWindowSize(windowSize);

    // TrackManager to create single-hit-per-layer tracks
    auto trackManager = make_shared<TrackManager>();
    
    map<TString, shared_ptr<TrackBase>> trackToPlot;
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        //auto recoTracks = dataReader->getRecoTracks();       
        auto recoTracks = dataReader->getTrueTracks();
        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());
        // auto layeredTracks = trackManager->getTrackSingleLayerHitInBarrel(recoTracks,7);
        //second argument true for barrel only tracks
        for (auto& trk: recoTracks)
        {
            cout<<"input barcode: "<< barcode << " track Barcode: "<< trk->getBarcode() << endl;
            //trk->printDebug();
            // if (trk->getBarcode() == barcode)
            // {   
                // trk->printDebug();
                //upcast from truetrack to trackbase
                shared_ptr<TrackBase> tTrk = trk;
                trackToPlot[""] = tTrk;
                // break;
            // } 
            TString trackImage("track");
            trackImage += TString::Itoa(trk->getBarcode(), 10);
            trackPlotter->plotTracks(trackToPlot, trackImage);
            trackToPlot.clear();

        }
    }
    
    TFile* outFile = new TFile(TString(opts["outputFile"]), "recreate");
    outFile->mkdir("track");
    outFile->cd("track");
    // trackPlotter->plotTracks(trackToPlot, trackImage);
    outFile->cd("");
        
    
    //outFile->cd("");

    outFile->Close();


}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ACTS_ODD.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["outputFile"]      = "outputFile.root";
    opts["windowSize"]      = "3";
    opts["radialError"]     = "10";
    opts["barcode"]         = "0";


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
            cout<<"--barcode            : The barcode of the track to print"<<endl;
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

