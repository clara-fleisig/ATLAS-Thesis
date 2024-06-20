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
#include "TrackNNDataReader/DataReaderHTT.h"
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

    auto dataReader = make_shared<DataReaderHTT>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->setLayerRegion(opts["LayerRegion"]);
    dataReader->initialize();
    
    auto outputManager = make_shared<OutputManager>(opts.at("outputFile"),10);
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
    
    auto trackPlotter = make_shared<TrackGeoPlotter>(dataReader->getDetectorModules());
    trackPlotter->initialize();

    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();
    auto randGen = new TRandom3();
    randGen->SetSeed(1);
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        auto recoTrackAll = dataReader->getRecoTracks();

        // For Plotting
        // vector<std::shared_ptr<TrackBase>> trkToPlot;

        // for(const auto& trk: recoTrack)
        // {
        //     trkToPlot.push_back(dynamic_pointer_cast<TrackBase, RecoTrack>(trk));
        //     // if(trkToPlot.size() > 1000) break;

        // }
        // trackPlotter->plotTracks(trkToPlot, "All");
        // exit(1);

        auto truthTracks = dataReader->getOfflineTruthTracks();
        std::vector<std::shared_ptr<RecoTrack>> recoTrack;


        for(auto& trk: recoTrackAll)
        {
            //trk->printDebug();
            //recoTrack.push_back(trk);

            if(opts["doSubSample"] == "true")
            {
                // Subsample tracks based on pT
                if(trk->getPt() > 4) recoTrack.push_back(trk);
                else if (trk->getPt() > 1.5)
                {
                    if(randGen->Uniform(1) < 0.15) recoTrack.push_back(trk);
                }
                else
                {
                    if(randGen->Uniform(1) < 0.10) recoTrack.push_back(trk);
                }
            }
            else
            {
                recoTrack.push_back(trk);
            }

            // Check if the current selected track is truth matched to any tracks
            double bestMatch = -1;
            std::shared_ptr<TrueTrack> trueTrack;
            for(auto& truthTrack: truthTracks)
            {
                auto barcode = truthTrack->getBarcode();
                double cScore = trk->getMatchedProbability(barcode);
                if(cScore > 0.5 && cScore > bestMatch)
                {
                    trueTrack = truthTrack;
                    bestMatch = cScore;
                }
            }

            // overwrite some matching info
            if(trueTrack)
            {
                trk->setEta(trueTrack->getEta());
                trk->setD0(trueTrack->getD0());
                trk->setZ0(trueTrack->getZ0());
            }

            // if(trueTrack)
            // {
            //     if(trueTrack->getIntMetadata("charge") != trk->getCharge() && trueTrack->getPt() < 5)
            //     {
            //         cout<<"Event i: "<<i<<" has a mismatch"<<endl;
            //         cout<<"-------- true track charge: "<<trueTrack->getIntMetadata("charge")<<endl;
            //         trueTrack->printDebug();
            //         cout<<"-------- reco track charge: "<<trk->getCharge()<<endl;
            //         trk->printDebug();
            //     }
            // }
            // else
            // {
            //     cout<<"can't find truth track for "<<endl;
            //     trk->printDebug();

            //     cout<<"List of true tracks"<<endl;
            //     for(auto& truthTrack: truthTracks) truthTrack->printDebug();
            // }


        }

        outputManager->saveHTTFakeTracks(recoTrack);
        outputManager->saveHTTTrueTracks(recoTrackAll);

    }

    outputManager->finalize();
}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["doSubSample"]     = "false";
    opts["LayerRegion"]  = "";
    opts["hitType"]         = "RotatedToZero";

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

        i++;
    }

    return true;
}
