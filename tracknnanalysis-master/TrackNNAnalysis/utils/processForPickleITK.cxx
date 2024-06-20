// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>
#include <stdlib.h>
#include <TRandom3.h>

// Local include
#include "TrackNNDataReader/DataReaderITK.h"
#include "TrackNNAnalysis/OutputManager.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"

#include "PathResolver/PathResolver.h"


using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);

int main(int argc, char *argv[])
{   
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    auto dataReader = make_shared<DataReaderITK>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->setLayerRegion(opts["LayerRegion"]);
    dataReader->initialize();
    std::cout<<"dataReader initialized"<<std::endl;

    int windowSize = atoi(opts["windowSize"].c_str());
    int nEvents = atoi(opts["nEvents"].c_str());
    bool barrelOnly = atoi(opts["barrelOnly"].c_str());
    bool endcapOnly = atoi(opts["endcapOnly"].c_str());
    bool cylindrical = atoi(opts["cylindrical"].c_str());

    auto outputManager = make_shared<OutputManager>(opts.at("outputFile"), windowSize);
    std::cout<<"outputManager initialied"<<std::endl;
    outputManager->setDoComputedTrackProb();
    if(opts["hitType"].find("Nominal")                  != std::string::npos) outputManager->setHitType(OutHitType::Nominal);
    if(opts["hitType"].find("RotatedToZero")            != std::string::npos) outputManager->setHitType(OutHitType::RotatedToZero);
    if(opts["hitType"].find("RoughRotatedToZero")       != std::string::npos) outputManager->setHitType(OutHitType::RoughRotatedToZero);
    if(opts["hitType"].find("SP")                       != std::string::npos) outputManager->setHitType(OutHitType::SP);
    if(opts["hitType"].find("SPWithRotatedToZero")      != std::string::npos) outputManager->setHitType(OutHitType::SPWithRotatedToZero);
    if(opts["hitType"].find("SPWithRoughRotatedToZero") != std::string::npos) outputManager->setHitType(OutHitType::SPWithRoughRotatedToZero);
    if(opts["hitType"].find("SPWithUnscaledRotatedToZero") != std::string::npos) outputManager->setHitType(OutHitType::SPWithUnscaledRotatedToZero);
    if(opts["hitType"].find("SPCylindrical") != std::string::npos)              outputManager->setHitType(OutHitType::SPCylindrical);
    if(opts["hitType"].find("SPCylindricalRotatedToZero") != std::string::npos) outputManager->setHitType(OutHitType::SPCylindricalRotatedToZero);
    
    std::cout<<"hitType configured"<<std::endl;

    auto trackPlotter = make_shared<TrackGeoPlotter>(dataReader->getDetectorModules());
    trackPlotter->initialize();
    std::cout<<"trackPlotter initialized"<<std::endl;

    if(nEvents < 0) nEvents = dataReader->getEntries();
    auto randGen = new TRandom3();
    randGen->SetSeed(1);

    vector<std::shared_ptr<TrackBase>> trkToPlot;

    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);
        //std::cout<<"entry loaded"<<std::endl;
        auto tracks = dataReader->getOfflineTruthTracks();
        //std::cout<<"reco tracks loaded"<<std::endl;
        //decltype(recoTrackAll) recoTrack;
        decltype(tracks) truthTracks;
        for(auto& trk: tracks)
        {
           // trkToPlot.push_back(dynamic_pointer_cast<TrackBase, RecoTrack>(trk));
           // if(trkToPlot.size() > 5) break;
            // trk->printDebug();
            if(opts["doSubSample"] == "true")
            {
                // Subsample tracks based on pT
                if(trk->getPt() > 4) truthTracks.push_back(trk);
                else if (trk->getPt() > 1.5)
                {
                    if(randGen->Uniform(1) < 0.15) truthTracks.push_back(trk);
                }
                else
                {
                    if(randGen->Uniform(1) < 0.075) truthTracks.push_back(trk);
                }
            }
            else
            {
                truthTracks.push_back(trk);
            }
        }
        //outputManager->saveHTTFakeTracks(recoTrack);
        //outputManager->saveHTTTrueTracks(recoTrackAll);
        // outputManager->saveTrueTruthTracks(dataReader->getOfflineTruthTracks());
        outputManager->saveTruthTrainingData(truthTracks, windowSize, barrelOnly, endcapOnly, cylindrical);
    }
    // trackPlotter->plotTracks(trkToPlot, "ACTS");


    outputManager->finalize();
}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "DetectorGeo/DetectorGeo_ACTS_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["doSubSample"]     = "false";
    opts["LayerRegion"]  = "noCut";
    opts["hitType"]         = "Nominal";
    opts["windowSize"]         = "3";
    opts["barrelOnly"]         = "0";
    opts["endcapOnly"]         = "0";
    opts["cylindrical"]        = "0";

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
            cout<<"--doSubSample        : true/false subsample low pT"<<endl;
            cout<<"--hitType            : Options Nominal, RotatedToZero, SP, SPWithRotatedToZero"<<endl;
            cout<<"--LayerRegion     : choose which layer config"<<endl;
            return false;
 
        }

        if(0!=opt.find("--")) {
            cout<<"ERROR: options start with '--'!"<<endl;
            cout<<"ERROR: options is: "<<opt<<endl;
            return false;
        }
        opt.erase(0,2);
        if (opt == "doSubSample") 
        {
            opts["doSubSample"] = "true";
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
        // if (opt == "endcapOnly") 
        // {
        //     opts["endcapOnly"] = "1";
        //     continue;
        // }
        // if (opt == "barrelOnly") 
        // {
        //     opts["barrelOnly"] = "1";
        //     continue;
        // }
        i++;
    }

    return true;
}