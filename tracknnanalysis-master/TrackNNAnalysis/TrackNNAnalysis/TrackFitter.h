#ifndef _TRACKFITTER_H
#define _TRACKFITTER_H
#include <iostream>
#include <cmath>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include "TrackNNEDM/TrackBase.h"
#include "TrackNNEDM/Hit.h"
using namespace std;

typedef double reals;       //  defines reals as double (standard for scientific calculations)
//typedef long double reals;  //  defines reals as long double 

//   Note: long double is an 80-bit format (more accurate, but more memory demanding and slower)

typedef long long integers;

//   next define some frequently used constants:

const reals One=1.0,Two=2.0,Three=3.0,Four=4.0,Five=5.0,Six=6.0,Ten=10.0;
//const reals One=1.0L,Two=2.0L,Three=3.0L,Four=4.0L,Five=5.0L,Six=6.0L,Ten=10.0L;
const reals Pi=3.141592653589793238462643383L;
const reals REAL_MAX=numeric_limits<reals>::max();
const reals REAL_MIN=numeric_limits<reals>::min();
const reals REAL_EPSILON=numeric_limits<reals>::epsilon();

//   next define some frequently used functions:
template<typename T>
inline T SQR(T t) { return t*t; }

reals pythag(reals a, reals b)
{
	reals absa=abs(a),	absb=abs(b);
	if (absa > absb) return absa*sqrt(One+SQR(absb/absa));
	else return (absb == 0.0 ? 0.0 : absb*sqrt(One+SQR(absa/absb)));
}


/************************************************************************
			DECLARATION OF THE CLASS CIRCLE
************************************************************************/
// Class for Circle
// A circle has 7 fields: 
//     a, b, r (of type reals), the circle parameters
//     s (of type reals), the estimate of sigma (standard deviation)
//     g (of type reals), the norm of the gradient of the objective function
//     i and j (of type int), the iteration counters (outer and inner, respectively)

class Circle
{
    public:

        // The fields of a Circle
        reals a, b, r, s, g, Gx, Gy;
        int i, j;

        Circle()
        {
            a=0.; b=0.; r=1.; s=0.; i=0; j=0;
        };

        // Constructor with assignment of the circle parameters only
        Circle(reals aa, reals bb, reals rr)
        {
            a=aa; b=bb; r=rr;
        };

        // Printing routine
        void print()
        {
            cout << endl;
            cout << setprecision(10) << "center (" <<a <<","<< b <<")  radius "
                << r << "  sigma " << s << "  gradient " << g << "  iter "<< i << "  inner " << j << endl;
        };

};

reals Sigma (std::vector<std::shared_ptr<Hit>> hitList, Circle& circle)
{
    reals sum=0.,dx,dy;
	auto nHits = hitList.size();
    for (auto& hit: hitList)
    {
        dx = hit->getX() - circle.a;
        dy = hit->getY() - circle.b;
        sum += SQR(sqrt(dx*dx+dy*dy) - circle.r);
    }
    return sqrt(sum/nHits);
}


class TrackFitter {
 
    map<pair<int,int>,float> m_barrelLoc; // Mapping of barrel volume/layerID to average rho of that layer
    int m_seedSize;
    public:
        // Constructor to provide the default
        // values to all the terms in the
        // object of class regression
        TrackFitter(int seedSize)
        {
            m_seedSize = seedSize; // Not currently implimented

            pair<int,int> vol16Layer4 = make_pair(16,4);
            m_barrelLoc[vol16Layer4] = 227.49887;
            pair<int,int> vol16Layer6 = make_pair(16,6);
            m_barrelLoc[vol16Layer6] = 290.464;
            pair<int,int> vol23Layer2 = make_pair(23,2);
            m_barrelLoc[vol23Layer2] = 395.6816;
            pair<int,int> vol23Layer4 = make_pair(23,4);
            m_barrelLoc[vol23Layer4] = 558.3952;
            pair<int,int> vol23Layer6 = make_pair(23,6);
            m_barrelLoc[vol23Layer6] = 757.9464;
            pair<int,int> vol23Layer8 = make_pair(23,8);
            m_barrelLoc[vol23Layer8] = 996.0959;
        }
    
        // Function that calculate the coefficient/
        // slope of the best fitting line

