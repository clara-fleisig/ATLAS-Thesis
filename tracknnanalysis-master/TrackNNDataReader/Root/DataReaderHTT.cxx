// Class includes
#include "TrackNNDataReader/DataReaderHTT.h"

// Local includes
// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <cmath>

// Root includes
#include <TLeaf.h>

using namespace std;


void DataReaderHTT::initialize()
{
    readGeo();


    m_inFile = TFile::Open(m_fileName);
    auto tree = (TTree*) m_inFile->Get("HTTHoughRootOutput");
    m_nEvents = tree->GetMaximum("treeindex");
    m_maxEntries = tree->GetEntries();
    m_HTReader = std::make_shared<HTTHoughRootOutput>(tree);

    tree = (TTree*) m_inFile->Get("HTTOfflineTree");
    m_offlineReader = std::make_shared<HTTOfflineTree>(tree);

    tree = (TTree*) m_inFile->Get("HTTTruthTree");
    m_truthReader = std::make_shared<HTTTruthTree>(tree);
    m_currIndex = 0;

}   


void DataReaderHTT::loadEntry(int iEvent)
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

    int localTrackIndex = 0;
    int localHitIndex = 0;


    double lwrEtaLim = 0;
    double uprEtaLim = 0;
    double lwrPhiLim = 0.3 + 0.02;
    double uprPhiLim = 0.5 - 0.02;
    if(m_LayerRegion.Contains("R0"))
    {
        lwrEtaLim = 0.1 + 0.02;
        uprEtaLim = 0.3 - 0.02;
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
    else
    {
        cout<<"Do not recongize the region: "<<m_LayerRegion<<endl;
        exit(1);
    }

    // initialize the Truth Tracks
    m_truthReader->getEntry(iEvent);
    int ntrueTracks = m_truthReader->truth_barcode().size();
    for(int i = 0; i < ntrueTracks; i++)
    {
        if(m_truthReader->truth_pt(i) < m_lwrPtLim * 1000) continue;
        if(m_truthReader->truth_pt(i) > m_uprPtLim * 1000) continue;

        if(m_truthReader->truth_eta(i) < lwrEtaLim) continue;
        if(m_truthReader->truth_eta(i) > uprEtaLim) continue;
        if(m_truthReader->truth_phi(i) < lwrPhiLim) continue;
        if(m_truthReader->truth_phi(i) > uprPhiLim) continue;
        if(fabs(m_truthReader->truth_d0(i)) > 2) continue;
        if(fabs(m_truthReader->truth_z0(i)) > 150) continue;

        if(m_pileupOnly)
        {
            if(m_truthReader->truth_barcode(i) > 400000) continue;
        }

        if(m_muonOnly)
        {
            if(m_truthReader->truth_barcode(i) < 400000) continue;
        }

        auto track = make_shared<TrueTrack>
        (
            m_truthReader->truth_barcode(i),
            m_truthReader->truth_pt(i)/1000.,
            m_truthReader->truth_eta(i),
            m_truthReader->truth_phi(i),
            0, //m_truthReader->true_track_vertDistance(i),
            m_truthReader->truth_pdg(i),
            1, //status,
            -1, //m_truthReader->true_track_recoTrackIndex(i),
            vector<Index>{}
        );
        // std::cout<<"True track"<<std::endl;
        // track->printDebug();
        m_trueTrackList.push_back(track);
        m_trueTrackCache[i] = track;
    }


    m_offlineReader->getEntry(iEvent);
    // initialize the Truth Tracks
    int nOfflineTracks = m_offlineReader->offline_barcode().size();
    for(int i = 0; i < nOfflineTracks; i++)
    {
        if(m_offlineReader->offline_pt(i) < m_lwrPtLim * 1000) continue;
        if(m_offlineReader->offline_pt(i) > m_uprPtLim * 1000) continue;

        if(m_offlineReader->offline_eta(i) < lwrEtaLim) continue;
        if(m_offlineReader->offline_eta(i) > uprEtaLim) continue;
        if(m_offlineReader->offline_phi(i) < lwrPhiLim) continue;
        if(m_offlineReader->offline_phi(i) > uprPhiLim) continue;
        if(fabs(m_offlineReader->offline_d0(i)) > 2) continue;
        if(fabs(m_offlineReader->offline_z0(i)) > 150) continue;
        if(m_pileupOnly)
        {
            if(m_offlineReader->offline_barcode(i) > 400000) continue;
        }
        if(m_muonOnly)
        {
            if(m_offlineReader->offline_barcode(i) < 400000) continue;
        }
        auto track = make_shared<TrueTrack>
        (
            m_offlineReader->offline_barcode(i),
            m_offlineReader->offline_pt(i)/1000.,
            m_offlineReader->offline_eta(i),
            m_offlineReader->offline_phi(i),
            0, //m_floatVectorVar true_track_vertDistance(i),
            m_offlineReader->offline_barcode(i),
            1, //status,
            -1, // m_offlineReader->true_track_recoTrackIndex(i),
            vector<Index>{}
        );
        // track->printDebug();
        track->setZ0(m_offlineReader->offline_z0(i));
        track->setD0(m_offlineReader->offline_d0(i));
        track->addIntMetadata("charge", m_offlineReader->offline_q(i));

        // track->addIntMetadata("nstrip_holes",           m_offlineReader->offline_nstrip_holes(i));
        // track->addIntMetadata("nstrip_inertmaterial",   m_offlineReader->offline_nstrip_inertmaterial(i));
        // track->addIntMetadata("nstrip_measurement",     m_offlineReader->offline_nstrip_measurement(i));
        // track->addIntMetadata("nstrip_brempoint",       m_offlineReader->offline_nstrip_brempoint(i));
        // track->addIntMetadata("nstrip_scatterer",       m_offlineReader->offline_nstrip_scatterer(i));
        // track->addIntMetadata("nstrip_perigee",         m_offlineReader->offline_nstrip_perigee(i));
        // track->addIntMetadata("nstrip_outlier",         m_offlineReader->offline_nstrip_outlier(i));
        // track->addIntMetadata("nstrip_other",           m_offlineReader->offline_nstrip_other(i));
        // track->addIntMetadata("npix_holes",             m_offlineReader->offline_npix_holes(i));
        // track->addIntMetadata("npix_inertmaterial",     m_offlineReader->offline_npix_inertmaterial(i));
        // track->addIntMetadata("npix_measurement",       m_offlineReader->offline_npix_measurement(i));
        // track->addIntMetadata("npix_brempoint",         m_offlineReader->offline_npix_brempoint(i));
        // track->addIntMetadata("npix_scatterer",         m_offlineReader->offline_npix_scatterer(i));
        // track->addIntMetadata("npix_perigee",           m_offlineReader->offline_npix_perigee(i));
        // track->addIntMetadata("npix_outlier",           m_offlineReader->offline_npix_outlier(i));
        // track->addIntMetadata("npix_other",             m_offlineReader->offline_npix_other(i));
        // std::cout<<"Offline track"<<std::endl;
        // track->printDebug();
        m_offlineTrackList.push_back(track);
        m_offlineTrackCache[i] = track;
    }



    for(int cIndex = m_currIndex; cIndex < m_maxEntries; cIndex++)
    {
        // load the entry
        m_HTReader->getEntry(cIndex);

        // If the current index is changes then we have read the event and we go break and go to the next one
        if(m_HTReader->treeindex() != iEvent)
        {
            m_currIndex = cIndex;
            break;
        }

        // initialize the hits
        int nHits = m_HTReader->x().size();


        if(m_pileupOnly)
        {
            bool drop = false;
            for(int i = 0; i < nHits; i++)
            {
                if(m_HTReader->barcode(i) > 400000) 
                {
                    drop = true;
                    break;
                }
            }
            if(drop) continue;
        }

        // Units mismatch
        double pt = fabs(1./(m_HTReader->invpt()));
        if((pt < m_lwrPtLim) && (m_lwrPtLim > 1)) continue;
        if(pt > m_uprPtLim) continue;


        int nPixHits = 0;
        int nSCTHits = 0;
        vector<Index> hitIndex = {};
        for(int i = 0; i < nHits; i++)
        {
            if(m_HTReader->isPixel(i)) nPixHits++;
            else  nSCTHits++;
            // If we want to drop pixel hits
            if((m_dropPixHit) && (m_HTReader->isPixel(i) == 1) ) continue;

            double x = m_HTReader->x(i);
            double y = m_HTReader->y(i);
            double z = m_HTReader->z(i);
            double r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
            double rho = sqrt(pow(x, 2) + pow(y, 2));

            if(x == 0) continue;

            // opposite logic between us and HTT
            int isPixOrSCT = 1;
            if(m_HTReader->isPixel(i) == 1) isPixOrSCT = 0;

            auto hit = make_shared<Hit>
            (
                localHitIndex,
                x,
                y,
                z,
                r,
                rho,
                m_HTReader->isBarrel(i),
                m_HTReader->layer(i),
                m_HTReader->ID(i),
                m_HTReader->etamodule(i),
                m_HTReader->phimodule(i),
                isPixOrSCT,
                0, // strip module side
                0, // Track index
                vector<Index>{(Index)m_HTReader->barcode(i)}
            );
            m_hitList.push_back(hit);
            m_hitCache[localHitIndex] = hit;
            hitIndex.push_back(localHitIndex);
            localHitIndex++;
        }   
        int charge = 1;
        if(signbit(m_HTReader->invpt())) charge = -1;

        // initialize the Reco Tracks
        auto track = make_shared<RecoTrack>
        (
            localTrackIndex, // track index
            pt, // pt
            0, //eta
            m_HTReader->phi(),
            m_HTReader->candidate_barcodefrac(), // truthProb
            0, // d0
            0, // z0
            m_HTReader->phi(),
            0, // theta
            m_HTReader->invpt(), //q/p
            charge, // charge,
            nPixHits,
            nSCTHits,
            0, // truth barcode
            hitIndex   
        );

        track->addIntMetadata("tracknumber", m_HTReader->tracknumber());
        track->addIntMetadata("roadnumber",  m_HTReader->roadnumber());
        m_recoTrackList.push_back(track);
        m_recoTrackCache[localTrackIndex] = track;
        localTrackIndex++;
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
        createSP(dynamic_pointer_cast<TrackBase, RecoTrack>(track));    

        // // set the truth track
        // auto truthBarcode = track->getTrueTrackBarcode();
        // if(m_trueTrackCache.find(truthBarcode) != m_trueTrackCache.end()) track->setOrgTrueTrack(m_trueTrackCache.at(truthBarcode));
        // track->printDebug();
    }
}

   

