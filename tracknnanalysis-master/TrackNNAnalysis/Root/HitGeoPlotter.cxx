// Class includes
#include "TrackNNAnalysis/HitGeoPlotter.h"
#include "TrackNNAnalysis/atlasstyle/AtlasLabels.h"
#include "TrackNNAnalysis/atlasstyle/AtlasStyle.h"

// Local includes
// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <math.h>
#include <limits>
#include <TMath.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TRotation.h>

using namespace std;


// Constructors and destructor
HitGeoPlotter::HitGeoPlotter(std::vector<std::shared_ptr<DetectorModule>> dectorModule)
{
    m_moduleList = dectorModule;
}

HitGeoPlotter::~HitGeoPlotter()
{

} 


// big setters
void HitGeoPlotter::initialize()
{
    SetAtlasStyle();
    // Get all the dector points
    vector<float> _x;
    vector<float> _y;
    vector<float> _z;
    vector<float> _rho;

    for(const auto& mod: m_moduleList)
    {
        // if(mod->getVolume() != 10 && mod->getVolume() != 0) continue;
        _x.push_back(mod->getX());
        _y.push_back(mod->getY());
        _z.push_back(mod->getZ());
        _rho.push_back(mod->getRho());
    }

    TGraph* xy = new TGraph(_x.size(), &_x[0], &_y[0]);
    xy->SetMarkerSize(0.01);
    xy->SetMarkerColor(kGray + 2);
    m_dectortorXY.push_back(xy);

    TGraph* zRho = new TGraph(_z.size(), &_z[0], &_rho[0]);
    zRho->SetMarkerSize(0.01);
    zRho->SetMarkerColor(kGray + 2);
    m_dectortorZrho.push_back(zRho);

}


void HitGeoPlotter::plotHitsList(std::vector<std::shared_ptr<Hit>> hitList, TString baseName, bool barrelOnly)
{
    TGraph* hitsXY = NULL;
    TGraph* hitszRho = NULL;

    vector<TPolyLine*> polyXY;
    vector<TPolyLine*> polyzRho;
    
    hitsXY = getPlotHits(hitList, PlotType::XY);
    hitszRho = getPlotHits(hitList, PlotType::zRho);

    for (const auto& hitModule: m_moduleList)
    {
        if(barrelOnly)
        {
            if( !hitModule->getIntMetadata("isBarrel")) continue;
        }

        polyXY.push_back(getModuleVolume(hitModule, PlotType::XY));
        polyzRho.push_back(getModuleVolume(hitModule, PlotType::zRho));
    }
    
    plotHits(baseName + "_XY.eps", m_dectortorXY, polyXY, hitsXY, PlotType::XY);
    plotHits(baseName + "_zRho.eps", m_dectortorZrho, polyzRho, hitszRho, PlotType::zRho);
}

void HitGeoPlotter::plotTrueAndPredictedHitsList(std::vector<std::shared_ptr<Hit>> trueHitList,std::vector<std::shared_ptr<Hit>> predHitList, TString baseName, bool barrelOnly)
{
    TGraph* trueHitsXY = NULL;
    TGraph* trueHitszRho = NULL;
    TGraph* predHitsXY = NULL;
    TGraph* predHitszRho = NULL;

    vector<TPolyLine*> polyXY;
    vector<TPolyLine*> polyzRho;
    
    trueHitsXY =   getPlotHits(trueHitList, PlotType::XY);
    trueHitszRho = getPlotHits(trueHitList, PlotType::zRho);
    predHitsXY =   getPlotHits(predHitList, PlotType::XY);
    predHitszRho = getPlotHits(predHitList, PlotType::zRho);

    for (const auto& hitModule: m_moduleList)
    {
        if(barrelOnly)
        {
            if( !hitModule->getIntMetadata("isBarrel")) continue;
        }

        polyXY.push_back(getModuleVolume(hitModule, PlotType::XY));
        polyzRho.push_back(getModuleVolume(hitModule, PlotType::zRho));
    }
    
    plotTrueAndPredictedHits(baseName + "_XY.eps", m_dectortorXY, polyXY, trueHitsXY, predHitsXY, PlotType::XY);
    plotTrueAndPredictedHits(baseName + "_zRho.eps", m_dectortorZrho, polyzRho, trueHitszRho, predHitszRho, PlotType::zRho);
}

