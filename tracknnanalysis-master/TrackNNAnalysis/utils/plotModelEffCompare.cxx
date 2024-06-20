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

//void plotHist(map<TString, TH1F*> histList, TString outName, bool setLog, bool norm, bool setLogX, std::string uncertaintyType);
void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLogY, bool setLogX, float max, std::string uncertaintyType);
map<TString, TEfficiency*> makeTEfficiency(map<TString, TH1F*> &histList);
TString getTrackingInfo(map<TString, TH1F*> &histList);

TString getUncertaintyLabel(std::string uncertaintyType);
TString getNHitsLabel();
TString getAlgoLabel();
map<TString,TString> getRegionLabel();
TString subFolderKey = "";
TString trackingInfoRNN = "";
TString trackingInfoMLP = "";
TString trackingInfobiRNN = "";
TString trackingInfoESN = "";
TString trackingInfoHT = "";

std::vector<TString> tokenizeStr(TString str, TString key);

map<std::string, std::string> opts;
plotHolderACTSExtrapolation* plotHolderRNN = NULL;
plotHolderACTSExtrapolation* plotHolderMLP = NULL;
plotHolderACTSExtrapolation* plotHolderbiRNN = NULL;
plotHolderACTSExtrapolation* plotHolderESN = NULL;
plotHolderACTSExtrapolation* plotHolderHT = NULL;

