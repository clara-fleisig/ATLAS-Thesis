// Class includes
#include "TrackNNEDM/RecoTrack.h"

// Local includes
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <climits>
using namespace std;


// Constructors and destructor
RecoTrack::RecoTrack(Index index, 
            float pt, 
            float eta, 
            float phi, 
            float truthProb,
            float track_d0, 
            float track_z0,
            float track_phi,
            float track_theta, 
            float track_qoverp,
            int charge, 
            int nPixHits, 
            int nSCTHits, 
            Index truthBarcode,
            std::vector<Index> hitIndex): TrackBase(hitIndex),
            m_index(index),
            m_pt(pt),
            m_eta(eta),
            m_phi(phi),
            m_truthProb(truthProb),
            m_track_d0(track_d0),
            m_track_z0(track_z0),
            m_track_phi(track_phi),
            m_track_theta(track_theta),
            m_track_qoverp(track_qoverp),
            m_charge(charge),
            m_nPixHits(nPixHits),
            m_nSCTHits(nSCTHits),
            m_truthBarcode(truthBarcode),
            m_trueTrack(nullptr),
            m_computedTrackProb(-1),
            m_totalWeight(-1)
{

}

// Constructors and destructor
RecoTrack::RecoTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits):
TrackBase(hitIndex)
{
    // Set the hits
    setOrgHits(hits);
}


RecoTrack::~RecoTrack()
{

} 


// big setters
void RecoTrack::setOrgTrueTrack(std::shared_ptr<TrueTrack> track)
{
    if(track->getBarcode() != m_truthBarcode)
    {
        cout<<"RecoTrack: wrong reco track being associated."<<endl;
        cout<<"Input track "<<track->getBarcode()<<endl;
        cout<<"Hit matched track "<<m_truthBarcode<<endl;
        exit(1);
    }
    m_trueTrack = track;
}

void RecoTrack::printDebug()
{
    std::cout<<"RecoTrack index: "<<m_index<<" pt: "<<m_pt<<" eta: "<<m_eta<<" phi: "<<m_phi<<" prob: "<<m_truthProb<<" nHits: "<<m_hitIndex.size()<<" truth: "<<m_truthBarcode<<" NN Score: "<<getNNScore()<<" computed prob: "<<getComputedTrackProbability()<<endl;
    for(const auto& var: m_intMetadata) std::cout<<var.first<<": "<<var.second<<" ";
    std::cout<<endl;
    
    std::cout<<"hit index: ";
    for(const auto& hit: m_hitIndex)
    {
        std::cout<<hit<<" ";
    }
    std::cout<<std::endl;
    for(const auto& hit: m_hitList)
    {
        hit.second->printDebug();
    }

    std::cout<<"---------- sp --------------"<<std::endl;
    for(const auto& hit: getSPHitsList())
    {
        hit->printDebug();
    }

}

double RecoTrack::getTrackProbability()
{
    return m_truthProb;
}
double RecoTrack::getComputedTrackProbability()
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

double RecoTrack::getMatchedProbability(Index barcode)
{
    // Trigger the calc
    if(m_totalWeight < 0) getComputedTrackProbability();

    if(m_truthBarcodeCountMap.find(barcode) == m_truthBarcodeCountMap.end()) return 0;

    return m_truthBarcodeCountMap.at(barcode)/m_totalWeight;
}


bool RecoTrack::isMatched(Index barcode, double threshold)
{
    // Trigger the calc
    if(m_totalWeight < 0) getComputedTrackProbability();

    if(m_truthBarcodeCountMap.find(barcode) == m_truthBarcodeCountMap.end()) return false;

    // if((m_hitList.size() == 7) && m_truthBarcodeCountMap.at(barcode) >= 5) return true;
    // if((m_hitList.size() == 8) && m_truthBarcodeCountMap.at(barcode) >= 5) return true;
    return (getMatchedProbability(barcode) > threshold);
}











