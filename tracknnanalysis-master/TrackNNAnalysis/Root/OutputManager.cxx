// Class includes
#include "TrackNNAnalysis/OutputManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNAnalysis/TrackManager.h"

// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <stdlib.h>
// Root includes

using namespace std;


// Constructors and destructor
OutputManager::OutputManager(TString outFileName, int trackSize):
m_fileName(outFileName)
{
    m_inFile = new TFile(outFileName, "recreate");
    m_treeList.clear();

    m_hitX = new vector<float>();
    m_hitX->reserve(40);
    m_hitY = new vector<float>();
    m_hitY->reserve(40);
    m_hitZ = new vector<float>();
    m_hitZ->reserve(40);
    m_hitR = new vector<float>();
    m_hitR->reserve(40);
    m_hitPixorSct = new vector<int>();
    m_hitPixorSct->reserve(40);


    for(int i = 0; i < trackSize; i++)
    {
        m_floatVarList["hitX" + TString::Itoa(i, 10)] = 0;
        m_floatVarList["hitY" + TString::Itoa(i, 10)] = 0;
        m_floatVarList["hitZ" + TString::Itoa(i, 10)] = 0;

        m_targetFloatVarList["tarHitX"] = 0;
        m_targetFloatVarList["tarHitY"] = 0;
        m_targetFloatVarList["tarHitZ"] = 0;

        std::vector<int> v (45,0);
        m_vectorVarList["hitDet"+ TString::Itoa(i, 10)] = v;
        m_targetVectorVarList["tarHitDet"] = v;

        // m_intVarList["detectorID" + TString::Itoa(i, 10)] = -1;
        // m_intVarList["layerID" + TString::Itoa(i, 10)] = -1;
        // m_intVarList["pixOrsct" + TString::Itoa(i, 10)] = 0;
    }


    m_isTrue = -999;
    m_pt = -999;
    m_truthProb = -999;
    m_doComputedTrackProb = false;
    m_hitType = OutHitType::Nominal;
    m_outputCylindrical = false;

    m_randGen = new TRandom3();
    m_randGen->SetSeed(2);

} 

OutputManager::~OutputManager()
{
    m_inFile->Close();
} 

void OutputManager::finalize()
{
    m_inFile->cd();
    for(auto& tree: m_treeList) tree.second->Write();
    m_inFile->Close();
}

TTree* OutputManager::getTree(TString name)
{
    TTree* currTree = NULL;
    if(m_treeList.find(name) == m_treeList.end())
    {
        m_inFile->cd();
        currTree = new TTree(name, name);
        currTree->Branch("x",           &m_hitX);
        currTree->Branch("y",           &m_hitY);
        currTree->Branch("z",           &m_hitZ);
        currTree->Branch("r",           &m_hitR);
        currTree->Branch("type",        &m_hitPixorSct);
        currTree->Branch("isTrue",      &m_isTrue);
        currTree->Branch("pt",          &m_pt);
        currTree->Branch("eta",         &m_eta);
        currTree->Branch("phi",         &m_phi);
        currTree->Branch("d0",          &m_d0);
        currTree->Branch("z0",          &m_z0);
        currTree->Branch("truthProb",   &m_truthProb);

        for(auto& var: m_floatVarList) currTree->Branch(var.first, &var.second);
        for(auto& var: m_intVarList)   currTree->Branch(var.first, &var.second);
        for(auto& var: m_targetFloatVarList) currTree->Branch(var.first, &var.second);
        for(auto& var: m_vectorVarList) currTree->Branch(var.first, &var.second);
        for(auto& var: m_targetVectorVarList) currTree->Branch(var.first, &var.second);

        m_treeList[name] = currTree;
    }
    else
    {
        currTree = m_treeList.at(name);
    }

    return currTree;
}

