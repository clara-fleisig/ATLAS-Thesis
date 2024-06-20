#ifndef _DETECTORMOUDLE_H
#define _DETECTORMOUDLE_H

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

// Base class 
class DetectorModule 
{
    friend class DataReader;
    friend class DataReaderHTT;

    public:
        DetectorModule(Index index, int volumeId, int layerId, int moduleId, int PixorSCT,
        float center_x, float center_y, float center_z, float center_r, float center_rho,
        float rot_xu, float rot_xv, float rot_xw, 
        float rot_yu, float rot_yv, float rot_yw,
        float rot_zu, float rot_zv, float rot_zw,
        float thickness,
        float minhu, float maxhu, float hv,
        float pitch_u, float pitch_v);

        virtual ~DetectorModule();   

        Index getIndex() const {return m_index; };

        float getX() const {return m_center_x; };
        float getY() const {return m_center_y; };
        float getZ() const {return m_center_z; };
        float getR() const {return m_center_r; };
        float getRho() const {return m_center_rho; };
        int getPixorSct() const{return m_PixorSCT; };
        int getVolume() const{return m_volumeId; };
        int getLayer() const{return m_layerId; };
        int getModule() const{return m_moduleId; };

        float getHalfLength()   const{return m_hv; };
        float getHalfMaxWidth() const{return m_maxhu; };
        float getHalfMinWidth() const{return m_minhu; };

        void printDebug() const;


        float rot(int xAxis, int yAxis)   const
        {
            if((xAxis == 0) && (yAxis == 0))return m_rot_xu; 
            if((xAxis == 0) && (yAxis == 1))return m_rot_xv; 
            if((xAxis == 0) && (yAxis == 2))return m_rot_xw; 

            if((xAxis == 1) && (yAxis == 0))return m_rot_yu; 
            if((xAxis == 1) && (yAxis == 1))return m_rot_yv; 
            if((xAxis == 1) && (yAxis == 2))return m_rot_yw; 

            if((xAxis == 2) && (yAxis == 0))return m_rot_zu; 
            if((xAxis == 2) && (yAxis == 1))return m_rot_zv; 
            if((xAxis == 2) && (yAxis == 2))return m_rot_zw; 

            return 0;
        };



        double distanceTo(float x, float y, float z); 

        void addIntMetadata(TString key, int val){m_intMetadata[key] = val; };
        int getIntMetadata(TString key) 
        {
            if(m_intMetadata.find(key) == m_intMetadata.end())
            {
                std::cout<<"Can't find int metadata in the module with the key: "<<key<<std::endl;
                exit(1);
            }
            return m_intMetadata.at(key);
        };


    protected:
        unsigned long long   m_index;
        int   m_volumeId;
        int   m_layerId;
        int   m_moduleId;
        int   m_PixorSCT;
        float m_center_x;
        float m_center_y;
        float m_center_z;
        float m_center_r;
        float m_center_rho;
        float m_rot_xu;
        float m_rot_xv;
        float m_rot_xw;
        float m_rot_yu;
        float m_rot_yv;
        float m_rot_yw;
        float m_rot_zu;
        float m_rot_zv;
        float m_rot_zw;
        float m_thickness;
        float m_minhu;
        float m_maxhu;
        float m_hv;
        float m_pitch_u;
        float m_pitch_v;

        std::map<TString, int> m_intMetadata;


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
