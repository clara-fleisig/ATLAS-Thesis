#ifndef _LINEARFITTER_H
#define _LINEARFITTER_H

#include <iostream>
#include <cmath>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/Hit.h"

using namespace std;
class LinearFitter {
 
    map<pair<int,int>,float> m_barrelLoc; // Mapping of barrel volume/layerID to average rho of that layer
 
    public:
        // Constructor to provide the default
        // values to all the terms in the
        // object of class regression
        LinearFitter()
        {
            pair<int,int> vol16Layer4 = make_pair(16,4);
            barrelLoc[vol16Layer4] = 227.49887;
            pair<int,int> vol16Layer6 = make_pair(16,6);
            barrelLoc[vol16Layer4] = 290.464;
            pair<int,int> vol23Layer2 = make_pair(23,2);
            barrelLoc[vol16Layer4] = 395.6816;
            pair<int,int> vol23Layer4 = make_pair(23,4);
            barrelLoc[vol16Layer4] = 558.3952;
            pair<int,int> vol23Layer6 = make_pair(23,6);
            barrelLoc[vol16Layer6] = 757.9464;
            pair<int,int> vol23Layer8 = make_pair(23,8);
            barrelLoc[vol16Layer8] = 996.0959;
        }
    
        // Function that calculate the coefficient/
        // slope of the best fitting line

        pair<float,float> fitLine(shared_ptr<TrackBase> track)
        {
            auto hits = track->getHitsList();
            N = hits.size();

            for (const auto& hit: hits)
            {
                sum_xy += hit->getZ() * hit->getRho();
                sum_x += hit->getZ();
                sum_y += hit->getRho();
                sum_x_square += hit->getZ() * hit->getZ();
                sum_y_square += hit->getRho() * hit->getRho();
            }
            
            //Calculate coefficient
            float numerator
                = (N * sum_xy - sum_x * sum_y);
            float denominator
                = (N * sum_x_square - sum_x * sum_x);
            coeff = numerator / denominator; 
            //Calculate constant term
            numerator
                = (sum_y * sum_x_square - sum_x * sum_xy);
            denominator
                = (N * sum_x_square - sum_x * sum_x);
            constTerm = numerator / denominator;

            pair<float,float> line = make_pair(coeff, constTerm);
            return line;
        }

        float predictZBarrelHit(shared_ptr<TrackBase> track, int volId, int layerId)
        {
            auto detector = make_pair(volId, layerId);
            auto trueRho = m_barrelLoc[detector];

            auto line = fitLine(track);
            // Predicted Z coordinate is where the line intersects the layer
            predZ = (trueRho - line.second) / line.first;
            return predZ;
        }
        



}

#endif