void OutputManager::saveTrueTruthTracks(std::vector<std::shared_ptr<TrueTrack>> trackList)
{
    TTree* currTree = getTree("TrueTruthTracks");
    auto trackManager = make_shared<TrackManager>();
    auto cleanTracks = trackManager->getTrackSingleLayerHit(trackList,6);

    for (const auto& track: cleanTracks)
    {
        // if(track->getBarcode() > 200000) continue;
        m_pt  = track->getPt();
        m_eta = track->getEta();
        m_phi = track->getPhi();
        m_d0  = track->getD0();
        m_z0  = track->getZ0();
        m_truthProb = track->getTrackProbability(); 
        if(m_doComputedTrackProb) m_truthProb = track->getComputedTrackProbability();
        if(m_truthProb < 0.9) continue;
        m_isTrue = 1;

        auto hitList = getHits(track);
        fillBaseHitInfo(hitList);
        currTree->Fill();
    }
}

void OutputManager::saveExtrapolatedFakeTracks(std::vector<std::shared_ptr<ExtrapolatedTrack>> trackList)
{
    TTree* currTree = getTree("ExtrapolatedFakeTracks");
    
    for (const auto& track: trackList)
    {
        m_isTrue = 0;

        auto hitList = getHits(track);
        fillBaseHitInfo(hitList);
        currTree->Fill();
    }
}

void OutputManager::saveExtrapolatedTrueTracks(std::vector<std::shared_ptr<ExtrapolatedTrack>> trackList)
{
    TTree* currTree = getTree("ExtrapolatedTrueTracks");
    

    for (const auto& track: trackList)
    {

        // m_pt  = track->getPt();
        // m_eta = track->getEta();
        // m_phi = track->getPhi();
        // m_d0  = track->getD0();
        // m_z0  = track->getZ0();
        // m_truthProb = track->getTrackProbability(); 
        // if(m_doComputedTrackProb) m_truthProb = track->getComputedTrackProbability();
        // if(m_truthProb < 0.6) continue;
        m_isTrue = 1;

        auto hitList = getHits(track);
        fillBaseHitInfo(hitList);
        currTree->Fill();
    }
}


void OutputManager::saveTrueTracks(std::vector<std::shared_ptr<RecoTrack>> trackList)
{
    TTree* currTree = getTree("TrueTracks");

    for (const auto& track: trackList)
    {
        m_pt = track->getPt();
        m_eta = track->getEta();
        m_phi = track->getPhi();
        m_d0  = track->getD0();
        m_z0  = track->getZ0();

        m_truthProb = track->getTrackProbability(); 
        if(m_doComputedTrackProb) m_truthProb = track->getComputedTrackProbability();
        if(m_truthProb < 0.7) continue;
        m_isTrue = 1;

        auto hitList = getHits(track);
        fillBaseHitInfo(hitList);

        currTree->Fill();
    }
}


void OutputManager::fillHitsWithSubsample(std::vector<std::shared_ptr<Hit>> hitList, TTree* currTree)
{
    // Do the simple think and continue
    if(hitList.size() == 7)
    {
        fillBaseHitInfo(hitList);
        currTree->Fill();            
    }
    if(hitList.size() == 8)
    {
        // Do the simple thing
        fillBaseHitInfo(hitList);
        currTree->Fill();     

        // // Randomly drop on hit with a probability of 0.1 probability
        // double randNum = m_randGen->Uniform(1);
        // if(randNum < 0.2)
        // {
        //     int randIndex = (rand() % hitList.size());
        //     hitList.erase(hitList.begin() + randIndex); 

        //     // Do the simple thing
        //     fillBaseHitInfo(hitList);
        //     currTree->Fill(); 
        // }

    }
}

void OutputManager::saveHTTFakeTracks(std::vector<std::shared_ptr<RecoTrack>> trackList)
{
    TTree* currTree = getTree("HTTFakeTracks");

    for (const auto& track: trackList)
    {
        m_pt = track->getPt();
        m_eta = track->getEta();
        m_phi = track->getPhi();
        m_d0  = track->getD0();
        m_z0  = track->getZ0();

        m_truthProb = track->getTrackProbability(); 
        if(m_doComputedTrackProb) m_truthProb = track->getComputedTrackProbability();
        if(m_truthProb > 0.7) continue;
        m_isTrue = 0;

        auto hitList = getHits(track);
        if(hitList[0]->getX() == 0)
        {
            track->printDebug();
        }

        fillBaseHitInfo(hitList);
        currTree->Fill();
    }
}

