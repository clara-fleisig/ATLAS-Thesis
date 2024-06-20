// Class includes
#include "TrackNNEDM/ExtrapolatedTrack.h"

// Local includes
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/SeedTrack.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <climits>

using namespace std;


// Constructors and destructor
ExtrapolatedTrack::ExtrapolatedTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits, std::shared_ptr<SeedTrack> seed):
TrackBase(hitIndex)
{
    // Set the hits
    setOrgHits(hits);
    m_baseSeedTrack = seed;

    // Copy base class info to store the extrapolated predictions
    m_hitPredIndex = m_hitIndex;
    m_hitPredList = m_hitList;

}

ExtrapolatedTrack::~ExtrapolatedTrack()
{

}

std::shared_ptr<RecoTrack> ExtrapolatedTrack::getBaseRecoTrack() 
{
    return m_baseSeedTrack->getBaseRecoTrack(); 
}

std::shared_ptr<TrueTrack> ExtrapolatedTrack::getBaseTrueTrack() 
{
    return m_baseSeedTrack->getBaseTrueTrack(); 
}

void ExtrapolatedTrack::printDebug()
{
    std::cout<<"------- Reconstructed "<<std::endl;
    std::cout<<"hit index: ";
    for(const auto& hit: m_hitIndex)
    {
        std::cout<<hit<<" ";
    }
    std::cout<<" NN Score: "<<getNNScore()<<std::endl;
    std::cout<<std::endl;
    for(const auto& hit: this->getHitsList()) 
    {
        hit->printDebug();
    }
    // std::cout<<"------- Predicted "<<std::endl;
    // std::cout<<"hit index: ";
    // for(const auto& hit: m_hitPredIndex)
    // {
    //     std::cout<<hit<<" ";
    // }
    // std::cout<<std::endl;
    // for(const auto& hit: this->getPredictedHitsList()) hit->printDebug();
}

void ExtrapolatedTrack::addPredictedHit(std::shared_ptr<Hit> hit)
{
    this->resetCaches();
    m_hitPredIndex.push_back(hit->getIndex());
    m_hitPredList[hit->getIndex()] = hit;
}
void ExtrapolatedTrack::addRecoveredHit(std::shared_ptr<Hit> hit)
{
    this->resetCaches();
    m_hitIndex.push_back(hit->getIndex());
    m_hitList[hit->getIndex()] = hit;
}

