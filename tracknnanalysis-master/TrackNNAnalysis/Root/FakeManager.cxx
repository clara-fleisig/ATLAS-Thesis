// Class includes
#include "TrackNNAnalysis/FakeManager.h"

// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <cstdlib>
#include <math.h>
#include <set>
// Root includes

using namespace std;


// Constructors and destructor
FakeManager::FakeManager()
{
    // Raw number provided by Alex
    vector<double> binEdges {0.0006,0.0346,0.0686,0.1026,0.1366,0.1706,0.2046,0.2386,0.2726,0.3066,0.3406,0.3746,0.4086,0.4426,0.4766,0.5106,0.5446,0.5786,0.6126,0.6466,0.6806,0.7146,0.7486,0.7826,0.8166,0.8506,0.8846,0.9186,0.9526,0.9866,1.0206,1.0546,1.0886,1.1226,1.1566,1.1906,1.2246,1.2586,1.2926,1.3266,1.3606,1.3946,1.4286,1.4626,1.4966,1.5306,1.5646,1.5986};
    vector<double> content{474,620,819,869,1427,2556,2636,3118,3090,3481,3050,1920,1317,1161,1376,1046,733,701,675,600,671,607,656,567,493,575,491,453,530,510,467,503,435,392,280,228,184,212,129,115,130,111,206,76,29,14,3};

    // Create the culumative vector
    std::vector<double> cumContent(content.size());
    std::partial_sum(content.begin(), content.end(), cumContent.begin(), plus<double>());
    double totalSum = cumContent.at(cumContent.size() - 1);

    // scale to it corresponds to a CDF
    for(auto& val: cumContent) val /= totalSum;

    // fill out hist
    m_inverseRandAngleDistribution = new TH1F("angleHist", "angleHist", cumContent.size() - 1, &cumContent[0]);
    for(unsigned int i = 0; i < cumContent.size(); i++)
    {
        m_inverseRandAngleDistribution->SetBinContent(i+1, (binEdges.at(i) + binEdges.at(i+1))/2);
    }

    m_randGen = new TRandom3();
    m_randGen->SetSeed(1);

} 

FakeManager::~FakeManager()
{
} 


std::vector<std::shared_ptr<FakeTrack>> FakeManager::getFakeTracks(FakeType fakeType, std::vector<std::shared_ptr<RecoTrack>> recoTracks, unsigned int nTracks, int phiSlice)
{

    if(fakeType == FakeType::FullCombinatoricsinRegions)
    {
        return getFullCombinatoricsFakeinRegions(phiSlice);
    }


    std::vector<std::shared_ptr<FakeTrack>> fakeTracks;
    unsigned trackinSize = recoTracks.size();


    while(fakeTracks.size() < nTracks)
    {
        int randIndex = (rand() % trackinSize);
        auto randTrack = dynamic_pointer_cast<TrackBase, RecoTrack> (recoTracks.at(randIndex));
        std::shared_ptr<FakeTrack> cFakeTrack;

        if(fakeType == FakeType::FullyRandom)
        {
            cFakeTrack = getFullyRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::LayerRandom)
        {
            cFakeTrack = getLayerRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::ModuleRandom)
        {
            cFakeTrack = getModuleRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::ModuleRotatedRandom)
        {
            cFakeTrack = getModuleRotatedRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::ModuleIterRandom)
        {
            cFakeTrack = getModuleIterRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::ModuleRotatedIterRandom)
        {
            cFakeTrack = getModuleRotatedIterRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::TrackHitChangeRandom)
        {
            cFakeTrack = getHitChangeRandomTrack(randTrack);
        }
        else if(fakeType == FakeType::TrackHitChangeClosest)
        {
            cFakeTrack = getHitChangeClosestTrack(randTrack);
        }
        else
        {
            cout<<"Cannot recognize faketype"<<endl;
            exit(1);
        }


        // Fake probability cut to define 
        if(cFakeTrack->getTrackProbability() > 0.7) continue;

        fakeTracks.push_back(cFakeTrack);
    }

    return fakeTracks;
}

