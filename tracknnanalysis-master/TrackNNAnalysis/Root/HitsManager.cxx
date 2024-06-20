// Class includes
#include "TrackNNAnalysis/HitsManager.h"

// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <cstdlib>
#include <stdlib.h>
#include <math.h>
// Root includes
#include <TVector2.h>

using namespace std;


// Constructors and destructor
HitsManager::HitsManager(std::vector<std::shared_ptr<Hit>> hits, std::vector<std::shared_ptr<DetectorModule>> detectorModules, bool m_doHitsToSpacepoints):
    m_hits(hits),
    m_detectorModules(detectorModules)
{

    // collapse down to space points
    // inner side has even module number and side==0, outer side odd module number and side==1
    // we're going to move all outer side hits onto the inner side
    // and combine them into spacepoints if they are "close enough" in eta and phi

    if(m_doHitsToSpacepoints) {
        // I think don't need to iterate twice as hits are all sorted by module ID :-)
        // let's test this assumption first by not doing any combination, just flattening

        // going to be easier if we do this module by module
        // NB: module IDs are unique, they don't repeat across layers or volumes
        unordered_set<int> done_moduleIDs;
        vector<int> hits_to_zap;
        int this_inner_moduleID = -1;
        // some configuration of which hits to make spacepoints from
        bool onlyBackToBack = false; // if false use the three options below
        float z_gap = 100.0; // only relevant in barrel
        float rho_gap = 100.0; // only relevant in endcap
        float dPhi_gap = 0.01; // using 10 mrad         
        float hit_merge_threshold = 0.01*0.01; // typical stub was about 2 mrad, so try 10 mrad as a start

        // loop over all hits, but we're going to do it module by module
        for(unsigned int i_hit=0; i_hit<m_hits.size(); i_hit++) 
        {
            if(m_hits[i_hit]->getPixorSct() == 1) 
            { // strip hits only
                if(m_hits[i_hit]->getSide() == 0) 
                { // only want inner side hits
                    if(done_moduleIDs.find(m_hits[i_hit]->getModule()) == done_moduleIDs.end()) 
                    { // only hits in modules I haven't done yet

                        this_inner_moduleID = m_hits[i_hit]->getModule();
                        done_moduleIDs.insert(this_inner_moduleID);

                        // step 1: get the distribution of hits in this module
                        std::pair<float,float> zRange(9999,-9999), rhoRange(9999,-9999), dPhiRange(9999,-9999);
                        TVector2 tv2_i(m_hits[i_hit]->getX(),m_hits[i_hit]->getY());
                        
                        for(unsigned int j_hit=i_hit; j_hit<m_hits.size(); j_hit++) 
                        {
                          if(m_hits[j_hit]->getModule() == this_inner_moduleID) {
                            float z = m_hits[j_hit]->getZ();
                            float rho = m_hits[j_hit]->getRho();
                            TVector2 tv2_j(m_hits[j_hit]->getX(),m_hits[j_hit]->getY());
                            float dPhi = tv2_i.DeltaPhi(tv2_j);
                            
                            if(z < zRange.first) zRange.first = z;
                            if(z > zRange.second) zRange.second = z;
                            if(rho < rhoRange.first) rhoRange.first = rho;
                            if(rho > rhoRange.second) rhoRange.second = rho;
                            if(dPhi < dPhiRange.first) dPhiRange.first = dPhi;
                            if(dPhi > dPhiRange.second) dPhiRange.second = dPhi;
                            
                          }
                        }

                        // make two lists: all hits in this module, and all hits in the adjacent one
                        vector<int> thisSide_hit_indices, thatSide_hit_indices;
                        for(unsigned int j_hit=i_hit; j_hit<m_hits.size(); j_hit++) 
                        {
                            int moduleID_diff = m_hits[j_hit]->getModule() - this_inner_moduleID;
                            if(moduleID_diff == 0) thisSide_hit_indices.push_back(j_hit);

                            // original: only look at the back-to-back module, which has module ID 1 higher
                            if(onlyBackToBack) {
                                if(moduleID_diff==0) continue;
                                else if(moduleID_diff == 1) thatSide_hit_indices.push_back(j_hit);
                                else if(moduleID_diff > 1) break;
                                else 
                                {
                                    cout << "WARNING!! Encountered unexpected module ID diff of " << moduleID_diff << " for hits " << i_hit << " and " << j_hit << endl;
                                    m_hits[i_hit]->printDebug();
                                    m_hits[j_hit]->printDebug();
                                }
                            }
                          
                            // now: look at other-side hits in back to back module plus those in other modules up to a certain distance in r-phi and z
                            else {
                                if(moduleID_diff==0) continue;
                                else if(moduleID_diff == 1) thatSide_hit_indices.push_back(j_hit);
                                else if(moduleID_diff % 2 == 1) { // only want opposite side
                                    // have the edges in zRange, rhoRange and dPhiRange
                                    if( (m_hits[j_hit]->getZ() - zRange.second < z_gap) && (zRange.first - m_hits[j_hit]->getZ() < z_gap) ) {
                                        if( (m_hits[j_hit]->getRho() - rhoRange.second < rho_gap) && (rhoRange.first - m_hits[j_hit]->getRho() < rho_gap) ) {
                                            // now dphi, relative to the first hit
                                            TVector2 tv2_j(m_hits[j_hit]->getX(),m_hits[j_hit]->getY());
                                            float dPhi = tv2_i.DeltaPhi(tv2_j);
                                            if( (dPhi - dPhiRange.second < dPhi_gap) && (dPhiRange.first - dPhi < dPhi_gap) ) {
                                                thatSide_hit_indices.push_back(j_hit);
                                            }
                                        }
                                    }
                                }
                            }
                            
                        }
                        

                        // there needs to be at least one hit on each side, otherwise we're just going to take them as-is

                        if(thisSide_hit_indices.size() > 0 && thatSide_hit_indices.size() > 0) 
                        {
                            // then, for all the this side modules, see if they have a pair
                            // matrix of dR2 (or whatever distance metric we think is appropriate)
                            vector<vector<float>> dR2_matrix(thisSide_hit_indices.size(),
                                    vector<float>(thatSide_hit_indices.size()));

                            for(unsigned int i_j=0; i_j<thisSide_hit_indices.size(); i_j++) 
                            {
                                int j_hit = thisSide_hit_indices[i_j];
                                TVector2 tv2_j(m_hits[j_hit]->getX(), m_hits[j_hit]->getY());
                                for(unsigned int i_k=0; i_k<thatSide_hit_indices.size(); i_k++) 
                                {
                                    int k_hit = thatSide_hit_indices[i_k];
                                    TVector2 tv2_k(m_hits[k_hit]->getX(), m_hits[k_hit]->getY());
                                    // float dPhi = m_hits[k_hit]->getPhi() - m_hits[j_hit]->getPhi();
                                    float dPhi = tv2_j.DeltaPhi(tv2_k);
                                    float dEta = m_hits[k_hit]->getEta() - m_hits[j_hit]->getEta();

                                    dR2_matrix[i_j][i_k] = dEta*dEta + dPhi*dPhi;
                                }
                            }

                            // if they do, merge them and modify m_hits, and add to hits_to_zap
                            // get the minimum value of the array, merge those hits, delete that row and colum of the matrix, repeat

                            std::vector<pair<int,int>> pairs_to_merge;

                            while(true) 
                            {
                                int min_j = 0;
                                int min_k = 0;
                                float min_val = 99;
                                for(unsigned int i_j = 0; i_j<dR2_matrix.size(); i_j++) 
                                {
                                    int this_min_k = std::min_element(dR2_matrix[i_j].begin(),dR2_matrix[i_j].end()) - dR2_matrix[i_j].begin();
                                    float this_min_k_val = dR2_matrix[i_j][this_min_k];
                                    if(this_min_k_val < min_val) 
                                    {
                                        min_j = i_j;
                                        min_k = this_min_k;
                                        min_val = this_min_k_val;
                                    }
                                }
                                if(min_val < hit_merge_threshold) 
                                {
                                    pairs_to_merge.push_back(make_pair(min_j, min_k));
                                    // cout << "found min pair: " << min_j << ", " << min_k << ", " << min_val << endl;
                                    // set the dR2 values to be large
                                    for(unsigned int i_j = 0; i_j<dR2_matrix.size(); i_j++) dR2_matrix[i_j][min_k] = 9;
                                    for(unsigned int i_k = 0; i_k<dR2_matrix[min_j].size(); i_k++) dR2_matrix[min_j][i_k] = 9;
                                }
                                else 
                                {
                                    break;
                                }
                            }

                            // modify the inner side hit, add the outer side one to the zap list
                            for(auto mergePair : pairs_to_merge) 
                            {
                                int i_in = thisSide_hit_indices[mergePair.first];
                                int i_out = thatSide_hit_indices[mergePair.second];
                                auto in_hit = m_hits[i_in];
                                auto out_hit = m_hits[i_out];

                                // make new spacepoint hit
                                // take mean of inner and outer hits for position, keep inner side module info
                                // designate as spacepoint by side == -1


                                // TODO: assigne a new index 
                                auto spacepoint = make_shared<Hit> (
                                     in_hit->getIndex(),
                                     0.5 * (in_hit->getX() + out_hit->getX()),
                                     0.5 * (in_hit->getY() + out_hit->getY()),
                                     0.5 * (in_hit->getZ() + out_hit->getZ()),
                                     0.5 * (in_hit->getR() + out_hit->getR()),
                                     0.5 * (in_hit->getRho() + out_hit->getRho()),
                                     in_hit->getVolume(),
                                     in_hit->getLayer(),
                                     in_hit->getModule(),
                                     in_hit->getEtaModule(),
                                     in_hit->getPhiModule(),
                                     in_hit->getPixorSct(),
                                     -1,
                                     in_hit->getRecoTrackIndex(),
                                     in_hit->getTruthTrackBarcode()
                                    );
                                m_hits[i_in] = spacepoint; 
                                //TODO: We should probably have a different container with spaces points

                                // make a note of the outer side hit to zap
                                hits_to_zap.push_back(i_out);
                            }
                        }
                    }
                }        
            }
        }
        // remove all zapped outer hits
        // sort and go from the biggest down so don't mess up ordering as we go
        int nHits_start = m_hits.size();
        sort(hits_to_zap.begin(), hits_to_zap.end());

        for(int i_zap = hits_to_zap.size()-1; i_zap>=0; i_zap--) m_hits.erase(m_hits.begin() + hits_to_zap[i_zap]);

        // check that exactly one hit was removed per entry
        if(int(m_hits.size() + hits_to_zap.size()) != nHits_start) cout << "WARNING!! With " << hits_to_zap.size() << " hits to zap and " << nHits_start << " to start with, ended up with " << m_hits.size() << endl;

        cout << "From " << m_hits.size() << " hits, merged " << hits_to_zap.size() << " pairs of hits into spacepoints, designated as such with side=-1" << endl;
    }

    m_nHits = m_hits.size();

    // Order the info
    for(auto& mod: m_detectorModules)
    {
        auto layerID  = getLayerUniqueId(mod->getVolume(), mod->getLayer());
        auto moduleID = getModuleUniqueId(mod->getVolume(), mod->getLayer(), mod->getModule());

        m_layerOrderedDectModules[layerID].push_back(mod);
        m_moduleOrderedDectModules[moduleID].push_back(mod);

        m_layerSize[layerID] = 0;
        m_moduleSize[moduleID] = 0;
    }


    // Order the info
    for(auto& hit: m_hits)
    {

        auto layerID  = getLayerUniqueId(hit->getVolume(), hit->getLayer());
        auto moduleID = getModuleUniqueId(hit->getVolume(), hit->getLayer(), hit->getModule());

        m_layerOrdered[layerID].push_back(hit);
        m_moduleOrdered[moduleID].push_back(hit);
    }
    // Cache the size
    for(const auto& info: m_layerOrdered)  m_layerSize [info.first] = info.second.size();
    for(const auto& info: m_moduleOrdered) m_moduleSize[info.first] = info.second.size();

    // // Order the info
    // for(auto& mod: m_detectorModules)
    // {
    //     auto layerID  = getLayerUniqueId(mod->getVolume(), mod->getLayer());
    //     auto moduleID = getModuleUniqueId(mod->getVolume(), mod->getLayer(), mod->getModule());

    //     cout<<"unique modules vol: "<<mod->getVolume()
    //     <<" lay: "<<mod->getLayer()
    //     <<" mod: "<<mod->getModule()
    //     <<" layID: "<<layerID
    //     <<" modID: "<<moduleID
    //     <<" layerHit: "<<m_layerSize[layerID]
    //     <<" moduleHit: "<<m_moduleSize[moduleID]<<endl;
    // }


} 

