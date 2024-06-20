// Class includes
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNEDM/Hit.h"
// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <cstdlib>
#include <math.h>
#include <utility>

using namespace std;

SeedManager::SeedManager()
{
    m_seedSize = -999;
    m_windowSize = -999;

}
SeedManager::~SeedManager(){}



std::vector<std::shared_ptr<SeedTrack>> SeedManager::getSeedTracks(SeedType seedType, std::vector<std::shared_ptr<TrueTrack>> recoTracks)
{
    std::vector<std::shared_ptr<SeedTrack>> cTrackList;
    if(seedType == SeedType::InitialSeeds)
    {
        cTrackList = getInitialSeeds(recoTracks);
    }
    else if(seedType == SeedType::SlidingWindow)
    {
        cTrackList = getSlidingWindowSeeds(recoTracks);
    }
    //else if(seedType == SeedType::SlidingWindowBarrelLayered)
    //{
    //    cTrackList = getSlidingWindowBarrelLayered;
    //}
    else
    {
        cout<<"Cannot recognize seedType"<<endl;
        exit(1);
    }

    return cTrackList;
}



// Get a set of sliding windows seeds
std::vector<std::shared_ptr<SeedTrack>> SeedManager::getSlidingWindowSeeds(std::vector<std::shared_ptr<TrueTrack>> recoTracks)
{
    if(m_windowSize <= 0)
    {
        cout<<"Please set window size to a postive non-zero integer"<<endl;
        exit(1);
    }
            
    // create a vector that holds pairs of pointers to seeds and their corresponding next hits
    std::vector<std::shared_ptr<SeedTrack>> seedTracks;
    for (auto& trk: recoTracks)
    {
        
        if ((int) trk->getHitsList().size() < m_windowSize) 
        {
            std::cout << "track is not long enough" << std::endl;
            continue;
        }
        auto hit = trk->getHitsList();
        int sr = hit.size();
        for(int i = 0; i < (sr - m_windowSize + 1); i++)
        {
            std::vector<Index> seedHitIndex;
            std::vector<std::shared_ptr<Hit>> seedHits;

            //select the hits for each window and corresponding next hit
            for(int j = i; j < i + m_windowSize; j++)
            {
                seedHitIndex.push_back(hit[j]->getIndex());
                seedHits.push_back(hit[j]);
            }
            std::shared_ptr<SeedTrack> seed = std::make_shared<SeedTrack>(seedHitIndex, seedHits);
            //seed->setBaseRecoTrack(trk);
            seedTracks.push_back(seed);
        }
    }
    return seedTracks;
}

//std::vector<std::shared_ptr<SeedTrack>> SeedManager::getSlidingWindowSeedsBarrelLayered(std::vector<std::shared_ptr<TrueTrack>> recoTracks)
//{
//    if(m_windowSize <= 0)
//    {
//        cout<<"Please set window size to a postive non-zero integer"<<endl;
//        exit(1);
//    }
//    // create a vector that holds pairs of pointers to seeds and their corresponding next hits
//    std::vector<std::shared_ptr<SeedTrack>> seedTracks;
//    for (auto& trk: recoTracks)
//    {
//        auto hit = trk->getHitsList();
//        auto sr = hit.size();
//        for(int i = 0; i < (sr - m_windowSize + 1); i++)
//        {
//            std::vector<Index> seedHitIndex;
//            std::vector<std::shared_ptr<Hit>> seedHits;
//            int layerID = 999;
//            int volumeID = 999;
//            //select the hits for each window
//            // If a hit is not in the barrel or in a consecutive layer, skip it
//            for(int j = i; j < i + m_windowSize; j++)
//            {
//                
//                seedHitIndex.push_back(hit[j]->getIndex());
//                seedHits.push_back(hit[j]);
//            }
//            std::shared_ptr<SeedTrack> seed = std::make_shared<SeedTrack>(seedHitIndex, seedHits);
//            seed->setBaseRecoTrack(trk);
//            seedTracks.push_back(seed);
//        }
//    }
//    return seedTracks;
//
//}
//