int main(int argc, char *argv[])
{   
    setPrettyStuff();

    // Process command line arguments
    if(!cmdline(argc,argv, opts)) return 0;

    plotHolderRNN = new plotHolderACTSExtrapolation();
    plotHolderMLP = new plotHolderACTSExtrapolation();
    plotHolderbiRNN = new plotHolderACTSExtrapolation();
    plotHolderESN = new plotHolderACTSExtrapolation();
    plotHolderHT = new plotHolderACTSExtrapolation();
    plotHolderRNN->  readHist("/usatlas/groups/bnl_local2/agekow/EffOutput/processed/muonOnly/Extrap_ACTS_ttbarWithOverlay_mu200_RNN_ws3.root");
    plotHolderMLP->  readHist("/usatlas/groups/bnl_local2/agekow/EffOutput/processed/muonOnly/Extrap_ACTS_ttbarWithOverlay_mu200_MLP_ws3.root");
    plotHolderbiRNN->readHist("/usatlas/groups/bnl_local2/agekow/EffOutput/processed/muonOnly/Extrap_ACTS_ttbarWithOverlay_mu200_biRNN_ws3.root");
    plotHolderESN->  readHist("/usatlas/groups/bnl_local2/agekow/EffOutput/processed/muonOnly/Extrap_ACTS_ttbarWithOverlay_mu200_ESN_ws3.root");
    plotHolderHT->  readHist("/usatlas/u/ckalderon/usatlasdata/hough_transform_efficiency/eff_Top_all_on-n20-mu200_s0-50_withOverlay_Zslice_-090eta090_00phi64_noDupSubTrk_eta07_fixPhi.root");
    // int uncertainty     = atoi(opts.at("uncertainty").c_str());
    // int minHits         = atoi(opts["minHits"].c_str());
    // float phiUncertainty = atof(opts["phiUncertainty"].c_str());
    // float zUncertainty   =  atof(opts["zUncertainty"].c_str());  
    std::string uncertaintyType = opts["uncertaintyType"];
    int minHits = 7;

    double nnCutVal = 0.1;
    double nnCutValHT = 0.2;
    int HW = 8;
    int HWHT = 5;
    map<TString, TH1F*> histListRNN;
    map<TString, TH1F*> histListMLP;
    map<TString, TH1F*> histListbiRNN;
    map<TString, TH1F*> histListESN;
    map<TString, TH1F*> histListHT;

    if(uncertaintyType == "radial")
    {
        histListRNN =     plotHolderRNN->getHistMap(20, minHits, nnCutVal, HW);
        histListMLP =     plotHolderMLP->getHistMap(20, minHits, nnCutVal, HW);
        histListbiRNN =     plotHolderbiRNN->getHistMap(20, minHits, nnCutVal, HW);
        histListESN =     plotHolderESN->getHistMap(20, minHits, nnCutVal, HW);
        histListHT =      plotHolderHT->getHistMap(nnCutValHT, HWHT);
        //subFolderKey7 = plotHolder->getKey(7, minHits);
        //histList10 =    plotHolderRNN->getHistMap(10, minHits);
        //subFolderKey10 = plotHolder->getKey(10, minHits);
        //histList20 =     plotHolderRNN->getHistMap(20, minHits);
        //histList7 =     plotHolderRNN->getHistMap(7, minHits, 0, 5);
        //subFolderKey7 = plotHolder->getKey(7, minHits);
        //histList10 =    plotHolderRNN->getHistMap(10, minHits, 0, 5);
        //subFolderKey10 = plotHolder->getKey(10, minHits);
        //histList20 =     plotHolderRNN->getHistMap(20, minHits, 0, 5);
        //subFolderKey20 = plotHolder->getKey(20, minHits);

    }
    // else if(uncertaintyType == "layer")
    // {

    //     histList = plotHolder->getHistMap(zUncertainty, phiUncertainty, minHits);
    //     subFolderKey = plotHolder->getKey(zUncertainty, phiUncertainty, minHits);
    // }
    // else
    // {
    //     std::cout<<"uncertainty type: "<<uncertaintyType<<" not understood, exiting"<<std::endl;
    //     exit(1);
    // }
    
    trackingInfoRNN = getTrackingInfo(histListRNN);
    trackingInfoMLP = getTrackingInfo(histListMLP);
    trackingInfobiRNN = getTrackingInfo(histListbiRNN);
    trackingInfoESN = getTrackingInfo(histListESN);
    trackingInfoHT = getTrackingInfo(histListHT);

    map<TString, TEfficiency*> efficiencyMLP = makeTEfficiency(histListMLP);
    map<TString, TEfficiency*> efficiencyRNN = makeTEfficiency(histListRNN);
    map<TString, TEfficiency*> efficiencyESN = makeTEfficiency(histListESN);
    map<TString, TEfficiency*> efficiencybiRNN = makeTEfficiency(histListbiRNN);
    map<TString, TEfficiency*> efficiencyHT = makeTEfficiency(histListHT);

    map<TString, TEfficiency*> toPlot;
    vector<string> variables = {"pt","eta","phi","Z0","D0"};
    for (auto& var: variables)
    {
        toPlot["MLP"] = efficiencyMLP[var];
        toPlot["RNN"] = efficiencyRNN[var];
        toPlot["ESN"] = efficiencyESN[var];
        toPlot["biRNN"] = efficiencybiRNN[var];
        toPlot["HT"] = efficiencyHT[var];
        
        for(auto& m: toPlot) std::cout<<m.first<<std::endl;

        if (var == "pt") {plotEff(toPlot, TString(var + ".eps"), false, true, 1.45, uncertaintyType);}
        else {plotEff(toPlot, TString(var + ".eps"), false, false, 1.45, uncertaintyType);}
        toPlot.clear();
    }
}

TString getTrackingInfo(map<TString, TH1F*> &histList)
{
    map<TString, float> valMap;
    for(int i = 0; i < 10; i++)
    {
        if(histList["counterHist"]->GetBinContent(i+1) <= 0) continue;
        //cout<<histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)<<" "<<histList["counterHist"]->GetBinContent(i+1)<<endl;
        valMap[histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)] = histList["counterHist"]->GetBinContent(i+1);
    }

    //TString trackingInfo =   Form("N_{true} = %.2E, N_{reco} = %.2E, purity %.3f", valMap["nTracks"], valMap["nRoads"], valMap["nTracks"]/valMap["nRoads"]);
    TString trackingInfo =   Form("purity %.3f", valMap["nTracks"]/valMap["nRoads"]);
    return trackingInfo;
}