std::vector<std::shared_ptr<Hit>> ExtrapolatedTrack::getPredictedHitsList()
{
    // Return the cached
    if(m_outPredList.size() > 0) return m_outPredList;

    // otherwise push it back into a list
    for(const auto& hitInfo: m_hitPredList) m_outPredList.push_back(hitInfo.second);

    static auto comp = [](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
    {
      return a->getRho() < b->getRho();
    };

    // So the hits line up
    std::sort (m_outPredList.begin(), m_outPredList.end(), comp);

    return m_outPredList;
}

std::vector<std::shared_ptr<Hit>> ExtrapolatedTrack::getPredictedHitsRotatedToZeroList()
{
    // Return the cached
    if(m_outPredRotatedList.size() > 0) return m_outPredRotatedList;
    
    // Get the normal hit list
    auto orgHitList = getPredictedHitsList();
    if(orgHitList.size() == 0) return m_outPredRotatedList;

    fillRotatedHits(orgHitList, m_outPredRotatedList);
    return m_outPredRotatedList;
}


void ExtrapolatedTrack::resetCaches()
{
    // Call the base class clearing of cache
    TrackBase::resetCaches();
    m_outPredList.clear();
    m_outPredRotatedList.clear();

}

float ExtrapolatedTrack::fracTrueHits()
{
    auto trackSeed = this->getBaseSeedTrack();
    int truthIndex = trackSeed->getHitsList().at(0)->getRecoTrackIndex();
    //std::cout<<"truth index: " << truthIndex<<std::endl;
    auto hitList = this->getHitsList();
    float numerator = 0;
    for (auto& hit: hitList)
    {
        //hit->printDebug();
        int idx = hit->getRecoTrackIndex();
        //std::cout<<"found index: " << idx << std::endl;
        if (idx == truthIndex) numerator+=1;
    }

    return numerator / this->getNHits();
}

Index ExtrapolatedTrack::isMatchedToTrack(float threshold)
{  
    auto seedSize = this->getBaseSeedTrack()->getHitsList().size();
    auto hitList = this->getHitsList();
    std::map<Index, int> trackFrequency; //store the frequency of the hits from unique true tracks in an extrap track
    for (size_t i=seedSize; i < hitList.size(); i++)
    {
        auto hit = hitList.at(i);
        Index idx = hit->getTruthTrackBarcode().at(0);
        trackFrequency[idx]++;
    }
    float max=0;
    Index trkBarcode=0;
    for (auto item: trackFrequency)
    {
        if(item.second > max)
        {
            max = item.second;
            trkBarcode = item.first;
        }
    }
    
    float truthFraction  = max / (this->getNHits() - seedSize);
    
    if (truthFraction > threshold) return trkBarcode;
    else return 0;
}

bool ExtrapolatedTrack::isMatchedToReco(Index recoIndex, float threshold, int seedSize)
{  
    auto hitList = this->getHitsList();
    float counter = 0;
    for (size_t i=seedSize; i < hitList.size(); i++)
    {
        auto hit = hitList.at(i);
        Index idx = hit->getTruthTrackBarcode().at(0);
        if (idx == recoIndex) counter += 1 ;
    }
    float truthFraction  = counter / (this->getNHits() - seedSize);
    
    if (truthFraction > threshold) return true;
    else return false;
}

bool ExtrapolatedTrack::isMatchedToRecoHits(Index recoIndex, int numMatchingHits)
{   
    float counter = 0;
    auto hitList = this->getHitsList();
    for (auto& hit: hitList)
    {
        Index idx = hit->getRecoTrackIndex();
        if (idx == recoIndex) counter += 1 ;
    }
    
    if (counter >= numMatchingHits) return true;
    else return false;
}

bool ExtrapolatedTrack::isMatched(float fraction)
{
    auto hitsList = getHitsList();
    float nHits = hitsList.size();
    for (size_t i=0; i<nHits; i++)
    {
        float tempCount = 0;
        auto iBarcode = hitsList.at(i)->getTruthTrackBarcode().at(0);
        for (size_t j=0; j<nHits; j++)
        {
            auto jBarcode = hitsList.at(j)->getTruthTrackBarcode().at(0);
            if (  jBarcode == iBarcode)
            {
                tempCount++;
            } 
        }
        //std::cout<<tempCount/nHits<<std::endl;
        if (tempCount/nHits > fraction){return true;}
    }

    return false;
}

double ExtrapolatedTrack::getComputedTrackProbability()
{
    if(m_computedTrackProb > 0) return m_computedTrackProb;

    m_computedTrackProb = 0;
    m_totalWeight = 0;
    auto hitList = this->getHitsList();

    // Count how many times a truth track contributes
    for(const auto& hit: hitList)
    {
        auto truthIndices = hit->getTruthTrackBarcode();
        int weight = 1;
        if(hit->getPixorSct() == 0) weight = 2;

        for(auto& index: truthIndices)
        {
            // Treat -1 as fakes
            if(index == ULLONG_MAX) continue;
            m_truthBarcodeCountMap[index] += weight;
        }
        m_totalWeight += weight;
    }

    // get the highest probability
    for(const auto& truthInfo: m_truthBarcodeCountMap)
    {
        if(truthInfo.first == ULLONG_MAX) continue;
        double cProb = truthInfo.second/m_totalWeight;
        if(cProb > m_computedTrackProb) m_computedTrackProb = cProb;
    }

    return m_computedTrackProb;
}
