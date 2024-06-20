#ifndef _TRACKINGMETRICSTOOL_H
#define _TRACKINGMETRICSTOOL_H

#include "TrackNNAnalysis/PlotHolder.h"
#include "TrackNNEDM/TrackTree.h"

/* Class to store tools for efficiency calculation and fake track rates, etc. */

class TrackingMetricsTool
{
    public:
        plotHolderACTSExtrapolation* m_plots;

    public:
        TrackingMetricsTool(){};
         ~TrackingMetricsTool(){};

        void efficiency();
        void fakeTrackRate();
        void setKey();
        std::vector<std::shared_ptr<FinalTrackTreeNode>> HitWarrior(std::vector<std::shared_ptr<FinalTrackTreeNode>> finalHits, float nnCut);
        std::vector<std::shared_ptr<FinalTrackTreeNode>> SelectNNTracks(std::vector<TrackTree*> trackTrees, float nnCut);
        std::vector<std::shared_ptr<FinalTrackTreeNode>> TDRAlgo(std::vector<TrackTree*> trackTrees, float nnCut);
        void efficiency(std::vector<std::shared_ptr<TrackTree*>> trackTrees, std::vector<std::shared_ptr<FinalTrackTreeNode>> finalHits);

};

#endif
