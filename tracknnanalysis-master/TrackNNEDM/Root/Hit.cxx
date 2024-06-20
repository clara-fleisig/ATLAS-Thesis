// Class includes
#include "TrackNNEDM/Hit.h"

// Local includes
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/RecoTrack.h"
#include "TrackNNEDM/DetectorModule.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <math.h>
using namespace std;


// Constructors and destructor

Hit::Hit()
{
    
}
Hit::Hit(Index index, float x, float y, float z)
{
    m_index = index;
    m_x = x;
    m_y = y;
    m_z = z;

    m_r = std::sqrt(std::pow(m_x, 2) + std::pow(m_y, 2) + std::pow(m_z, 2));
    m_rho = std::sqrt(std::pow(m_x, 2) + std::pow(m_y, 2));
    m_phi = atan(getY() / getX());
    m_eta = atanh(getZ() / getR());
}

Hit::Hit(Index index, 
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
        std::vector<Index> trueTrackBarcode):
        m_index(index),
        m_x(x),
        m_y(y),
        m_z(z),
        m_r(r),
        m_rho(rho),
        m_volumeID(volumeID),
        m_layerID(layerID),
        m_moduleID(moduleID),
        m_etaModule(etaModule),
        m_phiModule(phiModule),
        m_pixOrSCT(pixOrSCT),
        m_side(side),
        m_recoTrackIndex(recoTrackIndex),
        m_trueTrackBarcode(trueTrackBarcode),
        m_recoTrack(nullptr)
{
    m_isModifiedHit = false;

    m_phi = atan2(getY(), getX());
    m_eta = atanh(getZ() / getR());
    m_theta = acos(getZ() / getR());
}

Hit::~Hit()
{

} 

// Copy constructor
Hit::Hit(const Hit& hit)
{
    this->m_index            = hit.m_index;
    this->m_x                = hit.m_x;
    this->m_y                = hit.m_y;
    this->m_z                = hit.m_z;
    this->m_r                = hit.m_r;
    this->m_rho              = hit.m_rho;
    this->m_phi              = hit.m_phi;
    this->m_volumeID         = hit.m_volumeID;
    this->m_layerID          = hit.m_layerID;
    this->m_moduleID         = hit.m_moduleID;
    this->m_etaModule        = hit.m_etaModule;
    this->m_phiModule        = hit.m_phiModule;
    this->m_pixOrSCT         = hit.m_pixOrSCT;
    this->m_side             = hit.m_side;
    this->m_recoTrackIndex   = hit.m_recoTrackIndex;
    this->m_trueTrackBarcode = hit.m_trueTrackBarcode;
    this->m_isModifiedHit    = hit.m_isModifiedHit;
    this->m_recoTrack        = hit.m_recoTrack;
    this->m_trueTracks       = hit.m_trueTracks;

}


// big setters
void Hit::setOrgRecoTrack(std::shared_ptr<RecoTrack> track)
{
    if(track->getIndex() != m_recoTrackIndex)
    {
        cout<<"Hit: wrong reco track being associated."<<endl;
        cout<<"Input track "<<track->getIndex()<<endl;
        cout<<"Hit matched track "<<m_recoTrackIndex<<endl;
        exit(1);
    }
    m_recoTrack = track;
}

void Hit::setOrgTrueTracks(std::vector<std::shared_ptr<TrueTrack>> tracks)
{
    for(auto& track: tracks)
    {
        if(std::find(m_trueTrackBarcode.begin(), m_trueTrackBarcode.end(), track->getBarcode()) == m_trueTrackBarcode.end())
        {
            cout<<"Hit: wrong true track being associated."<<endl;
            cout<<"Input track "<<track->getBarcode()<<endl;
            exit(1);
        }
        m_trueTracks[track->getBarcode()] = track;
    }
}
void Hit::printDebug() const
{
    std::cout<<"Hit index: "<<m_index<<" "<<m_recoTrackIndex<<" x: "<<m_x<<" y: "<<m_y<<" z: "<<m_z<<" rho: "<<m_rho<<" r: "<<m_r<<" pixOrSCT: "<<m_pixOrSCT<<" volumeID: "<<m_volumeID<<" layerID: "<<m_layerID<<" moduleID: "<<m_moduleID<<" side: "<<m_side;
    //<<" nTruth: "<<m_trueTrackBarcode.size()<<" recoTrackIndex: "<<m_recoTrackIndex<<" modified: "<<m_isModifiedHit<<endl;
    if(m_trueTrackBarcode.size() > 0) std::cout<<" nTruthBarCode: "<<m_trueTrackBarcode.at(0);
    else std::cout<<" nTruthBarCode: -999";
    std::cout<<std::endl;
    // std::cout<<"Hit index: "<<m_index<<" vol: "<<m_volumeID<<" layer: "<<m_layerID<<" module: "<<m_moduleID<<endl;
}