void OutputManager::saveHTTTrueTracks(std::vector<std::shared_ptr<RecoTrack>> trackList)
{
    TTree* currTree = getTree("HTTTrueTracks");

    for (const auto& track: trackList)
    {
        m_pt = track->getPt();
        m_eta = track->getEta();
        m_phi = track->getPhi();
        m_d0  = track->getD0();
        m_z0  = track->getZ0();

        m_truthProb = track->getTrackProbability(); 
        if(m_doComputedTrackProb) m_truthProb = track->getComputedTrackProbability();
        if(m_truthProb < 0.99) continue;
        m_isTrue = 1;

        auto hitList = getHits(track);
        fillBaseHitInfo(hitList);
        currTree->Fill();
    }
}



void OutputManager::saveFakeTracks(std::vector<std::shared_ptr<FakeTrack>> trackList, TString fakeName)
{
    TTree* currTree = getTree(fakeName);

    for (const auto& track: trackList)
    {
        m_isTrue = 0;
        m_pt  = -999;
        m_eta = -999;
        m_phi = -999;
        m_d0  = -999;
        m_z0  = -999;
        m_truthProb = track->getTrackProbability(); 

        auto hitList = getHits(track);
        fillBaseHitInfo(hitList);
        currTree->Fill();
    }
}
std::vector<std::shared_ptr<Hit>> OutputManager::getHits(std::shared_ptr<TrackBase> trk)
{
    std::vector<std::shared_ptr<Hit>> hitList;
         if(m_hitType == OutHitType::Nominal)                   hitList = trk->getHitsList();
    else if(m_hitType == OutHitType::RotatedToZero)             hitList = trk->getHitsRotatedToZeroList();
    else if(m_hitType == OutHitType::RoughRotatedToZero)        hitList = trk->getHitsRoughRotatedToZeroList();
    else if(m_hitType == OutHitType::SP)                        hitList = trk->getSPHitsList();
    else if(m_hitType == OutHitType::SPWithRotatedToZero)       hitList = trk->getSPHitsRotatedToZeroList();
    else if(m_hitType == OutHitType::SPWithRoughRotatedToZero)  hitList = trk->getSPHitsRoughRotatedToZeroList();
    else if(m_hitType == OutHitType::SPWithUnscaledRotatedToZero)  hitList = trk->getSPHitsUnscaledRotatedToZeroList();
    else if(m_hitType == OutHitType::SPCylindrical)             hitList = trk->getSPHitsList();
    else if(m_hitType == OutHitType::SPCylindricalRotatedToZero)  hitList = trk->getSPHitsRotatedToZeroList();
    else                                                        hitList = trk->getHitsList(); 

    return hitList;
}

