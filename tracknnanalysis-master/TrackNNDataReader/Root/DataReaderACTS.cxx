// Class includes
#include "TrackNNDataReader/DataReaderACTS.h"

// Local includes
// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <cmath>
#include <math.h>
#include <set>
// Root includes
#include <TLeaf.h>

using namespace std;


void DataReaderACTS::initialize()
{   
    readGeo();

    m_inFile = TFile::Open(m_fileName);


    auto tree = (TTree*) m_inFile->Get("roads");
    m_roads = std::make_shared<RoadReaderClass>(tree);

    tree = (TTree*) m_inFile->Get("tracks");
    m_tracks = std::make_shared<TrackReaderClass>(tree);

    tree = (TTree*) m_inFile->Get("hits");
    m_hits = std::make_shared<HitReaderClass>(tree);

    tree = (TTree*) m_inFile->Get("particles_initial");
    m_truthTracks = std::make_shared<TruthTracksReaderClass>(tree);

    tree = (TTree*) m_inFile->Get("tracksummary_ckf-reconstruced-seeds");
    m_recoTracks = std::make_shared<RecoTracksReaderClass>(tree);

    TTree* roads = (TTree*) m_inFile->Get("roads");
    m_nEvents = roads->GetEntries();
}   


void DataReaderACTS::loadEntry(int iEvent)
{
    // Load the entry and process into a list of tracks and hits
    // m_inFile->cd();

    // Clear the memory first
    m_hitList       .clear();
    m_recoTrackList .clear();
    m_hitCache      .clear();
    m_recoTrackCache.clear();
    m_trueTrackList .clear();
    m_trueTrackCache.clear();
    m_offlineTrackList .clear();
    m_offlineTrackCache.clear();

    double lwrEtaLim = 0;
    double uprEtaLim = 0;
    double lwrPhiLim = -10000;
    double uprPhiLim =  10000;
    double Z0Lim = 10000;
    double D0Lim = 10000;
    //std::cout<<"m_LayerRegion: "<<m_LayerRegion<<std::endl;
    if(m_LayerRegion.Contains("R0"))
    {
        lwrEtaLim = 0.08;
        uprEtaLim = 0.32;
    }
    else if(m_LayerRegion.Contains("R1"))
    {
        lwrEtaLim = 0.7 + 0.02;
        uprEtaLim = 0.9 - 0.02;
    }
    else if(m_LayerRegion.Contains("R3"))
    {
        lwrEtaLim = 2.0 + 0.02;
        uprEtaLim = 2.2 - 0.02;
    }
    else if(m_LayerRegion.Contains("noCut"))
    {
        lwrEtaLim = -10000;
        uprEtaLim = 100000;

        lwrPhiLim = -10000;
        uprPhiLim = 100000;

    }
    else if(m_LayerRegion.Contains("barrel"))
    {
        lwrEtaLim = -0.7;
        uprEtaLim = 0.7;
        Z0Lim = 150;
        D0Lim = 2;
    }
    else if(m_LayerRegion.Contains("endcaps"))
    {
        lwrEtaLim = 2;
        uprEtaLim = 10000;
        Z0Lim = 150;
        D0Lim = 2;
    }
    else
    {
        cout<<"Do not recongize the region: "<<m_LayerRegion<<endl;
        exit(1);
    }

    m_roads->getEntry(iEvent);
    m_tracks->getEntry(iEvent);
    unsigned int eventID = m_roads->event_id();


    // Do the truth tracks first so that we can reject/only accept muons, if needed
    // Get the truth tracks
    m_truthTracks->getEntry(iEvent);
    if(eventID != m_truthTracks->event_id())
    {
        std::cout<<"Truth event id doesn' match road one"<<std::endl;
        std::cout<<"road_event_id: "<<eventID<<std::endl;
        std::cout<<"truth_event_id: "<<m_truthTracks->event_id()<<std::endl;
        exit(1);
    }

    std::set<Index> rejectedBarcode;
    std::set<Index> rejectedOverlapBarcode;
    int nTracks = m_truthTracks->pt().size();
    for(int i = 0; i < nTracks; i++)
    {

        if(m_truthTracks->pt(i) < m_lwrPtLim) continue;
        if(m_truthTracks->pt(i)  > m_uprPtLim) continue;

        if(m_truthTracks->phi(i)  < lwrPhiLim) continue;
        if(m_truthTracks->phi(i) > uprPhiLim) continue;

        // if(fabs(m_floatVectorVar.at("truth_d0")->At(i)) > 2) continue;
        //if(m_floatVectorVar.at("truth_eta")->At(i) < lwrEtaLim) continue;
        if(fabs(m_truthTracks->eta(i)) < lwrEtaLim) continue;
        if(fabs(m_truthTracks->eta(i)) > uprEtaLim) continue;
        if(fabs(m_truthTracks->vz(i)) > Z0Lim) continue;

        // std::cout<<"muonOnly: "<<m_muonOnly<<" pileupOnly: "<<m_pileupOnly<<std::endl;
        // Rejecting not muons
        if(m_muonOnly && fabs(m_truthTracks->particle_type(i)) != 13)
        {
            rejectedBarcode.insert(m_truthTracks->particle_id(i));
            continue;
        }
        if(m_pileupOnly && fabs(m_truthTracks->particle_type(i)) == 13)
        {
            rejectedBarcode.insert(m_truthTracks->particle_id(i));
            continue;            
        }

        auto track = make_shared<TrueTrack>
        (
            m_truthTracks->particle_id(i),
            m_truthTracks->pt(i),
            m_truthTracks->eta(i),
            m_truthTracks->phi(i),
            0, //m_floatVectorVar.at("true_track_vertDistance")->At(i),
            m_truthTracks->particle_type(i),
            1, //status,
            0, // m_intVectorVar.at("true_track_recoTrackIndex")->At(i),
            vector<Index>{0}
        );
        // track->printDebug();
        track->setZ0(m_truthTracks->vz(i));
        m_trueTrackList.push_back(track);
        m_trueTrackCache[i] = track;
    }

    // Overlap removal, pick the highest pT track
    std::vector<std::shared_ptr<TrueTrack>> t_trueTrackList;
    static auto deltaRCal= [](double eta_1, double phi_1, double eta_2, double phi_2)
    {
        double dR=0;
        double eta2 = (eta_1-eta_2)*(eta_1-eta_2);
        double tmp_dphi = (fabs(phi_1-phi_2) > M_PI) ? 2*M_PI-fabs(phi_1-phi_2) : fabs(phi_1-phi_2);
        double phi2 = tmp_dphi*tmp_dphi;
        dR = sqrt( eta2 + phi2 );
        return dR;
    };
    for(const auto& trk: m_trueTrackList)
    {
        bool overlap = false;
        for(const auto& trkToCheck: m_trueTrackList)
        {
            if (trk->getBarcode() == trkToCheck->getBarcode()){continue;}
            // < .025 too small
            if(deltaRCal(trk->getEta(), trk->getPhi(), trkToCheck->getEta(), trkToCheck->getPhi()) < m_deltaROverlap
            && (trkToCheck->getPt() > trk->getPt()))
            {
                rejectedOverlapBarcode.insert(trk->getBarcode());
                overlap = true;
                break;
            }
        }
        if(!overlap) t_trueTrackList.push_back(trk);
    }
    // Overwrite it with the overlap removed list
    m_trueTrackList = t_trueTrackList;

    //std::cout<<"m_trueTrackList size: "<<m_trueTrackList.size()<<std::endl;
    m_hits->getEntry(iEvent);

    if(eventID != m_hits->event_id())
    {
        std::cout<<"Hit event id doesn' match road one"<<std::endl;
        std::cout<<"road_event_id: "<<eventID<<std::endl;
        std::cout<<"hit_event_id: "<<m_hits->event_id()<<std::endl;
        exit(1);
    }

    std::set<Index> rejectedHits;
    // Get all the hits
    for(int i = 0; i < m_hits->nHits(); i++)
    {
        int localHitIndex = m_hits->index(i);

        
        // // TrackML geo
        // bool isBarrel = (m_hits->volume_id(i) == 8) || (m_hits->volume_id(i) == 13) || (m_hits->volume_id(i) == 17);
        // bool isPix = (m_hits->volume_id(i) == 7) || (m_hits->volume_id(i) == 8) || (m_hits->volume_id(i) == 9);

        // open detector
        // change isBarrel to isEndcap to be consistent with the neural network training data. To change back simply remove !
        //bool isBarrel = ((m_hits->volume_id(i) == 17) || (m_hits->volume_id(i) == 24) || (m_hits->volume_id(i) == 29));
        bool isBarrel = std::find(m_barrelVolumes.begin(), m_barrelVolumes.end(), m_hits->volume_id(i)) != m_barrelVolumes.end();
        bool isPix = (m_hits->volume_id(i) == 16) || (m_hits->volume_id(i) == 17) || (m_hits->volume_id(i) == 18);
        
        if(rejectedOverlapBarcode.find(m_hits->particle_id(i)) != rejectedOverlapBarcode.end())
        {
                rejectedHits.insert(localHitIndex);
                continue;
        }
        if(m_pileupOnly)
        {
            if(rejectedBarcode.find(m_hits->particle_id(i)) != rejectedBarcode.end())
            {
                rejectedHits.insert(localHitIndex);
                continue;
            }
        }

        auto hit = make_shared<Hit>
        (
            localHitIndex,
            m_hits->tx(i),
            m_hits->ty(i),
            m_hits->tz(i),
            sqrt(pow(m_hits->tx(i),2) + pow(m_hits->ty(i),2) + pow(m_hits->tz(i),2)),
            sqrt(pow(m_hits->tx(i),2) + pow(m_hits->ty(i),2) ),
            // isBarrel,
            m_hits->volume_id(i),
            m_hits->layer_id(i),
            m_hits->sensitive_id(i),
            0, //eta module
            0, //phi module
            isPix,
            0, // strip module side
            0, // Track index
            vector<Index>{m_hits->particle_id(i)}
        );


        hit->addIntMetadata("isBarrel", isBarrel);
        // hit->printDebug();
        m_hitList.push_back(hit);
        m_hitCache[localHitIndex] = hit;
    }


    // reco seeds
    // std::cout<<"Reco Summary ---------------"<<std::endl;
    m_recoTracks->getEntry(iEvent);
    if(eventID != m_recoTracks->event_nr())
    {
        std::cout<<"Reco Seeds id doesn' match road one"<<std::endl;
        std::cout<<"road_event_id: "<<eventID<<std::endl;
        std::cout<<"recoSeed_event_nr: "<<m_recoTracks->event_nr()<<std::endl;
        exit(1);
    }
    int nRecoTracks = m_recoTracks->eLOC0_fit().size();
    for(int i = 0; i < nRecoTracks; i++)
    {
        double pt =  fabs(sin(m_recoTracks->eTHETA_fit(i))/m_recoTracks->eQOP_fit(i));
        double eta = -log(tan(m_recoTracks->eTHETA_fit(i)/2));
        double phi = m_recoTracks->ePHI_fit(i);

        if(pt < m_lwrPtLim) continue;
        if(pt > m_uprPtLim) continue;

        // if(eta < lwrEtaLim) continue;
        // if(eta > uprEtaLim) continue;

        if(fabs(eta) < lwrEtaLim) continue;
        if(fabs(eta) > uprEtaLim) continue;

        if(phi < lwrPhiLim) continue;
        if(phi > uprPhiLim) continue;
        if(fabs(m_recoTracks->eLOC1_fit(i)) > Z0Lim) continue;
        if(fabs(m_recoTracks->eLOC0_fit(i)) > D0Lim) continue;

        if(m_recoTracks->nMeasurements(i) < 5) continue;
        vector<Index> truthHitIndex;

        bool rejectTrack = false;
        for(unsigned int k = 0 ; k < m_recoTracks->t_x(i).size(); k++)
        {
            auto index = (Index)m_recoTracks->t_index(i).at(k);
            if(m_pileupOnly)
            {
                if(rejectedHits.find(index) != rejectedHits.end())
                {
                    rejectTrack = true;
                    break;
                }
            }
            truthHitIndex.push_back(index);
        }

        if(rejectedOverlapBarcode.find(m_recoTracks->majorityParticleId(i)) != rejectedOverlapBarcode.end())
        {
                rejectTrack = true;
        }

        if(m_muonOnly)
        {
            if(rejectedBarcode.find(m_recoTracks->majorityParticleId(i)) != rejectedBarcode.end()) rejectTrack = true;
        }

        if(rejectTrack) continue;

        auto track = make_shared<TrueTrack>
        (
            m_recoTracks->majorityParticleId(i),
            pt,
            eta,
            phi,
            m_recoTracks->eLOC0_fit(i),
            m_recoTracks->majorityParticleId(i),
            1, //status,
            0, // m_intVectorVar.at("true_track_recoTrackIndex")->At(i),
            truthHitIndex
        );

        track->setZ0(m_recoTracks->eLOC1_fit(i));
        track->setD0(m_recoTracks->eLOC0_fit(i));
        int charge = 1;
        if(signbit(m_recoTracks->eQOP_fit(i))) charge = -1;
        track->addIntMetadata("charge", signbit(charge));
        track->addIntMetadata("nMeas", m_recoTracks->nMeasurements(i));
        track->addIntMetadata("nOutlier", m_recoTracks->nOutliers(i));
        track->addIntMetadata("nOther", m_recoTracks->nHoles(i));

        track->setTrackProbability(m_recoTracks->nMajorityHits(i)/(float)(m_recoTracks->nMeasurements(i)));
        track->setComputedTrackProbability(m_recoTracks->nMajorityHits(i)/(float)(m_recoTracks->nMeasurements(i)));
        // track->printDebug();
        m_offlineTrackList.push_back(track);
        m_offlineTrackCache[i] = track;
    }

    std::vector<std::shared_ptr<TrueTrack>> t_offlineTrackList;
    for(const auto& trk: m_offlineTrackList)
    {
        bool overlap = false;
        for(const auto& trkToCheck: m_offlineTrackList)
        {
            if (trk->getBarcode() == trkToCheck->getBarcode()){continue;}
            // < .025 too small
            if(deltaRCal(trk->getEta(), trk->getPhi(), trkToCheck->getEta(), trkToCheck->getPhi()) < m_deltaROverlap
            && (trkToCheck->getPt() > trk->getPt()))
            {
                //std::cout<<"tracks "<<trkToCheck->getBarcode()<<" , "<<trk->getBarcode()<<" are overlapping"<<std::endl;
                overlap = true;
                break;
            }
        }
        if(!overlap) t_offlineTrackList.push_back(trk);
    }
    // Overwrite it with the overlap removed list
    m_offlineTrackList = t_offlineTrackList;

    //std::cout<<"m_offlineTrackCache size: "<<m_offlineTrackCache.size()<<std::endl;


    int localRoadIndex = 0;
    int nTracksReco = m_tracks->nTracks();

    // caching to reduce the look up time to O(log n)
    std::map<int, float> road_cache_qpt;
    std::map<int, float> road_cache_phi;
    std::map<int, float> road_cache_eta;

     // Find the road
    for(int j = 0; j < m_roads->nRoads(); j++)
    {
        int index = m_roads->road_index(j);

        road_cache_qpt[index] = m_roads->road_qpt(j);
        road_cache_phi[index] = m_roads->road_phi(j);
        road_cache_eta[index] = m_roads->road_eta(j);
            
        
    }   
    // Get the hits in the road
    for(int i = 0; i < nTracksReco; i++)
    {    
        int roadIndex = m_tracks->track_road_index(i);
        float road_phi = road_cache_phi[roadIndex];
        float road_eta = road_cache_eta[roadIndex];
        float road_qp  = road_cache_qpt[roadIndex];


        std::vector<Index> track_hit_indices;

        bool rejectTrack = false;
        for(unsigned int k = 0 ; k < m_tracks->track_hit_indices(i).size(); k++)
        {
            auto index = (Index)m_tracks->track_hit_indices(i).at(k);
            if(m_pileupOnly)
            {
                if(rejectedHits.find(index) != rejectedHits.end())
                {
                    rejectTrack = true;
                    break;
                }
            }
            track_hit_indices.push_back(index);
        }
        if(rejectTrack) continue;

        int charge = 1;
        if(signbit(road_qp)) charge = -1;

        // initialize the Reco Tracks
        auto track = make_shared<RecoTrack>
        (
            m_tracks->track_index(i), // track index
            fabs(1/road_qp), // pt
            road_eta, //eta
            road_phi, //phi
            m_tracks->track_truth_match_fraction(i), // truthProb
            0, // d0
            0, // z0
            road_phi,
            0, // theta
            road_qp, //q/p
            charge, // charge,
            0, //nPixHits,
            0, //nSCTHits,
            m_tracks->track_truth_match_index(i), // truth barcode
            track_hit_indices
        );
        track->addIntMetadata("tracknumber", localRoadIndex);
        track->addIntMetadata("roadnumber",  roadIndex);
        m_recoTrackList.push_back(track);
        m_recoTrackCache[localRoadIndex] = track;
        localRoadIndex++;
    }



    // create the internal the references
    // Reco track
    for (auto& track: m_recoTrackList)
    {
        auto hitIndex = track->getOrgHitIndex();
        std::vector<std::shared_ptr<Hit>> hits;
        // find the associated hits
        for(const auto& index: hitIndex)
        {
            if(m_hitCache.find(index) != m_hitCache.end()) hits.push_back(m_hitCache.at(index));
        }

        track->setOrgHits(hits);
    }

    for (auto& track: m_offlineTrackList)
    {
        auto hitIndex = track->getOrgHitIndex();
        std::vector<std::shared_ptr<Hit>> hits;
        // find the associated hits
        for(const auto& index: hitIndex)
        {
            if(m_hitCache.find(index) != m_hitCache.end()) hits.push_back(m_hitCache.at(index));
        }

        track->setOrgHits(hits);
    }


    // remove duplicates
    // Order the track by their pT score
    sort(m_offlineTrackList.begin(), m_offlineTrackList.end(), 
    [](const std::shared_ptr<TrueTrack> & a, const std::shared_ptr<TrueTrack> & b) -> bool
    { 
        return a->getPt() > b->getPt(); 
    });

    std::set<int> indexToRemove;
    for(unsigned int i = 0; i < m_offlineTrackList.size(); i++)
    {
        // This has already been removed, don't worry about it
        if(indexToRemove.find(i) != indexToRemove.end()) continue;

        for(unsigned int j = i+1; j < m_offlineTrackList.size(); j++)
        {
            auto nOverlapHits = m_offlineTrackList[j]->getNHitOverlap(m_offlineTrackList[i]);
            if(nOverlapHits >= 2) // Let only tracks that share 2 hits 
            {
                // Since it is already sorted, just reject jth track
                indexToRemove.insert(j);
            }
        }
    }

    // Reset the selected tracks and add all that haven't been rejected
    decltype(m_offlineTrackList) offlineCleanTrackList;
    for(unsigned int i = 0; i < m_offlineTrackList.size(); i++)
    {
        if(indexToRemove.find(i) != indexToRemove.end()) continue;
        offlineCleanTrackList.push_back(m_offlineTrackList[i]);
        // m_offlineTrackList[i]->printDebug();
    }
    m_offlineTrackList = offlineCleanTrackList;



    // Creating the space points - For ACTS geo, there is currently no layers, so we just copy the org hits
    for(auto& trk: m_recoTrackList)    trk->setSPHits(trk->getHitsList());
    for(auto& trk: m_trueTrackList)    trk->setSPHits(trk->getHitsList());
    for(auto& trk: m_offlineTrackList) trk->setSPHits(trk->getHitsList());

}

