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
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/FakeManager.h"
#include "TrackNNAnalysis/OutputManager.h"
#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"

// ltwnn includes
#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"
#include "lwtnn/Exceptions.hh"
#include "lwtnn/lightweight_nn_streamers.hh"
#include "lwtnn/NanReplacer.hh"

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

int main(int argc, char *argv[])
{   
    setPrettyStuff();

    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;

    auto dataReader = make_shared<DataReader>(opts.at("inputFile"), PathResolver::find_calib_file (opts.at("detectorGeoFile")));
    dataReader->initialize();
    dataReader->dropPixHit();

    auto fakeManager = make_shared<FakeManager>();

    // create the ltwnn 
    std::string weightsFileName = opts["trainFile"];
    std::ifstream input_cfg(weightsFileName.c_str());
    auto cfg = lwt::parse_json_graph( input_cfg );
    auto lwnn = std::shared_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph( cfg ));

    TEfficiency* pTEff  = new TEfficiency("pTEff","pTEff",  40, 0 , 2*1000);
    TEfficiency* etaEff = new TEfficiency("etaEff","etaEff", 20,-3, 3);

    map<TString, TH1F*> m_histList;

    for(const auto& type: vector<TString> {"TrueTracks", "FullyLayerRandom", "ModuleRandom", "ModuleRotatedRandom", "ModuleIterRandom", "ModuleRotatedIterRandom", "FullCombo"})
    {
        m_histList[type] = new TH1F(type, type, 200, 0, 1);
    }

    // Need this to be a function
    auto evalNN = [&lwnn](std::shared_ptr<TrackBase> trk)
    {
        // Input maps for lwtnn
        std::map<std::string, std::map<std::string, double>> valMap;
        std::map<std::string, std::map<std::string, std::vector<double>>> vectorMap;
        vectorMap["TrackrNN"] = {};
        vectorMap["TrackrNN"]["x"].clear();
        vectorMap["TrackrNN"]["y"].clear();
        vectorMap["TrackrNN"]["z"].clear();
        
        for(const auto& hit: trk->getHitsRotatedToZeroList())
        {
            vectorMap["TrackrNN"]["x"].push_back(hit->getX());
            vectorMap["TrackrNN"]["y"].push_back(hit->getY());
            vectorMap["TrackrNN"]["z"].push_back(hit->getZ());
        }

        auto scoreMap = lwnn->compute(valMap, vectorMap);
        trk->setNNScore(scoreMap["trueNNProb"]);   
    };
    
    // Need to convert this into a function
    auto evalNReco = [&evalNN](std::vector<std::shared_ptr<RecoTrack>> trkList)
    {
        for(auto& trk: trkList)
        {
            evalNN(dynamic_pointer_cast<TrackBase, RecoTrack>(trk));
        }
    };

    auto evalNFake = [&evalNN](std::vector<std::shared_ptr<FakeTrack>> trkList)
    {
        for(auto& trk: trkList)
        {
            evalNN(dynamic_pointer_cast<TrackBase, FakeTrack>(trk));
        }
    };


    int nEvents = atoi(opts["nEvents"].c_str());
    if(nEvents < 0) nEvents = dataReader->getEntries();
    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        dataReader->loadEntry(i);

        auto recoTrack = dataReader->getRecoTracks();
        evalNReco(recoTrack);
        for(auto& trk: recoTrack)
        { 
            double pass = false;
            if(trk->getNNScore() > 0.50) pass = true;
            m_histList["TrueTracks"]->Fill(trk->getNNScore());

            pTEff->Fill(pass, trk->getPt());
            etaEff->Fill(pass, trk->getEta());
        }
        auto hitManager = make_shared<HitsManager>(dataReader->getHits(), dataReader->getDetectorModules());
        fakeManager->setHitManager(hitManager);


        if(opts["fakeTypes"].find("FullComb") != std::string::npos)
        {
            auto fullComb = fakeManager->getFakeTracks(FakeType::FullCombinatoricsinRegions, recoTrack, recoTrack.size());
            evalNFake(fullComb);
            for(auto& trk: fullComb) m_histList["FullCombo"]->Fill(trk->getNNScore());
        }
        if(opts["fakeTypes"].find("FullyRandom") != std::string::npos)
        {
            auto fullyRandom = fakeManager->getFakeTracks(FakeType::FullyRandom, recoTrack, recoTrack.size());
            evalNFake(fullyRandom);
            for(auto& trk: fullyRandom) m_histList["FullyLayerRandom"]->Fill(trk->getNNScore());
        }
        if(opts["fakeTypes"].find("LayerRandom") != std::string::npos)
        {
            auto layerRandom = fakeManager->getFakeTracks(FakeType::LayerRandom, recoTrack, recoTrack.size());
            evalNFake(layerRandom);
            for(auto& trk: layerRandom) m_histList["FullyLayerRandom"]->Fill(trk->getNNScore());
        }
        if(opts["fakeTypes"].find("ModuleRandom") != std::string::npos)
        {
            auto moduleRandom = fakeManager->getFakeTracks(FakeType::ModuleRandom, recoTrack, recoTrack.size());
            evalNFake(moduleRandom);
            for(auto& trk: moduleRandom) m_histList["ModuleRandom"]->Fill(trk->getNNScore());
        }
        if(opts["fakeTypes"].find("ModuleRotatedRandom") != std::string::npos)
        {
            auto moduleRotatedRandom = fakeManager->getFakeTracks(FakeType::ModuleRotatedRandom, recoTrack, recoTrack.size());
            evalNFake(moduleRotatedRandom);
            for(auto& trk: moduleRotatedRandom) m_histList["ModuleRotatedRandom"]->Fill(trk->getNNScore());
        }
        if(opts["fakeTypes"].find("ModuleIterRandom") != std::string::npos)
        {
            auto moduleIterRandom = fakeManager->getFakeTracks(FakeType::ModuleIterRandom, recoTrack, recoTrack.size());
            evalNFake(moduleIterRandom);
            for(auto& trk: moduleIterRandom) m_histList["ModuleIterRandom"]->Fill(trk->getNNScore());
        }
        if(opts["fakeTypes"].find("ModuleRotatedIterRandom") != std::string::npos)
        {
            auto moduleRotatedIterRandom = fakeManager->getFakeTracks(FakeType::ModuleRotatedIterRandom, recoTrack, recoTrack.size());
            evalNFake(moduleRotatedIterRandom);
            for(auto& trk: moduleRotatedIterRandom) m_histList["ModuleRotatedIterRandom"]->Fill(trk->getNNScore());
        }



    }





    ////////////////////////////
    // Normal discriminant plot
    ///////////////////////////
    int index = 0;
    double max = -1;
    for(const auto& hist: m_histList)
    {
        if(hist.second->Integral() == 0) continue;
        hist.second->Scale(1.0/hist.second->Integral());
        hist.second->SetLineColor(1754 + index);
        hist.second->SetLineWidth(2);
        if(hist.second->GetMaximum() > max) max = hist.second->GetMaximum();
        index ++;
    }

    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    TH1F* frame = new TH1F("frame", "frame", 1, 0, 1);
    frame->SetMaximum(max * 1.3);
    frame->GetXaxis()->SetTitle("NN score");
    frame->GetYaxis()->SetTitle("Entries");
    frame->Draw();
    
    for(const auto& hist: m_histList)
    {
        if(hist.second->Integral() == 0) continue;
        hist.second->Draw("histSame");
    }

    float leftDist = 0.19;

    ATLASLabel(leftDist, 0.875, "Internal", 1);

    TString channelStr = "TrackingNN";
    TLatex* tMain = new TLatex (leftDist, 0.81, channelStr);
    tMain->SetNDC();
    tMain->SetTextSize(0.0425);
    tMain->SetTextFont(42);
    tMain->Draw();

    TLegend *elLeg =  new TLegend (0.6, 0.76, 0.90, 0.925);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.0275);
    for(const auto& hist: m_histList)
    {
        if(hist.second->Integral() == 0) continue;
        elLeg->AddEntry(hist.second, hist.first, "l"); 
    }
    elLeg->Draw();  
    c1->SetLogy();

    c1->SaveAs("discriminant.pdf");

    c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);
    pTEff->SetTitle(";pT [MeV];#epsilon");
    pTEff->SetMarkerColor(1754);
    pTEff->SetLineColor(1754);
    pTEff->Draw("AP");
    c1->SaveAs("pTEff.pdf");

    c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);
    etaEff->SetTitle(";#eta;#epsilon");
    etaEff->SetMarkerColor(1754);
    etaEff->SetLineColor(1754);
    etaEff->Draw("AP");
    c1->SaveAs("EtaEff.pdf");

}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["detectorGeoFile"] = "TrackNNAnalysis/DetectorGeo/DetectorGeo_ITK.root";
    opts["outputFile"]      = "Outtree.root";
    opts["nEvents"]         = "-1";
    opts["fakeTypes"]       = "";
    opts["trainFile"]       = "NN_D7_trackingTest_T1.json";

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
            cout<<"--fakeTypes          : Comma seperated list of all the fake types to do. Options FullyRandom, LayerRandom, ModuleRandom, ModuleRotatedRandom, ModuleIterRandom, ModuleRotatedIterRandom"<<endl;
            cout<<"--trainFile          : JSON file for lwtnn for which to check the training"<<endl;
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
    //new TColor(ci, 0.6350,    0.0780,    0.1840); //62
    //ci++;
    //new TColor(ci, 142.0/255 , 0.0/255 , 62.0/255);
    //ci++;
    //new TColor(ci, 96.0/255 , 78.0/255 , 0.0/255);
    //ci++;
    //new TColor(ci, 92.0/255 , 174.0/255 , 0.0/255);
    //ci++;
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