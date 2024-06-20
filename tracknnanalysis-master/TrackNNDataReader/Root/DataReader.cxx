// Class includes
#include "TrackNNDataReader/DataReader.h"
#include "NtupleAnalysisUtils/NTAUTopLevelIncludes.h"

// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>

// Root includes
#include <TLeaf.h>

using namespace std;


// Constructors and destructor
DataReader::DataReader(TString fileName, TString geoFileName):
m_fileName(fileName),
m_geofileName(geoFileName)
{
    m_inFile        = NULL;
    m_inGeoFile     = NULL;
    m_dropPixHit    = false;
    m_LayerRegion = "";
    m_nLayerThreshold = -1;

    m_lwrEtaLim = -10;
    m_uprEtaLim = 10;
} 

DataReader::~DataReader()
{

} 

void DataReader::initialize()
{
    readGeo();
    m_inFile = TFile::Open(m_fileName);
    
    auto tree = (TTree*) m_inFile->Get("RecoTracks");
    m_recoTracksReader = std::make_shared<RecoTracksATLAS>(tree);

    tree = (TTree*) m_inFile->Get("HitInfo");
    m_nEvents = tree->GetEntries();
    m_hitsReader = std::make_shared<HitInfoATLAS>(tree);

    tree = (TTree*) m_inFile->Get("TruthTracks");
    m_truthTracksReader = std::make_shared<TruthTracksATLAS>(tree);

    tree = (TTree*) m_inFile->Get("SPInfo");
    m_SPReader = std::make_shared<SPInfoATLAS>(tree);
}   

void DataReader::fillVariableList(TTreeReader* reader, TString preFix)
{
    // Dynamically read in the variable list from ttreereader and initialize the read memory for it
    TTree* tree = reader->GetTree();

    TObjArray* listLf = tree->GetListOfLeaves();

    for (Int_t i = 0; i < listLf->GetEntries(); i++)
    {
        TLeaf* lf = (TLeaf*) listLf->At(i);
        TString typeName   = lf->GetTypeName();
        TString titleName  = lf->GetTitle();

             if(typeName.EqualTo("vector<int>"))                            m_intVectorVar[preFix + titleName]                  = new TTreeReaderArray<int>                 (*reader, titleName);
        else if(typeName.EqualTo("vector<bool>"))                           m_boolVectorVar[preFix + titleName]                 = new TTreeReaderArray<bool>                (*reader, titleName);     
        else if(typeName.EqualTo("vector<unsigned int>"))                   m_UintVectorVar[preFix + titleName]                 = new TTreeReaderArray<unsigned int>        (*reader, titleName);     
        else if(typeName.EqualTo("vector<unsigned long>"))                  m_UlongVectorVar[preFix + titleName]                = new TTreeReaderArray<unsigned long>       (*reader, titleName);     
        else if(typeName.EqualTo("vector<ULong64_t>"))                      m_UlonglongVectorVar[preFix + titleName]            = new TTreeReaderArray<unsigned long long>  (*reader, titleName);     
        else if(typeName.EqualTo("vector<float>"))                          m_floatVectorVar[preFix + titleName]                = new TTreeReaderArray<float>               (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<int> >"))                   m_intVectorVectorVar[preFix + titleName]            = new TTreeReaderArray<std::vector<int>>    (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<double> >"))                m_doubleVectorVectorVar[preFix + titleName]         = new TTreeReaderArray<std::vector<double>> (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<unsigned int> >"))          m_UintVectorVectorVar[preFix + titleName]           = new TTreeReaderArray<std::vector<unsigned int>> (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<vector<unsigned int> > >")) m_UintVectorVectorVectorVar[preFix + titleName]     = new TTreeReaderArray<std::vector<std::vector<unsigned int>>> (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<vector<int> > >"))          m_intVectorVectorVectorVar[preFix + titleName]      = new TTreeReaderArray<std::vector<std::vector<int>>> (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<vector<float> > >"))        m_floatVectorVectorVectorVar[preFix + titleName]    = new TTreeReaderArray<std::vector<std::vector<float>>> (*reader, titleName);
        else if(typeName.EqualTo("vector<vector<vector<bool> > >"))         m_boolVectorVectorVectorVar[preFix + titleName]     = new TTreeReaderArray<std::vector<std::vector<bool>>> (*reader, titleName);
        else if(typeName.EqualTo("Int_t"))                                  m_intVar[preFix + titleName]                        = new TTreeReaderValue<int>                 (*reader, titleName);
        else if(typeName.EqualTo("Float_t"))                                m_floatVar[preFix + titleName]                      = new TTreeReaderValue<float>               (*reader, titleName);
        else if(typeName.EqualTo("UInt_t"))                                 m_UintVar[preFix + titleName]                       = new TTreeReaderValue<unsigned int>        (*reader, titleName);
        else if(typeName.EqualTo("ULong64_t"))                              m_UlonglongVar[preFix + titleName]                  = new TTreeReaderValue<unsigned long long>  (*reader, titleName);
        else
        {
            cout<<"Don't recognize: "<<typeName<<" "<<titleName<<" prefix:"<< preFix<<endl;
            // exit(1);
        }        
    }
}