std::shared_ptr<FakeTrack> FakeManager::getFullyRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    for(int i = 0; i < nHits; i++)
    {
        auto hit = m_hitManager->getRandomHit();
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}

std::shared_ptr<FakeTrack> FakeManager::getLayerRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    for(int i = 0; i < nHits; i++)
    {
        auto orgHist = track->getHit(i);

        auto hit = m_hitManager->getLayerRandHit(orgHist->getVolume(), orgHist->getLayer());
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}

std::shared_ptr<FakeTrack> FakeManager::getModuleRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    for(int i = 0; i < nHits; i++)
    {
        auto orgHist = track->getHit(i);

        auto hit = m_hitManager->getModuleRandHit(orgHist->getVolume(), orgHist->getLayer(), orgHist->getModule());
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}

std::shared_ptr<FakeTrack> FakeManager::getHitChangeRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    auto geoDist = std::geometric_distribution<int>(1.0/nHits);
    unsigned int randIndexToChange = (geoDist(m_generator) % nHits);

    std::set<Index> hitIndexToChange;
    while(hitIndexToChange.size() < randIndexToChange)
    {
        hitIndexToChange.insert((rand() % nHits));
    }

    for(int i = 0; i < nHits; i++)
    {
        auto orgHist = track->getHit(i);

        auto hit = orgHist;
        if(hitIndexToChange.find(i) != hitIndexToChange.end()) hit = m_hitManager->getModuleRandHit(orgHist->getVolume(), orgHist->getLayer(), orgHist->getModule());
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}


std::shared_ptr<FakeTrack> FakeManager::getHitChangeClosestTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    auto geoDist = std::geometric_distribution<int>(1.0/nHits);
    unsigned int randIndexToChange = (geoDist(m_generator) % nHits);

    std::set<Index> hitIndexToChange;
    while(hitIndexToChange.size() < randIndexToChange)
    {
        hitIndexToChange.insert((rand() % nHits));
    }

    for(int i = 0; i < nHits; i++)
    {
        auto orgHist = track->getHit(i);

        auto hit = orgHist;
        if(hitIndexToChange.find(i) != hitIndexToChange.end()) hit = m_hitManager->getClosestHit(orgHist);
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}

std::shared_ptr<FakeTrack> FakeManager::getModuleRotatedRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    // double get a random angle
    double randAngle = (rand() % 360) * 3.14/180.; // convert to radians

    for(int i = 0; i < nHits; i++)
    {
        // find the rotated X/y
        auto orgHit = track->getHit(i);
        auto hit = m_hitManager->getModuleRotatedRandHit(orgHit, randAngle);
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}

std::shared_ptr<FakeTrack> FakeManager::getModuleIterRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    for(int i = 0; i < nHits; i++)
    {
        auto orgHist = track->getHit(i);

        auto hit = m_hitManager->getModuleIterRandHit(orgHist);
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}

std::shared_ptr<FakeTrack> FakeManager::getModuleRotatedIterRandomTrack(std::shared_ptr<TrackBase> track)
{
    std::vector<Index> hitIndex;
    std::vector<std::shared_ptr<Hit>> hits;

    int nHits = track->getNHits();

    // double get a random angle
    double randAngle = (rand() % 360) * 3.14/180.; // convert to radians

    for(int i = 0; i < nHits; i++)
    {
        // find the rotated X/y
        auto orgHit = track->getHit(i);
        auto hit = m_hitManager->getModuleRotatedIterRandHit(orgHit, randAngle);
        hitIndex.push_back(hit->getIndex());
        hits.push_back(hit);
    }

    std::shared_ptr<FakeTrack> fakeTrack = make_shared<FakeTrack>(hitIndex, hits);

    return fakeTrack;
}


