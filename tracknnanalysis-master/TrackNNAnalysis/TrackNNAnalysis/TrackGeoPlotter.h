#ifndef _TRACKGEOPLOTTER_H
#define _TRACKGEOPLOTTER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"


// c++ includes
#include <map>
#include <vector>
#include <memory>

// Root includes
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TPolyLine.h>

#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/FakeTrack.h"
#include "TrackNNEDM/ExtrapolatedTrack.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/DetectorModule.h"

// Base class 
class TrackGeoPlotter 
{
   
    public:
        TrackGeoPlotter(std::vector<std::shared_ptr<DetectorModule>> dectorModule);
        virtual ~TrackGeoPlotter();   

        void initialize();

        // void evalTrack(std::shared_ptr<RecoTrack> trk);
        // void evalTrack(std::shared_ptr<FakeTrack> trk);
        // void evalTrack(std::shared_ptr<TrackBase> trk);
        void plotTracks(std::map<TString, std::shared_ptr<TrackBase>> trkList, TString baseName);
        void plotExtrapolatedTracks(std::vector<std::shared_ptr<ExtrapolatedTrack>> goodTrkList, std::vector<std::shared_ptr<ExtrapolatedTrack>> badTrkList, TString baseName);

    protected:
        std::vector<std::shared_ptr<DetectorModule>> m_moduleList;
        std::vector<TGraph*> m_dectortorXY;
        std::vector<TGraph*> m_dectortorZrho;



        enum class PlotType 
        { 
            XY,
            zRho
        };

        void plotTrack(TString baseName, std::vector<TGraph*> detector, std::vector<TPolyLine*> detectorVol, std::map<TString, TGraph*> trackList, PlotType plotType);
        void plotTrack(TString baseName, std::vector<TGraph*> detectorList, std::vector<TPolyLine*> detectorVol, std::vector<TMultiGraph*> goodTrackList, std::vector<TMultiGraph*> badTrackList, PlotType plotType);
        TGraph* getPlotTrack(std::shared_ptr<TrackBase> trk, PlotType plotType);
        TMultiGraph* getPlotTrack(std::shared_ptr<ExtrapolatedTrack> trk, PlotType plotType, int color);
        TGraph* getPlotTrackModule(std::shared_ptr<TrackBase> trk, PlotType plotType);
        TPolyLine* getModuleVolume(std::shared_ptr<DetectorModule> mod, PlotType plotType);
        std::vector<TPolyLine*> getModuleVolumeTrk(std::shared_ptr<TrackBase> trk, PlotType plotType);

};


#endif
