#ifndef _HITSMANAGER_H
#define _HITSMANAGER_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"
#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/DetectorModule.h"

// c++ includes
#include <map>
#include <vector>
#include <memory>
#include <random>
#include <Eigen/Dense>


// Root includes
#include <TFile.h>
#include <TTree.h>
#include <TString.h>

// Base class 
class HitsManager 
{
    public:
        HitsManager(std::vector<std::shared_ptr<Hit>> hits, std::vector<std::shared_ptr<DetectorModule>> detectorModules, bool m_doHitsToSpacepoints = false);
        virtual ~HitsManager();   

        /* void setDoHitsToSpacepoints(bool set = false){m_doHitsToSpacepoints = set;}; */

        std::shared_ptr<Hit> getRandomHit();
        std::shared_ptr<Hit> getLayerRandHit(int volume, int layer);
        std::shared_ptr<Hit> getModuleRandHit(int volume, int layer, int module);
        std::shared_ptr<Hit> getModuleRotatedRandHit(std::shared_ptr<Hit> orgHit, float randAngle);
        std::shared_ptr<Hit> getModuleIterRandHit(std::shared_ptr<Hit> orgHit);
        std::shared_ptr<Hit> getModuleRotatedIterRandHit(std::shared_ptr<Hit> orgHit, float randAngle);
        std::shared_ptr<Hit> getClosestHit(std::shared_ptr<Hit> orgHit);
        std::vector<std::shared_ptr<Hit>> getHitsInRange(double lwrPhi, double uprPhi, double lwrEta, double uprEta);
        
        std::shared_ptr<Hit> getClosestHitFromPredictedHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, double maxDistance);
        std::shared_ptr<Hit> getClosestLayerHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, int volume_id, int layer_id);
        std::shared_ptr<Hit> getClosestHitGeodesic(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, int volume_id, int layer_id);

        std::vector<std::shared_ptr<Hit>> getClosestHitsListFromPredictedHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, double phiError, double thetaError);
        std::vector<std::shared_ptr<Hit>> getClosestHitsListFromPredictedHit(std::shared_ptr<Hit> predHit, const std::vector<std::shared_ptr<Hit>>& hits, double error);
        
        std::vector<std::shared_ptr<Hit>> getClosestLayerHitsList(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, double& error);
        std::vector<std::shared_ptr<Hit>> getClosestLayerHitsList(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, float& zUncertainty, float& phiUncertainty);
        std::vector<std::shared_ptr<Hit>> getClosestLayerHitsList(Eigen::VectorXf& predHit,      const std::vector<std::shared_ptr<Hit>>& hits,      int& volume_id, int& layer_id, float& error, bool sort = true);

        std::vector<std::shared_ptr<Hit>> getKNearestNeighborHits(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, int& volume_id, int& layer_id, double& error, int& k);
        std::vector<std::shared_ptr<Hit>> getKNearestNeighborHits(std::shared_ptr<Hit>& predHit, const std::vector<std::shared_ptr<Hit>>& hits, double& error, int& k);

        std::vector<int> getVolumesITK(){return m_volumesITK;}

    protected:
        bool m_doHitsToSpacepoints;

        std::vector<std::shared_ptr<Hit>> m_hits;
        std::vector<std::shared_ptr<DetectorModule>> m_detectorModules;

        std::map<int, std::vector<std::shared_ptr<Hit>>> m_layerOrdered;
        std::map<int, std::vector<std::shared_ptr<DetectorModule>>> m_layerOrderedDectModules;
        std::map<int, int> m_layerSize;
        int getLayerUniqueId(int volume, int layer);

        std::map<int, std::vector<std::shared_ptr<Hit>>> m_moduleOrdered;
        std::map<int, std::vector<std::shared_ptr<DetectorModule>>> m_moduleOrderedDectModules;
        std::map<int, int> m_moduleSize;
        int getModuleUniqueId(int volume, int layer, int module);

        std::default_random_engine m_generator;
        std::geometric_distribution<int> m_geoDist;

        std::vector<int> m_volumesITK = {2, 8, 9, 10, 13, 14, 15, 16, 18, 19, 20, 22, 23, 24, 25};

	    int m_nHits; 


        // Functions from stackoverflow
        static inline double angle_1to360(double angle)
        {
            angle=((int)angle % 360) + (angle-trunc(angle)); //converts angle to range -360 + 360
            if(angle>0.0) return angle;
            else return angle + 360.0;
        };

        static inline bool angle_is_between_angles(float N, float a, float b) 
        {
            // Convert to degrees
            N *= 180/3.14;
            a *= 180/3.14;
            b *= 180/3.14;

            N = angle_1to360(N); //normalize angles to be 1-360 degrees
            a = angle_1to360(a);
            b = angle_1to360(b);

            if (a < b) return a <= N && N <= b;
            return a <= N || N <= b;
        };


};


#endif