HitsManager::~HitsManager()
{
} 

std::shared_ptr<Hit> HitsManager::getRandomHit()
{
    int randIndex = (rand() % m_nHits);
    return m_hits.at(randIndex);
}


std::shared_ptr<Hit> HitsManager::getLayerRandHit(int volume, int layer)
{
    auto uniqueId = getLayerUniqueId(volume, layer);
    int nHits = m_layerSize.at(uniqueId);
    int randIndex = (rand() % nHits);

    return m_layerOrdered.at(uniqueId).at(randIndex);
}


std::shared_ptr<Hit> HitsManager::getModuleRotatedRandHit(std::shared_ptr<Hit> orgHit, float randAngle)
{
    // Get rotated hits coorginated
    float x = 0;
    float y = 0;
    float z = orgHit->getZ();
    float phi = 0;

    orgHit->fillrotatedXYPhi(randAngle, x, y, phi);

    // Find the layer id for this hit
    auto uniqueId = getLayerUniqueId(orgHit->getVolume(), orgHit->getLayer());

    // list of detector hits in the layer
    auto moduleList = m_layerOrderedDectModules.at(uniqueId);

    // find the closest module with more than 1 hit
    int closestModuleID = -1;
    double distance = std::numeric_limits<double>::infinity();
    for(const auto& mod: moduleList)
    {
        double cDist = mod->distanceTo(x, y, z);
        auto id = getModuleUniqueId(mod->getVolume(), mod->getLayer(), mod->getModule());

        // Want atleast a hit
        if((cDist < distance) && (m_moduleSize.at(id) > 0))
        {
            distance = cDist;
            closestModuleID = id; 
        }
    }
    int nHits = m_moduleSize.at(closestModuleID);
    int randIndex = (rand() % nHits);

    return m_moduleOrdered.at(closestModuleID).at(randIndex);
}

