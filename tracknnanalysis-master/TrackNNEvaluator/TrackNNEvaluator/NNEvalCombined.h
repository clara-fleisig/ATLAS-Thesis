#ifndef _NNEVALCOMBINED_H
#define _NNEVALCOMBINED_H

#include "TrackNNEvaluator/NNEvalExtrapUtils.h"
#include "TrackNNEvaluator/NNEvalClassUtils.h"

class NNEvalCombined
{
    public:
        NNEvalCombined(TString classifierFilename, TString extrapFilename, int hits);
        virtual ~NNEvalCombined();

        void initialize();
        std::pair< std::shared_ptr<Hit>, std::vector<int>> Extrap(std::vector<std::shared_ptr<Hit>> hitList, bool debug);
        std::shared_ptr<Hit> ClassifyAndExtrapolate(std::vector<std::shared_ptr<Hit>> hitList, bool debug);
        std::vector<std::pair<std::shared_ptr<Hit>, std::vector<int>>> ExtrapMultiple(std::vector<std::shared_ptr<Hit>> hitList, float threshold, bool debug);


    private:
        TString m_classifierFilename;
        TString m_extrapFilename;
        int n_hits;

        std::shared_ptr<NNEvalClassUtils> m_classifier;
        std::shared_ptr<NNEvalExtrapUtils> m_extrapolator;
        



};
#endif