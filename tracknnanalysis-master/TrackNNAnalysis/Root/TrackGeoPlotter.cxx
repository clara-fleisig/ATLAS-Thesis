// Class includes
#include "TrackNNAnalysis/TrackGeoPlotter.h"
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
#include <TLegend.h>
#include <TRotation.h>

using namespace std;


// Constructors and destructor
TrackGeoPlotter::TrackGeoPlotter(std::vector<std::shared_ptr<DetectorModule>> dectorModule)
{
    m_moduleList = dectorModule;
}

TrackGeoPlotter::~TrackGeoPlotter()
{

} 


// big setters
void TrackGeoPlotter::initialize()
{
    SetAtlasStyle();
    // Get all the dector points
    vector<float> _x;
    vector<float> _y;
    vector<float> _z;
    vector<float> _rho;

    for(const auto& mod: m_moduleList)
    {
        if(mod->getVolume() != 9 && mod->getVolume() != 16 && mod->getVolume() != 23) continue;
        _x.push_back(mod->getX());
        _y.push_back(mod->getY());
        _z.push_back(mod->getZ());
        _rho.push_back(mod->getRho());
    }

    TGraph* xy = new TGraph(_x.size(), &_x[0], &_y[0]);
    xy->SetMarkerSize(0.0001);
    xy->SetMarkerColor(kGray + 2);
    m_dectortorXY.push_back(xy);

    TGraph* zRho = new TGraph(_z.size(), &_z[0], &_rho[0]);
    zRho->SetMarkerSize(0.2);
    zRho->SetMarkerColor(kGray + 2);
    m_dectortorZrho.push_back(zRho);

}


void TrackGeoPlotter::plotTracks(map<TString, std::shared_ptr<TrackBase>> trkList, TString baseName)
{
    map<TString, TGraph*> trackXY;
    map<TString, TGraph*> trackzRho;

    vector<TPolyLine*> ployXY;
    vector<TPolyLine*> ployzRho;

    for(auto& trk: trkList)
    {
        trackXY[trk.first]=getPlotTrack(trk.second, PlotType::XY);
        trackzRho[trk.first]=getPlotTrack(trk.second, PlotType::zRho);

        // trackXY.push_back(getPlotTrackModule(trk, PlotType::XY));
        // trackzRho.push_back(getPlotTrackModule(trk, PlotType::zRho));
        auto modVol = getModuleVolumeTrk(trk.second, PlotType::XY);
        ployXY.insert(ployXY.end(), modVol.begin(), modVol.end());

        modVol = getModuleVolumeTrk(trk.second, PlotType::zRho);
        ployzRho.insert(ployzRho.end(), modVol.begin(), modVol.end());
    }

    plotTrack(baseName + "_XY.pdf", m_dectortorXY, ployXY, trackXY, PlotType::XY);
    plotTrack(baseName + "_zRho.pdf", m_dectortorZrho, ployzRho, trackzRho, PlotType::zRho);
}

void TrackGeoPlotter::plotExtrapolatedTracks(vector<std::shared_ptr<ExtrapolatedTrack>> goodTrkList, vector<std::shared_ptr<ExtrapolatedTrack>> badTrkList, TString baseName)
{
    vector<TMultiGraph*> goodTrackXY;
    vector<TMultiGraph*> goodTrackzRho;
    vector<TMultiGraph*> badTrackXY;
    vector<TMultiGraph*> badTrackzRho;

    vector<TPolyLine*> ployXY;
    vector<TPolyLine*> ployzRho;

    for(auto& trk: goodTrkList)
    {
        goodTrackXY.push_back(getPlotTrack(trk, PlotType::XY,1));
        goodTrackzRho.push_back(getPlotTrack(trk, PlotType::zRho,1));

        // trackXY.push_back(getPlotTrackModule(trk, PlotType::XY));
        // trackzRho.push_back(getPlotTrackModule(trk, PlotType::zRho));
        auto modVol = getModuleVolumeTrk(trk, PlotType::XY);
        ployXY.insert(ployXY.end(), modVol.begin(), modVol.end());

        modVol = getModuleVolumeTrk(trk, PlotType::zRho);
        ployzRho.insert(ployzRho.end(), modVol.begin(), modVol.end());
    }

    for(auto& trk: badTrkList)
    {
        badTrackXY.push_back(getPlotTrack(trk, PlotType::XY,4));
        badTrackzRho.push_back(getPlotTrack(trk, PlotType::zRho,4));

        // trackXY.push_back(getPlotTrackModule(trk, PlotType::XY));
        // trackzRho.push_back(getPlotTrackModule(trk, PlotType::zRho));
        auto modVol = getModuleVolumeTrk(trk, PlotType::XY);
        ployXY.insert(ployXY.end(), modVol.begin(), modVol.end());

        modVol = getModuleVolumeTrk(trk, PlotType::zRho);
        ployzRho.insert(ployzRho.end(), modVol.begin(), modVol.end());
    }

    plotTrack(baseName + "_XY.eps", m_dectortorXY, ployXY, goodTrackXY, badTrackXY,PlotType::XY);
    plotTrack(baseName + "_zRho.eps", m_dectortorZrho, ployzRho, goodTrackzRho, badTrackzRho, PlotType::zRho);
}