std::vector<std::shared_ptr<FakeTrack>> FakeManager::getFullCombinatoricsFakeinRegions(int phiSlice)
{
    std::vector<std::shared_ptr<FakeTrack>> fullTrackList = {};

    // Only define the positive ones... we will automatically define the negative ones
    vector<float> absphiBins = {0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.14};
    vector<float> absetaBins = {0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2};

    vector<float> phiBins;
    vector<float> etaBins;

    for(const auto& bin: absphiBins) {phiBins.push_back(-1 * bin); phiBins.push_back(bin);}
    for(const auto& bin: absetaBins) {etaBins.push_back(-1 * bin); etaBins.push_back(bin);}

    phiBins.push_back(0);
    etaBins.push_back(0);

    // sort these
    std::sort (phiBins.begin(),phiBins.end());
    std::sort (etaBins.begin(),etaBins.end());

    // for(const auto& bin: phiBins) cout<<"phi: "<<bin<<" size: "<<phiBins.size()<<endl;
    // for(const auto& bin: etaBins) cout<<"eta: "<<bin<<endl;

    for(unsigned int i = 0; i < etaBins.size() - 1; i++)
    {
        for(unsigned int j = 0; j < phiBins.size() - 1; j++)
        {

            if(phiSlice >= 0 && phiSlice != (int) j) continue;
            auto trkList = getFullCombinatoricsFakeinRegion(phiBins.at(j), phiBins.at(j+1), etaBins.at(i), etaBins.at(i+1), true);
            fullTrackList.insert(fullTrackList.end(), trkList.begin(), trkList.end()); 

            std::cout<<"Track size container: "<<fullTrackList.size()<<std::endl;           
        } 
    }


    // auto trkList = getFullCombinatoricsFakeinRegion(0, 0.2, 0, 0.2, true);
    // fullTrackList.insert(fullTrackList.end(), trkList.begin(), trkList.end());

    // trkList = getFullCombinatoricsFakeinRegion(2, 2.2, -1.20, -1, true);
    // fullTrackList.insert(fullTrackList.end(), trkList.begin(), trkList.end());


    return fullTrackList;
}

