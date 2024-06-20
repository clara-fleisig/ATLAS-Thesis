// Class includes
#include "TrackNNAnalysis/TrackManager.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/RecoTrack.h"
// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <cstdlib>
#include <math.h>
#include <utility>

TrackManager::TrackManager()
{

}

TrackManager::~TrackManager()
{

}

std::vector<std::shared_ptr<TrueTrack>> TrackManager::getTrackSingleLayerHit(std::vector<std::shared_ptr<TrueTrack>> recoTracks, int minHits, TrackBase::DetectorRegion detectorRegion)
{
    std::vector<std::shared_ptr<TrueTrack>> layeredTracks;
    for (auto& recoTrk: recoTracks)
    {   
        //std::cout<<"input track index: "<<recoTrk->getBarcode()<<std::endl;
        std::vector<Index> hitIndex;
        auto layeredHits = recoTrk->getLayeredHitsList(detectorRegion);
        if((int) layeredHits.size() < minHits) continue;
        for (auto& hit: layeredHits)
        {
            hitIndex.push_back(hit->getIndex());
        }
        //std::shared_ptr<TrueTrack> layeredTrack = std::make_shared<TrueTrack>(hitIndex, layeredHits);
        std::shared_ptr<TrueTrack> layeredTrack = std::make_shared<TrueTrack>(
            recoTrk->getBarcode(),
            recoTrk->getPt(),
            recoTrk->getEta(),
            recoTrk->getPhi(),
            recoTrk->getVertexDistance(),
            recoTrk->getPDGID(),
            recoTrk->getStatus(),
            recoTrk->getRecoTrackIndex(),
            hitIndex,
            layeredHits
        );
        layeredTrack->setD0(recoTrk->getD0());
        layeredTrack->setZ0(recoTrk->getZ0());
        layeredTracks.push_back(layeredTrack);
    }
    
    return layeredTracks;
}

// Get a set of track segments and associated next hits
std::vector<std::pair<std::shared_ptr<TrueTrack>, std::shared_ptr<Hit>>> TrackManager::getTrackSegmentPairs(std::vector<std::shared_ptr<TrueTrack>> recoTracks)
{
    
    // create a vector that holds pairs of pointers to seeds and their corresponding next hits
    std::vector<std::pair<std::shared_ptr<TrueTrack>, std::shared_ptr<Hit>>> trackSegmentPairs;
    for (auto& trk: recoTracks)
    {
        if ((int) trk->getHitsList().size() < (m_initialSeedSize+1)) 
        {
            //std::cout << "track is not long enough" << std::endl;
            continue;
        }
        // std::cout<<"----  track  ----" <<std::endl;
        // trk->printDebug();
        auto hitList = trk->getHitsList();
        int sr = hitList.size();

        // get track segments
        for(int i = 0; i < (sr - m_initialSeedSize); i++)
        {
            std::vector<Index> hitIndex;
            std::vector<std::shared_ptr<Hit>> trackSegmentHits;
            std::shared_ptr<Hit> nextHit;  
            //select the hits for each window and corresponding next hit
            for(int j = 0; j < (i + m_initialSeedSize); j++)
            {
                hitIndex.push_back(hitList[j]->getIndex());
                trackSegmentHits.push_back(hitList[j]);
                //std::cout<< hit[j]->getX()<<" " << hit[j]->getY()<<" " << hit[j]->getZ() <<std::endl;
                nextHit = hitList[j+1];
            }

            //std::cout << "new seed" << endl;
            //std::cout<< nextHit->getX()<<" " << nextHit->getY()<<" " << nextHit->getZ() <<std::endl;
            //std::cout<< seedHits.back()->getX()<<" " << seedHits.back()->getY()<<" " << seedHits.back()->getZ() <<std::endl;
            std::shared_ptr<TrueTrack> trackSegment = std::make_shared<TrueTrack>(
            trk->getBarcode(),
            trk->getPt(),
            trk->getEta(),
            trk->getPhi(),
            trk->getVertexDistance(),
            trk->getPDGID(),
            trk->getStatus(),
            trk->getRecoTrackIndex(),
            hitIndex,
            trackSegmentHits
            );
            //seed->setBaseRecoTrack(trk);
            std::pair <std::shared_ptr<TrueTrack>, std::shared_ptr<Hit>> trackSegmentPair(trackSegment, nextHit);
            trackSegmentPairs.push_back(trackSegmentPair); 
            
            // std::cout<<"----  seed  ----" <<std::endl;
            // seed->printDebug();
            // std::cout<<"----  target  ----" <<std::endl;
            // nextHit->printDebug();
        }  
    }
    return trackSegmentPairs;
}