        pair<float,float> fitLine(vector<shared_ptr<Hit>> hitList)
        {
            float N = m_seedSize;
            float sum_xy = 0; 
            float sum_x = 0;
            float sum_y =0;
            float sum_x_square = 0;
            float sum_y_square = 0;
            int nHits = hitList.size();

            //for (const auto& hit: hits)
            shared_ptr<Hit> hit;
            for (int i = nHits - m_seedSize; i < nHits; i++)
            {
                hit = hitList.at(i);
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
            float coeff = numerator / denominator; 
            //Calculate constant term
            numerator
                = (sum_y * sum_x_square - sum_x * sum_xy);
            denominator
                = (N * sum_x_square - sum_x * sum_x);
            float constTerm = numerator / denominator;

            pair<float,float> line = make_pair(coeff, constTerm);
            return line;
        }

        float predictZBarrelHit(vector<shared_ptr<Hit>> hitList, int volId, int layerId, bool debug)
        {
            auto detector = make_pair(volId, layerId);
            auto trueRho = m_barrelLoc[detector];

            auto line = fitLine(hitList);
            // Predicted Z coordinate is where the line intersects the layer
            float predZ = (trueRho - line.second) / line.first;
            if (debug) {
                std::cout<<"predicted line "<<line.first<<", "<<line.second<<std::endl;
                std::cout<<"predicted coordinate "<<predZ<<", "<<trueRho<<std::endl;
            }
            
            return predZ;
        }

        Circle CircleFitByTaubin(vector<shared_ptr<Hit>> hitList)
        /*  
            Circle fit to a given set of data points (in 2D)
            
            This is an algebraic fit, due to Taubin, based on the journal article
            
            G. Taubin, "Estimation Of Planar Curves, Surfaces And Nonplanar
                        Space Curves Defined By Implicit Equations, With 
                        Applications To Edge And Range Image Segmentation",
                        IEEE Trans. PAMI, Vol. 13, pages 1115-1138, (1991)

            Input:  data     - the class of data (contains the given points):
                
                data.n   - the number of data points
                data.X[] - the array of X-coordinates
                data.Y[] - the array of Y-coordinates
            
            Output:	       
                    circle - parameters of the fitting circle:
                        
                circle.a - the X-coordinate of the center of the fitting circle
                circle.b - the Y-coordinate of the center of the fitting circle
                circle.r - the radius of the fitting circle
                circle.s - the root mean square error (the estimate of sigma)
                circle.j - the total number of iterations
                    
            The method is based on the minimization of the function
            
                        sum [(x-a)^2 + (y-b)^2 - R^2]^2
                    F = -------------------------------
                            sum [(x-a)^2 + (y-b)^2]
                        
            This method is more balanced than the simple Kasa fit.
                
            It works well whether data points are sampled along an entire circle or
            along a small arc. 
            
            It still has a small bias and its statistical accuracy is slightly
            lower than that of the geometric fit (minimizing geometric distances),
            but slightly higher than that of the very similar Pratt fit. 
            Besides, the Taubin fit is slightly simpler than the Pratt fit
            
            It provides a very good initial guess for a subsequent geometric fit. 
            
            Nikolai Chernov  (September 2012)

        */
        {
            int i,iter,IterMAX=99;
            
            reals Xi,Yi,Zi;
            reals Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
            reals A0,A1,A2,A22,A3,A33;
            reals Dy,xnew,x,ynew,y;
            reals DET,Xcenter,Ycenter;
            
            auto nHits = hitList.size();
            Circle circle;
            
            // Compute x- and y- sample means
            double mean_x=0;
            double mean_y=0;
            //for (auto& hit: hitList)
            for(int i=nHits-m_seedSize; i<nHits; i++)
            {
                auto hit = hitList.at(i);
                auto x = hit->getX();
                auto y = hit->getY();
                mean_x += x;
                mean_y += y;
            }

            mean_x /= m_seedSize;
            mean_y /= m_seedSize;


        //     computing moments 

            Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;
            
            //for (auto& hit: hitList)
            shared_ptr<Hit> hit;
            for (size_t i= nHits- m_seedSize; i<nHits; i++)
            {
                hit = hitList.at(i);
                Xi = hit->getX() - mean_x;   //  centered x-coordinates
                Yi = hit->getY() - mean_y;   //  centered y-coordinates
                Zi = Xi*Xi + Yi*Yi;
                
                Mxy += Xi*Yi;
                Mxx += Xi*Xi;
                Myy += Yi*Yi;
                Mxz += Xi*Zi;
                Myz += Yi*Zi;
                Mzz += Zi*Zi;
            }
            Mxx /= m_seedSize;
            Myy /= m_seedSize;
            Mxy /= m_seedSize;
            Mxz /= m_seedSize;
            Myz /= m_seedSize;
            Mzz /= m_seedSize;
            
        //      computing coefficients of the characteristic polynomial
            
            Mz = Mxx + Myy;
            Cov_xy = Mxx*Myy - Mxy*Mxy;
            Var_z = Mzz - Mz*Mz;
            A3 = Four*Mz;
            A2 = -Three*Mz*Mz - Mzz;
            A1 = Var_z*Mz + Four*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
            A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
            A22 = A2 + A2;
            A33 = A3 + A3 + A3;

        //    finding the root of the characteristic polynomial
        //    using Newton's method starting at x=0  
        //     (it is guaranteed to converge to the right root)
            
            for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
            {
                Dy = A1 + x*(A22 + A33*x);
                xnew = x - y/Dy;
                if ((xnew == x)||(!isfinite(xnew))) break;
                ynew = A0 + xnew*(A1 + xnew*(A2 + xnew*A3));
                if (abs(ynew)>=abs(y))  break;
                x = xnew;  y = ynew;
            }
            
        //       computing paramters of the fitting circle
            
            DET = x*x - x*Mz + Cov_xy;
            Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/Two;
            Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/Two;

        //       assembling the output

            circle.a = Xcenter + mean_x;
            circle.b = Ycenter + mean_y;
            circle.r = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz);
            std::vector<std::shared_ptr<Hit>> useHits;
            for(int i=nHits-m_seedSize; i<nHits; i++){
                useHits.push_back(hitList.at(i));
            }
            circle.s = Sigma(useHits,circle); // Use all the hits or only the most recent 3 hits?
            circle.i = 0;
            circle.j = iter;  //  return the number of iterations, too
            
            return circle;
        }

