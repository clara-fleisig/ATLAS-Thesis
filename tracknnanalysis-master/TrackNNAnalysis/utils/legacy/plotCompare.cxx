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
void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLog, float max);
TString getNNLabel(TString nnCut);
TString getHWLabel(int hitThres);
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

    // plotHolder = new PlotHolder();
    // plotHolder->readHist(TString(opts["inputFile"]));
    // double nnCut        = atof(opts["NNCutVal"].c_str());
    // int nHitOverlap     = atoi(opts["nHitOverlapThres"].c_str());
    
    auto fileList       = tokenizeStr(TString(opts["fileList"]), ",");
    auto fileLabelList  = tokenizeStr(TString(opts["fileLabel"]), ",");
    auto HWList         = tokenizeStr(TString(opts["nHitOverlapThres"]), ",");
    auto NNList         = tokenizeStr(TString(opts["NNCutVal"]), ",");


    map<TString, map<TString, TEfficiency*>> effToPlot;
    map<TString, map<TString, TH1F*>> histToPlot;

    int counter = 0;
    int fileCounter = 0;
    for(const auto& fileName: fileList)
    {
        TString fileLabel = fileLabelList.at(fileCounter);
        fileCounter++;
        PlotHolder* plotHolder = new PlotHolder();
        plotHolder->readHist(fileName);

        for(const auto& HWtresh:HWList)
        {
            for(const auto& nnCut: NNList)
            {
                map<TString, TH1F*> histList = plotHolder->getHistMap(atof(nnCut), atoi(HWtresh));
                TString subFolderKey = plotHolder->getKey(atof(nnCut), atoi(HWtresh)) + Form("%d", counter);
                counter++;
                map<TString, TH1F*> toPlot;

                map<TString, float> valMap;
                for(int i = 0; i < 10; i++)
                {
                    if(histList["counterHist"]->GetBinContent(i+1) <= 0) continue;
                    valMap[histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)] = histList["counterHist"]->GetBinContent(i+1);
                }


                // rebin
                auto bins = vector<float>{1, 1.5, 2, 2.5, 3, 4, 5, 6, 10,  20, 100};
                histList["rTruthAllPt"]              = new TH1F("rTruthAllPt" + subFolderKey,                "rTruthAllPt" + subFolderKey,               bins.size()-1, &bins[0]);
                histList["rTruthMatchedPt"]          = new TH1F("rTruthMatchedPt" + subFolderKey,            "rTruthMatchedPt" + subFolderKey,           bins.size()-1, &bins[0]);
                histList["rTruthMatchedSelectedPt"]  = new TH1F("rTruthMatchedSelectedPt" + subFolderKey,    "rTruthMatchedSelectedPt" + subFolderKey,   bins.size()-1, &bins[0]);
                histList["rRecoAllPt"]               = new TH1F("rRecoAllPt" + subFolderKey,                 "rRecoAllPt" + subFolderKey,                bins.size()-1, &bins[0]);
                histList["rRecoSelectedPt"]          = new TH1F("rRecoSelectedPt" + subFolderKey,            "rRecoSelectedPt" + subFolderKey,           bins.size()-1, &bins[0]);
                histList["rRecoSelectedOverlapPt"]   = new TH1F("rRecoSelectedOverlapPt" + subFolderKey,     "rRecoSelectedOverlapPt" + subFolderKey,    bins.size()-1, &bins[0]);

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

                // TEfficiency* effHTTAll = new TEfficiency(*histList["TruthMatchedPt"], *histList["TruthAllPt"]);
                // TEfficiency* effSelectedAll = new TEfficiency(*histList["TruthMatchedSelectedPt"], *histList["TruthAllPt"]);
                TEfficiency* effSelectedHTT = new TEfficiency(*histList["TruthMatchedSelectedPt"], *histList["TruthMatchedPt"]);

                // TString name = Form("0_HT Only: Ave %.3f %d", histList["TruthMatchedPt"]->Integral()/histList["TruthAllPt"]->Integral(), counter);
                // effToPlot[name] = effHTTAll;
                // name = Form("1_HT & NN: Ave %.3f %d", histList["TruthMatchedSelectedPt"]->Integral()/histList["TruthAllPt"]->Integral(), counter);
                // effToPlot[name] = effSelectedAll;
                TString name = fileLabel + " " + getNNLabel(nnCut)+ "" + getHWLabel(atoi(HWtresh));
                effToPlot["TruthpTEff.eps"][name] = effSelectedHTT;
                // TString name = fileLabel;
                // effToPlot["TruthpTEff.eps"][name] = effHTTAll;

                histList["RecoSelectedOverlapPt"]->Scale(1./valMap["NEvents"]);
                histList["RecoSelectedOverlapPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
                histList["RecoSelectedOverlapPt"]->GetYaxis()->SetTitle("N_{Tracks}/Event");

                histToPlot["NumberOfFakeTrack.eps"][name]  = histList["RecoSelectedOverlapPt"];



                bins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100, 150,  200, 250, 300, 400, 500, 600, 700, 800, 900, 1050, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 12000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 200000, 300000, 400000, 500000, 600000};
                histList["rnTracksPerEventRaw"]             = new TH1F("rnTracksPerEventRaw",           "rnTracksPerEventRaw",          bins.size()-1, &bins[0]);
                histList["rnTracksPerEventNNSel"]           = new TH1F("rnTracksPerEventNNSel",         "rnTracksPerEventNNSel",        bins.size()-1, &bins[0]);
                histList["rnTracksPerEventNNSelOverlap"]    = new TH1F("rnTracksPerEventNNSelOverlap",  "rnTracksPerEventNNSelOverlap", 50, 0, 100);

                for(int i = 0; i < histList["nTracksPerEventRaw"]->GetNbinsX(); i++)
                {
                    double center = histList["nTracksPerEventRawNorm"]->GetBinCenter(i+1);
                    if(center < 1) center = 1.5;
                    histList["rnTracksPerEventRaw"]->Fill(center, histList["nTracksPerEventRaw"]->GetBinContent(i+1));
                    histList["rnTracksPerEventNNSel"]->Fill(center, histList["nTracksPerEventNNSel"]->GetBinContent(i+1));
                    histList["rnTracksPerEventNNSelOverlap"]->Fill(center, histList["nTracksPerEventNNSelOverlap"]->GetBinContent(i+1));
                }

                for(int i = 0; i < histList["rnTracksPerEventRaw"]->GetNbinsX(); i++)
                {
                    histList["rnTracksPerEventRaw"]         ->SetBinError(i+1, 0);
                    histList["rnTracksPerEventNNSel"]       ->SetBinError(i+1, 0);
                    histList["rnTracksPerEventNNSelOverlap"]->SetBinError(i+1, 0);
                }

                histList["rnTracksPerEventRaw"]->GetXaxis()->SetTitle("N_{Tracks}");
                histList["rnTracksPerEventNNSel"]->GetXaxis()->SetTitle("N_{Tracks}");
                histList["rnTracksPerEventNNSelOverlap"]->GetXaxis()->SetTitle("N_{Tracks}");
                histList["rnTracksPerEventRaw"]->GetYaxis()->SetTitle("Nomalized Events");
                histList["rnTracksPerEventNNSel"]->GetYaxis()->SetTitle("Nomalized Events");
                histList["rnTracksPerEventNNSelOverlap"]->GetYaxis()->SetTitle("Nomalized Events");


                name = fileLabel;
                histToPlot["NTrack_HT.eps"][name]                = histList["rnTracksPerEventRaw"];
                histToPlot["NTrack_HTNNFake.eps"][name]          = histList["rnTracksPerEventNNSel"];
                histToPlot["NTrack_HTNNFakeNNOverlap.eps"][name] = histList["rnTracksPerEventNNSelOverlap"];

            }
        }
    }


    for(const auto& plot: effToPlot)
    {
        plotEff(plot.second, plot.first, false, 1.1);
    }
    for(const auto& plot: histToPlot)
    {
        plotHist(plot.second, plot.first, false, false, true);
    }




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
    elLeg->SetTextSize(0.0225);


    for(const auto& hist: histList)
    {
        hist.second->SetLineColor(counter);
        hist.second->SetLineWidth(2);
        elLeg->AddEntry(hist.second, hist.first, "l"); 

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
        hist.second->SetMarkerSize(0);

        if(norm) hist.second->DrawNormalized("histSame");
        else hist.second->Draw("histSame");
        if(norm) hist.second->DrawNormalized("histESame");
        else hist.second->Draw("histESame");

    } 

    float leftDist = 0.19;
    ATLASLabel(leftDist, 0.875, "Simulation Internal", 1);


    TString label = getRegionLabel();
    TLatex* lumInfo = new TLatex (0.725, 0.835, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();


    // label = getAlgoLabel();
    // lumInfo = new TLatex (0.725, 0.805, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    // label = "";
    // label += getHWLabel();
    // lumInfo = new TLatex (0.70, 0.775, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    // label = "";
    // label += getNNLabel();
    // lumInfo = new TLatex (0.70, 0.745, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    label = "";
    if(TString(opts["inputFile"]).Contains("Muon")) label = "Muon Only";
    if(TString(opts["inputFile"]).Contains("Pion")) label = "Pion Only";
    lumInfo = new TLatex (leftDist, 0.715, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.0225);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();


    if(setLog) c1->SetLogy();
    if(setLogx) c1->SetLogx(false);
    TString folderName = opts["outFolder"];
    system("mkdir -vp SummaryPlots/" + folderName);
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName);

}