void DataReader::loadEntry(int iEvent)
{
    // Load the entry and process into a list of tracks and hits
    // m_inFile->cd();


    // Clear the memory first
    m_hitList      .clear();
    m_SPList       .clear();
    m_recoTrackList.clear();
    m_trueTrackList.clear();
    m_hitCache      .clear();
    m_SPCache       .clear();
    m_recoTrackCache.clear();
    m_trueTrackCache.clear();
    m_HitIndexToSPCache.clear();
    m_RecoTrackIndexToSPCache.clear();
    m_TruthTrackIndexToSPCache.clear();


    // load the entry
    m_recoTracksReader->getEntry(iEvent);
    m_hitsReader->getEntry(iEvent);
    m_truthTracksReader->getEntry(iEvent);
    m_SPReader->getEntry(iEvent);

    // initialize the hits
    int nHits = m_hitsReader->hit_index().size();
    
    for(int i = 0; i < nHits; i++)
    {
        // If we want to drop pixel hits
        //if((m_dropPixHit) && (m_hitsReader->hit_PixorSCT(i) == 0) ) continue;
        if((m_dropPixHit) && (m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) != 4) ) continue;
        if(m_LayerRegion.EqualTo("R0_8L") || m_LayerRegion.EqualTo("R1_8L"))
        {
            // If pixel, use layer 4
            if((m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) != 4)) continue;

            // If sct, remove the outer side of the inner strip
            if((m_hitsReader->hit_PixorSCT(i) == 1) && (m_hitsReader->hit_layerID(i) == 0) && (m_hitsReader->hit_side(i) == 1)) continue;

            // Use the barrel only
            if(m_hitsReader->hit_volumeID(i) != 0 && m_hitsReader->hit_volumeID(i) != 10) continue;
        }
        if(m_LayerRegion.EqualTo("R3_8L"))
        {
            // If pixel, use layer 4
            if((m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) != 4)) continue;

            // If sct, remove layer 0,1
            if((m_hitsReader->hit_PixorSCT(i) == 1) && (m_hitsReader->hit_layerID(i) == 0)) continue;
            if((m_hitsReader->hit_PixorSCT(i) == 1) && (m_hitsReader->hit_layerID(i) == 1)) continue;

            // If sct, remove the inside of layer 2
            if((m_hitsReader->hit_PixorSCT(i) == 1) && (m_hitsReader->hit_layerID(i) == 2) && (m_hitsReader->hit_side(i) == 1)) continue;

        }
        if(m_LayerRegion.EqualTo("R0_9L") || m_LayerRegion.EqualTo("R1_9L"))
        {
            // If pixel, use layer 4
            if((m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) != 4)) continue;

            // Use the barrel only
            if(m_hitsReader->hit_volumeID(i) != 0 && m_hitsReader->hit_volumeID(i) != 10) continue;
        }
        if(m_LayerRegion.EqualTo("R3_9L"))
        {
            // If pixel, use layer 4
            if((m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) != 4)) continue;

            // If sct, remove layer 0,1
            if((m_hitsReader->hit_PixorSCT(i) == 1) && (m_hitsReader->hit_layerID(i) == 0)) continue;
            if((m_hitsReader->hit_PixorSCT(i) == 1) && (m_hitsReader->hit_layerID(i) == 1)) continue;

        }
        if(m_LayerRegion.EqualTo("R0_11L") || m_LayerRegion.EqualTo("R1_11L"))
        {
            // If pixel, remove inner 2
            if((m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) == 0)) continue;
            if((m_hitsReader->hit_PixorSCT(i) == 0) && (m_hitsReader->hit_layerID(i) == 1)) continue;

            // Use the barrel only
            if(m_hitsReader->hit_volumeID(i) != 0 && m_hitsReader->hit_volumeID(i) != 10) continue;
        }

        std::vector<Index> barcode;
        for(const auto& var: m_hitsReader->hit_truthTrackBarcode(i)) barcode.push_back(var);


        auto hit = make_shared<Hit>
        (
            m_hitsReader->hit_index(i),
            m_hitsReader->hit_x(i),
            m_hitsReader->hit_y(i),
            m_hitsReader->hit_z(i),
            m_hitsReader->hit_r(i),
            m_hitsReader->hit_rho(i),
            m_hitsReader->hit_volumeID(i),
            m_hitsReader->hit_layerID(i),
            m_hitsReader->hit_moduleID(i),
            m_hitsReader->hit_etaModule(i),
            m_hitsReader->hit_phiModule(i),
            m_hitsReader->hit_PixorSCT(i),
            m_hitsReader->hit_side(i),
            m_hitsReader->hit_recoTrackIndex(i),
            barcode
        );
        m_hitList.push_back(hit);
        m_hitCache[m_hitsReader->hit_index(i)] = hit;
    }   

    // Space points
    int nSP = m_SPReader->SP_index().size();

    for(int i = 0; i < nSP; i++)
    {
        std::vector<Index> barcode;
        for(const auto& var: m_SPReader->SP_truthTrackBarcode(i)) barcode.push_back(var);

        std::shared_ptr<Hit> firstCluster = nullptr;
        std::shared_ptr<Hit> secondCluster = nullptr;

        if(m_hitCache.find(m_SPReader->SP_FirstClusterIndex(i)) != m_hitCache.end()) firstCluster = m_hitCache.at(m_SPReader->SP_FirstClusterIndex(i));
        if(m_hitCache.find(m_SPReader->SP_SecondClusterIndex(i)) != m_hitCache.end()) secondCluster = m_hitCache.at(m_SPReader->SP_SecondClusterIndex(i));

        auto sp = make_shared<SpacePoint>
        (
            m_SPReader->SP_index(i),
            m_SPReader->SP_x(i),
            m_SPReader->SP_y(i),
            m_SPReader->SP_z(i),
            m_SPReader->SP_r(i),
            m_SPReader->SP_rho(i),
            m_SPReader->SP_volumeID(i),
            m_SPReader->SP_layerID(i),
            m_SPReader->SP_moduleID(i),
            m_SPReader->SP_etaModule(i),
            m_SPReader->SP_phiModule(i),
            m_SPReader->SP_PixorSCT(i),
            m_SPReader->SP_side(i),
            m_SPReader->SP_recoTrackIndex(i),
            barcode,
            m_SPReader->SP_FirstClusterIndex(i),
            m_SPReader->SP_SecondClusterIndex(i)
        );

        sp->setFirstCluster(firstCluster);
        sp->setSecondCluster(secondCluster);


        // For future organization
        m_SPList.push_back(sp);
        m_SPCache[m_SPReader->SP_index(i)] = sp;
        if(m_SPReader->SP_FirstClusterIndex(i) >= 0) m_HitIndexToSPCache[m_SPReader->SP_FirstClusterIndex(i)] = sp;
        if(m_SPReader->SP_SecondClusterIndex(i) >= 0) m_HitIndexToSPCache[m_SPReader->SP_SecondClusterIndex(i)] = sp;

        if(m_SPReader->SP_recoTrackIndex(i) >= 0)
        {
            m_RecoTrackIndexToSPCache[m_SPReader->SP_recoTrackIndex(i)].push_back(sp);
        }

        for(const auto& var: m_SPReader->SP_truthTrackBarcode(i))
        {
            if(var > 0) m_TruthTrackIndexToSPCache[var].push_back(sp);
        }



    }

    // // initialize the Reco Tracks
    int nrecoTracks = m_recoTracksReader->track_index().size();
    for(int i = 0; i < nrecoTracks; i++)
    {
        if(m_recoTracksReader->track_Eta(i) < m_lwrEtaLim) continue;
        if(m_recoTracksReader->track_Eta(i) > m_uprEtaLim) continue;

        std::vector<Index> clusterIndex;
        for(const auto& var: m_recoTracksReader->track_clusterIndex(i)) clusterIndex.push_back(var);

        auto track = make_shared<RecoTrack>
        (
            m_recoTracksReader->track_index(i),
            m_recoTracksReader->track_Pt(i)/1000,
            m_recoTracksReader->track_Eta(i),
            m_recoTracksReader->track_phi(i),
            m_recoTracksReader->track_truthProb(i),
            m_recoTracksReader->track_perigee_d0(i),
            m_recoTracksReader->track_perigee_z0(i),
            m_recoTracksReader->track_perigee_phi(i),
            m_recoTracksReader->track_perigee_theta(i),
            m_recoTracksReader->track_perigee_qp(i),
            m_recoTracksReader->track_charge(i),
            m_recoTracksReader->track_nPixHits(i),
            m_recoTracksReader->track_nSCTHits(i),
            m_recoTracksReader->track_truthTrackBarcode(i),
            clusterIndex
        );
        m_recoTrackList.push_back(track);
        m_recoTrackCache[m_recoTracksReader->track_index(i)] = track;
    } 


    // initialize the Truth Tracks
    int ntrueTracks = m_truthTracksReader->true_track_barcode().size();
    for(int i = 0; i < ntrueTracks; i++)
    {
        if(m_truthTracksReader->true_track_Eta(i) < m_lwrEtaLim) continue;
        if(m_truthTracksReader->true_track_Eta(i) > m_uprEtaLim) continue;
        
        std::vector<Index> clusterIndex;
        for(const auto& var: m_truthTracksReader->true_track_clusterIndex(i)) clusterIndex.push_back(var);

        auto track = make_shared<TrueTrack>
        (
            m_truthTracksReader->true_track_barcode(i),
            m_truthTracksReader->true_track_Pt(i)/1000,
            m_truthTracksReader->true_track_Eta(i),
            m_truthTracksReader->true_track_phi(i),
            m_truthTracksReader->true_track_vertDistance(i),
            m_truthTracksReader->true_track_pdgid(i),
            m_truthTracksReader->true_track_status(i),
            // m_truthTracksReader->true_track_recoTrackIndex(i),
            -1,
            clusterIndex
        );
        m_trueTrackList.push_back(track);
        m_trueTrackCache[m_truthTracksReader->true_track_barcode(i)] = track;
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

        // set the truth track
        auto truthBarcode = track->getTrueTrackBarcode();
        if(m_trueTrackCache.find(truthBarcode) != m_trueTrackCache.end()) track->setOrgTrueTrack(m_trueTrackCache.at(truthBarcode));


        // Find the SP points
        std::vector<std::shared_ptr<Hit>> spHits;
        if(m_RecoTrackIndexToSPCache.find(track->getIndex()) != m_RecoTrackIndexToSPCache.end()) spHits = m_RecoTrackIndexToSPCache.at(track->getIndex());
        track->setSPHits(spHits);

        // createSP(dynamic_pointer_cast<TrackBase, RecoTrack>(track));    
        // track->printDebug();

    }

    // Truth track 
    for (auto& track: m_trueTrackList)
    {
        auto hitIndex = track->getOrgHitIndex();
        std::vector<std::shared_ptr<Hit>> hits;
        // find the associated hits
        for(const auto& index: hitIndex)
        {
            if(m_hitCache.find(index) != m_hitCache.end()) hits.push_back(m_hitCache.at(index));
        }

        track->setOrgHits(hits);

        // set the truth track
        auto index = track->getRecoTrackIndex();
        if(m_recoTrackCache.find(index) != m_recoTrackCache.end()) track->setOrgRecoTrack(m_recoTrackCache.at(index));

        // Find the SP points
        std::vector<std::shared_ptr<Hit>> spHits;
        if(m_TruthTrackIndexToSPCache.find(track->getBarcode()) != m_TruthTrackIndexToSPCache.end()) spHits = m_TruthTrackIndexToSPCache.at(track->getBarcode());
        track->setSPHits(spHits);

        // createSP(dynamic_pointer_cast<TrackBase, TrueTrack>(track));    
        // track->printDebug();
    }

    // Hits 
    for (auto& hit: m_hitList)
    {
        auto truthBarcodes = hit->getTruthTrackBarcode();
        std::vector<std::shared_ptr<TrueTrack>> trueTracks;

        for(const auto& truthBarcode: truthBarcodes)
        {
            if(m_trueTrackCache.find(truthBarcode) != m_trueTrackCache.end()) trueTracks.push_back(m_trueTrackCache.at(truthBarcode));
        }
        hit->setOrgTrueTracks(trueTracks);

        auto index = hit->getRecoTrackIndex();
        if(m_recoTrackCache.find(index) != m_recoTrackCache.end()) hit->setOrgRecoTrack(m_recoTrackCache.at(index));

        // hit->printDebug();
    }

    // Clean the tracks
    cleanTrackHits();
}


