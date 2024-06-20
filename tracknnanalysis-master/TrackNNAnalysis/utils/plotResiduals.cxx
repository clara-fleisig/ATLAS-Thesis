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

#include "TH1F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TString.h"

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);
void setPrettyStuff();

void plotHist(map<TString, TH1F*> histList, TString outName, bool setLog, bool norm, bool setLogX);
map<TString, TH1F*> getLayerHist(map<TString, TH1F*> histList, TString key);
map<TString, TH1F*> getMatchedHitHist(map<TString, TH1F*> histList);
std::vector<TString> tokenizeStr(TString str, TString key);

map<std::string, std::string> opts;
PlotHolderExtrap* plotHolder = NULL;

TString subFolderKey = "";
int main(int argc, char*argv[])
{
    setPrettyStuff();

    // Process command line arguments
    if(!cmdline(argc,argv, opts)) return 0;

    plotHolder = new PlotHolderExtrap();
    plotHolder->readHist(TString(opts["inputFile"]));

    std::cout<<"read hists"<<std::endl;
    map<TString, TH1F*> histList = plotHolder->getHistMap();
    map<TString, TH1F*> toPlot;
    subFolderKey = opts["subFolderKey"];

    std::cout<<"beginning histogram making"<<std::endl;
    toPlot["trueRho"] = histList["true_rho"];

    histList["true_rho"]->GetXaxis()->SetTitle("#rho [mm]");
    histList["true_rho"]->GetYaxis()->SetTitle("Normalized Events");
    toPlot["predictedRho"] = histList["predicted_rho"];
    histList["predicted_rho"]->GetXaxis()->SetTitle("#rho [mm]");
    histList["predicted_rho"]->GetYaxis()->SetTitle("Normalized Events");
    toPlot["matchedHit_trueRho"] = histList["matchedHit_true_rho"];
    histList["matchedHit_true_rho"]->GetXaxis()->SetTitle("#rho [mm]");
    histList["matchedHit_true_rho"]->GetYaxis()->SetTitle("Normalized Events");

    // toPlot["unmatchedHit_trueRho"] = histList["unmatchedHit_true_rho"];
    // histList["unmatchedHit_true_rho"]->GetXaxis()->SetTitle("#rho [mm]");
    // histList["unmatchedHit_true_rho"]->GetYaxis()->SetTitle("Normalized Events");

    plotHist(toPlot, "RhoDistribution.eps", false, true, false);
    toPlot.clear();
    std::cout<<"plotted matched Hits truth"<<std::endl;

    toPlot["truePhi"] = histList["true_phi"];
    histList["true_phi"]->GetXaxis()->SetTitle("#phi [rad]");
    histList["true_phi"]->GetYaxis()->SetTitle("Normalized Events");
    toPlot["predictedphi"] = histList["predicted_phi"];
    histList["predicted_phi"]->GetXaxis()->SetTitle("#phi [rad]");
    histList["predicted_phi"]->GetYaxis()->SetTitle("Normalized Events");
    toPlot["matchedHit_truePhi"] = histList["matchedHit_true_phi"];
    histList["matchedHit_true_phi"]->GetXaxis()->SetTitle("#phi [rad]");
    histList["matchedHit_true_phi"]->GetYaxis()->SetTitle("Normalized Events");
    // toPlot["unmatchedHit_truePhi"] = histList["unmatchedHit_true_phi"];
    // histList["unmatchedHit_true_phi"]->GetXaxis()->SetTitle("#phi [rad]");
    // histList["unmatchedHit_true_phi"]->GetYaxis()->SetTitle("Normalized Events");

    plotHist(toPlot, "phiDistribution.eps", false, true, false);
    toPlot.clear();

    toPlot["truez"] = histList["true_z"];
    histList["true_z"]->GetXaxis()->SetTitle("#z [mm]");
    histList["true_z"]->GetYaxis()->SetTitle("Normalized Events");
    toPlot["predictedz"] = histList["predicted_z"];
    histList["predicted_z"]->GetXaxis()->SetTitle("z [mm]");
    histList["predicted_z"]->GetYaxis()->SetTitle("Normalized Events");
    toPlot["matchedHit_trueZ"] = histList["matchedHit_true_z"];
    histList["matchedHit_true_z"]->GetXaxis()->SetTitle("#z [mm]");
    histList["matchedHit_true_z"]->GetYaxis()->SetTitle("Normalized Events");
    // toPlot["unmatchedHit_trueZ"] = histList["unmatchedHit_true_z"];
    // histList["unmatchedHit_true_z"]->GetXaxis()->SetTitle("#z [mm]");
    // histList["unmatchedHit_true_z"]->GetYaxis()->SetTitle("Normalized Events");

    plotHist(toPlot, "zDistribution.eps", false, true, false);
    toPlot.clear();

    toPlot["rho_residuals"] =  histList["residuals_rho"];
    histList["residuals_rho"]->GetXaxis()->SetTitle("#rho_{true} - #rho_{pred} [mm] per hit ");
    plotHist(toPlot, "rhoResiduals.eps", false, true, false);
    toPlot.clear();

    toPlot["phi_residuals"] =  histList["residuals_phi"];
    histList["residuals_phi"]->GetXaxis()->SetTitle("#phi_{true} - #phi_{pred} [rad] per hit");
    plotHist(toPlot, "phiResiduals.eps", false, true, false);
    toPlot.clear();

    toPlot["z_residuals"] =  histList["residuals_z"];
    histList["residuals_z"]->GetXaxis()->SetTitle("z_{true} - z_{pred} [mm] per hit");
    plotHist(toPlot, "zResiduals.eps", false, true, false);
    toPlot.clear();

    toPlot["matchedHit_residualsZ"] =  histList["matchedHit_resZ"];
    histList["matchedHit_resZ"]->GetXaxis()->SetTitle("z_{true} - z_{pred} [mm] per hit");
    plotHist(toPlot, "matchedHit_zResiduals.eps", true, false, false);
    toPlot.clear();

    toPlot["matchedHit_residualsRho"] =  histList["matchedHit_resRho"];
    histList["matchedHit_resRho"]->GetXaxis()->SetTitle("#rho_{true} - #rho_{pred} [mm] per hit");
    plotHist(toPlot, "matchedHit_rhoResiduals.eps", true, false, false);
    toPlot.clear();

    toPlot["matchedHit_residualsPhi"] =  histList["matchedHit_resPhi"];
    histList["matchedHit_resPhi"]->GetXaxis()->SetTitle("#phi_{true} - #phi_{pred} [rad] per hit");
    plotHist(toPlot, "matchedHit_phiResiduals.eps", true, false, false);
    toPlot.clear();

    toPlot["unmatchedHit_residualsZ"] =  histList["unmatchedHit_resZ"];
    histList["unmatchedHit_resZ"]->GetXaxis()->SetTitle("z_{true} - z_{pred} [mm] per hit");
    plotHist(toPlot, "unmatchedHit_zResiduals.eps", true, false, false);
    toPlot.clear();

    toPlot["unmatchedHit_residualsRho"] =  histList["unmatchedHit_resRho"];
    histList["unmatchedHit_resZ"]->GetXaxis()->SetTitle("#rho_{true} - #rho_{pred} [mm] per hit");
    plotHist(toPlot, "unmatchedHit_rhoResiduals.eps", true, false, false);
    toPlot.clear();

    toPlot["unmatchedHit_residualsPhi"] =  histList["unmatchedHit_resPhi"];
    histList["unmatchedHit_resPhi"]->GetXaxis()->SetTitle("#phi_{true} - #phi_{pred} [rad] per hit");
    plotHist(toPlot, "unmatchedHit_phiResiduals.eps", true, false, false);
    toPlot.clear();

    // per layer info
    std::vector<TString> coordinates = {"rho", "phi", "z"};
    for (auto& c: coordinates)
    {
        auto hList = getLayerHist(histList, c);
        plotHist(hList, c+"_layerResiduals.eps", true, true, false);
    }

    auto hList = getMatchedHitHist(histList);
    plotHist(hList, "matchedHitFrac.eps", true, false, false);
}

