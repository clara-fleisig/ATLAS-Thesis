#ifndef _HIT_H
#define _HIT_H

// Local includes
#include "TrackNNCommon/CommonDefs.h"

// Local forwards
class RecoTrack;
class TrueTrack;
class DataReader;

// c++ includes
#include <map>
#include <vector>
#include <memory>
#include <iostream>

// Root includes
#include <TString.h>

// Local forward includes
class DetectorModule;

// Base class 
class Hit 
{
    friend class DataReader;
    friend class DataReaderHTT;

    public:
        Hit();
        Hit(Index index, float x, float y, float z);
        Hit(Index index, 
            float x, 
            float y, 
            float z, 
            float r, 
            float rho, 
            int volumeID, 
            int layerID, 
            int moduleID,
            int etaModule, 
            int phiModule, 
            int pixOrSCT, 
            int side,
            Index recoTrackIndex, 
            std::vector<Index> trueTrackBarcode);
        virtual ~Hit();   

        // Copy constructor
        Hit(const Hit& hit);

        Index getIndex() const {return m_index; };

        void printDebug() const;

        std::vector<Index> getTruthTrackBarcode() const  {return m_trueTrackBarcode; };
        Index getRecoTrackIndex() const                  {return m_recoTrackIndex; };


        float getX()        const {return m_x; };
        float getY()        const {return m_y; };
        float getZ()        const {return m_z; };
        float getR()        const {return m_r; };
        float getRho()      const {return m_rho; };
        int   getPixorSct() const {return m_pixOrSCT; };
        int   getSide()     const {return m_side; };
        int   getVolume()   const {return m_volumeID; };
        int   getLayer()    const {return m_layerID; };
        int   getModule()   const {return m_moduleID; };
        float getPhiModule() const {return m_phiModule; };
        float getEtaModule() const {return m_etaModule; };
        float getPhi()      const {return m_phi; };
        float getEta()      const {return m_eta; };

        // Flip the orientation of this hit
        void flipZ() { m_z *= -1; m_isModifiedHit = true;};

        // Input angle is in radians
        void rotateXY(double angle);

        // Input angle is in radians
        void rotateXY(double cos_angle, double sin_angle);


        // Input angle is in radians
        void fillrotatedXYPhi(double angle, float& x, float& y, float& phi) const;
        void fillrotatedXYPhi(double cos_angle, double sin_angle, float& x, float& y, float& phi) const;

        float distanceTo(const std::shared_ptr<Hit> hit) const;
        float distanceTo(float x, float y, float z) const;
        float angularDistanceTo(const std::shared_ptr<Hit> hit) const;

        float getUnrolledX(float cylWidth); // convert hit coordinate in barrel into unrolled XZ Plane

        // if sign, consider the signed error to see if network more frquently over/under predicts. If not, take the absolute value to get actual error
        float distanceToRho(const std::shared_ptr<Hit> hit, bool sign = false) const;
        float distanceToZ(const std::shared_ptr<Hit> hit,   bool sign = false) const;
        float distanceToPhi(const std::shared_ptr<Hit> hit, bool sign = false) const;

        float distanceToX(const std::shared_ptr<Hit> hit, bool sign = false) const;
        float distanceToY(const std::shared_ptr<Hit> hit, bool sign = false) const;

        std::vector<int> OneHotEncodeVolumeID(std::vector<int> volumeIDs);
        std::vector<int> OneHotEncodeLayerID(int maxLayers);
        std::vector<int> OneHotEncoding(std::vector<int> volumeIDs, int maxLayers);


        // static float getXScale() { return 1015;};
        // static float getYScale() { return 1015;};
        // static float getZScale() { return 3000;};


        // static float getXScale() { return 1031;};
        // static float getYScale() { return 1031;};
        // static float getZScale() { return 1145;};

        static constexpr float getXScale() { return 1/0.00094;};
        static constexpr float getYScale() { return 1/0.00094;};
        static constexpr float getZScale() { return 1/0.00033;};
        static constexpr float getRoughRotateScale() { return 1/0.00344;};
        ////ITK
        //static float getXScale() { return 1005;};
        //static float getYScale() { return 1005;};
        //static float getZScale() { return 3500;};
        //static float getRoughRotateScale() { return 291;}

        void addIntMetadata(TString key, int val){m_intMetadata[key] = val; };
        int getIntMetadata(TString key) 
        {
            if(m_intMetadata.find(key) == m_intMetadata.end())
            {
                std::cout<<"Can't find int metadata in the track with the key: "<<key<<std::endl;
                exit(1);
            }
            return m_intMetadata.at(key);
        };


        bool isHitinModule(std::shared_ptr<DetectorModule> mod);

    protected:
        Index m_index; 
        float m_x; 
        float m_y; 
        float m_z; 
        float m_r;
        float m_rho;
        int m_volumeID; 
        int m_layerID;
        int m_moduleID;
        int m_etaModule; 
        int m_phiModule; 
        int m_pixOrSCT;

        int m_side;
        Index m_recoTrackIndex;
        std::vector<Index> m_trueTrackBarcode;
        bool m_isModifiedHit;

        float m_phi;
        float m_eta;
        float m_theta;

        std::shared_ptr<RecoTrack> m_recoTrack;
        std::map<Index, std::shared_ptr<TrueTrack>> m_trueTracks;


        std::map<TString, int> m_intMetadata;


        void setOrgRecoTrack(std::shared_ptr<RecoTrack> track);
        void setOrgTrueTracks(std::vector<std::shared_ptr<TrueTrack>> tracks);


};

/////// Volume id
// - 2 - left EC (Z < 0)
// 0 - barrel
// 2 = right EC (Z > 0)

// layer ID
// For SCT barrel -> goes inside out (0 - 3)
// For SCT EC -> goes inside out (0 - 4)

// For Pix barrel
// 0/1 are two inner most, 2,3,4 include inclinded sensors
// EC is most normal, second layer include the small sublefts

// For SCT EC -> goes inside out (0 - 4)


#endif