int HitsManager::getLayerUniqueId(int volume, int layer)
{
    return volume * 1000 + layer;
}

std::shared_ptr<Hit> HitsManager::getModuleRandHit(int volume, int layer, int module)
{
    auto uniqueId = getModuleUniqueId(volume, layer, module);
    int nHits = m_moduleSize.at(uniqueId);
    int randIndex = (rand() % nHits);

    return m_moduleOrdered.at(uniqueId).at(randIndex);
}

int HitsManager::getModuleUniqueId(int volume, int layer, int module)
{
    // Module is <20000
    // Layer is the next number
    // 
    return volume * 1000000 +  layer * 100000 + module;
}

std::shared_ptr<Hit> HitsManager::getModuleIterRandHit(std::shared_ptr<Hit> orgHit)
{
    auto uniqueId = getModuleUniqueId(orgHit->getVolume(), orgHit->getLayer(), orgHit->getModule());
    int nHits = m_moduleSize.at(uniqueId);
    // If there is nothing just return this
    if(nHits <= 1) return orgHit;

    auto moduleHits = m_moduleOrdered.at(uniqueId);

    auto comp = [&orgHit](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
    {
        return a->distanceTo(orgHit) < b->distanceTo(orgHit);
    };

    // So the hits line up
    // Order these according to the distance from the orgHit
    std::sort (moduleHits.begin(),moduleHits.end(), comp);

    // Move the first element to the last, since the first is our orgHit
    // The last element will have the lower probability of being picked
    std::rotate(moduleHits.begin(), moduleHits.begin() + 1, moduleHits.end());

    m_geoDist = std::geometric_distribution<int>(1.0/nHits);
    int randIndex = (m_geoDist(m_generator) % nHits);

    return moduleHits.at(randIndex);
}
std::shared_ptr<Hit> HitsManager::getModuleRotatedIterRandHit(std::shared_ptr<Hit> orgHit, float randAngle)
{
    // Get rotated hits coorginated
    float x = 0;
    float y = 0;
    float z = orgHit->getZ();
    float phi = 0;

    orgHit->fillrotatedXYPhi(randAngle, x, y, phi);

    // Find the layer id for this hit
    auto uniqueId = getLayerUniqueId(orgHit->getVolume(), orgHit->getLayer());

    // list of detector hits in the layer
    auto moduleList = m_layerOrderedDectModules.at(uniqueId);

    // find the closest module with more than 1 hit
    int closestModuleID = -1;
    double distance = std::numeric_limits<double>::infinity();
    for(const auto& mod: moduleList)
    {
        double cDist = mod->distanceTo(x, y, z);
        auto id = getModuleUniqueId(mod->getVolume(), mod->getLayer(), mod->getModule());

        // Want atleast a hit
        if((cDist < distance) && (m_moduleSize.at(id) > 0))
        {
            distance = cDist;
            closestModuleID = id; 
        }
    }

    int nHits = m_moduleSize.at(closestModuleID);
    auto moduleHits = m_moduleOrdered.at(closestModuleID);

    auto comp = [&x, &y, &z](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
    {
        return a->distanceTo(x, y, z) < b->distanceTo(x, y, z);
    };

    // So the hits line up
    // Order these according to the distance from the orgHit
    std::sort (moduleHits.begin(),moduleHits.end(), comp);

    m_geoDist = std::geometric_distribution<int>(1.0/nHits);
    int randIndex = (m_geoDist(m_generator) % nHits);

    return moduleHits.at(randIndex);
}


std::vector<std::shared_ptr<Hit>> HitsManager::getHitsInRange(double lwrPhi, double uprPhi, double lwrEta, double uprEta)
{
    std::vector<std::shared_ptr<Hit>> hitList = {};

    for(auto& hit: m_hits)
    {
        float phi = hit->getPhi();
        float eta = hit->getEta();

        if(eta > uprEta) continue;
        if(eta < lwrEta) continue;

        if(!angle_is_between_angles(phi, lwrPhi, uprPhi)) continue;

        hitList.push_back(hit);
    }

    return hitList;
}

std::shared_ptr<Hit> HitsManager::getClosestHit(std::shared_ptr<Hit> orgHit)
{
    // initialize minimum distance and Closest Hit
    float minDistance = 10000;
    std::shared_ptr<Hit> currentHit;
    auto layerList = m_layerOrdered.at(orgHit->getLayer());

    for(auto& hit: layerList)
    {

        float distance = hit->distanceTo(orgHit);

        if((distance < minDistance) && (distance > 0))
        {
            minDistance = distance;
            currentHit = hit;
        }
    }

    return currentHit;
}

std::shared_ptr<Hit> HitsManager::getClosestHitGeodesic(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, int volume_id, int layer_id)
{
    // predHit is not a full hit but rather a hit with only x,y,z predicted coordinates
    // the volume_id and layer_id refer to the volume and layer we are interested in searching for hits
    std::shared_ptr<Hit> currentHit;
    auto layerID  = getLayerUniqueId(volume_id, layer_id);
    if(m_layerOrdered.find(layerID) == m_layerOrdered.end()) return currentHit;
    auto hitList =  m_layerOrdered.at(layerID);

    // initialize minimum distance and Closest Hit
    float minDistance = 10000;
    //width of the cylindrical plane determined by the layer. For now, just use the rho value of a hit in this layer
    float cylWidth;
    int counter = 0;
    float distance;
    for(auto& hit: hitList)
    {
        if ((hit->getVolume()!=volume_id) || (hit->getLayer() != layer_id)) continue;

        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        if (counter==0)
        {
            cylWidth = 2*M_PI*(hit->getRho());
            counter +=1;
        }

        float hitX = hit->getUnrolledX(hit->getRho());
        float predX = predHit->getUnrolledX(hit->getRho());

        if (std::abs(hitX - predX) < (cylWidth / 2)) distance = hit->distanceTo(predX, predHit->getZ(), 0);
        // check this!
        else distance = std::sqrt(pow(cylWidth-std::abs(hitX-predX),2)+std::pow(hit->getZ() - predHit->getZ(),2));

        if((distance < minDistance) && (distance > 0))
        {
            minDistance = distance;
            currentHit = hit;
        }
    }

    return currentHit;
}

std::shared_ptr<Hit> HitsManager::getClosestLayerHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, int volume_id, int layer_id)
{
    // predHit is not a full hit but rather a hit with only x,y,z predicted coordinates
    // the volume_id and layer_id refer to the volume and layer we are interested in searching for hits

    // initialize minimum distance and Closest Hit
    float minDistance = 10000;

    std::shared_ptr<Hit> currentHit;
    auto layerID  = getLayerUniqueId(volume_id, layer_id);
    if(m_layerOrdered.find(layerID) == m_layerOrdered.end()) return currentHit;
    auto hitList =  m_layerOrdered.at(layerID);


    for(auto& hit: hitList)
    {
        if ((hit->getVolume()!=volume_id) || (hit->getLayer() != layer_id)) continue;

        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float distance = hit->distanceTo(predHit);

        if((distance < minDistance) && (distance > 0))
        {
            minDistance = distance;
            currentHit = hit;
        }
    }

    return currentHit;
}

std::shared_ptr<Hit> HitsManager::getClosestHitFromPredictedHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, double maxDistance)
{
    // initialize minimum distance and Closest Hit
    float minDistance = 99999999999;
    std::shared_ptr<Hit> currentHit = nullptr;

    for(auto& hit: m_hits)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float distance = hit->distanceTo(predHit);

        if((distance < minDistance) && (distance > 0) && (distance < maxDistance))
        {
            minDistance = distance;
            currentHit = hit;
        }
    }

    return currentHit;
}

