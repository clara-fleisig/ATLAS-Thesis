// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <memory>

// Local includes
#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"
#include "TrackNNAnalysis/PlotHolder.h"

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
#include "TGraphAsymmErrors.h"

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
void setPrettyStuff();

void plotHist(map<TString, TH1F*> histList, TString outName, bool setLog, bool norm, bool setLogX, std::string uncertaintyType);
void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLogY, bool setLogX, float min, float max, std::string uncertaintyType);
TString getUncertaintyLabel(std::string uncertaintyType);
TString getNHitsLabel();
TString getAlgoLabel();
TString getRegionLabel();
TString subFolderKey = "";
TString trackingInfo = "";
std::vector<TString> tokenizeStr(TString str, TString key);

map<std::string, std::string> opts;
plotHolderACTSExtrapolation* plotHolder = NULL;

int main(int argc, char *argv[])
{   
    setPrettyStuff();

    // Process command line arguments
    if(!cmdline(argc,argv, opts)) return 0;

    plotHolder = new plotHolderACTSExtrapolation();
    plotHolder->readHist(TString(opts["inputFile"]));
    int uncertainty     = atoi(opts.at("uncertainty").c_str());
    int minHits         = atoi(opts["minHits"].c_str());
    float phiUncertainty = atof(opts["phiUncertainty"].c_str());
    float zUncertainty   =  atof(opts["zUncertainty"].c_str());  
    float NNCutVal = atof(opts["NNCutVal"].c_str());
    float nHitOverlapThres   =  atof(opts["nHitOverlapThres"].c_str());  

    std::string uncertaintyType = opts["uncertaintyType"];

    map<TString, TH1F*> histList;
    if(uncertaintyType == "radial")
    {
        // histList = plotHolder->getHistMap(uncertainty, minHits);
        // subFolderKey = plotHolder->getKey(uncertainty, minHits);
        histList = plotHolder->getHistMap(uncertainty, minHits, NNCutVal, nHitOverlapThres);
        subFolderKey = plotHolder->getKey(uncertainty, minHits, NNCutVal, nHitOverlapThres);
    }
    else if(uncertaintyType == "layer")
    {

        histList = plotHolder->getHistMap(zUncertainty, phiUncertainty, minHits);
        subFolderKey = plotHolder->getKey(zUncertainty, phiUncertainty, minHits);
    }
    else
    {
        std::cout<<"uncertainty type: "<<uncertaintyType<<" not understood, exiting"<<std::endl;
        exit(1);
    }
    map<TString, TH1F*> histToPlot;

    map<TString, float> valMap;
    for(int i = 0; i < 10; i++)
    {
        if(histList["counterHist"]->GetBinContent(i+1) <= 0) continue;
        cout<<histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)<<" "<<histList["counterHist"]->GetBinContent(i+1)<<endl;
        valMap[histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)] = histList["counterHist"]->GetBinContent(i+1);
    }

    cout<<"nTracksPerEvent: "<<histList["nTracksPerEvent"]->GetMean()<<endl;
    cout<<"nTracksPerEventNNSelected: "<<histList["nTracksPerEventNNSelected"]->GetMean()<<endl;
    cout<<"nTracksPerEventNNOverlap: "<<histList["nTracksPerEventNNOverlap"]->GetMean()<<endl;
    cout<<"nTracksPerEventMatched: "<<histList["nTracksPerEventMatched"]->GetMean()<<endl;
    cout<<"nTracksPerEventMatchedNNNSelected: "<<histList["nTracksPerEventMatchedNNSelected"]->GetMean()<<endl;
    cout<<"nTracksPerEventMatchedNNOverlap: "<<histList["nTracksPerEventMatchedNNOverlap"]->GetMean()<<endl;

    //trackingInfo =   Form("N_{true} = %.2E, N_{reco} = %.2E, purity %.3f", valMap["nTracks"], valMap["nRoads"], valMap["nTracks"]/valMap["nRoads"]);


    vector<float> bins;
    TString name;
    map<TString, TEfficiency*> effToPlot;
    map<TString, TH1F*> toPlot;


    bins = {1, 1.5, 2, 2.5, 3, 4, 5, 6, 10,  20, 100};

    histList["rTruthAllPt"]              = new TH1F("rTruthAllPt",                "rTruthAllPt",               bins.size()-1, &bins[0]);
    histList["rTruthMatchedPt"]          = new TH1F("rTruthMatchedPt",            "rTruthMatchedPt",           bins.size()-1, &bins[0]);
    histList["rTruthSelectedPt"]         = new TH1F("rTruthSelectedPt",           "rTruthSelectedPt",          bins.size()-1, &bins[0]);
    histList["rTruthOverlapPt"]          = new TH1F("rTruthOverlapPt",            "rTruthOverlapPt",           bins.size()-1, &bins[0]);


    for(int i = 0; i < histList["TruthAllPt"]->GetNbinsX(); i++)
    {
        double center = histList["TruthAllPt"]->GetBinCenter(i+1);
        if(center > 100) center = 50;
        if(center < 1) center = 1.25;
        histList["rTruthAllPt"]             ->Fill(center, histList["TruthAllPt"]->GetBinContent(i+1));
        histList["rTruthMatchedPt"]         ->Fill(center, histList["TruthMatchedPt"]->GetBinContent(i+1));
        histList["rTruthSelectedPt"]        ->Fill(center, histList["TruthMatchedNNSelectedPt"]->GetBinContent(i+1));
        histList["rTruthOverlapPt"]         ->Fill(center, histList["TruthMatchedNNOverlapPt"]->GetBinContent(i+1));
    }

    for(int i = 0; i < histList["rTruthAllPt"]->GetNbinsX(); i++)
    {
        histList["rTruthAllPt"]             ->SetBinError(i+1,  sqrt(histList["rTruthAllPt"]->GetBinContent(i+1)));
        histList["rTruthMatchedPt"]         ->SetBinError(i+1,  sqrt(histList["rTruthMatchedPt"]->GetBinContent(i+1)));
        histList["rTruthSelectedPt"]        ->SetBinError(i+1,  sqrt(histList["rTruthSelectedPt"]->GetBinContent(i+1)));
        histList["rTruthOverlapPt"]         ->SetBinError(i+1,  sqrt(histList["rTruthOverlapPt"]->GetBinContent(i+1)));
    }

    histList["TruthAllPt"]                  = histList["rTruthAllPt"];
    histList["TruthMatchedPt"]              = histList["rTruthMatchedPt"];
    histList["TruthMatchedNNSelectedPt"]    = histList["rTruthSelectedPt"];
    histList["TruthMatchedNNOverlapPt"]     = histList["rTruthOverlapPt"];


    histList["TruthAllPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["TruthMatchedPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["TruthMatchedNNSelectedPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedNNSelectedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["TruthMatchedNNOverlapPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedNNOverlapPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");

    TEfficiency* effHTTAll = new TEfficiency(*histList["TruthMatchedPt"], *histList["TruthAllPt"]);
    name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedPt"]->Integral()/histList["TruthAllPt"]->Integral());
    effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNSelectedPt"], *histList["TruthAllPt"]);
    // name = Form("1_NN Extrap + Cut: Ave %.3f", histList["TruthMatchedNNSelectedPt"]->Integral()/histList["TruthAllPt"]->Integral());
    // effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNOverlapPt"], *histList["TruthAllPt"]);
    // name = Form("2_NN Extrap + HW + Cut: Ave %.3f", histList["TruthMatchedNNOverlapPt"]->Integral()/histList["TruthAllPt"]->Integral());
    // effToPlot[name] = effHTTAll;


    plotEff(effToPlot, "EffpT", false, true, 0,1.1, uncertaintyType);
    effToPlot.clear();

    histList["TruthAllEta"]->Rebin(100);
    histList["TruthMatchedEta"]->Rebin(100);
    histList["TruthMatchedNNSelectedEta"]->Rebin(100);
    histList["TruthMatchedNNOverlapEta"]->Rebin(100);

    histList["TruthAllEta"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllEta"]->GetXaxis()->SetTitle("#eta");
    histList["TruthMatchedEta"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedEta"]->GetXaxis()->SetTitle("#eta");
    effHTTAll = new TEfficiency(*histList["TruthMatchedEta"], *histList["TruthAllEta"]);
    name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedEta"]->Integral()/histList["TruthAllEta"]->Integral());
    effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNSelectedEta"], *histList["TruthAllEta"]);
    // name = Form("1_NN Extrap + Cut: Ave %.3f", histList["TruthMatchedNNSelectedEta"]->Integral()/histList["TruthAllEta"]->Integral());
    // effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNOverlapEta"], *histList["TruthAllEta"]);
    // name = Form("2_NN Extrap + HW + Cut: Ave %.3f", histList["TruthMatchedNNOverlapEta"]->Integral()/histList["TruthAllEta"]->Integral());
    // effToPlot[name] = effHTTAll;

    plotEff(effToPlot, "EffEta", false, false, 0,1.1, uncertaintyType);
    effToPlot.clear();

    histList["TruthAllPhi"]->Rebin(100);
    histList["TruthMatchedPhi"]->Rebin(100);
    histList["TruthMatchedNNSelectedPhi"]->Rebin(100);
    histList["TruthMatchedNNOverlapPhi"]->Rebin(100);
    histList["TruthAllPhi"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllPhi"]->GetXaxis()->SetTitle("#phi");
    histList["TruthMatchedPhi"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedPhi"]->GetXaxis()->SetTitle("#phi");
    effHTTAll = new TEfficiency(*histList["TruthMatchedPhi"], *histList["TruthAllPhi"]);
    name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedPhi"]->Integral()/histList["TruthAllPhi"]->Integral());
    effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNSelectedPhi"], *histList["TruthAllPhi"]);
    // name = Form("1_NN Extrap + Cut: Ave %.3f", histList["TruthMatchedNNSelectedPhi"]->Integral()/histList["TruthAllPhi"]->Integral());
    // effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNOverlapPhi"], *histList["TruthAllPhi"]);
    // name = Form("2_NN Extrap + HW + Cut: Ave %.3f", histList["TruthMatchedNNOverlapPhi"]->Integral()/histList["TruthAllPhi"]->Integral());
    // effToPlot[name] = effHTTAll;

    plotEff(effToPlot, "EffPhi", false, false, 0,1.1, uncertaintyType);
    effToPlot.clear();

    histList["TruthAllZ0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllZ0"]->GetXaxis()->SetTitle("Z0");
    histList["TruthMatchedZ0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedZ0"]->GetXaxis()->SetTitle("Z0");
    effHTTAll = new TEfficiency(*histList["TruthMatchedZ0"], *histList["TruthAllZ0"]);
    name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedZ0"]->Integral()/histList["TruthAllZ0"]->Integral());
    effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNSelectedZ0"], *histList["TruthAllZ0"]);
    // name = Form("1_NN Extrap + Cut: Ave %.3f", histList["TruthMatchedNNSelectedPhi"]->Integral()/histList["TruthAllZ0"]->Integral());
    // effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNOverlapZ0"], *histList["TruthAllZ0"]);
    // name = Form("2_NN Extrap + HW + Cut: Ave %.3f", histList["TruthMatchedNNOverlapZ0"]->Integral()/histList["TruthAllZ0"]->Integral());
    // effToPlot[name] = effHTTAll;

    plotEff(effToPlot, "EffZ0", false, false, 0,1.1, uncertaintyType);
    effToPlot.clear();

    histList["TruthAllD0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllD0"]->GetXaxis()->SetTitle("D0");
    histList["TruthMatchedD0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedD0"]->GetXaxis()->SetTitle("D0");
    effHTTAll = new TEfficiency(*histList["TruthMatchedD0"], *histList["TruthAllD0"]);
    name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedD0"]->Integral()/histList["TruthAllD0"]->Integral());
    effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNSelectedD0"], *histList["TruthAllD0"]);
    // name = Form("1_NN Extrap + Cut: Ave %.3f", histList["TruthMatchedNNSelectedD0"]->Integral()/histList["TruthAllD0"]->Integral());
    // effToPlot[name] = effHTTAll;

    // effHTTAll = new TEfficiency(*histList["TruthMatchedNNOverlapD0"], *histList["TruthAllD0"]);
    // name = Form("2_NN Extrap + HW + Cut: Ave %.3f", histList["TruthMatchedNNOverlapD0"]->Integral()/histList["TruthAllD0"]->Integral());
    // effToPlot[name] = effHTTAll;

    plotEff(effToPlot, "EffD0", false, false, 0,1.1, uncertaintyType);
    effToPlot.clear();

    histToPlot["TruthMatchedNHitsPerTrack"] = histList["TruthMatchedNHitsPerTrack"];
    histList["TruthMatchedNHitsPerTrack"]->GetYaxis()->SetTitle("Number of Hits Per Matched Track");
    histList["TruthMatchedNHitsPerTrack"]->GetXaxis()->SetTitle("N Hits");
    histToPlot["TruthAllMatchedNHitsPerTrack"] = histList["TruthAllNHitsPerTrack"];
    histList["TruthAllNHitsPerTrack"]->GetYaxis()->SetTitle("Number of Hits Per Matched Track");
    histList["TruthAllNHitsPerTrack"]->GetXaxis()->SetTitle("N Hits");
    
    plotHist(histToPlot, "nMatchedHits", false, false, false, uncertaintyType);
    histToPlot.clear();

    bins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100, 150,  200, 250, 300, 400, 500, 600, 700, 800, 900, 1050, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 12000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 200000, 300000, 400000, 500000, 600000};
    histList["rnTrueTracksPerEvent"]         = new TH1F("rnTrueTracksPerEvent",           "rnTrueTracksPerEvent",           bins.size()-1, &bins[0]);
    histList["rnTracksPerEvent"]             = new TH1F("rnTracksPerEvent",           "rnTracksPerEvent",           bins.size()-1, &bins[0]);
    histList["rnTracksPerEventNNSelected"]   = new TH1F("rnTracksPerEventNNSelected", "rnTracksPerEventNNSelected", bins.size()-1, &bins[0]);
    histList["rnTracksPerEventNNOverlap"]    = new TH1F("rnTracksPerEventNNOverlap",  "rnTracksPerEventNNOverlap",  bins.size()-1, &bins[0]);

    histList["rnTracksPerEventMatched"]             = new TH1F("rnTracksPerEventMatched",           "rnTracksPerEventMatched",           bins.size()-1, &bins[0]);
    histList["rnTracksPerEventMatchedNNSelected"]   = new TH1F("rnTracksPerEventMatchedNNSelected", "rnTracksPerEventMatchedNNSelected", bins.size()-1, &bins[0]);
    histList["rnTracksPerEventMatchedNNOverlap"]    = new TH1F("rnTracksPerEventMatchedNNOverlap",  "rnTracksPerEventMatchedNNOverlap",  bins.size()-1, &bins[0]);

    for(int i = 0; i < histList["nTracksPerEvent"]->GetNbinsX(); i++)
    {
        double center = histList["nTracksPerEvent"]->GetBinCenter(i+1);
        if(center < 1) center = 1.5;
        histList["rnTracksPerEvent"]->Fill(center, histList["nTracksPerEvent"]->GetBinContent(i+1));
        histList["rnTracksPerEventMatched"]->Fill(center, histList["nTracksPerEventMatched"]->GetBinContent(i+1));
        histList["rnTrueTracksPerEvent"]->Fill(center, histList["nTrueTracksPerEvent"]->GetBinContent(i+1));
        histList["rnTracksPerEventMatchedNNSelected"]->Fill(center, histList["nTracksPerEventMatchedNNSelected"]->GetBinContent(i+1));
        histList["rnTracksPerEventMatchedNNOverlap"]->Fill(center, histList["nTracksPerEventMatchedNNOverlap"]->GetBinContent(i+1));
        histList["rnTracksPerEventNNSelected"]->Fill(center, histList["nTracksPerEventNNSelected"]->GetBinContent(i+1));
        histList["rnTracksPerEventNNOverlap"]->Fill(center, histList["nTracksPerEventNNOverlap"]->GetBinContent(i+1));
    }

    name = Form("0_NN Extrap Only: Ave %.1f", histList["rnTracksPerEvent"]->GetMean());
    toPlot[name] = histList["rnTracksPerEvent"];
    name = Form("1_NN Extrap + Cut: Ave %.1f", histList["rnTracksPerEventNNSelected"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventNNSelected"];
    name = Form("2_NN Extrap + HW + Cut: Ave %.1f", histList["rnTracksPerEventNNOverlap"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventNNOverlap"];


    histList["rnTracksPerEvent"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventNNSelected"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventNNOverlap"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEvent"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventNNSelected"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventNNOverlap"]->GetYaxis()->SetTitle("Nomalized Events");

    plotHist(toPlot, "NallTracks", false, true, true, uncertaintyType);
    toPlot.clear();


    name = Form("0_NN Extrap Only: Ave %.1f", histList["rnTracksPerEventMatched"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventMatched"];
    name = Form("1_NN Extrap + Cut: Ave %.1f", histList["rnTracksPerEventMatchedNNSelected"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventMatchedNNSelected"];
    name = Form("2_NN Extrap + HW + Cut: Ave %.1f", histList["rnTracksPerEventMatchedNNOverlap"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventMatchedNNOverlap"];


    histList["rnTracksPerEventMatched"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventMatchedNNSelected"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventMatchedNNOverlap"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventMatched"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventMatchedNNSelected"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventMatchedNNOverlap"]->GetYaxis()->SetTitle("Nomalized Events");

    plotHist(toPlot, "NMatchedTracks", false, true, true, uncertaintyType);
    toPlot.clear();

    name = Form("0_True Tracks: Ave %.1f", histList["rnTrueTracksPerEvent"]->GetMean());
    toPlot[name] = histList["rnTrueTracksPerEvent"];
    histList["rnTrueTracksPerEvent"]->GetXaxis()->SetTitle("N_{True Tracks}");
    histList["rnTrueTracksPerEvent"]->GetYaxis()->SetTitle("Nomalized Events");
    plotHist(toPlot, "NTrueTracks", false, true, true, uncertaintyType);
    toPlot.clear();


    // cout<<"Truth Track per event: "<<valMap["TruthMatchedSelected"]/valMap["NEvents"]<<endl;
    // cout<<"HT Track per event: "<<valMap["TotalReco"]/valMap["NEvents"]<<endl;
    // cout<<"HT & NN Track per event: "<<valMap["SelectedReco"]/valMap["NEvents"]<<endl;
    // cout<<"HT & NN & HW Track per event: "<<valMap["OverlapRemovedReco"]/valMap["NEvents"]<<endl;
    // cout<<"NN Truth eff: "<<valMap["TruthMatchedSelected"]/valMap["TruthMatched"]<<endl;
    // cout<<"NN & HT Truth eff: "<<valMap["TruthMatchedSelected"]/valMap["TotalTruth"]<<endl;


}

void plotHist(map<TString, TH1F*> histList, TString outName, bool setLog, bool norm, bool setLogx, std::string uncertaintyType)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    TH1F* frame = (TH1F*) (histList.begin()->second->Clone("frame"));
    frame->Reset();

    double maxLim = 0;
    int counter = 1754;

    TLegend *elLeg =  new TLegend (0.19, 0.75, 0.65, 0.865);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.03);


    for(const auto& hist: histList)
    {
        hist.second->SetLineColor(counter);
        hist.second->SetLineWidth(2);

        TString name = hist.first;
        name.ReplaceAll("0_", "");
        name.ReplaceAll("1_", "");
        name.ReplaceAll("2_", "");
        elLeg->AddEntry(hist.second, name, "l"); 

        double cMax = hist.second->GetMaximum();
        if(norm) cMax = hist.second->GetMaximum()/hist.second->Integral();

        if(cMax > maxLim) maxLim = cMax;
        counter++;


    }

    if(setLog) maxLim *= 10;
    else maxLim *= 1.5;

    frame->SetMaximum(maxLim);
    // if(setLog) frame->SetMinimum(10);

    frame->Draw();
    elLeg->Draw();
    for(const auto& hist: histList)
    {
        if(norm) hist.second->DrawNormalized("histSame");
        else hist.second->Draw("histSame");
    } 

    float leftDist = 0.2;
    //ACTSLabel(leftDist, 0.875, "", 1);


    TString label;
    TLatex* lumInfo = NULL;
    // TString label = getRegionLabel();
    // TLatex* lumInfo = new TLatex (0.725, 0.835, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    double top = .7;
    double space = .035;
    label = getAlgoLabel();
    lumInfo = new TLatex (leftDist, top, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.025);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getNHitsLabel();
    lumInfo = new TLatex (leftDist, top-space, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.025);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getUncertaintyLabel(uncertaintyType);
    lumInfo = new TLatex (leftDist, top-2*space, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.025);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    if(TString(opts["inputFile"]).Contains("Muon")) label = "Muon Only";
    if(TString(opts["inputFile"]).Contains("Pion")) label = "Pion Only";
    lumInfo = new TLatex (leftDist, 0.715, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();


    if(setLog) c1->SetLogy();
    if(setLogx) c1->SetLogx();
    auto parts = tokenizeStr(TString(opts["inputFile"]), "/");
    TString folderName = parts.at(parts.size() - 1);
    folderName.ReplaceAll(".root", "");
    folderName += "/" + subFolderKey + "/";

    system("mkdir -vp SummaryPlots/" + folderName);

    
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName + ".eps");
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName + ".C");

}

void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLogY, bool setLogX,float minLim, float maxLim, std::string uncertaintyType)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    int counter = 1754;

    TLegend *elLeg =  new TLegend (0.37, 0.8, 0.90, 0.9);
    //TLegend *elLeg =  new TLegend (0.2, 0.8, 0.70, 0.9);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.03);

    for(const auto& eff: effList)
    {
        eff.second->SetLineColor(counter);
        eff.second->SetLineWidth(1.5);
        eff.second->SetMarkerColor(counter);
        eff.second->SetMarkerSize(0.75);
        TString name = eff.first;
        name.ReplaceAll("0_", "");
        name.ReplaceAll("1_", "");
        name.ReplaceAll("2_", "");
        elLeg->AddEntry(eff.second, name, "l"); 

        counter++;
    }

    effList.begin()->second->Draw("A");
    elLeg->Draw();
    for(const auto& hist: effList) hist.second->Draw("plsame");

    effList.begin()->second->Paint("p");
    effList.begin()->second->GetPaintedGraph()->SetMaximum(maxLim);
    effList.begin()->second->GetPaintedGraph()->SetMinimum(minLim);
    effList.begin()->second->GetPaintedGraph()->GetYaxis()->SetTitle("Efficiency");
    effList.begin()->second->GetPaintedGraph()->Set(0);


    float leftDist = 0.65;
    //ACTSLabel(leftDist, 0.875, "", 1);

    TString label = "";
    TLatex* lumInfo = NULL;
    //TString label = getRegionLabel();
    // TLatex* lumInfo = new TLatex (leftDist, 0.835, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();
    double top = .29;
    double space = .035;
    label = getAlgoLabel();
    lumInfo = new TLatex (leftDist, top, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.03);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getNHitsLabel();
    lumInfo = new TLatex (leftDist, top-space, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.03);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getUncertaintyLabel(uncertaintyType);
    lumInfo = new TLatex (leftDist, top-2*space, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.03);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    if(setLogY) c1->SetLogy();
    if(setLogX) c1->SetLogx();
    auto parts = tokenizeStr(TString(opts["inputFile"]), "/");
    TString folderName = parts.at(parts.size() - 1);
    folderName.ReplaceAll(".root", "");
    folderName += "/" + subFolderKey + "/";

    system("mkdir -vp SummaryPlots/" + folderName);


    //TFile* f = new TFile("SummaryPlots/" + folderName + "/" + outName + ".root", "RECREATE");
    //for (auto& hist: effList) hist.second->Write();
    //f->Close();
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName + ".eps");
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName + ".C");
}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]    = "outputFile.root";
    opts["uncertainty"]  = "20";
    opts["minHits"]      = "7"; 
    opts["uncertaintyType"] = "radial";
    opts["phiUncertainty"]  = ".05";
    opts["zUncertainty"]    = "20";
    opts["NNCutVal"]        = "0.5";
    opts["nHitOverlapThres"]= "5";
    for(int i = 1; i < argc; ++i)
    {
        string opt=argv[i];
        if(opt=="--help" || opt == "--options" || opt =="--h")
        {
            cout<< "Options menu \n ============================================== \n" << endl;
            cout<<"--inputFile          : Path to the input file to process"<< endl;

 
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

TString getUncertaintyLabel(std::string uncertaintyType)
{
    if (uncertaintyType == "radial")
    {
        double nnCut = atof(opts["uncertainty"].c_str());
        return TString("Uncertainty = " + opts["uncertainty"]+ " mm");
    }
    else if (uncertaintyType == "layer")
    {
        return TString("z(#phi) uncertainty = " + opts["zUncertainty"] + "(" + opts["phiUncertainty"] + ")");
    }
    return TString("");
    
}

TString getNHitsLabel()
{
    int nHitOverlap     = atoi(opts["minHits"].c_str());
    //return TString("Nhits - " + opts["minHits"] + " hits");
    return TString("");
}

TString getAlgoLabel()
{
    TString label = "";
    if(TString(opts["inputFile"]).Contains("RNN"))        label = "RNN Extrapolator";
    if(TString(opts["inputFile"]).Contains("MLP"))        label = "MLP Extrapolator";
    if(TString(opts["inputFile"]).Contains("biRNN"))        label = "biRNN Extrapolator";
    if(TString(opts["inputFile"]).Contains("_Nom_"))        label = "Without Stub Filtering";
    if(TString(opts["inputFile"]).Contains("_sp_"))         label = "Spacepoints";
    if(TString(opts["inputFile"]).Contains("_spstubs_"))    label = "Spacepoints + stubs";
    if(TString(opts["inputFile"]).Contains("_spstub_"))     label = "Spacepoints + stubs";
    if(TString(opts["inputFile"]).Contains("_stubs_"))      label = "With Stub Filtering";
    if(TString(opts["inputFile"]).Contains("_stubs_"))      label = "With Stub Filtering";
    if(TString(opts["inputFile"]).Contains("_stub_"))       label = "With Stub Filtering";
    if(TString(opts["inputFile"]).Contains("_Stubs_"))      label = "With Stub Filtering";
    if(TString(opts["inputFile"]).Contains("_Stub_"))       label = "With Stub Filtering";

    return label;
}

TString getRegionLabel()
{
    return trackingInfo;
    TString label = "";
    if(TString(opts["inputFile"]).Contains("Region_0p1_0p3")) label = "#eta #in [0.1, 0.3], #mu = 200";
    if(TString(opts["inputFile"]).Contains("Region_0p7_0p9")) label = "#eta #in [0.7, 0.9], #mu = 200";
    if(TString(opts["inputFile"]).Contains("Region_2p0_2p2")) label = "#eta #in [2.0, 2.2], #mu = 200";
    return label;
}
