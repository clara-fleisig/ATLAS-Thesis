// Class includes
#include "TrackNNAnalysis/MLTrackExtrapolatorReco.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "PathResolver/PathResolver.h"

using namespace std;


// Constructors and destructor
MLTrackExtrapolatorReco::MLTrackExtrapolatorReco()
{
}

MLTrackExtrapolatorReco::~MLTrackExtrapolatorReco()
{

} 

// To initialize
void MLTrackExtrapolatorReco::initialize()
{
    processConfig();
}

// To initialize all the variables
void MLTrackExtrapolatorReco::processConfig()
{
    std::ifstream f(m_configFile);
    nlohmann::json data = nlohmann::json::parse(f);

    m_detectorGeoFile = PathResolver::find_calib_file (data["detectorGeoFile"]);
    m_NNPredictorOnnxFile = PathResolver::find_calib_file (data["NNPredictorOnnxFile"]);
    m_NNClassifierOnnxFile = PathResolver::find_calib_file (data["NNClassifierOnnxFile"]);

    m_layerRegion = data["LayerRegion"];

    m_uncertainty = data["uncertainty"];
    m_windowSize = data["windowSize"];
    m_batchSize = data["batchSize"];
    m_maxBranches = data["maxBranches"];
    m_minHits = data["minHits"];
    m_NNClassifierHitLength = data["NNClassifierHitLength"];
    m_deltaROverlap = data["deltaROverlap"];
    m_layerPredictionSize = data["NNLayerSize"];
    m_volumePredictionSize = data["NNVolumeSize"];

    m_HitCoordinatePredictor = std::make_shared<NNHitPredictor>(m_NNPredictorOnnxFile);
    m_HitCoordinatePredictor->initialize();
    m_HitDetectorPredictor = std::make_shared<NNDetectorClassifier>(m_NNClassifierOnnxFile);
    m_HitDetectorPredictor->setLayerSize(m_layerPredictionSize);
    m_HitDetectorPredictor->setVolumeSize(m_volumePredictionSize);
    m_HitDetectorPredictor->initialize();

    m_dataReader = std::make_shared<DataReaderITK>(m_inputFile, m_detectorGeoFile);
    m_dataReader->setDeltaROverlap(m_deltaROverlap);
    m_dataReader->setLayerRegion(m_layerRegion);
    m_dataReader->initialize();

    if(m_nEvents < 0) m_nEvents = m_dataReader->getEntries();

    m_seedManager = std::make_shared<SeedManager>();
    m_seedManager->setSeedSize(m_windowSize);
    m_seedManager->setWindowSize(m_windowSize);

    m_trackManager = std::make_shared<TrackManager>();
}

std::vector<std::shared_ptr<SeedTrack>> MLTrackExtrapolatorReco::getSeedTracks()
{
    // Get the tracks from which we want to make the seeds from
    //auto recoTracks = m_dataReader->getRecoTracks();       
    auto recoTracks = m_dataReader->getOfflineTruthTracks();

    // Clean the tracks by removing the extra hits
    std::vector<std::shared_ptr<TrueTrack>> cleanTracks;
         if(m_layerRegion == "barrel")  cleanTracks = m_trackManager->getTrackSingleLayerHitInBarrel(recoTracks, m_minHits);
    else if(m_layerRegion == "endcaps") cleanTracks = m_trackManager->getTrackSingleLayerHitInEndcap(recoTracks, m_minHits);
    else                                cleanTracks = m_trackManager->getTrackSingleLayerHit(recoTracks, m_minHits);

    // Get the seeds from this
    auto seedTracks = m_seedManager->getSeedTracks(SeedType::InitialSeeds, cleanTracks);

    return seedTracks;
}


void MLTrackExtrapolatorReco::getPredictions(Eigen::MatrixXf& predictedDet, Eigen::MatrixXf& predictedHitMatrix)
{
    auto networkInput = BatchTracksForGeoPrediction(m_trackTrees, m_batchSize);
    predictedDet = m_HitDetectorPredictor->PredictVolumeAndLayer(networkInput);
    // std::cout<<"Pred size: row "<<predictedDet.rows() <<" col "<<predictedDet.cols()<<" inputs: "<<networkInput.rows()<<" col: "<<networkInput.cols()<<std::endl;
    NetworkBatchInput coordinateNetworkInput(networkInput.rows(), networkInput.cols() + predictedDet.cols());
    coordinateNetworkInput << networkInput, predictedDet;
    predictedHitMatrix = m_HitCoordinatePredictor->PredictHitCoordinate(coordinateNetworkInput);
}