map<TString, TEfficiency*> makeTEfficiency(map<TString, TH1F*> &histList)
{
    map<TString, TEfficiency*> effToPlot;
    vector<float> bins = {1, 1.5, 2, 2.5, 3, 4, 5, 6, 10,  20, 100};
    TString name;

    histList["rTruthAllPt"]              = new TH1F("rTruthAllPt",                "rTruthAllPt",               bins.size()-1, &bins[0]);
    histList["rTruthMatchedPt"]          = new TH1F("rTruthMatchedPt",            "rTruthMatchedPt",           bins.size()-1, &bins[0]);

    for(int i = 0; i < histList["TruthAllPt"]->GetNbinsX(); i++)
    {
        double center = histList["TruthAllPt"]->GetBinCenter(i+1);
        if(center > 100) center = 50;
        if(center < 1) center = 1.25;
        histList["rTruthAllPt"]             ->Fill(center, histList["TruthAllPt"]->GetBinContent(i+1));
        histList["rTruthMatchedPt"]         ->Fill(center, histList["TruthMatchedPt"]->GetBinContent(i+1));
    }

    for(int i = 0; i < histList["rTruthAllPt"]->GetNbinsX(); i++)
    {
        histList["rTruthAllPt"]             ->SetBinError(i+1,  sqrt(histList["rTruthAllPt"]->GetBinContent(i+1)));
        histList["rTruthMatchedPt"]         ->SetBinError(i+1,  sqrt(histList["rTruthMatchedPt"]->GetBinContent(i+1)));
    }

    histList["TruthAllPt"]              = histList["rTruthAllPt"];
    histList["TruthMatchedPt"]          = histList["rTruthMatchedPt"];

    histList["TruthAllPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");
    histList["TruthMatchedPt"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedPt"]->GetXaxis()->SetTitle("p_{T} [GeV]");

    TEfficiency* effHTTAll = new TEfficiency(*histList["TruthMatchedPt"], *histList["TruthAllPt"]);
    //name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedPt"]->Integral()/histList["TruthAllPt"]->Integral());
    name = "pt";
    effToPlot[name] = effHTTAll;
    histList["TruthAllEta"]->Rebin(100);
    histList["TruthMatchedEta"]->Rebin(100);

    histList["TruthAllEta"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllEta"]->GetXaxis()->SetTitle("#eta");
    histList["TruthMatchedEta"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedEta"]->GetXaxis()->SetTitle("#eta");
    effHTTAll = new TEfficiency(*histList["TruthMatchedEta"], *histList["TruthAllEta"]);
    //name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedEta"]->Integral()/histList["TruthAllEta"]->Integral());
    name = "eta";
    effToPlot[name] = effHTTAll;

    histList["TruthAllPhi"]->Rebin(100);
    histList["TruthMatchedPhi"]->Rebin(100);
    histList["TruthAllPhi"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllPhi"]->GetXaxis()->SetTitle("#phi [rad]");
    histList["TruthMatchedPhi"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedPhi"]->GetXaxis()->SetTitle("#phi [rad]");
    effHTTAll = new TEfficiency(*histList["TruthMatchedPhi"], *histList["TruthAllPhi"]);
    //name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedPhi"]->Integral()/histList["TruthAllPhi"]->Integral());
    name = "phi";
    effToPlot[name] = effHTTAll;

    histList["TruthAllZ0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllZ0"]->GetXaxis()->SetTitle("Z0 [mm]");
    histList["TruthMatchedZ0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedZ0"]->GetXaxis()->SetTitle("Z0 [mm]");
    effHTTAll = new TEfficiency(*histList["TruthMatchedZ0"], *histList["TruthAllZ0"]);
    //name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedZ0"]->Integral()/histList["TruthAllZ0"]->Integral());
    name = "Z0";
    effToPlot[name] = effHTTAll;

    histList["TruthAllD0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthAllD0"]->GetXaxis()->SetTitle("D0 [mm]");
    histList["TruthMatchedD0"]->GetYaxis()->SetTitle("Efficiency");
    histList["TruthMatchedD0"]->GetXaxis()->SetTitle("D0 [mm]");
    effHTTAll = new TEfficiency(*histList["TruthMatchedD0"], *histList["TruthAllD0"]);
    //name = Form("0_NN Extrap Only: Ave %.3f", histList["TruthMatchedD0"]->Integral()/histList["TruthAllD0"]->Integral());
    name = "D0";
    effToPlot[name] = effHTTAll;

    return effToPlot;

    // histToPlot["TruthMatchedNHitsPerTrack"] = histList["TruthMatchedNHitsPerTrack"];
    // histList["TruthMatchedNHitsPerTrack"]->GetYaxis()->SetTitle("Number of Hits Per Matched Track");
    // histList["TruthMatchedNHitsPerTrack"]->GetXaxis()->SetTitle("N Hits");
    // histToPlot["TruthAllMatchedNHitsPerTrack"] = histList["TruthAllNHitsPerTrack"];
    // histList["TruthAllNHitsPerTrack"]->GetYaxis()->SetTitle("Number of Hits Per Matched Track");
    // histList["TruthAllNHitsPerTrack"]->GetXaxis()->SetTitle("N Hits");
    
}

void plotEff(map<TString, TEfficiency*> effList, TString outName, bool setLogY, bool setLogX, float maxLim, std::string uncertaintyType)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    int counter = 1754;
    auto labelMap = getRegionLabel();
    TLegend *elLeg =  new TLegend (0.62, 0.75, 0.91, 0.90);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.028);
    std::vector<TString> keys =  {"HT", "biRNN", "RNN", "MLP", "ESN"}; // do it this way so it is correctly ordered
    //for(const auto& eff: effList)
    for(const auto& k: keys)
    {
        auto eff = effList[k];
        eff->SetLineColor(counter);
        eff->SetLineWidth(1.5);
        eff->SetMarkerColor(counter);
        eff->SetMarkerSize(0.75);
        TString name = k;
        name.ReplaceAll("0_", "");
        name.ReplaceAll("1_", "");
        name.ReplaceAll("2_", "");
        TH1* passed = eff->GetCopyPassedHisto();
        TH1* total = eff->GetCopyTotalHisto();

        TString meanEff = Form(" ave. %.3f", passed->Integral() / total->Integral());
        elLeg->AddEntry(eff, name + meanEff, "l");
        //elLeg->AddEntry(eff.second, name + " " + labelMap[eff.first], "l"); 

        counter++;
    }

    effList.begin()->second->Draw("A");
    elLeg->Draw();
    for(const auto& hist: effList) hist.second->Draw("plsame");

    effList.begin()->second->Paint("p");
    effList.begin()->second->GetPaintedGraph()->SetMaximum(maxLim);
    if(maxLim > 0.8) effList.begin()->second->GetPaintedGraph()->SetMinimum(0.8);
     effList.begin()->second->GetPaintedGraph()->SetMinimum(0);
    effList.begin()->second->GetPaintedGraph()->GetYaxis()->SetTitle("Efficiency");
    effList.begin()->second->GetPaintedGraph()->Set(0);

    float leftDist = 0.21;
    //ACTSLabel(leftDist, 0.875, "", 1);
    
    TString label("uncertainty 20 mm") ;
    TLatex* lumInfo = new TLatex (leftDist, 0.855, label);

    TString ESNlabel_str("<#mu> = 200") ;
    TLatex* ESNlabel = new TLatex (leftDist, 0.820, ESNlabel_str);
    //TString ESNlabel_str("*ESN uncertainty 20 mm") ;
    //TLatex* ESNlabel = new TLatex (leftDist, 0.800, ESNlabel_str);
    
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.03);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();

    ESNlabel->SetNDC();
    ESNlabel->SetTextSize(0.03);
    ESNlabel->SetTextFont(42);
    ESNlabel->Draw();

    // label = getAlgoLabel();
    // lumInfo = new TLatex (leftDist, 0.805, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    // label = "";
    // label += getNHitsLabel();
    // lumInfo = new TLatex (leftDist, 0.775, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    // label = "";
    // label += getUncertaintyLabel(uncertaintyType);
    // lumInfo = new TLatex (leftDist, 0.745, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0225);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    if(setLogY) c1->SetLogy();
    if(setLogX) c1->SetLogx();
    auto parts = tokenizeStr(TString(opts["inputFile"]), "/");
    TString folderName = parts.at(parts.size() - 1);
    folderName.ReplaceAll(".root", "");
    folderName += "EfficiencyOverlay/";

    system("mkdir -vp SummaryPlots/" + folderName);


    //TFile* f = new TFile("SummaryPlots/" + folderName + "/" + outName + ".root", "RECREATE");
    //for (auto& hist: effList) hist.second->Write();
    //f->Close();
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName);
    c1->SaveAs("SummaryPlots/" + folderName + "/" + outName.ReplaceAll(".eps", ".C"));
}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]    = "outputFile.root";
    opts["uncertainty"]  = "20";
    opts["minHits"]      = "6"; 
    opts["uncertaintyType"] = "radial";
    opts["phiUncertainty"]  = ".05";
    opts["zUncertainty"]    = "20";

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
    new TColor(ci, 255.0/255 , 77.0/255 , 244.0/255);
    ci++;
    new TColor(ci, 62/255.,    153/255.,    247/255.); //54
    ci++;

    new TColor(ci, 0.8500,    0.3250,    0.0980); //57
    ci++;          
    new TColor(ci, 0.4940,    0.1840,    0.5560); //58
    ci++;
    //new TColor(ci, 0.9290,    0.6940,    0.1250); //59
    //ci++; 
    new TColor(ci, 0.4660,    0.6740,    0.1880); //60
    ci++;
    // new TColor(ci, 0.3010,    0.7450,    0.9330); //61
    // ci++;
    //new TColor(ci, 0,    0.4470,    0.7410); //56
    //ci++;
    //new TColor(ci, 254/255., 139/255., 113/255.); //55
    //ci++;
    //new TColor(ci, 0.6350,    0.0780,    0.1840); //62
    //ci++;
    //new TColor(ci, 142.0/255 , 0.0/255 , 62.0/255);
    //ci++;
    //new TColor(ci, 96.0/255 , 78.0/255 , 0.0/255);
    //ci++;
    //new TColor(ci, 92.0/255 , 174.0/255 , 0.0/255);
    //ci++;
    //new TColor(ci, 28.0/255 , 0.0/255 , 19.0/255);
    //ci++;
    
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
        return TString("uncertainty = " + opts["uncertainty"]);
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
    return TString("Nhits - " + opts["minHits"] + " hits");
}

TString getAlgoLabel()
{
    TString label = "Nominal Hits";
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

map<TString, TString> getRegionLabel()
{   
    map<TString, TString> trackingInfo;
    trackingInfo["RNN"] = trackingInfoRNN;
    trackingInfo["MLP"] = trackingInfoMLP;
    trackingInfo["biRNN"] = trackingInfobiRNN;
    trackingInfo["ESN"] = trackingInfoESN;
    return trackingInfo;
    // TString label = "";
    // if(TString(opts["inputFile"]).Contains("Region_0p1_0p3")) label = "#eta #in [0.1, 0.3], #mu = 200";
    // if(TString(opts["inputFile"]).Contains("Region_0p7_0p9")) label = "#eta #in [0.7, 0.9], #mu = 200";
    // if(TString(opts["inputFile"]).Contains("Region_2p0_2p2")) label = "#eta #in [2.0, 2.2], #mu = 200";
    // return label;
}
