#ifndef _HITGEOPLOTTER_H
#define _HITGEOPLOTTER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"


// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes
#include <TGraph.h>
#include <TPolyLine.h>

#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/DetectorModule.h"

// Base class 
class HitGeoPlotter 
{
   
    public:
        HitGeoPlotter(std::vector<std::shared_ptr<DetectorModule>> dectorModule);
        virtual ~HitGeoPlotter();   

        void initialize();

        // void evalTrack(std::shared_ptr<RecoTrack> trk);
        // void evalTrack(std::shared_ptr<FakeTrack> trk);
        // void evalTrack(std::shared_ptr<TrackBase> trk);
        void plotHitsList(std::vector<std::shared_ptr<Hit>> hitList, TString baseName, bool barrelOnly=true);
        void plotTrueAndPredictedHitsList(std::vector<std::shared_ptr<Hit>> trueHitList,std::vector<std::shared_ptr<Hit>> predHitList, TString baseName, bool barrelOnly=true);



    protected:
        std::vector<std::shared_ptr<DetectorModule>> m_moduleList;
        std::vector<TGraph*> m_dectortorXY;
        std::vector<TGraph*> m_dectortorZrho;



        enum class PlotType 
        { 
            XY,
            zRho
        };

        void plotHits(TString baseName, std::vector<TGraph*> detector, std::vector<TPolyLine*> detectorVol, TGraph* hitsList, PlotType plotType);
        void plotTrueAndPredictedHits(TString baseName, std::vector<TGraph*> detectorList, std::vector<TPolyLine*> detectorVol, TGraph* trueHitGraph, TGraph* predHitGraph, PlotType plotType);
        TGraph* getPlotHits(std::vector<std::shared_ptr<Hit>> hitList, PlotType plotType);
        TGraph* getPlotHitsModule(std::vector<std::shared_ptr<Hit>> hitList, PlotType plotType);
        TPolyLine* getModuleVolume(std::shared_ptr<DetectorModule> mod, PlotType plotType);
        std::vector<TPolyLine*> getModuleVolumeHits(std::vector<std::shared_ptr<Hit>> hitList, PlotType plotType);

};


#endif