void TrackGeoPlotter::plotTrack(TString baseName, std::vector<TGraph*> detectorList, std::vector<TPolyLine*> detectorVol, std::map<TString, TGraph*> trackList, PlotType plotType)
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
    // TLegend* elLeg = new TLegend(.2,.6,.4,.7);

    if(plotType == PlotType::XY)
    {
        frame->GetXaxis()->SetTitle("x [mm]");
        frame->GetYaxis()->SetTitle("y [mm]");
    }
    if(plotType == PlotType::zRho)
    {
        frame->GetXaxis()->SetTitle("z [mm]");
        frame->GetYaxis()->SetTitle("rho [mm]");
    }
    frame->Draw();
    for(auto gr:detectorVol) gr->Draw("f");
    for(auto gr:detectorVol) gr->Draw();
    for(auto gr:detectorList) gr->Draw("p");
    int colorCounter = 1;
    for(auto gr:trackList)
    {
        gr.second->SetLineColor(colorCounter);
        gr.second->SetMarkerColor(colorCounter);
        gr.second->Draw("pl");
        // elLeg->AddEntry(gr.second, gr.first);
        colorCounter++;
    }
     
    // elLeg->Draw();


    //float leftDist = 0.19;
    //ATLASLabel(leftDist, 0.875, "Internal", 1);
    c->Update();
    c->SaveAs(baseName);

    delete c;

}

// This should be used for only one extrapolated seed at a time
// trackList = a vector of all the extrapolated tracks from an initial seedstd::vector<TMultiGraph*> trackList
// otherwise the true tracks will not be plotted correctly
void TrackGeoPlotter::plotTrack(TString baseName, std::vector<TGraph*> detectorList, std::vector<TPolyLine*> detectorVol, std::vector<TMultiGraph*> goodTrackList,std::vector<TMultiGraph*> badTrackList, PlotType plotType)
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

    TH2F* frame = new TH2F("frame", "frame", 1, -1100, 100, 1, -100, 1100 );
    // TH2F* frame = new TH2F("frame", "frame", 1, minRangeX, maxRangeX, 1, minRangeY, maxRangeY );

    TCanvas* c = new TCanvas("", "", 0, 0, 600, 600);

    TLegend* legend = NULL;
    if(plotType == PlotType::XY)
    {
        frame->GetXaxis()->SetTitle("x [mm]");
        frame->GetYaxis()->SetTitle("y [mm]");
        legend = new TLegend(0.3 ,0.2,0.9,0.3);
    }
    if(plotType == PlotType::zRho)
    {
        frame->GetXaxis()->SetTitle("z [mm]");
        frame->GetYaxis()->SetTitle("#rho [mm]");
        legend = new TLegend(0.3 ,0.8,0.9,0.9);
    }
    frame->GetXaxis()->SetLabelSize(.03);
    frame->GetYaxis()->SetLabelSize(.03);
    frame->Draw();
    for(auto gr:detectorVol) gr->Draw("f");
    for(auto gr:detectorVol) gr->Draw();
    for(auto gr:detectorList) gr->Draw("p");
    for(auto gr:goodTrackList)
    {
        gr->Draw("pl");
    }
    for(auto& gr:badTrackList)
    {
        gr->Draw("pl");
    }

    legend->SetBorderSize(0);
    legend->SetTextFont(42);
    legend->SetTextSize(.03);
    legend->AddEntry(goodTrackList.at(0)->GetListOfGraphs()->At(0), "Extrap. Matched Track");
    //legend->AddEntry(badTrackList.at(0)->GetListOfGraphs()->At(0), "Extrap. Unmatched Track");
    //legend->AddEntry(badTrackList.at(0)->GetListOfGraphs()->At(1), "Unmatched Corresponding True Track");
    // legend->Draw();

    //float leftDist = 0.19;
    //ATLASLabel(leftDist, 0.875, "Internal", 1);
    c->SaveAs(baseName);

    delete c;

}

TGraph* TrackGeoPlotter::getPlotTrack(std::shared_ptr<TrackBase> trk, PlotType plotType)
{
    // Get all the dector points
    vector<float> _x;
    vector<float> _y;


    for(const auto& hit: trk->getHitsList())
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
    gr->SetMarkerSize(0.25);
    gr->SetMarkerColor(kRed);
    gr->SetLineColor(kRed);

    return gr;
}

