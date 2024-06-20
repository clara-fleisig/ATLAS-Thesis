// Class includes
#include "TrackNNEDM/FakeTrack.h"

// Local includes
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>

using namespace std;


// Constructors and destructor
FakeTrack::FakeTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits): 
TrackBase(hitIndex)
{
    // Set the hits
    setOrgHits(hits);
    m_prob = -999;
}

FakeTrack::~FakeTrack()
{

} 

void FakeTrack::printDebug()
{
    for(const auto& hit: this->getHitsList()) hit->printDebug();
}



double FakeTrack::getTrackProbability()
{
    // If >= 0, the calculation has happened already
    if(m_prob >= 0) return m_prob;
    m_prob = 0;
    auto hitList = this->getHitsList();

    // Count how many times a truth track contributes
    std::map<int, float> countMap;
    countMap[-1] = 0;
    for(const auto& hit: hitList)
    {
        auto truthIndices = hit->getTruthTrackBarcode();
        int weight = 1;
        if(hit->getPixorSct() == 0) weight = 2;

        for(auto& index: truthIndices)
        {
            // we have changed index to unsigned int, and sometimes change it back to int.
            // -1 in both case is treated as a fake 
            static auto fakeIndex = static_cast<Index> (-1);
            // Treat -1 as fakes
            if(index == fakeIndex) continue;
            countMap[index] += weight;
        }
        countMap[-1] += weight;
    }

    // get the highest probability
    for(const auto& truthInfo: countMap)
    {
        if(truthInfo.first < 0) continue;
        double cProb = truthInfo.second/countMap[-1];
        if(cProb > m_prob) m_prob = cProb;
    }

    return m_prob;


}

