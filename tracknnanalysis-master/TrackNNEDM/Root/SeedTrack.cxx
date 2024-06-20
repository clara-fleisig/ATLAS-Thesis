// Class includes
#include "TrackNNEDM/SeedTrack.h"

// Local includes
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/RecoTrack.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>

using namespace std;


// Constructors and destructor
SeedTrack::SeedTrack(std::vector<Index> hitIndex, std::vector<std::shared_ptr<Hit>> hits):
TrackBase(hitIndex)
{
    // Set the hits
    setOrgHits(hits);
}

SeedTrack::~SeedTrack()
{

}

void SeedTrack::printDebug()
{
    std::cout<<"hit index: ";
    for(const auto& hit: m_hitIndex)
    {
        std::cout<<hit<<" ";
    }
    std::cout<<std::endl;
    
    for(const auto& hit: this->getHitsList()) hit->printDebug();
}

