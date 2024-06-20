// Class includes
#include "TrackNNEDM/SpacePoint.h"

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

SpacePoint::SpacePoint():Hit()
{
    
}
SpacePoint::SpacePoint(Index index, float x, float y, float z): Hit(index, x, y, z)
{}

SpacePoint::SpacePoint(Index index, 
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
        std::vector<Index> trueTrackBarcode,
        Index FirstHitClusterIndex,
        Index SecondHitClusterIndex):
        Hit(index, x, y, z, r, rho, volumeID, layerID, moduleID, etaModule, phiModule, pixOrSCT, side, recoTrackIndex, trueTrackBarcode),
        m_FirstHitClusterIndex(FirstHitClusterIndex),
        m_SecondHitClusterIndex(SecondHitClusterIndex)
{

}

SpacePoint::~SpacePoint()
{

} 