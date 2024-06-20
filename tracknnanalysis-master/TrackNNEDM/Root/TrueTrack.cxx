// Class includes
#include "TrackNNEDM/TrueTrack.h"

// Local includes
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/Hit.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>

using namespace std;


// Constructors and destructor
TrueTrack::TrueTrack(Index barcode, 
            float pt, 
            float eta, 
            float phi, 
            float vertexDistance,
            int pdgid, 
            int status, 
            Index recoTrackIndex,
            std::vector<Index> hitIndex): TrackBase(hitIndex),
            m_barcode(barcode),
            m_pt(pt),
            m_eta(eta),
            m_phi(phi),
            m_vertexDistance(vertexDistance),
            m_z0(0),
            m_d0(0),
            m_pdgid(pdgid),
            m_status(status),
            m_recoTrackIndex(recoTrackIndex),
            m_recoTrack(nullptr),
            m_computedTrackProb(1),
            m_trackProb(1)
{

}
TrueTrack::TrueTrack(Index barcode, 
            float pt, 
            float eta, 
            float phi, 
            float vertexDistance,
            int pdgid, 
            int status, 
            Index recoTrackIndex,
            std::vector<Index> hitIndex,
            std::vector<std::shared_ptr<Hit>> hits): TrackBase(hitIndex),
            m_barcode(barcode),
            m_pt(pt),
            m_eta(eta),
            m_phi(phi),
            m_vertexDistance(vertexDistance),
            m_z0(0),
            m_d0(0),
            m_pdgid(pdgid),
            m_status(status),
            m_recoTrackIndex(recoTrackIndex),
            m_recoTrack(nullptr),
            m_computedTrackProb(1),
            m_trackProb(1)
{
    // Set the hits
    setOrgHits(hits);
}

TrueTrack::~TrueTrack()
{

} 

// big setters
void TrueTrack::setOrgRecoTrack(std::shared_ptr<RecoTrack> track)
{
    if(track->getIndex() != m_recoTrackIndex)
    {
        cout<<"TrueTrack: wrong reco track being associated."<<endl;
        cout<<"Input track "<<track->getIndex()<<endl;
        cout<<"Hit matched track "<<m_recoTrackIndex<<endl;
        exit(1);
    }
    m_recoTrack = track;
}



void TrueTrack::printDebug()
{
    std::cout<<"TrueTrack barecode: "<<m_barcode<<" pt: "<<m_pt<<" eta: "<<m_eta<<" phi: "<<m_phi<<" z0: "<<m_z0<<" d0: "<<m_d0<<" pdgid: "<<m_pdgid<<" nHits: "<<m_hitIndex.size()<<" recoTrackIndex: "<<m_recoTrackIndex<<" track prob: "<<m_computedTrackProb<<endl;
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

    // std::cout<<"---------- sp --------------"<<std::endl;
    // for(const auto& hit: getSPHitsList())
    // {
    //     hit->printDebug();
    // }

}







