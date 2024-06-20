// Class includes
#include "TrackNNEDM/TrackBase.h"

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
TrackBase::TrackBase(std::vector<Index> hitIndex):
    m_hitIndex(hitIndex),
    m_nnScore(-999)
{
    resetCaches();
}

TrackBase::~TrackBase()
{

} 


// big setters
void TrackBase::setOrgHits(std::vector<std::shared_ptr<Hit>> Hits)
{
    for(auto& hit: Hits)
    {
        if(std::find(m_hitIndex.begin(), m_hitIndex.end(), hit->getIndex()) == m_hitIndex.end())
        {
            cout<<"TrackBase: wrong true hit being associated."<<endl;
            cout<<"Input hit index "<<hit->getIndex()<<endl;
            

            for(const auto& hit: m_hitIndex) cout<<"associated hits: "<<hit<<endl;

            exit(1);
        }
        m_hitList[hit->getIndex()] = hit;
    }
}



std::vector<std::shared_ptr<Hit>> TrackBase::getHitsList()
{
    // Return the cached
    if(m_outList.size() > 0) return m_outList;

    // otherwise push it back into a list
    for(const auto& hitInfo: m_hitList) 
    {
        m_outList.push_back(hitInfo.second);
    }

    static auto comp = [](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
    {
      return a->getRho() < b->getRho();
    };

    // So the hits line up
    std::sort (m_outList.begin(),m_outList.end(), comp);

    return m_outList;
}

std::vector<std::shared_ptr<Hit>> TrackBase::getLayeredHitsList(DetectorRegion detectorRegion)
{
    m_outLayeredList.clear();
    // Get the normal hit list
    auto orgHitList = getHitsList();
    if(orgHitList.size() == 0) return m_outRotatedList;
    fillLayeredHits(orgHitList, m_outLayeredList, detectorRegion);
    return m_outLayeredList;
}

void TrackBase::fillLayeredHits(std::vector<std::shared_ptr<Hit>> orgHitList, std::vector<std::shared_ptr<Hit>>& layeredHits, DetectorRegion detectorRegion)
{
    layeredHits.push_back(orgHitList.front());
    int prev_layer = orgHitList.front()->getLayer();
    int prev_volume = orgHitList.front()->getVolume();
    int j = -1;
    for(const auto& hit: orgHitList)
    {   
        j++;
        //skip the first hit
        if (j == 0)
        {
            continue;
        }

        int hit_layer = hit->getLayer();
        int hit_volume = hit->getVolume();
        if ((detectorRegion == DetectorRegion::barrel) && (!hit->getIntMetadata("isBarrel"))) 
        {
            
            break;
        }
        
        // For endccap only, allow the first 3 hits to be within the barrel 
        if (j > 2)
        {
            if ((detectorRegion == DetectorRegion::endcap) && (hit->getIntMetadata("isBarrel")))
            {
                break;
            }
        }
        // if the current hit is in the same volume as the previous hit, check to see if it is in a new layer
        if (hit_volume == prev_volume)
        {
            if (hit_layer != prev_layer)
            //if (hit_layer == prev_layer+2)
            {
                layeredHits.push_back(hit);
                prev_layer = hit_layer;
            }
            else continue;
        }
        else // TODO: if the current hit is in a new volume, check to see that it is in the first layer of the volume
        {
            layeredHits.push_back(hit);
            prev_layer = hit_layer;
            prev_volume = hit_volume;
        }
    }
}


// Haider: This is far too hardcoded to be in the base class
// std::vector<std::shared_ptr<Hit>> TrackBase::getBarrelLayeredHits()
// {
    
//     std::vector<int> barrelVolumes = {12, 13, 14, 80, 81, 82, 83, 84};
//     std::vector<std::shared_ptr<Hit>> outList;
//     int layerID = 999;
//     int volumeID = 999;
//     for(const auto& hitInfo: m_hitList)
//     {
//         auto hit = hitInfo.second;
//         // only use hits in consecutive layers in the barrel
//         bool seedInBarrel = (std::find(barrelVolumes.begin(), barrelVolumes.end(), hit->getVolume()) != barrelVolumes.end());
//         if(!seedInBarrel){continue;}
        
//         // if the next hit is in the same volume, ensure it is in a consecutive layer    
//         if (hit->getVolume() == volumeID)
//         {
//             if (hit->getLayer() != layerID + 1){continue;} 
//         }
//         else
//         {
//             if(hit->getLayer() != 0){continue;}
//         }
//         m_outList.push_back(hit);
//         layerID = hit->getLayer();
//         volumeID = hit->getVolume();
//     }

//     static auto comp = [](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
//     {
//       return a->getRho() < b->getRho();
//     };

//     // So the hits line up
//     std::sort (m_outList.begin(),m_outList.end(), comp);
//     return outList;


// }

std::vector<std::shared_ptr<Hit>> TrackBase::getHitsRotatedToZeroList()
{
    // Return the cached
    if(m_outRotatedList.size() > 0) return m_outRotatedList;
    
    // Get the normal hit list
    auto orgHitList = getHitsList();

    if(orgHitList.size() == 0) return m_outRotatedList;

    fillRotatedHits(orgHitList, m_outRotatedList);
    return m_outRotatedList;
}

std::vector<std::shared_ptr<Hit>> TrackBase::getHitsRoughRotatedToZeroList()
{
    // Return the cached
    if(m_outRoughRotatedList.size() > 0) return m_outRoughRotatedList;
    
    // Get the normal hit list
    auto orgHitList = getHitsList();

    if(orgHitList.size() == 0) return m_outRoughRotatedList;

    fillRotatedHits(orgHitList, m_outRoughRotatedList);
    return m_outRoughRotatedList;
}

///////// Space point formulation of the above
std::vector<std::shared_ptr<Hit>> TrackBase::getSPHitsList()
{
    // Return the cached
    if(m_outSPList.size() > 0) return m_outSPList;

    // otherwise push it back into a list
    for(const auto& hitInfo: m_inSPList) 
    {
        m_outSPList.push_back(hitInfo);
    }

    static auto comp = [](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
    {
      return a->getRho() < b->getRho();
    };

    // So the hits line up
    std::sort (m_outSPList.begin(),m_outSPList.end(), comp);

    return m_outSPList;
}

std::vector<std::shared_ptr<Hit>> TrackBase::getSPHitsRotatedToZeroList()
{
    // Return the cached
    if(m_outRotatedSPList.size() > 0) return m_outRotatedSPList;
    
    // Get the normal hit list
    auto orgHitList = getSPHitsList();
    if(orgHitList.size() == 0) return m_outRotatedSPList;

    fillRotatedHits(orgHitList, m_outRotatedSPList);
    return m_outRotatedSPList;
}

std::vector<std::shared_ptr<Hit>> TrackBase::getSPHitsRoughRotatedToZeroList()
{
    // Return the cached
    if(m_outRoughRotatedSPList.size() > 0) return m_outRoughRotatedSPList;
    
    // Get the normal hit list
    auto orgHitList = getSPHitsList();
    if(orgHitList.size() == 0) return m_outRoughRotatedSPList;

    fillRoughRotatedHits(orgHitList, m_outRoughRotatedSPList);
    return m_outRoughRotatedSPList;
}



std::vector<std::shared_ptr<Hit>> TrackBase::getSPHitsUnscaledRotatedToZeroList()
{
    // Return the cached
    if(m_outRoughRotatedSPList.size() > 0) return m_outUnscaledRotatedSPList;
    
    // Get the normal hit list
    auto orgHitList = getSPHitsList();
    if(orgHitList.size() == 0) return m_outUnscaledRotatedSPList;

    fillUnscaledRotatedHits(orgHitList, m_outUnscaledRotatedSPList);
    return m_outUnscaledRotatedSPList;
}

void TrackBase::fillRotatedHits(std::vector<std::shared_ptr<Hit>> orgHitList, std::vector<std::shared_ptr<Hit>>& rotatedHits)
{
    bool flipZ = false;
    double rotateAngle = 0;

    if(orgHitList[0]->getZ() < 0) flipZ = true;
    rotateAngle = atan(orgHitList[0]->getX()/orgHitList[0]->getY());
    if(orgHitList[0]->getY() < 0) rotateAngle += 3.14;

    for(const auto& hit: orgHitList)
    {
        // deep copy the hit
        auto rHit = make_shared<Hit>(*hit.get());
        if(flipZ) rHit->flipZ();
        rHit->rotateXY(rotateAngle);
        rotatedHits.push_back(rHit);
    }
}


void TrackBase::fillRoughRotatedHits(std::vector<std::shared_ptr<Hit>> orgHitList, std::vector<std::shared_ptr<Hit>>& rotatedHits)
{
    bool flipZ = false;
    double cos_angle = orgHitList[0]->getX()/orgHitList[0]->getRoughRotateScale();
    double sin_angle = -1 * orgHitList[0]->getY()/orgHitList[0]->getRoughRotateScale();

    if(orgHitList[0]->getZ() < 0) flipZ = true;

    for(const auto& hit: orgHitList)
    {
        // deep copy the hit
        auto rHit = make_shared<Hit>(*hit.get());
        if(flipZ) rHit->flipZ();
        rHit->rotateXY(cos_angle, sin_angle);
        rotatedHits.push_back(rHit);
    }
}


void TrackBase::fillUnscaledRotatedHits(std::vector<std::shared_ptr<Hit>> orgHitList, std::vector<std::shared_ptr<Hit>>& rotatedHits)
{
    bool flipZ = false;
    double cos_angle = orgHitList[0]->getX();
    double sin_angle = -1 * orgHitList[0]->getY();

    if(orgHitList[0]->getZ() < 0) flipZ = true;

    for(const auto& hit: orgHitList)
    {
        // deep copy the hit
        auto rHit = make_shared<Hit>(*hit.get());
        if(flipZ) rHit->flipZ();
        rHit->rotateXY(cos_angle, sin_angle);
        rotatedHits.push_back(rHit);
    }
}

int TrackBase::getNHits()
{
    // Return cached value
    if(m_nHits >= 0) return m_nHits;

    // otherwise calculate it
    m_nHits = getHitsList().size();
    return m_nHits;
}

std::shared_ptr<Hit> TrackBase::getHit(int i)
{
    return getHitsList().at(i);
}


void TrackBase::resetCaches()
{
    m_nHits = -1;
    m_outRotatedList.clear();
    m_outList.clear();
}



bool TrackBase::hasHit(Index index)
{
    if(std::find(m_hitIndex.begin(), m_hitIndex.end(), index) == m_hitIndex.end()) return false;
    return true;
}


double TrackBase::calcOverlapProbability(std::shared_ptr<TrackBase> track)
{
    auto currHitList = this->getHitsList();
    return float(getNHitOverlap(track))/currHitList.size();
}


int TrackBase::getNHitOverlap(std::shared_ptr<TrackBase> track)
{
    auto currHitList = this->getHitsList();
    auto compHitList = track->getHitsList();

    int match = 0;
    for(const auto& hit: currHitList)
    {
        for(const auto& toCompare: compHitList)
        {
            if(hit->distanceToX(toCompare) > 0.1) continue;
            if(hit->distanceToY(toCompare) > 0.1) continue;
            if(hit->distanceToZ(toCompare) > 0.1) continue;

            if(hit->distanceTo(toCompare) < 0.1)
            {
                compHitList.erase(std::remove(compHitList.begin(), compHitList.end(), toCompare), compHitList.end());
                match++;
                break;
            }
        }
    }

    return match;
}

void TrackBase::printArray()
{
    std::cout<<"===================="<<std::endl;
    for (auto& hit: this->getHitsList())
    {
        std::cout<<"["<<hit->getX()<<", "<<hit->getY()<<", "<<hit->getZ()<<"],"<<std::endl;
    }
}






