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
#include "TMath.h"
#include "TF1.h"
#include "TLine.h"
#include "TGraphAsymmErrors.h"

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
void setPrettyStuff();
std::vector<TString> tokenizeStr(TString str, TString key);

void plotGraph(map<TString, TGraph*> gList, TString outName, bool setLog);

int main(int argc, char *argv[])
{   
    setPrettyStuff();

    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;

    // map<TString, map<double, TString>> fileName;

    auto fileList  = tokenizeStr(TString(opts["fileList"]), ",");
    auto labelList = tokenizeStr(TString(opts["label"]), ",");
    auto HWList    = tokenizeStr(TString(opts["nHitOverlapThres"]), ",");

    map<TString, TGraph*> graphList;
    int counter = 0;
    for(const auto& fileName: fileList)
    {
        PlotHolder* plotHolder = new PlotHolder();
        plotHolder->readHist(fileName);

        PlotHolder* pionPlotHolder = NULL;
        if(fileName.Contains("muonOnly"))
        {
            pionPlotHolder = new PlotHolder();
            TString fileNamePion = fileName;
            fileNamePion = fileNamePion.ReplaceAll("muonOnly", "pileupOnly");
            cout<<"Trying to read: "<<fileNamePion<<endl;
            pionPlotHolder->readHist(fileNamePion);
        }

        for(const auto& HWtresh:HWList)
        {
            auto NNList = plotHolder->getNNList(atoi(HWtresh));
            map<float, float> rocCurveBkg;
            map<float, float> rocCurveSig;
            map<float, float> signalEff;
            for(const auto& nnCut: NNList)
            {
                if(nnCut < 0) continue;
                map<TString, TH1F*> histList = plotHolder->getHistMap(nnCut, atoi(HWtresh));
                map<TString, TH1F*> histPionList;
                if(pionPlotHolder) histPionList = pionPlotHolder->getHistMap(nnCut, atoi(HWtresh));

                map<TString, float> numList;
                map<TString, float> numPionList;

                double maxBkgEff = 0;
                double maxSigEff = 0;

                for(int i = 0; i < 10; i++)
                {
                    if(histList["counterHist"]->GetBinContent(i+1) <= 0) continue;
                    numList[histList["counterHist"]->GetXaxis()->GetBinLabel(i+1)] = histList["counterHist"]->GetBinContent(i+1);
                    if(pionPlotHolder) numPionList[histPionList["counterHist"]->GetXaxis()->GetBinLabel(i+1)] = histPionList["counterHist"]->GetBinContent(i+1);
                }

                // WRT Htt
                double truthEff = numList["TruthMatchedSelected"]/numList["TruthMatched"];

                // WRT Full Truth
                // double truthEff = numList["TruthMatchedSelected"]/numList["TotalTruth"];

                // double truthEff = numList["TruthMatched"]/numList["TotalTruth"];

                // double bkgRej = numList["SelectedReco"]/numList["TotalReco"];
                // if(numList.find("OverlapRemovedReco") != numList.end()) bkgRej = numList["OverlapRemovedReco"]/numList["TotalReco"];

                double bkgRej = numList["SelectedReco"]/numList["NEvents"];
                if(numList.find("OverlapRemovedReco") != numList.end()) bkgRej = numList["OverlapRemovedReco"]/numList["NEvents"];

                if(pionPlotHolder)
                {
                    bkgRej = numPionList["SelectedReco"]/numPionList["NEvents"];
                    if(numPionList.find("OverlapRemovedReco") != numList.end()) bkgRej = numPionList["OverlapRemovedReco"]/numPionList["NEvents"];
                }

                cout<<fileName<<" NN: "<<nnCut<<" HW: "<<HWtresh<<" truthEff: "<<truthEff<<" bkgRej: "<<bkgRej<<" truth Tracks: "<<numList["TruthMatched"]<<endl;

                rocCurveBkg[nnCut] = bkgRej;
                rocCurveSig[nnCut] = truthEff;
                signalEff[nnCut] = truthEff;

                if(bkgRej > maxBkgEff)
                {
                    maxBkgEff = bkgRej;
                    maxSigEff = truthEff;
                }

            }

            vector<float> _x;
            vector<float> _y;
            for(const auto& var: rocCurveBkg)
            {
                _x.push_back(rocCurveBkg[var.first]);
                _y.push_back(rocCurveSig[var.first]);
            }

            vector<float> _xSigEff;
            vector<float> _ySigEff;
            for(const auto& var: signalEff)
            {
                _ySigEff.push_back(var.first);
                _xSigEff.push_back(var.second);
            }

            // TString key = Form("%d", counter) + fileName + "_nHit" + HWtresh;
            TString key = labelList.at(counter);
            counter++;
            TGraph* g = new TGraph(_x.size(), &_x[0], &_y[0]);
            graphList[key] = g;


            // Inverse to get the rejection
            TGraph* g2 = new TGraph(_x.size(), &_x[0], &_y[0]);

            TGraph* invSigEff = new TGraph(_xSigEff.size(), &_xSigEff[0], &_ySigEff[0]);


            cout<<key<<" 99% bkg eff: "<<g2->Eval(0.98)<<" rejection: "<<1./g2->Eval(0.98)<< " NN val: "<<invSigEff->Eval(0.98)<<endl;



        }
    }

               
   plotGraph(graphList, "SummaryPlots/" + TString(opts["outFolder"]) + "_Compare.eps", false);

}