void Hit::rotateXY(double angle)
{
    m_isModifiedHit = true;

    fillrotatedXYPhi(angle, m_x, m_y, m_phi);
}

void Hit::rotateXY(double cos_angle, double sin_angle)
{
    m_isModifiedHit = true;
    fillrotatedXYPhi(cos_angle, sin_angle, m_x, m_y, m_phi);
}



//don't modify the original x value of the hit, just return a new x coordinate of the barrel unrolled into the XZ plane
float Hit::getUnrolledX(float rho)
{    
    float cylWidth = 2*M_PI*rho;
    //unroll barrel into xz plane. EC distances in xy plane
    float hitX;
    if (this->getY() < 0)
    {
        if (this->getX() > 0) hitX = cylWidth + rho*std::atan(this->getX() / this->getY()); //atan<0
        else hitX = -cylWidth + rho*std::atan(this->getX() / this->getY()); //atan>0

    }
    else
    {
        hitX = rho*std::atan(this->getX() / this->getY()); //if x > 0, atan > 0, if X<0, atan<0
    }

    return hitX;
}

// Input angle is in radians
void Hit::fillrotatedXYPhi(double angle, float& x, float& y, float& phi) const
{
    float _x = getX();
    float _y = getY();
    // float _phi = getPhi();

    x = _x * cos(angle) - _y * sin(angle);
    y = _x * sin(angle) + _y * cos(angle); 
    phi = std::atan(x/y);

    // std::cout<<"Hit: x: "<<x<<" y: "<<y<<" phi: "<<phi<<" _x: "<<_x<<" _y: "<<_y<<" _phi: "<<_phi<<std::endl;
}

void Hit::fillrotatedXYPhi(double cos_angle, double sin_angle, float& x, float& y, float& phi) const
{
    float _x = getX();
    float _y = getY();
    // float _phi = getPhi();

    x = _x * cos_angle - _y * sin_angle;
    y = _x * sin_angle + _y * cos_angle;
    // phi = _phi + std::acos(cos_angle);
    phi = std::atan(x/y);

}


// could use getVar() instead of recalculatin everytime....
float Hit::distanceTo(const std::shared_ptr<Hit> hit) const
{
    return sqrt(pow(this->getX() - hit->getX(), 2) + pow(this->getY() - hit->getY(), 2) + pow(this->getZ() - hit->getZ(), 2));
}

float Hit::distanceToX(const std::shared_ptr<Hit> hit, bool sign) const
{
    if(sign) return (this->getX() - hit->getX());
    return std::abs(this->getX() - hit->getX());
}

float Hit::distanceToY(const std::shared_ptr<Hit> hit, bool sign) const
{
    if(sign) return this->getY() - hit->getY();
    return std::abs(this->getY() - hit->getY());
}

float Hit::distanceTo(float x, float y, float z) const
{
    return sqrt(pow(this->getX() - x, 2) + pow(this->getY() - y, 2) + pow(this->getZ() - z, 2));
}

float Hit::angularDistanceTo(const std::shared_ptr<Hit> hit) const
{
    // float len_a = std::sqrt(pow(this->getX(),2) + pow(this->getY(),2) + pow(this->getZ(),2));
    // float len_b = std::sqrt(pow(hit->getX(),2) + pow(hit->getY(),2) + pow(this->getZ(),2));
    // float norm_aX = this->getX() / len_a;
    // float norm_aY = this->getY() / len_a;
    // float norm_aZ = this->getZ() / len_a;
    // float norm_bX = hit->getX() / len_b;
    // float norm_bY = hit->getY() / len_b;
    // float norm_bZ = hit->getZ() / len_b;
    // float angle = std::acos(norm_aX*norm_bX + norm_aY*norm_bY + norm_aZ*norm_bZ);
    // return abs(angle);
    
    return std::sqrt(std::pow(this->getPhi() - hit->getPhi(), 2) + std::pow(this->getEta() - hit->getEta(),2));

}

