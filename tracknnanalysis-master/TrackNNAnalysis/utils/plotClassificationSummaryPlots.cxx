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
#include "TrackNNDataReader/DataReaderHTT.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/FakeManager.h"
#include "TrackNNAnalysis/TrackGeoPlotter.h"
#include "TrackNNEvaluator/NNEvalUtils.h"
#include "TrackNNAnalysis/OutputManager.h"
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

void plotHist(map<TString, TH1F*> histList, TString outName, bool setLog, bool norm, bool setLogX);
void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLogY, bool setLogX, float max);
TString getNNLabel();
TString getHWLabel();
TString getAlgoLabel();
TString getRegionLabel();
TString subFolderKey = "";
std::vector<TString> tokenizeStr(TString str, TString key);

map<std::string, std::string> opts;
PlotHolder* plotHolder = NULL;

int main(int argc, char *argv[])
{   
    setPrettyStuff();

    // Process command line arguments
    if(!cmdline(argc,argv, opts)) return 0;

    plotHolder = new PlotHolder();
    plotHolder->readHist(TString(opts["inputFile"]));
    double nnCut        = atof(opts["NNCutVal"].c_str());
    int nHitOverlap     = atoi(opts["nHitOverlapThres"].c_str());

    map<TString, TH1F*> histList = plotHolder->getHistMap(nnCut, nHitOverlap);
    subFolderKey = plotHolder->getKey(nnCut, nHitOverlap);

    map<TString, TH1F*> toPlot;

    map<TString, float> valMap;
    for(int i = 0; i < 10; i++)
    {
        if(histList["counterHist"]->GetBinContent(i+1) <= 0) continue;
        cout<<histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)<<" "<<histList["counterHist"]->GetBinContent(i+1)<<endl;
        valMap[histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)] = histList["counterHist"]->GetBinContent(i+1);
    }



    toPlot["All HT Tracks"] = histList["nTracksPerRoad"];
    histList["nTracksPerRoad"]->SetMinimum(0.0001);
    histList["nTracksPerRoad"]->GetXaxis()->SetTitle("Track Probability");
    histList["nTracksPerRoad"]->GetYaxis()->SetTitle("Nomalized Events");

    plotHist(toPlot, "NTracksPerRoad.pdf", false, true, false);
    toPlot.clear();


    vector<float> bins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100, 150,  200, 250, 300, 400, 500, 600, 700, 800, 900, 1050, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 12000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 200000, 300000, 400000, 500000, 600000};
    histList["rnTracksPerEventRaw"]         = new TH1F("rnTracksPerEventRaw",               "rnTracksPerEventRaw",          bins.size()-1, &bins[0]);
    histList["rnTracksPerEventNNSel"]           = new TH1F("rnTracksPerEventNNSel",         "rnTracksPerEventNNSel",        bins.size()-1, &bins[0]);
    histList["rnTracksPerEventNNSelOverlap"]    = new TH1F("rnTracksPerEventNNSelOverlap",  "rnTracksPerEventNNSelOverlap", bins.size()-1, &bins[0]);

    for(int i = 0; i < histList["nTracksPerEventRaw"]->GetNbinsX(); i++)
    {
        double center = histList["nTracksPerEventRawNorm"]->GetBinCenter(i+1);
        if(center < 1) center = 1.5;
        histList["rnTracksPerEventRaw"]->Fill(center, histList["nTracksPerEventRaw"]->GetBinContent(i+1));
        histList["rnTracksPerEventNNSel"]->Fill(center, histList["nTracksPerEventNNSel"]->GetBinContent(i+1));
        histList["rnTracksPerEventNNSelOverlap"]->Fill(center, histList["nTracksPerEventNNSelOverlap"]->GetBinContent(i+1));
    }

    TString name = "";
    name = Form("0_Hough Transform (HT): Ave %.1f", histList["nTracksPerEventRaw"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventRaw"];
    name = Form("1_HT + NN Fake: Ave %.1f", histList["nTracksPerEventNNSel"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventNNSel"];
    name = Form("2_HT + NN Fake + NN Overlap: Ave %.1f", histList["nTracksPerEventNNSelOverlap"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventNNSelOverlap"];


    histList["rnTracksPerEventRaw"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventNNSel"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventNNSelOverlap"]->GetXaxis()->SetTitle("N_{Tracks}");
    histList["rnTracksPerEventRaw"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventNNSel"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventNNSelOverlap"]->GetYaxis()->SetTitle("Nomalized Events");

    plotHist(toPlot, "NTracksPerEvent.pdf", false, true, true);
    toPlot.clear();


    bins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100, 150,  200, 250, 300, 400, 500, 600, 700, 800, 900, 1050, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 12000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 200000, 300000, 400000, 500000, 600000};
    histList["rnTracksPerEventRawNorm"]             = new TH1F("rnTracksPerEventRawNorm",               "rnTracksPerEventRawNorm",          bins.size()-1, &bins[0]);
    histList["rnTracksPerEventNNSelNorm"]           = new TH1F("rnTracksPerEventNNSelNorm",         "rnTracksPerEventNNSelNorm",        bins.size()-1, &bins[0]);
    histList["rnTracksPerEventNNSelOverlapNorm"]    = new TH1F("rnTracksPerEventNNSelOverlapNorm",  "rnTracksPerEventNNSelOverlapNorm", bins.size()-1, &bins[0]);

    for(int i = 0; i < histList["nTracksPerEventRawNorm"]->GetNbinsX(); i++)
    {
        double center = histList["nTracksPerEventRawNorm"]->GetBinCenter(i+1);
        if(center < 1) center = 1.5;
        histList["rnTracksPerEventRawNorm"]->Fill(center, histList["nTracksPerEventRawNorm"]->GetBinContent(i+1));
        histList["rnTracksPerEventNNSelNorm"]->Fill(center, histList["nTracksPerEventNNSelNorm"]->GetBinContent(i+1));
        histList["rnTracksPerEventNNSelOverlapNorm"]->Fill(center, histList["nTracksPerEventNNSelOverlapNorm"]->GetBinContent(i+1));
    }


    name = "";
    name = Form("0_Hough Transform (HT): Ave %.1f", histList["nTracksPerEventRawNorm"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventRawNorm"];
    name = Form("1_HT + NN Fake: Ave %.1f", histList["nTracksPerEventNNSelNorm"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventNNSelNorm"];
    name = Form("2_HT + NN Fake + NN Overlap: Ave %.1f", histList["nTracksPerEventNNSelOverlapNorm"]->GetMean());
    toPlot[name] = histList["rnTracksPerEventNNSelOverlapNorm"];


    histList["rnTracksPerEventRawNorm"]->GetXaxis()->SetTitle("N_{Tracks} - N_{Offline}");
    histList["rnTracksPerEventNNSelNorm"]->GetXaxis()->SetTitle("N_{Tracks} - N_{Offline}");
    histList["rnTracksPerEventNNSelOverlapNorm"]->GetXaxis()->SetTitle("N_{Tracks} - N_{Offline}");
    histList["rnTracksPerEventRawNorm"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventNNSelNorm"]->GetYaxis()->SetTitle("Nomalized Events");
    histList["rnTracksPerEventNNSelOverlapNorm"]->GetYaxis()->SetTitle("Nomalized Events");

    plotHist(toPlot, "NTracksmNOfflinePerEvent.pdf", false, true, true);
    toPlot.clear();


    histList["rnRecoPerTruth"]           = new TH1F("rnRecoPerTruth",                "rnRecoPerTruth",    5, 0, 5);

    for(int i = 0; i < histList["nRecoPerTruth"]->GetNbinsX(); i++)
    {
        double center = histList["nRecoPerTruth"]->GetBinCenter(i+1);
        histList["rnRecoPerTruth"]          ->Fill(center, histList["nRecoPerTruth"]->GetBinContent(i+1));
    }
    histList["rnRecoPerTruth"]->Scale(1/valMap["TotalTruth"]);
    // histList["rnRecoPerTruth"]->Scale(1/histList["rnRecoPerTruth"]->Integral());
    histList["nRecoPerTruth"]  = histList["rnRecoPerTruth"];

    toPlot["HT & NN"] = histList["nRecoPerTruth"];
    histList["nRecoPerTruth"]->SetMinimum(0.0001);
    histList["nRecoPerTruth"]->GetXaxis()->SetTitle("N_{Reco} per Truth");
    histList["nRecoPerTruth"]->GetYaxis()->SetTitle("Fraction of truth tracks");
    plotHist(toPlot, "nRecoPerTruth.pdf", true, false, false);
    toPlot.clear();


    histList["nMaxSharedHits"]->Scale(1/histList["nMaxSharedHits"]->Integral());
    toPlot["HT & NN"] = histList["nMaxSharedHits"];
    // histList["nMaxSharedHits"]->SetMinimum(0.0001);
    histList["nMaxSharedHits"]->GetXaxis()->SetTitle("Max N_{shared} hits");
    histList["nMaxSharedHits"]->GetYaxis()->SetTitle("Normalized");
    plotHist(toPlot, "nMaxSharedHits.pdf", false, false, false);
    toPlot.clear();




    bins = {1, 1.5, 2, 2.5, 3, 4, 5, 6, 10,  20, 100};

    histList["rTruthAllPt"]              = new TH1F("rTruthAllPt",                "rTruthAllPt",               bins.size()-1, &bins[0]);
    histList["rTruthMatchedPt"]          = new TH1F("rTruthMatchedPt",            "rTruthMatchedPt",           bins.size()-1, &bins[0]);
    histList["rTruthMatchedSelectedPt"]  = new TH1F("rTruthMatchedSelectedPt",    "rTruthMatchedSelectedPt",   bins.size()-1, &bins[0]);
    histList["rRecoAllPt"]               = new TH1F("rRecoAllPt",                 "rRecoAllPt",                bins.size()-1, &bins[0]);
    histList["rRecoSelectedPt"]          = new TH1F("rRecoSelectedPt",            "rRecoSelectedPt",           bins.size()-1, &bins[0]);
    histList["rRecoSelectedOverlapPt"]   = new TH1F("rRecoSelectedOverlapPt",            "rRecoSelectedOverlapPt",           bins.size()-1, &bins[0]);

    for(int i = 0; i < histList["TruthAllPt"]->GetNbinsX(); i++)
    {
        double center = histList["TruthAllPt"]->GetBinCenter(i+1);
        if(center > 100) center = 50;
        if(center < 1) center = 1.25;
        histList["rTruthAllPt"]             ->Fill(center, histList["TruthAllPt"]->GetBinContent(i+1));
        histList["rTruthMatchedPt"]         ->Fill(center, histList["TruthMatchedPt"]->GetBinContent(i+1));
        histList["rTruthMatchedSelectedPt"] ->Fill(center, histList["TruthMatchedSelectedPt"]->GetBinContent(i+1));
        histList["rRecoAllPt"]              ->Fill(center, histList["RecoAllPt"]->GetBinContent(i+1));
        histList["rRecoSelectedPt"]         ->Fill(center, histList["RecoSelectedPt"]->GetBinContent(i+1));
        histList["rRecoSelectedOverlapPt"]  ->Fill(center, histList["RecoSelectedOverlapPt"]->GetBinContent(i+1));
    }

    for(int i = 0; i < histList["rTruthAllPt"]->GetNbinsX(); i++)
    {
        histList["rTruthAllPt"]             ->SetBinError(i+1,  sqrt(histList["rTruthAllPt"]->GetBinContent(i+1)));
        histList["rTruthMatchedPt"]         ->SetBinError(i+1,  sqrt(histList["rTruthMatchedPt"]->GetBinContent(i+1)));
        histList["rTruthMatchedSelectedPt"] ->SetBinError(i+1,  sqrt(histList["rTruthMatchedSelectedPt"]->GetBinContent(i+1)));
        histList["rRecoAllPt"]              ->SetBinError(i+1,  sqrt(histList["rRecoAllPt"]->GetBinContent(i+1)));
        histList["rRecoSelectedPt"]         ->SetBinError(i+1,  sqrt(histList["rRecoSelectedPt"]->GetBinContent(i+1)));
        histList["rRecoSelectedOverlapPt"]  ->SetBinError(i+1,  sqrt(histList["rRecoSelectedOverlapPt"]->GetBinContent(i+1)));
    }

    histList["TruthAllPt"]              = histList["rTruthAllPt"];
    histList["TruthMatchedPt"]          = histList["rTruthMatchedPt"];
    histList["TruthMatchedSelectedPt"]  = histList["rTruthMatchedSelectedPt"];
    histList["RecoAllPt"]               = histList["rRecoAllPt"];
    histList["RecoSelectedPt"]          = histList["rRecoSelectedPt"];
    histList["RecoSelectedOverlapPt"]   = histList["rRecoSelectedOverlapPt"];

    histList["TruthAllPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["TruthMatchedPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["TruthMatchedSelectedPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedSelectedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["RecoAllPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["RecoAllPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["RecoSelectedPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["RecoSelectedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");

    TEfficiency* effHTTAll = new TEfficiency(*histList["TruthMatchedPt"], *histList["TruthAllPt"]);
    TEfficiency* effSelectedAll = new TEfficiency(*histList["TruthMatchedSelectedPt"], *histList["TruthAllPt"]);
    TEfficiency* effSelectedHTT = new TEfficiency(*histList["TruthMatchedSelectedPt"], *histList["TruthMatchedPt"]);

    map<TString, TEfficiency*> effToPlot;
    name = Form("0_Hough Transform (HT) Only: Ave %.3f", histList["TruthMatchedPt"]->Integral()/histList["TruthAllPt"]->Integral());
    effToPlot[name] = effHTTAll;
    name = Form("1_HT & NN: Ave %.3f", histList["TruthMatchedSelectedPt"]->Integral()/histList["TruthAllPt"]->Integral());
    effToPlot[name] = effSelectedAll;
    // name = Form("2_Only NN: Ave %.3f", histList["TruthMatchedSelectedPt"]->Integral()/histList["TruthMatchedPt"]->Integral());
    // effToPlot[name] = effSelectedHTT;


    plotEff(effToPlot, "EffpT.pdf", false, true, 1.1);
    effToPlot.clear();


    histList["TruthAllEta"]->Rebin(100);
    histList["TruthMatchedEta"]->Rebin(100);
    histList["TruthMatchedSelectedEta"]->Rebin(100);


    for(const auto& var: vector<TString> {"Z0", "Eta", "Phi", "D0", "Charge", "NMeas", "NOutlier", "NOther"})
    {
   
        histList["TruthAll" + var]->GetYaxis()->SetTitle("Efficiency");
        histList["TruthAll" + var]->GetXaxis()->SetTitle(var);
        histList["TruthMatched" + var]->GetYaxis()->SetTitle("Efficiency");
        histList["TruthMatched" + var]->GetXaxis()->SetTitle(var);
        histList["TruthMatchedSelected" + var]->GetYaxis()->SetTitle("Efficiency");
        histList["TruthMatchedSelected" + var]->GetXaxis()->SetTitle(var);

        effHTTAll = new TEfficiency(*histList["TruthMatched" + var], *histList["TruthAll" + var]);
        effSelectedAll = new TEfficiency(*histList["TruthMatchedSelected" + var], *histList["TruthAll" + var]);
        effSelectedHTT = new TEfficiency(*histList["TruthMatchedSelected" + var], *histList["TruthMatched" + var]);

        name = Form("0_Hough Transform (HT) Only: Ave %.3f", histList["TruthMatched" + var]->Integral()/histList["TruthAll" + var]->Integral());
        effToPlot[name] = effHTTAll;
        name = Form("1_HT & NN: Ave %.3f", histList["TruthMatchedSelected" + var]->Integral()/histList["TruthAll" + var]->Integral());
        effToPlot[name] = effSelectedAll;
        plotEff(effToPlot, "Eff" + var + ".pdf", false, false, 1.1);
        effToPlot.clear();

    
    }


    TEfficiency* effHTTRecoAll = new TEfficiency(*histList["RecoSelectedPt"], *histList["RecoAllPt"]);
    effToPlot["Selected/All HT reco"] = effHTTRecoAll;
    plotEff(effToPlot, "bkgpTEff.pdf", false, true, 0.05);

    histList["RecoAllPt"]->Scale(1./valMap["NEvents"]);
    histList["RecoSelectedPt"]->Scale(1./valMap["NEvents"]);
    histList["RecoSelectedOverlapPt"]->Scale(1./valMap["NEvents"]);

    // toPlot["HT"]                    = histList["RecoAllPt"];
    toPlot["Hough Transform + NN Fake"]          = histList["RecoSelectedPt"];
    toPlot["Hough Transform + NN Fake + NN Overlap"]  = histList["RecoSelectedOverlapPt"];
    histList["RecoSelectedPt"]->SetMinimum(0);
    histList["RecoSelectedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["RecoSelectedPt"]->GetYaxis()->SetTitle("N^{Fake}_{Tracks}/Event");

    plotHist(toPlot, "NumberOfFakeTrack.pdf", false, false, true);
    toPlot.clear();

    toPlot["HT + NN + Overlap/HT + NN"] = (TH1F*) histList["RecoSelectedOverlapPt"]->Clone();
    toPlot["HT + NN + Overlap/HT + NN"]->Divide(histList["RecoSelectedPt"]);
    toPlot["HT + NN + Overlap/HT + NN"]->SetMinimum(0.6);
    plotHist(toPlot, "RatioNumberOfFakeTrack.pdf", false, false, true);
    toPlot.clear();


    cout<<"Truth Track per event: "<<valMap["TruthMatchedSelected"]/valMap["NEvents"]<<endl;
    cout<<"HT Track per event: "<<valMap["TotalReco"]/valMap["NEvents"]<<endl;
    cout<<"HT & NN Track per event: "<<valMap["SelectedReco"]/valMap["NEvents"]<<endl;
    cout<<"HT & NN & HW Track per event: "<<valMap["OverlapRemovedReco"]/valMap["NEvents"]<<endl;
    cout<<"NN Truth eff: "<<valMap["TruthMatchedSelected"]/valMap["TruthMatched"]<<endl;
    cout<<"NN & HT Truth eff: "<<valMap["TruthMatchedSelected"]/valMap["TotalTruth"]<<endl;


}

void plotHist(map<TString, TH1F*> histList, TString outName, bool setLog, bool norm, bool setLogx)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    TH1F* frame = (TH1F*) (histList.begin()->second->Clone("frame"));
    frame->Reset();

    double maxLim = 0;
    int counter = 1754;

    TLegend *elLeg =  new TLegend (0.19, 0.75, 0.4, 0.865);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.025);


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

    float leftDist = 0.19;
    ATLASLabel(leftDist, 0.875, "Simulation Internal", 1);


    TString label = getRegionLabel();
    TLatex* lumInfo = new TLatex (0.725, 0.835, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();


    label = getAlgoLabel();
    lumInfo = new TLatex (0.725, 0.805, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getHWLabel();
    lumInfo = new TLatex (0.725, 0.775, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getNNLabel();
    lumInfo = new TLatex (0.725, 0.745, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
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



    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName);

}

void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLogY, bool setLogX, float maxLim)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    int counter = 1754;

    TLegend *elLeg =  new TLegend (0.475, 0.75, 0.90, 0.855);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.0225);

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
    if(maxLim > 0.8) effList.begin()->second->GetPaintedGraph()->SetMinimum(0.8);
     effList.begin()->second->GetPaintedGraph()->SetMinimum(0.5);
    effList.begin()->second->GetPaintedGraph()->GetYaxis()->SetTitle("Efficiency");
    effList.begin()->second->GetPaintedGraph()->Set(0);


    float leftDist = 0.19;
    ATLASLabel(leftDist, 0.875, "Simulation Internal", 1);

    TString label = getRegionLabel();
    TLatex* lumInfo = new TLatex (leftDist, 0.835, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = getAlgoLabel();
    lumInfo = new TLatex (leftDist, 0.805, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getHWLabel();
    lumInfo = new TLatex (leftDist, 0.775, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    label = "";
    label += getNNLabel();
    lumInfo = new TLatex (leftDist, 0.745, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    if(setLogY) c1->SetLogy();
    if(setLogX) c1->SetLogx();
    auto parts = tokenizeStr(TString(opts["inputFile"]), "/");
    TString folderName = parts.at(parts.size() - 1);
    folderName.ReplaceAll(".root", "");
    folderName += "/" + subFolderKey + "/";

    system("mkdir -vp SummaryPlots/" + folderName);



    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName);

}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "outputFile.root";
    opts["NNCutVal"]        = "0.2";
    opts["nHitOverlapThres"] = "5";

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

TString getNNLabel()
{
    double nnCut        = atof(opts["NNCutVal"].c_str());
    if(nnCut == -1) return "p_{T} dependant cut";

    return TString("NN = " + opts["NNCutVal"]);
}

TString getHWLabel()
{
    int nHitOverlap     = atoi(opts["nHitOverlapThres"].c_str());
    if(nHitOverlap == 10) return "No NN Overlap";

    return TString("NN Overlap - " + opts["nHitOverlapThres"] + " hits");
}

TString getAlgoLabel()
{
    TString label = "Algo not supported";
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
    TString label = "";
    if(TString(opts["inputFile"]).Contains("Region_0p1_0p3")) label = "#eta #in [0.1, 0.3], #mu = 200";
    if(TString(opts["inputFile"]).Contains("Region_0p7_0p9")) label = "#eta #in [0.7, 0.9], #mu = 200";
    if(TString(opts["inputFile"]).Contains("Region_2p0_2p2")) label = "#eta #in [2.0, 2.2], #mu = 200";
    return label;
}