void DataReader::cleanTrackHits()
{
    int threshold = 6;
    if(m_dropPixHit) threshold = 5;
    if(m_nLayerThreshold >=0) threshold = m_nLayerThreshold;

    auto trackList = getRecoTracks();
    m_recoTrackList.clear();
    for(const auto& track: trackList)
    {
        if(track->getNHits() < threshold) continue;
        m_recoTrackList.push_back(track);
    }

    auto trueTrackList = getTrueTracks();
    m_trueTrackList.clear();
    for(const auto& track: trueTrackList)
    {
        if(track->getNHits() < threshold) continue;
        m_trueTrackList.push_back(track);
    }
}

void DataReader::readGeo()
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
            (unsigned long long)(**m_intVar.at("module_index")),
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
        m_detectorModule.push_back(mod);
        m_detectorModuleCache[**m_intVar.at("module_index")] = mod;
    }

}


void DataReader::createSP(std::shared_ptr<TrackBase> trk)
{
    static Index SPIndex = 0;

    auto hits = trk->getHitsList();

    std::vector<std::shared_ptr<Hit>> spHits;
    std::map<int, std::vector<std::shared_ptr<Hit>>> spHitMap;
    std::vector<int> layersDone;
    for(auto& hit: hits)
    {
        int moduleID = hit->getModule();
        if(moduleID %2 == 1) moduleID--;

        int index = hit->getPixorSct()*10000000 + hit->getVolume()*100000 + hit->getLayer()*1000 + moduleID;
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