void OutputManager::saveTruthTrainingData(std::vector<std::shared_ptr<TrueTrack>> trackList, int window_size, bool barrelOnly, bool endcapOnly, bool cylindrical)
{
    if (barrelOnly && endcapOnly)
    {
        std::cout<<"cannot be both barrel onlyl and endcap only. Exitting"<<std::endl;
        exit(1);
    }
    // m_volumes_ITK = m_barrelVolumes_ITK;
    // m_volumes_ITK.insert(m_volumes_ITK.end(), m_endcapVolumes_ITK.begin(), m_endcapVolumes_ITK.end());
    // std::sort(m_volumes_ITK.begin(), m_volumes_ITK.end());
    
    // Format tracks for NN training. Input consists of track segments with target values as the next hit coordinates
    TTree* currTree = getTree("TrainingInputs");
    auto trackManager = make_shared<TrackManager>();
    auto seedManager = make_shared<SeedManager>();
    seedManager->setWindowSize(window_size);
    auto cleanTracks = trackManager->getTrackSingleLayerHit(trackList, window_size+1);
    auto inputTargetPair = seedManager->getSlidingWindowSeedPairs(cleanTracks);

    for (const auto& inpTar: inputTargetPair)
    {
        auto input = inpTar.first;
        auto target = inpTar.second;
        bool cont = false;
        if (barrelOnly)
        {
            if (std::find(m_barrelVolumes_ITK.begin(), m_barrelVolumes_ITK.end(), target->getVolume()) == m_barrelVolumes_ITK.end()) continue;
            for(const auto& hit: input->getHitsList())
            {
                if(std::find(m_barrelVolumes_ITK.begin(), m_barrelVolumes_ITK.end(), hit->getVolume()) == m_barrelVolumes_ITK.end()) cont=true;
            }
            if (cont) continue;
        }
        if (endcapOnly)
        {
            if (std::find(m_barrelVolumes_ITK.begin(), m_barrelVolumes_ITK.end(), target->getVolume()) != m_barrelVolumes_ITK.end()) continue;
            for(const auto& hit: input->getHitsList())
            {
                if(std::find(m_barrelVolumes_ITK.begin(), m_barrelVolumes_ITK.end(), hit->getVolume()) != m_barrelVolumes_ITK.end()) 
                {
                    cont = true;
                    break;
                }
            }
            if (cont) continue;
        }
        fillBaseHitInfo(input->getHitsList(), target, cylindrical);
        currTree->Fill();
    }
}

void OutputManager::fillBaseHitInfo(std::vector<std::shared_ptr<Hit>> hits)
{
    m_hitX->clear();
    m_hitY->clear();
    m_hitZ->clear();
    m_hitR->clear();
    m_hitPixorSct->clear();

    for(auto& var: m_floatVarList) var.second = 0;
    for(auto& var: m_intVarList)   var.second = 0;

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    for(const auto& hit: hits)
    {
        if(m_outputCylindrical)
        {
            m_hitX->push_back(hit->getR());
            m_hitY->push_back(hit->getPhi());
            m_hitZ->push_back(hit->getZ());
            m_hitR->push_back(hit->getRho());
            //m_hitPixorSct->push_back(hit->getPixorSct());
        }
        else
        {
            m_hitX->push_back(hit->getX() / xScale);
            m_hitY->push_back(hit->getY() / yScale);
            m_hitZ->push_back(hit->getZ() / zScale);
            m_hitR->push_back(hit->getRho());
            //m_hitPixorSct->push_back(hit->getPixorSct());
        }
    }
    for (size_t i = 0; i < hits.size(); i++)
    {
        if(m_outputCylindrical)
        {
            m_floatVarList["hitX" + TString::Itoa(i, 10)] = hits.at(i)->getR();  
            m_floatVarList["hitY" + TString::Itoa(i, 10)] = hits.at(i)->getPhi();
            m_floatVarList["hitZ" + TString::Itoa(i, 10)] = hits.at(i)->getZ();  
        }
        else
        {
            m_floatVarList["hitX" + TString::Itoa(i, 10)] = hits.at(i)->getX(); // xScale;
            m_floatVarList["hitY" + TString::Itoa(i, 10)] = hits.at(i)->getY(); // yScale;
            m_floatVarList["hitZ" + TString::Itoa(i, 10)] = hits.at(i)->getZ(); // zScale;
        }
        m_intVarList["detectorID" + TString::Itoa(i, 10)] = hits.at(i)->getVolume();
        
        // Save volume and layer id as a category label
        // The categories are consecutive using an ascending sorted list of volume ids
        // std::vector<int>::iterator volumeItr = std::find(m_volumes_ITK.begin(), m_volumes_ITK.end(), hits.at(i)->getVolume());;
        // int volumeID = std::distance(m_volumes_ITK.begin(), volumeItr);
        // int layerID  = hits.at(i)->getLayer()/2;
        // m_intVarList["detectorID" + TString::Itoa(i, 10)] = volumeID;
        // m_intVarList["layerID" + TString::Itoa(i, 10)]    = layerID;

        // Save one hot encoded volume and layer id 
        auto volumeID = hits.at(i)->OneHotEncodeVolumeID(m_volumes_ITK);
        auto layerID  = hits.at(i)->OneHotEncodeLayerID(30);
        volumeID.insert( volumeID.end(), layerID.begin(), layerID.end() );
        m_vectorVarList["hitDet"+ TString::Itoa(i, 10)] = volumeID;

        // Save detector and layer id per hit
        //m_intVarList["detectorID" + TString::Itoa(i, 10)] = hits.at(i)->getVolume();
        //m_intVarList["detectorID" + TString::Itoa(i, 10)] = hits.at(i)->getIntMetadata("isBarrel");
        //m_intVarList["layerID" + TString::Itoa(i, 10)]    = hits.at(i)->getLayer();
        //m_intVarList["pixOrsct" + TString::Itoa(i, 10)]     = hits.at(i)->getPixorSct();
    }
}