std::vector<std::shared_ptr<Hit>> HitsManager::getClosestLayerHitsList(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, double& error)
{
    std::vector<std::shared_ptr<Hit>> hitsInRange;

    auto layerID  = getLayerUniqueId(volume_id, layer_id);
    // std::cout<<"layerID: "<<layerID<<" volume: "<<volume_id<<" layer: "<<layer_id<<std::endl;
    // for(const auto& var: m_layerOrdered) std::cout<<"var: "<<var.first<<std::endl;
    if(m_layerOrdered.find(layerID) == m_layerOrdered.end()) return hitsInRange;
    auto hitList =  m_layerOrdered.at(layerID);

    for(auto& hit: hitList)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float distance = hit->distanceTo(predHit);

        if((distance < error) && (distance > 0))
        {
            hitsInRange.push_back(hit);
        }
    }

    return hitsInRange;
}

std::vector<std::shared_ptr<Hit>> HitsManager::getClosestLayerHitsList(Eigen::VectorXf& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, float& error, bool sort)
{
    std::vector<std::shared_ptr<Hit>> hitsInRange;
    std::vector<float> distances;

    std::vector<std::pair<std::shared_ptr<Hit>, float>> hitDistances;

    auto layerID  = getLayerUniqueId(volume_id, layer_id);
    // std::cout<<"layerID: "<<layerID<<" volume: "<<volume_id<<" layer: "<<layer_id<<std::endl;
    // for(const auto& var: m_layerOrdered) std::cout<<"var: "<<var.first<<std::endl;
    if(m_layerOrdered.find(layerID) == m_layerOrdered.end()) return hitsInRange;
    auto hitList =  m_layerOrdered.at(layerID);

    auto errorSquare = error*error;
    for(auto& hit: hitList)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) != hits.end()) continue;

        float distance = std::pow((hit->getX() - predHit(0)),2) + std::pow((hit->getY() - predHit(1)),2) + std::pow((hit->getZ() - predHit(2)),2);
             
        if((distance < errorSquare) && (distance > 0))
        {
            hitDistances.emplace_back(std::make_pair(hit, distance));
        }
    }
    if (sort)
    {
        std::sort(hitDistances.begin(), hitDistances.end(), [](std::pair<std::shared_ptr<Hit>, float> a, std::pair<std::shared_ptr<Hit>, float> b)->bool
        {
            return a.second < b.second;
        });
    }

    for (auto& pair: hitDistances)
    {
        hitsInRange.push_back(pair.first);
    }

    return hitsInRange;
}

