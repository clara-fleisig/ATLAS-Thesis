#ifndef _MLTRACKEXTRAPOLATORRECO_H
#define _MLTRACKEXTRAPOLATORRECO_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNDataReader/DataReaderITK.h"
#include "TrackNNAnalysis/HitsManager.h"
#include "TrackNNAnalysis/SeedManager.h"
#include "TrackNNEvaluator/NNEvalExtrapUtils.h"
#include "TrackNNEvaluator/NNEvalUtils.h"
#include "TrackNNEvaluator/NNEvalClassUtils.h"
#include "TrackNNEvaluator/NNHitPredictor.h"
#include "TrackNNEDM/TrackTree.h"
#include "TrackNNAnalysis/TrackManager.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>
#include <random>



class MLTrackExtrapolatorReco
{
    public:
        MLTrackExtrapolatorReco();
        virtual ~MLTrackExtrapolatorReco();
        
        void initialize();
        void run(int iEvent);

        void setConfigFile(std::string inputFile){m_configFile = inputFile; };
        void setInputFile(std::string inputFile){m_inputFile = inputFile; };
        void setNEvents(int nEvents) {m_nEvents = nEvents;};

        int getNEvents() {return m_nEvents; };

        std::vector<std::shared_ptr<TrackTree>> getTrackTree() {return m_trackTrees; };
        
    protected:
        std::string m_configFile = "";
        std::string m_inputFile = "";

        // -1 is for it run all the envents
        int m_nEvents = -1;

        // Input configuration
        std::string m_detectorGeoFile = "";
        std::string m_NNPredictorOnnxFile = "";
        std::string m_NNClassifierOnnxFile = "";

        std::string m_layerRegion = "";

        float m_uncertainty = 10;
        int m_windowSize = 3;
        int m_batchSize = 100;
        int m_maxBranches = 6;
        int m_minHits = 6;
        int m_NNClassifierHitLength = 10;
        float m_deltaROverlap = 0.0;
        size_t m_layerPredictionSize = 30;
        size_t m_volumePredictionSize = 15;

        // Tool
        std::shared_ptr<TrackManager> m_trackManager;
        std::shared_ptr<SeedManager> m_seedManager;
        std::shared_ptr<DataReaderITK> m_dataReader;
        std::shared_ptr<NNDetectorClassifier> m_HitDetectorPredictor;
        std::shared_ptr<NNHitPredictor> m_HitCoordinatePredictor;

        // The main track tree that we will grow
        std::vector<std::shared_ptr<TrackTree>> m_trackTrees;


        void processConfig();

        // For prediction
        NetworkBatchInput BatchTracksForGeoPrediction(std::vector<std::shared_ptr<TrackTree>>& trackTrees, int batch_size = 128, int window_size = 3);
        NetworkBatchInput BatchTracksForClassification(std::vector<std::shared_ptr<TrackTree>>& trackTrees, int batch_size, std::pair<int, int>& startTreeIndex, int& maxLength);
       
        // Get seeds
        std::vector<std::shared_ptr<SeedTrack>> getSeedTracks();

        // Get the predictions
        void getPredictions(Eigen::MatrixXf& predictedDet, Eigen::MatrixXf& predictedHitMatrix);

        bool isLastHit(std::shared_ptr<Hit> hit)
        {
            return (hit->getRho() > 975 || std::abs(hit->getZ() > 3400));
        }

};

#endif