void MLTrackExtrapolatorReco::run(int iEvent)
{

    // Read the event
    m_dataReader->loadEntry(iEvent);

    // hit manager to search for hits
    auto hitManager = std::make_shared<HitsManager>(m_dataReader->getHits(), m_dataReader->getDetectorModules());

    // Get seed tracks to start the track info from
    auto seedTracks = getSeedTracks();

    // Create track tree from each active seeds
    int totalActiveHits = 0;
    for(auto& seed:seedTracks)
    {
        // seed->printDebug();
        // seed->getBaseTrueTrack()->printDebug();

        auto tree = std::make_shared<TrackTree>(seed);
        tree->setMaxBranches(m_maxBranches);
        m_trackTrees.push_back(tree);

        totalActiveHits += tree->getActiveHits().size();
    }


    // This needs to change.... HardCoded with ITK numbers
    auto detectorLabels = hitManager->getVolumesITK();

    std::cout<<"number of initial seeds: "<<m_trackTrees.size()<<std::endl;

    // For debug
    // for(auto tree:m_trackTrees)
    // {
    //     for(auto h:tree->getActiveHits())
    //     {
    //         tree->printDebug(h);
    //     }
    // }

    // Main Hit Search loop
    while (totalActiveHits > 0)
    {
        // Get predictions that will be used for the future
        Eigen::MatrixXf predictedDet;
        Eigen::MatrixXf predictedHitMatrix;
        getPredictions(predictedDet, predictedHitMatrix);

        // To keep a track of which tip we are doing this. This is to ensure we don't go past the prediction
        int iTipIndex = 0; 
        bool breakLoop = false;

        // Loop over each tree
        for (auto& tree : m_trackTrees)
        {
            // If there are not active nodes, continue
            auto activeHitNodes = tree->getActiveHits();
            if (activeHitNodes.size() == 0)  continue; 
            std::reverse(activeHitNodes.begin(), activeHitNodes.end()); // reverse so we can use pop_back to remove items from tree's active hits

            // Loop over each acrive hit
            for(auto& hitNode : activeHitNodes)
            {
                // Get the 
                Eigen::VectorXf predictedHitCoordinate = predictedHitMatrix.row(iTipIndex);
                int layerId = -999; 
                int volumeId = -999; 
                int layerIdIdx = -999; 
                int volumeIdIdx = -999;

                // Get first 15 elements of this row corresponding to the volume one hot encoding
                Eigen::VectorXf volumeRow = predictedDet.block<1,15>(iTipIndex,0);
                volumeRow.maxCoeff(&volumeIdIdx);

                // Get last 30 elements of this row corresponding to the layer one hot encoding
                Eigen::VectorXf layerRow = predictedDet.block<1,30>(iTipIndex,15);
                layerRow.maxCoeff(&layerIdIdx);

                volumeId = detectorLabels[volumeIdIdx];
                layerId = layerIdIdx*2;

                // @ALEX: Why do we need this hit list? We are checking in a new layer ultimately or no?
                std::vector<std::shared_ptr<Hit>> hitsList;
                tree->getHitsList(hitNode, hitsList, m_windowSize);
                auto recoveredHits = hitManager->getClosestLayerHitsList(predictedHitCoordinate, hitsList, volumeId, layerId, m_uncertainty, true);

                std::vector<std::shared_ptr<Hit>> FinalHits;
                std::vector<std::shared_ptr<Hit>> activeHits;

                // add recovered hits to node.
                // Need to determine better track finish condition
                int nAddedHits = 0;
                for(const auto& hit: recoveredHits)
                {
                    if (isLastHit(hit))
                    {
                        FinalHits.push_back(hit);
                    }
                    else
                    {
                        if (nAddedHits < m_maxBranches) activeHits.push_back(hit);
                        nAddedHits++;
                    }
                }
                
                // If there were no hits found, move the active hit to final hits as it is the end of a track
                if (recoveredHits.size() == 0)
                {
                    tree->activeHitToFinalHits(hitNode);
                    // std::cout<<"no hits found. Ending branch "<<std::endl;
                }

                // Want to clear active hits after they are used. They are no longer active!
                // Cannot clear the whole active hits vector yet as we are iterating over it!
                if (FinalHits.size() > 0)
                {
                    // returns the number of final hits added. If number would bring tree > max size those hits will be skipped
                    tree->addFinalHitNodes(hitNode, FinalHits);
                }

                tree->eraseActiveHits(); // erases the active hit already used. Pop_back from the reversed list
                int nActiveHits = tree->addActiveHitNodes(hitNode, activeHits);

                // Only add the number of hits that were used
                // Some may not be because the tree is overgrown
                // totalActive hits  = number of newly added hits minus the hit that was just used
                totalActiveHits += nActiveHits - 1;
                iTipIndex++;
                if(iTipIndex >= m_batchSize)
                {
                    breakLoop = true;
                    break;
                    
                }
            }
            if(breakLoop) break;
        }
        // int r = static_cast<int>(coordinateNetworkInput.rows());
        // totalActiveHits -= r;
        // std::cout<<"total active hits "<<totalActiveHits<<std::endl;
    }
}