void HitGeoPlotter::plotHits(TString baseName, std::vector<TGraph*> detectorList, std::vector<TPolyLine*> detectorVol, TGraph* hitGraph, PlotType plotType)
{
    double minRangeX =  std::numeric_limits<double>::infinity();
    double maxRangeX = -std::numeric_limits<double>::infinity();

    double minRangeY =  std::numeric_limits<double>::infinity();
    double maxRangeY = -std::numeric_limits<double>::infinity();

    for(const auto& gr: detectorList)
    {
        double maxX = TMath::MaxElement(gr->GetN(), gr->GetX());
        double minX = TMath::MinElement(gr->GetN(), gr->GetX());

        double maxY = TMath::MaxElement(gr->GetN(), gr->GetY());
        double minY = TMath::MinElement(gr->GetN(), gr->GetY());

        if(maxX > maxRangeX) maxRangeX = maxX;
        if(maxY > maxRangeY) maxRangeY = maxY;

        if(minX < minRangeX) minRangeX = minX;
        if(minY < minRangeY) minRangeY = minY;

    }

    if(plotType == PlotType::XY)
    {
        double range = maxRangeX - minRangeX;
        maxRangeX += 0.1 * range; 
        minRangeX -= 0.1 * range; 

        maxRangeY += 0.1 * range; 
        minRangeY -= 0.1 * range; 

        // minRangeY = 0;
        // minRangeX = 0;
    }
    
    if(plotType == PlotType::zRho)
    {
        double range = maxRangeX - minRangeX;
        maxRangeX += 0.1 * range; 
        minRangeX -= 0.1 * range; 

        maxRangeY *= 1.3; 
        minRangeY = 0; 
    }

    TH2F* frame = new TH2F("frame", "frame", 1, minRangeX, maxRangeX, 1, minRangeY, maxRangeY );

    TCanvas* c = new TCanvas("", "", 0, 0, 600, 600);
    c->SetRightMargin(0.09);
    c->SetLeftMargin(0.15);
    c->SetBottomMargin(0.15);
    
    if(plotType == PlotType::XY)
    {
        frame->GetXaxis()->SetTitle("x [mm]");
        frame->GetYaxis()->SetTitle("y [mm]");
    }
    if(plotType == PlotType::zRho)
    {
        frame->GetXaxis()->SetTitle("z [mm]");
        frame->GetYaxis()->SetTitle("#rho [mm]");
    }

    frame->GetXaxis()->SetLabelSize(.042);
    frame->GetXaxis()->SetNdivisions(5, kTRUE);

    frame->GetYaxis()->SetLabelSize(.042);

    frame->Draw();
    for(auto gr:detectorVol) gr->Draw("f");
    for(auto gr:detectorVol) gr->Draw();
    //for(auto gr:detectorList) gr->Draw("p");
    
    TGraph* hitGraphUnderlay = new TGraph(*hitGraph);

    hitGraph->SetMarkerSize(.5);
    hitGraphUnderlay->SetMarkerSize(0.55);
    hitGraph->SetMarkerColor(kAzure+7);
    hitGraphUnderlay->SetMarkerColor(kBlack);
    hitGraphUnderlay->Draw("p");
    hitGraph->Draw("p");

    
    // elLeg->Draw();


    //float leftDist = 0.19;
    //ATLASLabel(leftDist, 0.875, "Internal", 1);

    c->SaveAs(baseName);
    c->SaveAs(baseName.ReplaceAll(".eps",".C"));

    delete c;

}

