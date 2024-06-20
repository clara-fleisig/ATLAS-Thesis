// Class includes
#include "TrackNNEvaluator/NNEvalLwtnnUtils.h"

// Local includes
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <math.h>

using namespace std;


// Constructors and destructor
NNEvalLwtnnUtils::NNEvalLwtnnUtils(TString fileName)
{
    m_fileName = fileName;
}

NNEvalLwtnnUtils::~NNEvalLwtnnUtils()
{

} 


// big setters
void NNEvalLwtnnUtils::initialize()
{
    std::ifstream input_cfg(m_fileName.Data());
    auto cfg = lwt::parse_json_graph( input_cfg );
    m_lwnn = std::shared_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph( cfg ));
}



void NNEvalLwtnnUtils::evalTrack(std::shared_ptr<RecoTrack> trk)
{
    evalTrack(dynamic_pointer_cast<TrackBase, RecoTrack>(trk));
}
void NNEvalLwtnnUtils::evalTrack(std::shared_ptr<FakeTrack> trk)
{
    evalTrack(dynamic_pointer_cast<TrackBase, FakeTrack>(trk));
}
void NNEvalLwtnnUtils::evalTrack(std::shared_ptr<TrackBase> trk)
{
    // Input maps for lwtnn
    std::map<std::string, std::map<std::string, double>> valMap;
    std::map<std::string, std::map<std::string, std::vector<double>>> vectorMap;

    // Initalize to default
    for(int i = 1; i <= 8; i++)
    {
        TString indexStr = Form("%d", i);
        valMap["dNN"][("hitX" + indexStr).Data()] = 0;
        valMap["dNN"][("hitY" + indexStr).Data()] = 0;
        valMap["dNN"][("hitZ" + indexStr).Data()] = 0;
    }

    int index = 1;
    for(const auto& hit: trk->getHitsRotatedToZeroList())
    {
        TString indexStr = Form("%d", index);
        valMap["dNN"][("hitX" + indexStr).Data()] = hit->getX()/Hit::getXScale();
        valMap["dNN"][("hitY" + indexStr).Data()] = hit->getY()/Hit::getYScale();
        valMap["dNN"][("hitZ" + indexStr).Data()] = hit->getZ()/Hit::getZScale();
        index++;
    }

    for(const auto& var: valMap)
    {
        for(const auto& val: var.second)
        {
            cout<<var.first<<" "<<val.first<<" "<<val.second<<endl;
        }
    }

    auto scoreMap = m_lwnn->compute(valMap, vectorMap);

    std::cout<<"NN score: "<<scoreMap.at("NNScore")<<std::endl;
    trk->setNNScore(scoreMap.at("NNScore"));   
}