NetworkBatchInput MLTrackExtrapolatorReco::BatchTracksForGeoPrediction(std::vector<std::shared_ptr<TrackTree>>& trackTrees, int batch_size, int window_size)
{
    // Check if there are enough active hits to fill the matrix
    int nActiveHits = 0;
    for(auto& tree: trackTrees)
    {
        nActiveHits += tree->getActiveHits().size();
    }
    if (nActiveHits < batch_size) 
    { 
        batch_size = nActiveHits; 
    }
    NetworkBatchInput networkInput(batch_size, window_size * 3); // 3 times the number of window size
    int trkIndex = 0;
    bool breakWhile = false;
    while(true) 
    {
        for(auto tree = trackTrees.begin(); tree != trackTrees.end(); tree++)
        {
            if((*tree)->getActiveHits().size() == 0) 
            {
                if(tree == (trackTrees.end()-1)) 
                {
                    breakWhile = true;
                    break;
                } 
                continue; 
            }
            
            for(auto& hitNode: (*tree)->getActiveHits())
            {
                std::vector<std::shared_ptr<Hit>> hitsList;
                (*tree)->getHitsList(hitNode, hitsList, window_size);

                // Fill in reverse order b.c hitsList is in reverse order (descending in r)
                int counter = window_size - 1;
                while(counter >= 0)
                {
                    networkInput(trkIndex, (window_size - 1 - counter)*3    ) =  hitsList.at(counter)->getX() / hitsList.at(counter)->getXScale();
                    networkInput(trkIndex, (window_size - 1 - counter)*3 + 1) =  hitsList.at(counter)->getY() / hitsList.at(counter)->getYScale();
                    networkInput(trkIndex, (window_size - 1 - counter)*3 + 2) =  hitsList.at(counter)->getZ() / hitsList.at(counter)->getZScale();
                    counter--;
                }
                trkIndex++;
                if(trkIndex >= batch_size)
                {  
                    breakWhile = true;
                    break;
                }
            }
            if(breakWhile) break;
        }
        if(breakWhile) break;
    }
  
    return networkInput;
}

NetworkBatchInput MLTrackExtrapolatorReco::BatchTracksForClassification(std::vector<std::shared_ptr<TrackTree>>& trackTrees, int batch_size, std::pair<int, int>& startTreeIndex, int& maxLength)
{    
    // Check if there are enough active hits to fill the matrix
    int nFinalHits = 0;
    for(auto& tree: trackTrees)
    {
        nFinalHits += tree->getFinalHits().size();
    }
    if (nFinalHits < batch_size) 
    { 
        batch_size = nFinalHits; 
    }
    
    NetworkBatchInput networkInput(batch_size, maxLength * 3); // 30 hard coded as the number of inputs (max length 10 hits 3 features each)


    int treeIndex = startTreeIndex.first;
    int trkIndex = startTreeIndex.second;

    // Check if we have used all the hits in this tree
    if(trkIndex == ((int) trackTrees.at(treeIndex)->getFinalHits().size()-1) )
    {
        trkIndex = 0;
        treeIndex++;
    }

    std::vector<std::shared_ptr<TrackTree>>::iterator beginIt = trackTrees.begin() + startTreeIndex.first;
    for(auto treeIt = beginIt; treeIt != trackTrees.end(); treeIt++)
    {   
        // Deference the iter to it is easier
        auto tree = *treeIt;

        for(auto hitNode_itr = tree->getFinalHits().begin() + startTreeIndex.second; hitNode_itr != tree->getFinalHits().end(); hitNode_itr++)
        // for(auto& hitNode: tree->getFinalHits())
        {

            std::vector<std::shared_ptr<Hit>> hitsList;
            tree->getHitsList((*hitNode_itr), hitsList, -1);

            // Fill in reverse order b.c hitsList is in reverse order (descending in r)
            int counter = hitsList.size() - 1;
            for(size_t i = 0; i < hitsList.size(); i++)
            {
                if(i >= maxLength) continue;
                networkInput(trkIndex, i*3    ) =  hitsList.at(counter-i)->getX() / hitsList.at(counter-i)->getXScale();
                networkInput(trkIndex, i*3 + 1) =  hitsList.at(counter-i)->getY() / hitsList.at(counter-i)->getYScale();
                networkInput(trkIndex, i*3 + 2) =  hitsList.at(counter-i)->getZ() / hitsList.at(counter-i)->getZScale();
                counter--;
            }
            // Pad the rest with zeros
            for (size_t i = hitsList.size() ; i < maxLength ; i++)
            {
                networkInput(trkIndex, i*3    ) =  0;
                networkInput(trkIndex, i*3 + 1) =  0;
                networkInput(trkIndex, i*3 + 2) =  0;
            }

            trkIndex++;
            if(trkIndex >= batch_size)
            {  
                startTreeIndex.first = treeIndex;
                startTreeIndex.second = trkIndex;
                break;
            }
        }
        treeIndex++;
    }
    
  
    return networkInput;
}