map<TString, TH1F*> getLayerHist(map<TString, TH1F*> histList, TString key)
{
    //key should be either z, phi, rho
    // cout<<"Looking for key: "<<key<<endl;
    map<TString, TH1F*> toPlot;
    for(const auto& hist: histList)
    {
        if(hist.first.Contains("residuals_"+key+"_vol"))
        {
            TString histName = hist.first;
            histName.ReplaceAll("residuals_"+key+"_", "");
            histName += Form(" #sigma=%.3f", hist.second->GetStdDev());
            if (key == "rho") hist.second->GetXaxis()->SetTitle("#rho_{true} - #rho_{pred} [mm] per hit");
            else if (key == "phi") hist.second->GetXaxis()->SetTitle("#phi_{true} - #phi_{pred} [rad] per hit");
            else if (key == "z") hist.second->GetXaxis()->SetTitle("z_{true} - z_{pred} [rad] per hit");
            else
            {
                 cout<<"coordinate unknown"<<endl;
                 exit(1);
            }
            toPlot[histName] = hist.second;
        } 
    
    }
    return toPlot;
}

map<TString, TH1F*> getMatchedHitHist(map<TString, TH1F*> histList)
{
    // cout<<"Looking for key: "<<key<<endl;
    map<TString, TH1F*> toPlot;
    for(const auto& hist: histList)
    {
        if(hist.first.Contains("matchedHitFrac"))
        {
            TString histName = hist.first;
            histName.ReplaceAll("matchedHItFrac_", "");
            histName += Form(" frac=%.3f", hist.second->GetMean());
            toPlot[histName] = hist.second;
        } 
    
    }
    return toPlot;
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
        hist.second->SetLineWidth(1.5);
        hist.second->GetXaxis()->SetLabelSize(.015);

        TString name = hist.first;
        // name.ReplaceAll("0_", "");
        // name.ReplaceAll("1_", "");
        // name.ReplaceAll("2_", "");
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
        if(hist.first.Contains("residuals"))
        {
            auto stddev = hist.second->GetStdDev();
            TString label = Form("#sigma = %.3f", stddev) ;
            TLatex* lumInfo = new TLatex (0.725, 0.780, label);
            lumInfo->SetNDC();
            lumInfo->SetTextSize(0.0325);
            lumInfo->SetTextFont(42);
            lumInfo->Draw();   
        }
        if(norm) hist.second->DrawNormalized("histSame");
        else hist.second->Draw("histSame");
    } 

    float leftDist = 0.19;
    ATLASLabel(leftDist, 0.875, "Simulation Internal", 1);


    TString label = "RNN";
    TLatex* lumInfo = new TLatex (0.725, 0.835, label);
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


bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "outputFile.root";
    opts["NNCutVal"]        = "0.2";
    opts["nHitOverlapThres"] = "5";
    opts["subFolderKey"]     = "RNN";

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