void DataReaderHTT::createSP(std::shared_ptr<TrackBase> trk)
{
    static Index SPIndex = 0;

    auto hits = trk->getHitsList();

    std::vector<std::shared_ptr<Hit>> spHits;
    std::map<int, std::vector<std::shared_ptr<Hit>>> spHitMap;
    std::vector<int> layersDone;
    for(auto& hit: hits)
    {
        int layer = hit->getLayer();

        int index = 0;
        // Had code for this now
             if(layer == 0)               index = 0;
        else if(layer == 1 || layer == 2) index = 1;
        else if(layer == 3 || layer == 4) index = 2;
        else if(layer == 5 || layer == 6) index = 3;
        else if(layer == 7 || layer == 8) index = 4;

        spHitMap[index].push_back(hit);
    }
    for(const auto& hitVec: spHitMap)
    {
        int nHits =  hitVec.second.size();
        if(nHits == 1) 
        {
            spHits.push_back(hitVec.second.at(0));
            continue;
        }

        double xAve = 0;
        double yAve = 0;
        double zAve = 0;


        for(const auto& hit: hitVec.second)
        {
            if(hit->getX() == 0)
            {
                nHits--;
                continue;
            }
            xAve += hit->getX();
            yAve += hit->getY();
            zAve += hit->getZ();

        }

        xAve /= nHits;
        yAve /= nHits;
        zAve /= nHits;
        std::shared_ptr<Hit> egHit = hitVec.second.at(0);

        auto spacepoint = make_shared<Hit> (
                 SPIndex,
                 xAve,
                 yAve,
                 zAve,
                 std::sqrt(xAve*xAve + yAve*yAve + zAve*zAve),
                 std::sqrt(xAve*xAve + yAve*yAve),
                 egHit->getVolume(),
                 egHit->getLayer(),
                 egHit->getModule(),
                 egHit->getEtaModule(),
                 egHit->getPhiModule(),
                 egHit->getPixorSct(),
                 -1,
                 egHit->getRecoTrackIndex(),
                 egHit->getTruthTrackBarcode()
                );

        spHits.push_back(spacepoint);
        SPIndex++;
    }
    trk->setSPHits(spHits);
}



