std::vector<std::shared_ptr<Hit>> HitsManager::getKNearestNeighborHits(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, double& error, int& k)
{
    std::vector<std::shared_ptr<Hit>> hitsInRange;

    auto layerID  = getLayerUniqueId(volume_id, layer_id);
    // std::cout<<"layerID: "<<layerID<<" volume: "<<volume_id<<" layer: "<<layer_id<<std::endl;
    // for(const auto& var: m_layerOrdered) std::cout<<"var: "<<var.first<<std::endl;
    if(m_layerOrdered.find(layerID) == m_layerOrdered.end()) return hitsInRange;
    auto hitList =  m_layerOrdered.at(layerID);
    std::vector<std::pair<std::shared_ptr<Hit>,float>> distanceMap;
    for(auto& hit: hitList)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float distance = hit->distanceTo(predHit);
        if (distance < error)
            distanceMap.push_back({hit,distance});
    }
    // sort hits by distance
    sort(distanceMap.begin(), distanceMap.end(), 
    [](std::pair<std::shared_ptr<Hit>,float> &a, std::pair<std::shared_ptr<Hit>,float> &b) -> bool
    { 
        return a.second < b.second; 
    });

    // return the first k hits within the given uncertainty radius
    if (k > (int) distanceMap.size()) k = distanceMap.size();
    for (int i = 0; i < k; i++)
    {
        hitsInRange.push_back(distanceMap.at(i).first);
    }
    return hitsInRange;
}