float Hit::distanceToRho(const std::shared_ptr<Hit> hit, bool sign) const
{
    if (sign) return this->getRho() - hit->getRho();
    return abs(this->getRho() - hit->getRho());
}

float Hit::distanceToZ(const std::shared_ptr<Hit> hit, bool sign) const
{
    if (sign) return hit->getZ() - this->getZ(); 
    return abs(hit->getZ() - this->getZ());
}

float Hit::distanceToPhi(const std::shared_ptr<Hit> hit, bool /*sign*/) const
{
    //if (sign) return hit->getPhi()- this->getPhi();
    float tan = std::tan(hit->getPhi()- this->getPhi());
    return std::atan(tan);
    //return abs(hit->getPhi()- this->getPhi());
}

bool Hit::isHitinModule(std::shared_ptr<DetectorModule> mod)
{
    // Module info
    vector<float> xValOrg;
    vector<float> yValOrg;
    vector<float> zValOrg;

    xValOrg.push_back(-mod->getHalfMaxWidth());
    yValOrg.push_back(mod->getHalfLength());
    zValOrg.push_back(0);

    xValOrg.push_back(0);
    yValOrg.push_back(mod->getHalfLength());
    zValOrg.push_back(0);


    xValOrg.push_back(mod->getHalfMaxWidth());
    yValOrg.push_back(mod->getHalfLength());
    zValOrg.push_back(0);


    xValOrg.push_back(mod->getHalfMaxWidth());
    yValOrg.push_back(-mod->getHalfLength());
    zValOrg.push_back(0);

    xValOrg.push_back(0);
    yValOrg.push_back(-mod->getHalfLength());
    zValOrg.push_back(0);


    xValOrg.push_back(-mod->getHalfMaxWidth());
    yValOrg.push_back(-mod->getHalfLength());
    zValOrg.push_back(0);


    // xValOrg.push_back(xValOrg.at(0));
    // yValOrg.push_back(yValOrg.at(0));
    // zValOrg.push_back(zValOrg.at(0));


    // Hit rotated info
    float _xOrg = getX();
    float _yOrg = getY();
    float _zOrg = getZ();

    _xOrg -= mod->getX();
    _yOrg -= mod->getY();
    _zOrg -= mod->getZ();
    
    float xVal = -1;
    float yVal = -1;
    float zVal = -1;


    xVal = _xOrg * mod->rot(0, 0) + _yOrg * mod->rot(1, 0) + _zOrg * mod->rot(2, 0);
    yVal = _xOrg * mod->rot(0, 1) + _yOrg * mod->rot(1, 1) + _zOrg * mod->rot(2, 1);
    zVal = _xOrg * mod->rot(0, 2) + _yOrg * mod->rot(1, 2) + _zOrg * mod->rot(2, 2);


    // https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
    static auto checkinSidePoly = [](int nvert, float *vertx, float *verty, float testx, float testy)
    {
        int i, j, c = 0;
        for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((verty[i]>testy) != (verty[j]>testy)) &&
         (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
           c = !c;
        }
        return c; 
    };



    if(fabs(zVal) > 1) return false;


    return checkinSidePoly(xValOrg.size(), &xValOrg[0],  &yValOrg[0], xVal, yVal);

}

std::vector<int> Hit::OneHotEncodeLayerID(int maxLayers)
{ 
    //assumes layers are incrimented by 2
    std::vector<int> ohe(maxLayers,0);
    ohe[this->getLayer()/2] = 1;
    return ohe;
}

std::vector<int> Hit::OneHotEncodeVolumeID(std::vector<int> volumeIDs)
{
    //input a sorted list of volume IDs in the detector
    std::vector<int> ohe(volumeIDs.size(),0);
    std::vector<int>::iterator itr = std::find(volumeIDs.begin(), volumeIDs.end(), this->getVolume());
    if (itr != volumeIDs.end()) 
    {
        ohe[std::distance(volumeIDs.begin(), itr)] = 1;
    }

    return ohe;
}

std::vector<int> Hit::OneHotEncoding(std::vector<int> volumeIDs, int maxLayers)
{
    auto volume = this->OneHotEncodeVolumeID(volumeIDs);
    auto layer = this->OneHotEncodeLayerID(maxLayers);

    volume.insert(volume.end(), layer.begin(), layer.end());
    return volume;
}







