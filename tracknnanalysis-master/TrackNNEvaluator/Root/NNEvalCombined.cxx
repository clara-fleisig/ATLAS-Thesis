#include "TrackNNEvaluator/NNEvalCombined.h"

NNEvalCombined::NNEvalCombined(TString classifierFilename, TString extrapFilename, int hits)
{
    m_classifierFilename = classifierFilename;
    m_extrapFilename = extrapFilename;
    n_hits = hits;
}

NNEvalCombined::~NNEvalCombined()
{

}

void NNEvalCombined::initialize()
{
    m_classifier = std::make_shared<NNEvalClassUtils>(m_classifierFilename, n_hits);
    m_extrapolator = std::make_shared<NNEvalExtrapUtils>(m_extrapFilename, n_hits);

    m_classifier->initialize();
    std::cout<<"Initialized Classifier"<<"\n"<<std::endl;
    m_extrapolator->initialize();
    std::cout<<"Initialized Extrapolator"<<"\n"<<std::endl;
}

std::pair< std::shared_ptr<Hit>, std::vector<int> >  NNEvalCombined::Extrap(std::vector<std::shared_ptr<Hit>> hitList, bool debug)
{
    std::pair< std::shared_ptr<Hit>, std::vector<int> > output;
    if (debug) std::cout<<"==============\n"<<"Classification\n"<<"=============="<<std::endl;
    // Classify detector volume and layer
    auto predDetector = m_classifier->classify(hitList, debug);
    if (debug) std::cout<<"=================\n"<<"Extrap\n"<<"================="<<std::endl;
    // Extrapolator only uses hit coordinates and next hit detector info
    auto outputHit = m_extrapolator->extrapSeed(hitList, predDetector, debug);
    output = std::make_pair(outputHit, predDetector);
    return output;
}

std::vector<std::pair<std::shared_ptr<Hit>, std::vector<int>>> NNEvalCombined::ExtrapMultiple(std::vector<std::shared_ptr<Hit>> hitList, float threshold, bool debug)
{
    // Classify multiple detector vectors if the classification predictions are over a given threshold
    // Extrapolate a hit using each OHE vector
    std::vector< std::pair< std::shared_ptr<Hit>, std::vector<int> >>  outputs;
    if (debug) std::cout<<"==============\n"<<"Classification\n"<<"=============="<<std::endl;
    // Classify detector volume and layer
    auto predDetector = m_classifier->classify(hitList, threshold, debug);
    if (debug) std::cout<<"=================\n"<<"Extrap\n"<<"================="<<std::endl;
    // Extrapolator only uses hit coordinates and next hit detector info
    for (auto& vec: predDetector)
    {
        auto outputHit = m_extrapolator->extrapSeed(hitList, vec, debug);
        auto pair = std::make_pair(outputHit, vec);
        outputs.push_back(pair);
    }
    return outputs;
}

std::shared_ptr<Hit> NNEvalCombined::ClassifyAndExtrapolate(std::vector<std::shared_ptr<Hit>> hitList, bool debug)
{
    // For a model that was trained to classify and extraplate at the same time
    auto outputHit = m_extrapolator->ClassifyAndExtrapolate(hitList, debug);

    return outputHit;
}