std::vector<std::shared_ptr<FakeTrack>> FakeManager::getFullCombinatoricsFakeinRegion(double lwrPhi, double uprPhi, double lwrEta, double uprEta, bool doOverlapEdges)
{

    auto zeroHit = make_shared<Hit>
    (  -1, // m_intVectorVar.at("hit_index")->At(i),
        0 ,// m_floatVectorVar.at("hit_x")->At(i),
        0 ,// m_floatVectorVar.at("hit_y")->At(i),
        0 ,// m_floatVectorVar.at("hit_z")->At(i),
        0 ,// m_floatVectorVar.at("hit_r")->At(i),
        0 ,// m_floatVectorVar.at("hit_rho")->At(i),
        -1,// m_intVectorVar.at("hit_volumeID")->At(i),
        -1,// m_intVectorVar.at("hit_layerID")->At(i),
        -1,// m_intVectorVar.at("hit_moduleID")->At(i),
        -1,// m_intVectorVar.at("hit_etaModule")->At(i),
        -1,// m_intVectorVar.at("hit_phiModule")->At(i),
        -1,// m_intVectorVar.at("hit_PixorSCT")->At(i),
        -1,// m_intVectorVar.at("hit_side")->At(i),
        -1,// m_intVectorVar.at("hit_recoTrackIndex")->At(i),
        std::vector<Index>{}// m_intVectorVectorVar.at("hit_truthTrackBarcode")->At(i)
    );

    // To prevent large memeory increases
    unsigned int maxSize = 250000;
    double rndkeepPrunFraction = 0.1;


    float hitSelRange_lwrPhi = lwrPhi;
    float hitSelRange_uprPhi = uprPhi;

    float hitSelRange_lwrEta = lwrEta;
    float hitSelRange_uprEta = uprEta;

    // if do overlap, add %5 on each size
    if(doOverlapEdges)
    {
        double phiRange = fabs(hitSelRange_lwrPhi - hitSelRange_uprPhi);
        hitSelRange_lwrPhi -= 0.05*phiRange;
        hitSelRange_uprPhi += 0.05*phiRange;

        double etaRange = fabs(hitSelRange_lwrEta - hitSelRange_uprEta);
        hitSelRange_lwrEta -= 0.05*etaRange;
        hitSelRange_uprEta += 0.05*etaRange;
    }
    // Get the hit list
    auto hitInRegion = m_hitManager->getHitsInRange(hitSelRange_lwrPhi, hitSelRange_uprPhi, hitSelRange_lwrEta, hitSelRange_uprEta);

    // Order them in reverse in R, with the outer most hit first and then inner most at the end
    static auto comp = [](shared_ptr<Hit> const a, shared_ptr<Hit> const b)
    {
        return a->getR() < b->getR();
    };
    std::sort (hitInRegion.begin(), hitInRegion.end(), comp);

    // Create a fake track and initialize a track with one hit
    std::vector<std::shared_ptr<FakeTrack>> initTrackList = {};
    for(const auto& hit: hitInRegion)
    {
        auto fakeTrack = make_shared<FakeTrack>(std::vector<Index> {hit->getIndex()}, std::vector<std::shared_ptr<Hit>> {hit});
        initTrackList.push_back(fakeTrack);
    }

    // Start 
    std::vector<std::shared_ptr<FakeTrack>> trackList;
    int iter = 0;
    //for(const auto& hit: hitInRegion) hit->printDebug();
    // double maxHitScatterAngle = 30 * 3.14/180;
    // double maxHitScatterCos = cos(maxHitScatterAngle);

    int nHitSize = 7;

    // Inside out track reco
    while(initTrackList.size() > 0)
    // while(initTrackList.size() > 0)
    {
        iter++;
        Index indexTrack = 0;
        std::vector<std::shared_ptr<FakeTrack>> appendTrackList = {};

        bool randomPruning = false;
        if(initTrackList.size() == maxSize) randomPruning = true;
        if(randomPruning)
        {
            // cout<<"Going to do random pruning as the initial track container is already at the max size"<<endl;
        }
        for(const auto& track: initTrackList)
        {
            auto hitList            = track->getHitsList();
            auto firstHit           = hitList[0];
            auto secondToLastHist   = zeroHit;
            if(hitList.size() >=2 ) secondToLastHist = hitList[hitList.size() - 2];
            auto lastHit            = hitList[hitList.size() - 1];

            // Create the direction vector
            // double rDistRho = lastHit->getRho() - secondToLastHist->getRho();
            double rDistX   = lastHit->getX() - secondToLastHist->getX();
            double rDistY   = lastHit->getY() - secondToLastHist->getY();
            double rDistZ   = lastHit->getZ() - secondToLastHist->getZ();
            double rDistVec = sqrt(pow(rDistX, 2) + pow(rDistY, 2) + pow(rDistZ, 2));


            std::vector<Index> hitIndex {};
            for(const auto& hit: hitList) hitIndex.push_back(hit->getIndex());

            for(auto& hit: hitInRegion)
            {
                /// We only want to consider hits that are larger R than the last hit
                /// If the hit has a smaller R, we don't want to consider it
                if(hit->getR() <= lastHit->getR()) continue;

                // Don't take any in same module
                if(lastHit->getModule() == hit->getModule()) continue;

                // Don't add the same hit again
                if(track->hasHit(hit->getIndex())) continue;


                // If the last hit and current hit are in the same volume, make sure that the layer difference is not more than 1
                // We want to make sure the track traverse the layers and leaves a hit
                // Volume cut is to make that the barrel to EC transition is treated properly
                if(lastHit->getVolume() == hit->getVolume())
                {
                    if(fabs(lastHit->getLayer() - hit->getLayer()) > 1) continue;
                }

                // make sure rho and Z are increasing
                if(hit->getRho() < lastHit->getRho()) continue;
                if(fabs(hit->getZ()) < fabs(lastHit->getZ())) continue;

                // Make sure the track is in a cone defined by the angle
                // if(fakeTrack->getNHits() != (iter + 1) )
                if(hitList.size() >=2)
                {
                    // double cDistRho = hit->getRho() - lastHit->getRho();
                    double cDistX   = hit->getX() - lastHit->getX();
                    double cDistY   = hit->getY() - lastHit->getY();
                    double cDistZ   = hit->getZ() - lastHit->getZ();
                    double cDistVec = sqrt(pow(cDistX, 2) + pow(cDistY, 2) + pow(cDistZ, 2));

                    double dotProd = (cDistX * rDistX + cDistY * rDistY + cDistZ * rDistZ)/(rDistVec * cDistVec);
                    double scatterAngle = acos(dotProd);


                    // sample the max angle
                    double rand = m_randGen->Uniform(1);
                    double maxHitScatterAngle = m_inverseRandAngleDistribution->GetBinContent( m_inverseRandAngleDistribution->GetXaxis()->FindBin(rand));

                    if(fabs(scatterAngle) > fabs(maxHitScatterAngle)) continue;
                }

                // If random pruning
                if(randomPruning)
                {
                    if(m_randGen->Uniform(1) > rndkeepPrunFraction) continue;
                }


                // Add for the SCT to not take the hits from the same layer/same side
                // Probably not take from the same eta/phi module for pixel


                // This looks like a good track
                auto cHitList = hitList;
                cHitList.push_back(hit);
                auto cHitIndex = hitIndex;
                cHitIndex.push_back(hit->getIndex());

                indexTrack++;            
                auto fakeTrack = make_shared<FakeTrack>(cHitIndex, cHitList);
                appendTrackList.push_back(fakeTrack);

                if(fakeTrack->getNHits() != (iter + 1) )
                {
                    cout<<"---------------: "<<indexTrack<< "-------- "<<iter<<endl;
                    cout<<"----------Org track:"<<endl;
                    track->printDebug();
                    cout<<"----------adding hit:"<<endl;
                    hit->printDebug();
                    cout<<"----------New track:"<<endl;
                    fakeTrack->printDebug();
                }
            }
        }

        // We run out memory when we have too many combination... Randomly xx percent of evnets
        int currSize = appendTrackList.size();
        if(appendTrackList.size() > maxSize)
        {       
            // cout<<"Vector size is "<< currSize<<" for iter " << iter<< " exceeds limit of "<< maxSize<<". Will sample down to max size"<<endl;

            std::set<Index> indexList;
            while(indexList.size() < maxSize) indexList.insert(rand() % currSize);
            std::vector<std::shared_ptr<FakeTrack>> cappendTrackList;
            for(const auto& index: indexList) cappendTrackList.push_back(appendTrackList.at(index));

            appendTrackList.clear();
            appendTrackList = cappendTrackList;

        }

        // cout<<"iter: "<<iter<<endl;
        // cout<<"InitialList: "<<initTrackList.size()<<endl;
        // cout<<"appendTrackList: "<<appendTrackList.size()<<endl;

        if(iter >= nHitSize) trackList.insert(trackList.end(), appendTrackList.begin(), appendTrackList.end());
        initTrackList = appendTrackList;
    }


    std::vector<std::shared_ptr<FakeTrack>> cleanTrackList;
    Index indexTrack = 0;
    for(const auto& track:trackList)
    {
        if(track->getNHits() < nHitSize) continue;
        // cout<<"---------------: "<<indexTrack<< "-------- "<<endl;
        // track->printDebug();
        cleanTrackList.push_back(track);

        indexTrack++;
    }

    cout<<"lwrEta: "<<lwrEta<<" uprEta: "<<uprEta<<" lwrPhi: "<<lwrPhi<<" uprPhi: "<<uprPhi<<" nHits: "<<hitInRegion.size()<<" nTracks: "<<cleanTrackList.size()<<endl;



    return cleanTrackList;

}



















