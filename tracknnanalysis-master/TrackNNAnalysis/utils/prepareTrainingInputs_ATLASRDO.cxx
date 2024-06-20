// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>
#include <stdlib.h>

// Local include
#include "TrackNNDataReader/DataReader.h"

#include "PathResolver/PathResolver.h"

enum class OutHitType 
{ 
    Nominal,
    SP
};

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);

int main(int argc, char *argv[])
{   
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    auto dataReader = make_shared<DataReader>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    std::cout<<"dataReader initialized"<<std::endl;

    int nEvents = atoi(opts["nEvents"].c_str());


    if(nEvents < 0) nEvents = dataReader->getEntries();


    // variables to save in the output
    std::vector<float>* track_hit_x = new std::vector<float>();
    track_hit_x->reserve(40);
    std::vector<float>* track_hit_y = new std::vector<float>();
    track_hit_y->reserve(40);
    std::vector<float>* track_hit_z = new std::vector<float>();
    track_hit_z->reserve(40);
    std::vector<float>* track_hit_r = new std::vector<float>();
    track_hit_r->reserve(40);
    std::vector<float>* track_hit_rho = new std::vector<float>();
    track_hit_rho->reserve(40);
    std::vector<float>* track_hit_layerID = new std::vector<float>();
    track_hit_layerID->reserve(40);
    std::vector<float>* track_hit_volumeID = new std::vector<float>();
    track_hit_volumeID->reserve(40);
    float track_pT = -999;
    float track_qoverpT = -999;
    float track_eta = -999;
    float track_phi = -999;
    float track_d0 = -999;
    float track_z0 = -999;

    // setup and create output
    TFile* outFile = new TFile(opts.at("outputFile").c_str(), "RECREATE");
    TTree* outTree = new TTree("AthenaRecoTracks", "AthenaRecoTracks");
    outTree->Branch("track_hit_x", &track_hit_x);
    outTree->Branch("track_hit_y", &track_hit_y);
    outTree->Branch("track_hit_z", &track_hit_z);
    outTree->Branch("track_hit_r", &track_hit_r);
    outTree->Branch("track_hit_rho", &track_hit_rho);
    outTree->Branch("track_hit_layerID", &track_hit_layerID);
    outTree->Branch("track_hit_volumeID", &track_hit_volumeID);
    outTree->Branch("track_qoverpT", &track_qoverpT);
    outTree->Branch("track_pT", &track_pT);
    outTree->Branch("track_eta", &track_eta);
    outTree->Branch("track_phi", &track_phi);
    outTree->Branch("track_d0", &track_d0);
    outTree->Branch("track_z0", &track_z0);


    // To not have string comparisions 
    OutHitType hitType = OutHitType::Nominal;
    if(opts["hitType"].find("Nominal") != std::string::npos) hitType = OutHitType::Nominal;
    if(opts["hitType"].find("SP")      != std::string::npos) hitType = OutHitType::SP;

    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        auto tracks = dataReader->getRecoTracks();

        for(auto& trk: tracks)
        {
            track_hit_x->clear();
            track_hit_y->clear();
            track_hit_z->clear();
            track_hit_r->clear();
            track_hit_rho->clear();
            track_hit_layerID->clear();
            track_hit_volumeID->clear();

            track_pT  = trk->getPt();
            track_eta = trk->getEta();
            track_phi = trk->getPhi();
            track_d0  = trk->getD0();
            track_z0  = trk->getZ0();
            track_qoverpT = trk->getCharge()/trk->getPt();

            std::vector<std::shared_ptr<Hit>> hitList = trk->getHitsList();
            if(hitType == OutHitType::SP) hitList = trk->getSPHitsList();
            for(const auto& hit:hitList)
            {
                track_hit_x->push_back(hit->getX());
                track_hit_y->push_back(hit->getY());
                track_hit_z->push_back(hit->getZ());
                track_hit_r->push_back(hit->getR());
                track_hit_rho->push_back(hit->getRho());
                track_hit_layerID->push_back(hit->getLayer());
                track_hit_volumeID->push_back(hit->getVolume());
            }

            trk->printDebug();

            outTree->Fill();

        }
    }

    outTree->Write();
    outFile->Close();

}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "DetectorGeo/DetectorGeo_ACTS_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["hitType"]         = "Nominal";


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
            cout<<"--hitType            : Options Nominal, SP"<<endl;
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