        bool get_circle_intersections_with_barrelLayer(pair<pair<float,float>, pair<float,float>>& intersectingPoints,vector<shared_ptr<Hit>> hitList, int volumeId, int layerId, bool debug)
        // Return the points of intersection of a fitted circle with the detector layer 
        {
            Circle fitCircle = CircleFitByTaubin(hitList);
            if(debug){
                std::cout<<"Fit Circle"<<std::endl;
                std::cout<<fitCircle.a<<", "<<fitCircle.b<<", "<<fitCircle.r<<", "<<fitCircle.s<<", "<<fitCircle.j<<std::endl;
            }
            float x0 = fitCircle.a;
            float y0 = fitCircle.b;
            float r0 = fitCircle.r;

            float x1 = 0;
            float y1 = 0;
            auto detector = make_pair(volumeId, layerId);
            float r1 = m_barrelLoc[detector];

            float d = sqrt(pow(x1-x0,2) + pow(y1-y0,2));
            if (d > r0 + r1){
                cout<<"non intersection"<<endl;
                return false;
                // exit(1);
            }
            else if (d < abs(r0 - r1)){
                cout<<"nested circles"<<endl;
                return false;
                // exit(1);
            }
            else if (d==0 && r0 == r1){
                cout<<"coincident circles"<<endl;
                return false;
                //exit(1);
            }
            else{
                float a=(r0*r0-r1*r1+d*d)/(2*d);
                float h = sqrt(r0*r0 - a*a);
                float x2=x0+a*(x1-x0)/d;
                float y2=y0+a*(y1-y0)/d;
                float x3=x2+h*(y1-y0)/d;
                float y3=y2-h*(x1-x0)/d;
                float x4=x2-h*(y1-y0)/d;
                float y4=y2+h*(x1-x0)/d;

                pair<float, float> p1 = make_pair(x3,y3);
                pair<float, float> p2 = make_pair(x4,y4);
                pair<pair<float,float>, pair<float,float>> points = make_pair(p1,p2);
                intersectingPoints= points;
                return true;
            }
            
            

        }

        bool get_circle_intersection_with_barrelLayer(pair<float,float>& intersectingPoint, vector<shared_ptr<Hit>> hitList, int volumeId, int layerId, bool debug)
        {
            // Find the intersection point closest to our hits //
            pair<pair<float,float>, pair<float,float>> intersectingPoints;
            auto success = get_circle_intersections_with_barrelLayer(intersectingPoints, hitList, volumeId, layerId, debug);
            if(!success){
                return success;
            }
            auto p1 = intersectingPoints.first;
            auto p2 = intersectingPoints.second;
            auto p1x = p1.first;
            auto p1y = p1.second;
            auto p2x = p2.first;
            auto p2y = p2.second;

            float meanX = 0;
            float meanY = 0;
            for(const auto& hit: hitList)
            {
                meanX += hit->getX();
                meanY += hit->getY();
            }
            meanX /= hitList.size();
            meanY /= hitList.size();

            float d1 = sqrt(pow(p1x - meanX,2) + pow(p1y - meanY,2));
            float d2 = sqrt(pow(p2x - meanX,2) + pow(p2y - meanY,2));
            
            if(debug){
                std::cout<<"predicted coordinate ";
                if (d1<d2) std::cout<<p1x<<", "<<p1y<<std::endl;
                else std::cout<<p2x<<", "<<p2y<<std::endl;
            }
            intersectingPoint =  d1<d2 ? p1: p2;
            return true;
            

        }
        



};





#endif