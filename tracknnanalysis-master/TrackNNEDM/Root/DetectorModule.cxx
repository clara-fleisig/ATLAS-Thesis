// Class includes
#include "TrackNNEDM/DetectorModule.h"

// Local includes

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <math.h>
using namespace std;


// Constructors and destructor
DetectorModule::DetectorModule(unsigned long long index, int volumeId, int layerId, int moduleId, int PixorSCT,
        float center_x, float center_y, float center_z, float center_r, float center_rho,
        float rot_xu, float rot_xv, float rot_xw, 
        float rot_yu, float rot_yv, float rot_yw,
        float rot_zu, float rot_zv, float rot_zw,
        float thickness,
        float minhu, float maxhu, float hv,
        float pitch_u, float pitch_v):
    m_index(index), m_volumeId(volumeId), m_layerId(layerId), m_moduleId(moduleId), m_PixorSCT(PixorSCT),
    m_center_x(center_x), m_center_y(center_y), m_center_z(center_z), m_center_r(center_r), m_center_rho(center_rho),
    m_rot_xu(rot_xu), m_rot_xv(rot_xv), m_rot_xw(rot_xw), m_rot_yu(rot_yu),
    m_rot_yv(rot_yv), m_rot_yw(rot_yw), m_rot_zu(rot_zu), m_rot_zv(rot_zv), m_rot_zw(rot_zw),
    m_thickness(thickness),
    m_minhu(minhu), m_maxhu(maxhu), m_hv(hv),
    m_pitch_u(pitch_u), m_pitch_v(pitch_v)
{

}

DetectorModule::~DetectorModule()
{

} 


double DetectorModule::distanceTo(float x, float y, float z)
{
    return sqrt(pow(x - getX(), 2) + pow(y - getY(), 2) + pow(z - getZ(), 2));
}

void DetectorModule::printDebug() const
{
    std::cout<<"Module index: "<<m_index<<" x: "<<m_center_x<<" y: "<<m_center_y<<" z: "<<m_center_z<<endl;
    std::cout<<" m_minhu: "<<m_minhu<<" m_maxhu: "<<m_maxhu<<" m_hv: "<<m_hv<<endl;
    // std::cout<<" xu: "<<m_rot_xu<<" xv: "<<m_rot_xv<<" xw: "<<m_rot_xw<<endl;
    // std::cout<<" yu: "<<m_rot_yu<<" yv: "<<m_rot_yv<<" yw: "<<m_rot_yw<<endl;
    // std::cout<<" zu: "<<m_rot_zu<<" zv: "<<m_rot_zv<<" zw: "<<m_rot_zw<<endl;

}