// Get a set of sliding window seeds and associated next hits
std::vector<std::pair<std::shared_ptr<SeedTrack>, std::shared_ptr<Hit>>> SeedManager::getSlidingWindowSeedPairs(std::vector<std::shared_ptr<TrueTrack>> recoTracks)
{
    if(m_windowSize <= 0)
    {
        cout<<"Please set window size to a postive non-zero integer"<<endl;
        exit(1);
    }
    
    // create a vector that holds pairs of pointers to seeds and their corresponding next hits
    std::vector<std::pair<std::shared_ptr<SeedTrack>, std::shared_ptr<Hit>>> seedTrackPairs;
    for (auto& trk: recoTracks)
    {
        if ((int) trk->getHitsList().size() < (m_windowSize+3)) 
        {
            //std::cout << "track is not long enough" << std::endl;
            continue;
        }
        // std::cout<<"----  track  ----" <<std::endl;
        // trk->printDebug();
        auto hit = trk->getHitsList();
        int sr = hit.size();
        for(int i = 0; i < (sr - m_windowSize); i++)
        {
            std::vector<Index> seedHitIndex;
            std::vector<std::shared_ptr<Hit>> seedHits;
            std::shared_ptr<Hit> nextHit;  
            //select the hits for each window and corresponding next hit
            for(int j = i; j < (i + m_windowSize); j++)
            {
                seedHitIndex.push_back(hit[j]->getIndex());
                seedHits.push_back(hit[j]);
                //std::cout<< hit[j]->getX()<<" " << hit[j]->getY()<<" " << hit[j]->getZ() <<std::endl;
                nextHit = hit[j+1];
            }
            //std::cout << "new seed" << endl;
            //std::cout<< nextHit->getX()<<" " << nextHit->getY()<<" " << nextHit->getZ() <<std::endl;
            //std::cout<< seedHits.back()->getX()<<" " << seedHits.back()->getY()<<" " << seedHits.back()->getZ() <<std::endl;
            std::shared_ptr<SeedTrack> seed = std::make_shared<SeedTrack>(seedHitIndex, seedHits);
            seed->setBaseTrueTrack(trk);
            std::pair <std::shared_ptr<SeedTrack>, std::shared_ptr<Hit>> seedTrackPair (seed, nextHit);
            seedTrackPairs.push_back(seedTrackPair); 
            
            // std::cout<<"----  seed  ----" <<std::endl;
            // seed->printDebug();
            // std::cout<<"----  target  ----" <<std::endl;
            // nextHit->printDebug();
        }  
    }
    return seedTrackPairs;
}

// Get get the first n hits as the seed
std::vector<std::shared_ptr<SeedTrack>> SeedManager::getInitialSeeds(std::vector<std::shared_ptr<TrueTrack>> recoTracks)
{ 
    // create a vector that holds pairs of pointers to seeds and their corresponding next hits
    std::vector<std::shared_ptr<SeedTrack>> seedTracks;
    for (auto& trk: recoTracks)
    {
        // require tracks have at least N hits
        if ((int) trk->getHitsList().size() < m_windowSize+3) 
        {
            //std::cout << "track is not long enough" << std::endl;
            continue;
        }
        std::vector<Index> seedHitIndex;
        std::vector<std::shared_ptr<Hit>> seedHits;

        auto hit = trk->getHitsList();
          
        //select the first n hits of the track where n=window_size as initial track seeds.
        for(int j = 0; j < m_seedSize; j++)
        {
            seedHitIndex.push_back(hit[j]->getIndex());
            seedHits.push_back(hit[j]);
        }
        std::shared_ptr<SeedTrack> seed = std::make_shared<SeedTrack>(seedHitIndex, seedHits);
        seed->setBaseTrueTrack(trk);
        //seed->setBaseRecoTrack(trk);
        seedTracks.push_back(seed);
    }
    return seedTracks;
}