void plotGraph(map<TString, TGraph*> gList, TString outName, bool setLog)
{
    TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 600);

    double minXF = 100000000;
    double maxXF = -1000000000;
    double minYF = 10000000000000;
    double maxYF = -1000000000000;


    for(const auto& g: gList)
    {
        double minX = TMath::MinElement(g.second->GetN(),g.second->GetX());
        double maxX = TMath::MaxElement(g.second->GetN(),g.second->GetX());
        double minY = (TMath::MinElement(g.second->GetN(),g.second->GetY()));
        double maxY = (TMath::MaxElement(g.second->GetN(),g.second->GetY()));
        if(minX < minXF) minXF = minX;  
        if(minY < minYF) minYF = minY;  
        if(maxX > maxXF) maxXF = maxX;  
        if(maxY > maxYF) maxYF = maxY;  
    }
    map<TString, double> rocIntegral;

    // maxXF = 50;
    // for(const auto& g: gList)
    // {
    //     float distance = 0.0001;
    //     float integral = 0;
    //     for(float i = minXF; i < maxXF - distance; i+= distance)
    //     {
    //         integral += ((g.second->Eval(i) + g.second->Eval(i+distance))/2) * distance;
    //     }
    //     // cout<<g.first<<" "<<integral/(maxXF-minXF)<<endl;
    //     rocIntegral[g.first] = integral/(maxXF-minXF);

    // }

    // Pick the top 5 to plot
    map<TString, TGraph*> gCopy = gList;
    // gList.clear();


    int counter = 1754;

    TLegend *elLeg =  new TLegend (0.45, 0.65, 0.90, 0.85);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.0225);


    for(const auto& g: gList)
    {
        g.second->SetLineColor(counter);
        g.second->SetMarkerColor(counter);
        g.second->SetLineWidth(2);
        g.second->SetMarkerSize(0);
        counter++;

        TString name = g.first;


             if(TString(g.first).Contains("_Nom_"))     name = "Without Stub Filtering";
        else if(TString(g.first).Contains("_sp_"))      name = "Spacepoints";
        else if(TString(g.first).Contains("_spstubs_")) name = "Spacepoints + stubs";
        else if(TString(g.first).Contains("_spstub_"))  name = "Spacepoints + stubs";
        else if(TString(g.first).Contains("_stubs_"))   name = "With Stub Filtering";
        else if(TString(g.first).Contains("_stub_"))    name = "With Stub Filtering";
        else if(TString(g.first).Contains("_Stub_"))    name = "With Stub Filtering";
        else if(TString(g.first).Contains("_Stubs_"))   name = "With Stub Filtering";

        if(g.first.Contains("nHit101"))name += " No NN Overlap";
        if(g.first.Contains("nHit4"))  name += " NN Overlap - 4 hit threshold";
        if(g.first.Contains("nHit5"))  name += " NN Overlap - 5 hit threshold";
        if(g.first.Contains("nHit6"))  name += " NN Overlap - 6 hit threshold";
        if(g.first.Contains("nHit7"))  name += " NN Overlap - 7 hit threshold";
        if(g.first.Contains("nHit2"))  name += " NN Overlap - 2 hit threshold";

        //      if(g.first.Contains("pt1_2")) name += " pt1_2";
        // else if(g.first.Contains("pt2_4")) name += " pt2_4";
        // else if(g.first.Contains("pt4_6")) name += " pt4_6";
        // else if(g.first.Contains("pt6_Inf")) name += " pt6_Inf";
        // else  name += " Inc";

        // if(g.first.Contains("_HTTFake_HTTTrueMuPi_SingleP_ttbar")) name += " HTTTrueMuPi_SingleP_ttbar";
        // else if(g.first.Contains("_HTTFake_HTTTrueMu_SingleP_")) name += " HTTTrueMu_SingleP";
        // else if(g.first.Contains("_HTTFake_HTTTrueMuPi_")) name += " HTTTrueMuPi";
        // else if(g.first.Contains("_HTTFake_HTTTrueMu_")) name += " HTTTrueMu";
        // else if(g.first.Contains("_HTTFake_singleP_ttbar_")) name += " SingleP_ttbar";
        // else if(g.first.Contains("_HTTFake_singleP_")) name += " SingleP";
        // else if(g.first.Contains("_HTTFake_HTTTrue_singleP_")) name += " HTTTrueMu_SingleP";
        // else if(g.first.Contains("_HTTFake_HTTTrue_")) name += " HTTTrueMu";

        if(g.first.Contains("muonOnly")) name += " muon Only";
        else if(g.first.Contains("pileupOnly")) name += " pion Only";


        // cout<<g.first<<" ---------------" <<name<<endl;

        elLeg->AddEntry(g.second, name, "l"); 
    }


    double dist = maxYF - minYF;
    maxYF += 0.70 *dist;
    minYF -= 0.05 *dist;

    TH1F* frame = new TH1F("axis","axis", 10, 0, maxXF);    
    frame->SetMaximum(maxYF);
    frame->SetMinimum(minYF);
    frame->GetXaxis()->SetNdivisions(505);

    frame->GetYaxis()->SetTitle("Truth Efficiency");
    frame->GetXaxis()->SetTitle("N_{Tracks}/Event");


    frame->Draw();
    elLeg->Draw();
    for(const auto& g: gList)
    {
        g.second->Draw("plSame");
    } 

    float leftDist = 0.19;
    ATLASLabel(leftDist, 0.875, "Simulation Internal", 1);


    // TLine* line = new TLine(0, 0.99, maxXF, 0.99);
    // line->SetLineColor(kGray+3);
    // line->SetLineStyle(2);
    // line->Draw();

    TString label = "";
    if(TString(gList.begin()->first).Contains("Region_0p1_0p3")) label = "#eta #in [0.1, 0.3], #mu = 200";
    if(TString(gList.begin()->first).Contains("Region_0p7_0p9")) label = "#eta #in [0.7, 0.9], #mu = 200";
    if(TString(gList.begin()->first).Contains("Region_2p0_2p2")) label = "#eta #in [2.0, 2.2], #mu = 200";
    TLatex* lumInfo = new TLatex (leftDist, 0.825, label);
    lumInfo->SetNDC();
    lumInfo->SetTextSize(0.03);
    lumInfo->SetTextFont(42);
    lumInfo->Draw();


    // label = "";
    // if(TString(gList.begin()->first).Contains("_Nom_"))     label = "Nominal hits";
    // if(TString(gList.begin()->first).Contains("_sp_"))      label = "Spacepoints";
    // if(TString(gList.begin()->first).Contains("_spstubs_")) label = "Spacepoints + stubs";
    // if(TString(gList.begin()->first).Contains("_spstub_"))  label = "Spacepoints + stubs";
    // if(TString(gList.begin()->first).Contains("_stubs_"))   label = "Stubs";
    // if(TString(gList.begin()->first).Contains("_stub_"))    label = "Stubs";
    // if(TString(gList.begin()->first).Contains("_Stub_"))    label = "Stubs";
    // if(TString(gList.begin()->first).Contains("_Stubs_"))   label = "Stubs";
    // lumInfo = new TLatex (leftDist, 0.785, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0325);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    // label = "";
    // if(outName.Contains("Muon")) label = "Muon Only";
    // if(outName.Contains("Pion")) label = "Pion Only";
    // lumInfo = new TLatex (leftDist, 0.755, label);
    // lumInfo->SetNDC();
    // lumInfo->SetTextSize(0.0275);
    // lumInfo->SetTextFont(42);
    // lumInfo->Draw();

    if(setLog) c1->SetLogy();

    c1->SaveAs(outName);

}


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["fileList"]            = "All";
    opts["nHitOverlapThres"]    = "5";
    opts["outFolder"]           = "DNN_Region_0p1_0p3_HTTFake_HTTTrueMuPi_SingleP_ttbar_8L_Stubs_v6";
    opts["label"]               = "label";

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