void HitGeoPlotter::plotTrueAndPredictedHits(TString baseName, std::vector<TGraph*> detectorList, std::vector<TPolyLine*> detectorVol, TGraph* trueHitGraph, TGraph* predHitGraph, PlotType plotType)
{
    double minRangeX =  std::numeric_limits<double>::infinity();
    double maxRangeX = -std::numeric_limits<double>::infinity();

    double minRangeY =  std::numeric_limits<double>::infinity();
    double maxRangeY = -std::numeric_limits<double>::infinity();

    for(const auto& gr: detectorList)
    {
        double maxX = TMath::MaxElement(gr->GetN(), gr->GetX());
        double minX = TMath::MinElement(gr->GetN(), gr->GetX());

        double maxY = TMath::MaxElement(gr->GetN(), gr->GetY());
        double minY = TMath::MinElement(gr->GetN(), gr->GetY());

        if(maxX > maxRangeX) maxRangeX = maxX;
        if(maxY > maxRangeY) maxRangeY = maxY;

        if(minX < minRangeX) minRangeX = minX;
        if(minY < minRangeY) minRangeY = minY;

    }

    if(plotType == PlotType::XY)
    {
        double range = maxRangeX - minRangeX;
        maxRangeX += 0.1 * range; 
        minRangeX -= 0.1 * range; 

        maxRangeY += 0.1 * range; 
        minRangeY -= 0.1 * range; 

        // minRangeY = 0;
        // minRangeX = 0;
    }
    
    if(plotType == PlotType::zRho)
    {
        // maxRangeX = 2000;
        // minRangeX = -2000;

        // maxRangeY = 1200;
        // minRangeY = -100;
        double range = maxRangeX - minRangeX;
        maxRangeX += 0.1 * range; 
        minRangeX -= 0.1 * range; 

        maxRangeY *= 1.3; 
        minRangeY = 0; 
    }

    TH2F* frame = new TH2F("frame", "frame", 1, minRangeX, maxRangeX, 1, minRangeY, maxRangeY );

    TCanvas* c = new TCanvas("", "", 0, 0, 600, 600);
    c->SetRightMargin(0.09);
    c->SetLeftMargin(0.15);
    c->SetBottomMargin(0.15);
    
    if(plotType == PlotType::XY)
    {
        frame->GetXaxis()->SetTitle("x [mm]");
        frame->GetYaxis()->SetTitle("y [mm]");
    }
    if(plotType == PlotType::zRho)
    {
        frame->GetXaxis()->SetTitle("z [mm]");
        frame->GetYaxis()->SetTitle("#rho [mm]");
    }

    frame->GetXaxis()->SetLabelSize(.042);
    frame->GetXaxis()->SetNdivisions(5, kTRUE);

    frame->GetYaxis()->SetLabelSize(.042);

    frame->Draw();
    for(auto gr:detectorVol) gr->Draw("f");
    for(auto gr:detectorVol) gr->Draw();
    //for(auto gr:detectorList) gr->Draw("p");
    
    TGraph* trueHitGraphUnderlay = new TGraph(*trueHitGraph);
    TGraph* predHitGraphUnderlay = new TGraph(*predHitGraph);

    trueHitGraph->SetMarkerSize(.5);
    trueHitGraphUnderlay->SetMarkerSize(0.55);
    trueHitGraph->SetMarkerColor(kAzure+7);
    trueHitGraphUnderlay->SetMarkerColor(kBlack);
    trueHitGraphUnderlay->Draw("p");
    trueHitGraph->Draw("p");

    predHitGraph->SetMarkerSize(.5);
    predHitGraphUnderlay->SetMarkerSize(0.55);
    predHitGraph->SetMarkerColor(kRed);
    predHitGraphUnderlay->SetMarkerColor(kBlack);
    predHitGraphUnderlay->Draw("p");
    predHitGraph->Draw("p");

    
    // elLeg->Draw();


    //float leftDist = 0.19;
    //ATLASLabel(leftDist, 0.875, "Internal", 1);

    c->SaveAs(baseName);
    c->SaveAs(baseName.ReplaceAll(".eps",".C"));

    delete c;

}

TGraph* HitGeoPlotter::getPlotHits(std::vector<std::shared_ptr<Hit>> hitList, PlotType plotType)
{
    // Get all the dector points
    vector<float> _x;
    vector<float> _y;


    for(const auto& hit: hitList)
    {
        std::shared_ptr<DetectorModule> hitModule = NULL;

        if(plotType == PlotType::XY)
        {
            _x.push_back(hit->getX());
            _y.push_back(hit->getY());
        }
        if(plotType == PlotType::zRho)
        {
            _x.push_back(hit->getZ());
            _y.push_back(hit->getRho());
        }
    }

    TGraph* gr = new TGraph(_x.size(), &_x[0], &_y[0]);

    return gr;
}


TGraph* HitGeoPlotter::getPlotHitsModule(std::vector<std::shared_ptr<Hit>> hitList, PlotType plotType)
{
    // Get all the dector points
    vector<float> _modx;
    vector<float> _mody;

    for(const auto& hit: hitList)
    {
        std::shared_ptr<DetectorModule> hitModule = NULL;

        for(const auto& mod: m_moduleList)
        {
            if(hit->getModule() != mod->getModule()) continue;
            // if(hit->getLayer() != mod->getLayer()) continue;
            // if(hit->getPixorSct() != mod->getPixorSct()) continue;

            hitModule = mod;
        }

        if(plotType == PlotType::XY)
        {
            if(hitModule)
            {
                _modx.push_back(hitModule->getX());
                _mody.push_back(hitModule->getY());                
            }
        }
        if(plotType == PlotType::zRho)
        {
            if(hitModule)
            {
                _modx.push_back(hitModule->getZ());
                _mody.push_back(hitModule->getRho());                
            }

        }
    }

    TGraph* gr = new TGraph(_modx.size(), &_modx[0], &_mody[0]);
    gr->SetMarkerSize(0.5);
    gr->SetMarkerColor(kRed);
    gr->SetLineColor(kRed);

    return gr;
}