void DataReaderACTS::readGeo()
{
    // read the detector geometry first
    m_inGeoFile = TFile::Open(m_geofileName);
    m_readerList["geoFile"]   = new TTreeReader("DetectorGeo",     m_inGeoFile);

    fillVariableList(m_readerList["geoFile"]);

    
    int nMod = m_readerList["geoFile"]->GetEntries(true);


    for(int i = 0; i < nMod; i++)
    {
        m_readerList["geoFile"]->SetEntry(i);
        auto mod = make_shared<DetectorModule>
        (
            **m_UlonglongVar.at("module_index"),
            **m_intVar.at("module_volumeId"),
            **m_intVar.at("module_layerId"),
            **m_intVar.at("module_moduleId"),
            **m_intVar.at("module_PixorSCT"),
            **m_floatVar.at("module_center_x"),
            **m_floatVar.at("module_center_y"),
            **m_floatVar.at("module_center_z"),
            **m_floatVar.at("module_center_r"),
            **m_floatVar.at("module_center_rho"),
            **m_floatVar.at("module_rot_xu"),
            **m_floatVar.at("module_rot_xv"),
            **m_floatVar.at("module_rot_xw"),
            **m_floatVar.at("module_rot_yu"),
            **m_floatVar.at("module_rot_yv"),
            **m_floatVar.at("module_rot_yw"),
            **m_floatVar.at("module_rot_zu"),
            **m_floatVar.at("module_rot_zv"),
            **m_floatVar.at("module_rot_zw"),
            **m_floatVar.at("module_thickness"),
            **m_floatVar.at("module_minhu"),
            **m_floatVar.at("module_maxhu"),
            **m_floatVar.at("module_hv"),
            **m_floatVar.at("module_pitch_u"),
            **m_floatVar.at("module_pitch_v")
        );
        
        mod->addIntMetadata("isBarrel", 
            std::find(m_barrelVolumes.begin(), m_barrelVolumes.end(), **m_intVar.at("module_volumeId")) != m_barrelVolumes.end());
        m_detectorModule.push_back(mod);
        m_detectorModuleCache[**m_UlonglongVar.at("module_index")] = mod;

    }

}