std::vector<std::shared_ptr<Hit>> HitsManager::getKNearestNeighborHits(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, double& error, int& k)
{
    std::vector<std::shared_ptr<Hit>> hitsInRange;

    std::vector<std::pair<std::shared_ptr<Hit>,float>> distanceMap;
    for(auto& hit: m_hits)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float distance = hit->distanceTo(predHit);
        if (distance < error)
            distanceMap.push_back({hit,distance});
    }
    // sort hits by increasing distance
    sort(distanceMap.begin(), distanceMap.end(), 
    [](std::pair<std::shared_ptr<Hit>,float> &a, std::pair<std::shared_ptr<Hit>,float> &b) -> bool
    { 
        return a.second < b.second; 
    });

    // return the first k hits within the given uncertainty radius
    if (k > (int) distanceMap.size()) k = distanceMap.size();
    for (int i = 0; i < k; i++)
    {
        // std::vector<std::pair<std::shared_ptr<Hit>,float>>::iterator it = distanceMap.begin()+i;
        // it->first->printDebug();
        hitsInRange.push_back(distanceMap.at(i).first);
        // hitsInRange.push_back(it->first);
    }

    return hitsInRange;
}

std::vector<std::shared_ptr<Hit>> HitsManager::getClosestLayerHitsList(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, float& zUncertainty, float& phiUncertainty)
{
    std::vector<std::shared_ptr<Hit>> hitsInRange;

    auto layerID  = getLayerUniqueId(volume_id, layer_id);
    // std::cout<<"layerID: "<<layerID<<" volume: "<<volume_id<<" layer: "<<layer_id<<std::endl;
    // for(const auto& var: m_layerOrdered) std::cout<<"var: "<<var.first<<std::endl;
    if(m_layerOrdered.find(layerID) == m_layerOrdered.end()) return hitsInRange;
    auto hitList =  m_layerOrdered.at(layerID);

    for(auto& hit: hitList)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float zDistance = hit->distanceToZ(predHit, false);
        float phiDistance = hit->distanceToPhi(predHit, false);
        if((zDistance < zUncertainty) && (zDistance > 0) && (phiDistance < phiUncertainty) && (phiDistance > 0))
        {
            hitsInRange.push_back(hit);
        }
    }

    return hitsInRange;
}


// get a list of hits within a given distance to the predicted hit
std::vector<std::shared_ptr<Hit>> HitsManager::getClosestHitsListFromPredictedHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, double maxDistance)
{
    
    std::vector<std::shared_ptr<Hit>> hitsInRange;

    for(auto& hit: m_hits)
    {
        // if it is already in the list continue
        if(std::find(hits.begin(), hits.end(), hit) !=  hits.end()) continue;

        float distance = hit->distanceTo(predHit);

        if((distance < maxDistance) && (distance > 0))
        {
            hitsInRange.push_back(hit);
        }
    }

    return hitsInRange;
}