TPolyLine* HitGeoPlotter::getModuleVolume(std::shared_ptr<DetectorModule> mod, PlotType plotType)
{

    vector<float> xValOrg;
    vector<float> yValOrg;
    vector<float> zValOrg;

    xValOrg.push_back(-mod->getHalfMaxWidth());
    yValOrg.push_back(mod->getHalfLength());
    zValOrg.push_back(0);

    xValOrg.push_back(0);
    yValOrg.push_back(mod->getHalfLength());
    zValOrg.push_back(0);


    xValOrg.push_back(mod->getHalfMaxWidth());
    yValOrg.push_back(mod->getHalfLength());
    zValOrg.push_back(0);


    xValOrg.push_back(mod->getHalfMaxWidth());
    yValOrg.push_back(-mod->getHalfLength());
    zValOrg.push_back(0);

    xValOrg.push_back(0);
    yValOrg.push_back(-mod->getHalfLength());
    zValOrg.push_back(0);


    xValOrg.push_back(-mod->getHalfMaxWidth());
    yValOrg.push_back(-mod->getHalfLength());
    zValOrg.push_back(0);


    vector<float> xVal;
    vector<float> yVal;
    vector<float> zVal;

    for(unsigned int i = 0; i < xValOrg.size(); i++)
    {
        xVal.push_back(xValOrg.at(i) * mod->rot(0, 0) + yValOrg.at(i) * mod->rot(0, 1) + zValOrg.at(i) * mod->rot(0, 2));
        yVal.push_back(xValOrg.at(i) * mod->rot(1, 0) + yValOrg.at(i) * mod->rot(1, 1) + zValOrg.at(i) * mod->rot(1, 2));
        zVal.push_back(xValOrg.at(i) * mod->rot(2, 0) + yValOrg.at(i) * mod->rot(2, 1) + zValOrg.at(i) * mod->rot(2, 2));
    }



    // Translate
    for(auto& a: xVal) a += mod->getX();
    for(auto& a: yVal) a += mod->getY();
    for(auto& a: zVal) a += mod->getZ();


    vector<float> plotXVal;
    vector<float> plotYVal;
    if(plotType == PlotType::XY)
    {
        plotXVal =  xVal;
        plotYVal =  yVal;
    }
    
    if(plotType == PlotType::zRho)
    {
        plotXVal =  zVal;
    
        for(size_t i = 0; i < zVal.size(); i++)
        {
            plotYVal.push_back(sqrt(pow(xVal.at(i),2) + pow(yVal.at(i),2)));
        }
    }

    // close loop
    plotXVal.push_back(plotXVal.at(0));
    plotYVal.push_back(plotYVal.at(0));

    TPolyLine* pline = new TPolyLine(plotXVal.size(), &plotXVal[0], &plotYVal[0]);
    pline->SetFillColor(kGray+1);
    pline->SetLineColor(kGray + 1);
    pline->SetLineWidth(1);

    // pline->Print("v");

    // for(size_t i = 0; i < xVal.size(); i++)
    // {
    //     cout<<xVal.at(i)<<" "<<yVal.at(i)<<" "<<zVal.at(i)<<endl;
    // }

    return pline;
}

std::vector<TPolyLine*> HitGeoPlotter::getModuleVolumeHits(std::vector<std::shared_ptr<Hit>> hitList, PlotType plotType)
{
    std::vector<TPolyLine*>  modVolume;

    for(const auto& hit: hitList)
    {
        std::shared_ptr<DetectorModule> hitModule = NULL;
        // double distance =  std::numeric_limits<double>::infinity();

        for(const auto& mod: m_moduleList)
        {
            // double cDisc = hit->distanceTo(mod->getX(), mod->getY(), mod->getZ());
            // if(cDisc < distance)
            // {
            //     distance = cDisc;
            //     hitModule = mod;
            // }

            if(hit->isHitinModule(mod)) hitModule = mod;
        }


       if(hitModule)
       {
            hit->isHitinModule(hitModule);
            modVolume.push_back(getModuleVolume(hitModule, plotType));
       }
    }

    return modVolume;
}