void DataReaderACTS::predictNextVolumeAndLayer(std::shared_ptr<Hit> hit, std::vector<std::pair<int, int>>& nextID)
{
    auto layerID = hit->getLayer();
    auto volID = hit->getVolume();

    // pix barrel
    if(volID == 17)
    {   
        // Extrapolations to the next layer in the barrel
        if(layerID <= 6) nextID.push_back({volID, layerID+2});
        // This will go into the lowest layer in the barrel
        if(layerID == 8) nextID.push_back({24, 2});

        // // Postive and negative endcaps
        // // The hit can potentially escape to the endcap
        // if(hit->getZ() > 500)   nextID.push_back({18, 2}); 
        // if(hit->getZ() < -500)  nextID.push_back({16, 16}); 
        

    }
    // Short strip
    if(volID == 24)
    {
        // Extrapolations to the next layer in the barrel
        if(layerID <= 6) nextID.push_back({volID, layerID+2});
        // This will go into the lowest layer in the barrel
        if(layerID == 8) nextID.push_back({29, 2});

        // // Postive and negative endcaps
        // // The hit can potentially escape to the endcap
        // if(hit->getZ() > 750)   nextID.push_back({25, 2}); 
        // if(hit->getZ() < -750)  nextID.push_back({23, 14}); 

        // if(layerID == 8)
        // {
        //     if(hit->getZ() > 750)   nextID.push_back({30, 2}); 
        //     if(hit->getZ() < -750)  nextID.push_back({28, 14}); 
        // }
    }
    // Long strip
    if(volID == 29)
    {
        // Extrapolations to the next layer in the barrel
        if(layerID <= 2) nextID.push_back({volID, layerID+2});
        // This will go into the lowest layer in the barrel
        // if(volID == 4) This is outside of the last layer 
        // if(layerID <= 8)
        // {
        //     if(hit->getZ() > 750)   nextID.push_back({30, 2}); 
        //     if(hit->getZ() < -750)  nextID.push_back({28, 14}); 
        // }

    }


    // -ve pix EC
    if(volID == 16)
    {   
        // -ve pixel barrel is weird... goes from 16->4
        if(layerID >= 6) nextID.push_back({volID, layerID-2});
    }
    // -ve Short strip EC
    if(volID == 23)
    {
        // Layers go 14 -> 2
        if(layerID >= 4) nextID.push_back({volID, layerID-2});
    }
    // -ve Long strip EN
    if(volID == 28)
    {
        // Layers go 14 -> 2
        if(layerID >= 4 ) nextID.push_back({volID, layerID-2});
    }

    // +ve pix EC
    if(volID == 18)
    {   
        // layers go 2 -> 14
        if(layerID <= 12) nextID.push_back({volID, layerID+2});
    }
    // +ve Short strip EC
    if(volID == 25)
    {
        // layers go 2 -> 12
        if(layerID <= 10) nextID.push_back({volID, layerID+2});
    }
    // +ve Long strip EN
    if(volID == 30)
    {
        // layers go 2 -> 12
        if(layerID <= 10 ) nextID.push_back({volID, layerID+2});
    }



}