TMultiGraph* TrackGeoPlotter::getPlotTrack(std::shared_ptr<ExtrapolatedTrack> trk, PlotType plotType, int /*color*/)
{
    // Get all the dector points
    vector<float> _x;
    vector<float> _y;

    vector<float> _xPred;
    vector<float> _yPred;

    vector<float> _xTrue;
    vector<float> _yTrue;


    for(const auto& hit: trk->getHitsList())
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
    
    for(const auto& hit: trk->getPredictedHitsList())
    {
        std::shared_ptr<DetectorModule> hitModule = NULL;

        if(plotType == PlotType::XY)
        {
            _xPred.push_back(hit->getX());
            _yPred.push_back(hit->getY());
        }
        if(plotType == PlotType::zRho)
        {
            _xPred.push_back(hit->getZ());
            _yPred.push_back(hit->getRho());
        }
    }

    auto trueTrack = trk->getBaseTrueTrack();
    for(const auto& hit: trueTrack->getHitsList())
    {
        std::shared_ptr<DetectorModule> hitModule = NULL;

        if(plotType == PlotType::XY)
        {
            _xTrue.push_back(hit->getX());
            _yTrue.push_back(hit->getY());
        }
        if(plotType == PlotType::zRho)
        {
            _xTrue.push_back(hit->getZ());
            _yTrue.push_back(hit->getRho());
        }
    }

    // TGraph* gr1 = new TGraph(_x.size(), &_x[0], &_y[0]);
    // gr1->SetMarkerSize(0.5);
    // gr1->SetMarkerColor(color);
    // gr1->SetLineColor(color);
    // gr1->SetName("extrapolated");

    TGraph* gr2 = new TGraph(_xPred.size(), &_xPred[0], &_yPred[0]);
    gr2->SetMarkerSize(0.5);
    gr2->SetMarkerColor(kRed);
    gr2->SetMarkerStyle(4);
    // gr2->SetLineColorAlpha(kBlue, 1);
    gr2->SetLineStyle(3);
    gr2->SetLineWidth(.001);
    gr2->SetLineColor(kWhite);
    gr2->SetName("predicted");

    TGraph* gr3 = new TGraph(_xTrue.size(), &_xTrue[0], &_yTrue[0]);
    gr3->SetMarkerSize(0.01);
    gr3->SetMarkerColor(kBlue);
    gr3->SetLineColor(kBlue);
    gr3->SetLineStyle(1);
    gr3->SetName("true");



    TMultiGraph* mgr = new TMultiGraph();
    // mgr->Add(gr1);
    mgr->Add(gr3);
    mgr->Add(gr2);
    return mgr;
}


TGraph* TrackGeoPlotter::getPlotTrackModule(std::shared_ptr<TrackBase> trk, PlotType plotType)
{
    // Get all the dector points
    vector<float> _modx;
    vector<float> _mody;

    for(const auto& hit: trk->getHitsList())
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

TPolyLine* TrackGeoPlotter::getModuleVolume(std::shared_ptr<DetectorModule> mod, PlotType plotType)
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
    pline->SetFillColor(kGray);
    pline->SetLineColor(kGray + 1);
    pline->SetLineWidth(1);

    // pline->Print("v");

    // for(size_t i = 0; i < xVal.size(); i++)
    // {
    //     cout<<xVal.at(i)<<" "<<yVal.at(i)<<" "<<zVal.at(i)<<endl;
    // }

    return pline;
}

std::vector<TPolyLine*> TrackGeoPlotter::getModuleVolumeTrk(std::shared_ptr<TrackBase> trk, PlotType plotType)
{
    std::vector<TPolyLine*>  modVolume;

    // Do only modules from track
    // for(const auto& hit: trk->getHitsList())
    // {
    //     std::shared_ptr<DetectorModule> hitModule = NULL;
    //     // double distance =  std::numeric_limits<double>::infinity();

    //     for(const auto& mod: m_moduleList)
    //     {
    //         // double cDisc = hit->distanceTo(mod->getX(), mod->getY(), mod->getZ());
    //         // if(cDisc < distance)
    //         // {
    //         //     distance = cDisc;
    //         //     hitModule = mod;
    //         // }

    //         if(hit->isHitinModule(mod)) hitModule = mod;
    //     }


    //    if(hitModule)
    //    {
    //         hit->isHitinModule(hitModule);
    //         modVolume.push_back(getModuleVolume(hitModule, plotType));
    //    }
    // }

    // If doing all modules
    for(const auto& mod: m_moduleList)
    {
        if(mod->getVolume() != 9 && mod->getVolume() != 16 && mod->getVolume() != 23) continue;
        // If it belongs to the track color it different
        auto modVol = getModuleVolume(mod, plotType);

        for(const auto& hit: trk->getHitsList())
        {
            if(hit->isHitinModule(mod))
            {
                modVol->SetLineColor(kYellow+1);
                modVol->SetFillColor(kYellow+1);
                
            }
        }
        modVolume.push_back(modVol);
        
    }

    return modVolume;
}