void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLog, float maxLim)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    int counter = 1754;


    TLegend *elLeg =  new TLegend (0.4, 0.75, 0.85, 0.855);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.0215);

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

    // label = "";
    // label += getHWLabel();
    // lumInfo = new TLatex (leftDist, 0.775, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    // label = "";
    // label += getNNLabel();
    // lumInfo = new TLatex (leftDist, 0.745, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    if(setLog) c1->SetLogy();
    c1->SetLogx();
    TString folderName = opts["outFolder"];
    system("mkdir -vp SummaryPlots/" + folderName);
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName);

}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["fileList"]            = "outputFile.root";
    opts["fileLabel"]           = " , ";
    opts["NNCutVal"]            = "0.2";
    opts["nHitOverlapThres"]    = "5";
    opts["outFolder"]           = "";

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

TString getNNLabel(TString nnCut)
{
    if(nnCut.Contains("-1")) return "NN: Fake - Cut = p_{T} dependant";

    return TString("NN: Fake - Cut = " + nnCut);
}

TString getHWLabel(int nHitOverlap)
{
    if(nHitOverlap == 10) return "No NN Overlap";

    return TString(", Overlap - " + TString(Form("%d", nHitOverlap)) + " hits");
}

TString getAlgoLabel()
{
    TString label = "Algo not supported";
    if(TString(opts["fileList"]).Contains("_Nom_"))        label = "Without Stub Filtering";
    if(TString(opts["fileList"]).Contains("_sp_"))         label = "Spacepoints";
    if(TString(opts["fileList"]).Contains("_spstubs_"))    label = "Spacepoints + stubs";
    if(TString(opts["fileList"]).Contains("_spstub_"))     label = "Spacepoints + stubs";
    if(TString(opts["fileList"]).Contains("_stubs_"))      label = "With Stub Filtering";
    if(TString(opts["fileList"]).Contains("_stubs_"))      label = "With Stub Filtering";
    if(TString(opts["fileList"]).Contains("_stub_"))       label = "With Stub Filtering";
    if(TString(opts["fileList"]).Contains("_Stubs_"))      label = "With Stub Filtering";
    if(TString(opts["fileList"]).Contains("_Stub_"))       label = "With Stub Filtering";

    return label;
}

TString getRegionLabel()
{
    TString label = "";
    if(TString(opts["fileList"]).Contains("Region_0p1_0p3")) label += "#eta #in [0.1, 0.3], #mu = 200";
    if(TString(opts["fileList"]).Contains("Region_0p7_0p9")) label += "#eta #in [0.7, 0.9], #mu = 200";
    if(TString(opts["fileList"]).Contains("Region_2p0_2p2")) label += "#eta #in [2.0, 2.2], #mu = 200";
    return label;
}