void OutputManager::fillBaseHitInfo(std::vector<std::shared_ptr<Hit>> inputHits, std::shared_ptr<Hit> targetHit, bool cylindrical)
{
    m_hitX->clear();
    m_hitY->clear();
    m_hitZ->clear();
    m_hitR->clear();
    m_hitPixorSct->clear();

    for(auto& var: m_floatVarList) var.second = 0;
    for(auto& var: m_intVarList)   var.second = 0;

    float xScale = Hit::getXScale();
    float yScale = Hit::getYScale();
    float zScale = Hit::getZScale();

    if (cylindrical)
    {
        for (size_t i = 0; i < inputHits.size(); i++)
        {
            m_floatVarList["hitX" + TString::Itoa(i, 10)] = inputHits.at(i)->getX() / xScale;
            m_floatVarList["hitY" + TString::Itoa(i, 10)] = inputHits.at(i)->getY() / yScale;
            m_floatVarList["hitZ" + TString::Itoa(i, 10)] = inputHits.at(i)->getZ() / zScale;
        }
        m_targetFloatVarList["tarHitX"] = targetHit->getX() / xScale;
        m_targetFloatVarList["tarHitY"] = targetHit->getY() / 3.15;
        m_targetFloatVarList["tarHitZ"] = targetHit->getZ() / zScale;
    }

    else
    {
        for (size_t i = 0; i < inputHits.size(); i++)
        {
            m_floatVarList["hitX" + TString::Itoa(i, 10)] = inputHits.at(i)->getX() / xScale;
            m_floatVarList["hitY" + TString::Itoa(i, 10)] = inputHits.at(i)->getY() / yScale;
            m_floatVarList["hitZ" + TString::Itoa(i, 10)] = inputHits.at(i)->getZ() / zScale;
        }
        m_targetFloatVarList["tarHitX"] = targetHit->getX() / xScale;
        m_targetFloatVarList["tarHitY"] = targetHit->getY() / yScale;
        m_targetFloatVarList["tarHitZ"] = targetHit->getZ() / zScale;
    }

    for (size_t i = 0; i < inputHits.size(); i++)
    {
        auto volumeID = inputHits.at(i)->OneHotEncodeVolumeID(m_volumes_ITK);
        auto layerID  = inputHits.at(i)->OneHotEncodeLayerID(30);
        volumeID.insert( volumeID.end(), layerID.begin(), layerID.end() );
        m_vectorVarList["hitDet"+ TString::Itoa(i, 10)] = volumeID;
    }
    auto volumeID = targetHit->OneHotEncodeVolumeID(m_volumes_ITK);
    auto layerID  = targetHit->OneHotEncodeLayerID(30);
    volumeID.insert( volumeID.end(), layerID.begin(), layerID.end() );
    m_targetVectorVarList["tarHitDet"] = volumeID;
}

// void OutputManager::fillSlidingWindowHitInfo(std::vector<TrueTrack> tracks, int window_size, bool onehotencode)
// {
//     auto seedManager = make_shared<SeedManager>();
//     seedManager->setSeedSize(window_size);
//     auto inputTargetPair =seedManager->getSlidingWindowSeedPairs(tracks);


// }

