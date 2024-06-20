#ifndef _CIRCLEFITTER_H
#define _CIRCLEFITTER_H

#include <iostream>
#include <cmath>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include "TrackNNEDM/TrueTrack.h"
#include "TrackNNEDM/Hit.h"
using namespace std;

//  define precision by commenting out one of the two lines:

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

reals Sigma (shared_ptr<TrueTrack> track, Circle& circle)
{
    reals sum=0.,dx,dy;
	auto nHits = track->getNHits();
	auto hitsList = track->getHitsList();
    for (auto& hit: hitsList)
    {
        dx = hit->getX() - circle.a;
        dy = hit->getY() - circle.b;
        sum += SQR(sqrt(dx*dx+dy*dy) - circle.r);
    }
    return sqrt(sum/nHits);
}

Circle CircleFitByTaubin(shared_ptr<TrueTrack> track)
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
    
    auto nHits = track->getNHits();
    auto hitList = track->getHitsList();
    Circle circle;
    
    // Compute x- and y- sample means
    double mean_x=0;
    double mean_y=0;
    for (auto& hit: hitList)
    {
        auto x = hit->getX();
        auto y = hit->getY();
        mean_x += x;
        mean_y += y;
    }

    mean_x /= nHits;
    mean_y /= nHits;



//     computing moments 

	Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;
    
    for (auto& hit: hitList)
    {
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
    Mxx /= nHits;
    Myy /= nHits;
    Mxy /= nHits;
    Mxz /= nHits;
    Myz /= nHits;
    Mzz /= nHits;
    
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
    circle.s = Sigma(track,circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too
    
    return circle;
}

int CircleFitByLevenbergMarquardtFull (shared_ptr<TrueTrack> track, Circle& circleIni, reals LambdaIni, Circle& circle)
/*                                     <------------------ Input ------------------->  <-- Output -->

       Geometric circle fit to a given set of data points (in 2D)
		
       Input:  data     - the class of data (contains the given points):
		
	       data.n   - the number of data points
	       data.X[] - the array of X-coordinates
	       data.Y[] - the array of Y-coordinates
		          
               circleIni - parameters of the initial circle ("initial guess")
		        
	       circleIni.a - the X-coordinate of the center of the initial circle
	       circleIni.b - the Y-coordinate of the center of the initial circle
	       circleIni.r - the radius of the initial circle
		        
	       LambdaIni - the initial value of the control parameter "lambda"
	                   for the Levenberg-Marquardt procedure
	                   (common choice is a small positive number, e.g. 0.001)
		        
       Output:
	       integer function value is a code:
	                  0:  normal termination, the best fitting circle is 
	                      successfully found
	                  1:  the number of outer iterations exceeds the limit (99)
	                      (indicator of a possible divergence)
	                  2:  the number of inner iterations exceeds the limit (99)
	                      (another indicator of a possible divergence)
	                  3:  the coordinates of the center are too large
	                      (a strong indicator of divergence)
		                   
	       circle - parameters of the fitting circle ("best fit")
		        
	       circle.a - the X-coordinate of the center of the fitting circle
	       circle.b - the Y-coordinate of the center of the fitting circle
 	       circle.r - the radius of the fitting circle
 	       circle.s - the root mean square error (the estimate of sigma)
 	       circle.i - the total number of outer iterations (updating the parameters)
 	       circle.j - the total number of inner iterations (adjusting lambda)
 		        
       Algorithm:  Levenberg-Marquardt running over the full parameter space (a,b,r)
                         
       See a detailed description in Section 4.5 of the book by Nikolai Chernov:
       "Circular and linear regression: Fitting circles and lines by least squares"
       Chapman & Hall/CRC, Monographs on Statistics and Applied Probability, volume 117, 2010.
         
		Nikolai Chernov,  February 2014
*/
{
    int code,i,iter,inner,IterMAX=99;
    
    reals factorUp=10.,factorDown=0.04,lambda,ParLimit=1.e+6;
    reals dx,dy,ri,u,v;
    reals Mu,Mv,Muu,Mvv,Muv,Mr,UUl,VVl,Nl,F1,F2,F3,dX,dY,dR;
    reals epsilon=3.e-8;
    reals G11,G22,G33,G12,G13,G23,D1,D2,D3;
    auto hitList = track->getHitsList();
    auto nHits = track->getNHits();
    Circle Old,New;

    // Compute x- and y- sample means
    double mean_x=0;
    double mean_y=0;
    for (auto& hit: hitList)
    {
        auto x = hit->getX();
        auto y = hit->getY();
        mean_x += x;
        mean_y += y;
    }

    mean_x /= nHits;
    mean_y /= nHits;
    
    //       starting with the given initial circle (initial guess)
    
    New = circleIni;
    
    //       compute the root-mean-square error via function Sigma; see Utilities.cpp

        New.s = Sigma(track,New);
        
    //       initializing lambda, iteration counters, and the exit code
        
        lambda = LambdaIni;
        iter = inner = code = 0;
        
    NextIteration:
        
        Old = New;
        if (++iter > IterMAX) {code = 1;  goto enough;}
        
    //       computing moments
        
        Mu=Mv=Muu=Mvv=Muv=Mr=0.;
        
        for (auto& hit: hitList)
        {
            dx = hit->getX() - Old.a;
            dy = hit->getY() - Old.b;
            ri = sqrt(dx*dx + dy*dy);
            u = dx/ri;
            v = dy/ri;
            Mu += u;
            Mv += v;
            Muu += u*u;
            Mvv += v*v;
            Muv += u*v;
            Mr += ri;
        }
        Mu  /=  nHits;
        Mv  /=  nHits;
        Muu /=  nHits;
        Mvv /=  nHits;
        Muv /=  nHits;
        Mr  /=  nHits;
        
    //       computing matrices
        
        F1 = Old.a + Old.r*Mu - mean_x;
        F2 = Old.b + Old.r*Mv - mean_y;
        F3 = Old.r - Mr;
        
        Old.g = New.g = sqrt(F1*F1 + F2*F2 + F3*F3);
        
    try_again:
        
        UUl = Muu + lambda;
        VVl = Mvv + lambda;
        Nl = One + lambda;
        
    //         Cholesly decomposition
        
        G11 = sqrt(UUl);
        G12 = Muv/G11;
        G13 = Mu/G11;
        G22 = sqrt(VVl - G12*G12);
        G23 = (Mv - G12*G13)/G22;
        G33 = sqrt(Nl - G13*G13 - G23*G23);
        
        D1 = F1/G11;
        D2 = (F2 - G12*D1)/G22;
        D3 = (F3 - G13*D1 - G23*D2)/G33;
        
        dR = D3/G33;
        dY = (D2 - G23*dR)/G22;
        dX = (D1 - G12*dY - G13*dR)/G11;
        
        if ((abs(dR)+abs(dX)+abs(dY))/(One+Old.r) < epsilon) goto enough;
        
    //       updating the parameters
        
        New.a = Old.a - dX;
        New.b = Old.b - dY;
        
        if (abs(New.a)>ParLimit || abs(New.b)>ParLimit) {code = 3; goto enough;}
        
        New.r = Old.r - dR;
        
        if (New.r <= 0.)
        {
            lambda *= factorUp;
            if (++inner > IterMAX) {code = 2;  goto enough;}
            goto try_again;
        }
        
    //       compute the root-mean-square error via function Sigma; see Utilities.cpp

        New.s = Sigma(track,New);   
        
    //       check if improvement is gained
        
        if (New.s < Old.s)    //   yes, improvement
        {
            lambda *= factorDown;
            goto NextIteration;
        }
        else                       //   no improvement
        {
            if (++inner > IterMAX) {code = 2;  goto enough;}
            lambda *= factorUp;
            goto try_again;
        }
        
        //       exit
        
    enough:
        
        Old.i = iter;    // total number of outer iterations (updating the parameters)
        Old.j = inner;   // total number of inner iterations (adjusting lambda)
        
        circle = Old;
        
        return code;

}

pair<pair<float,float>, pair<float,float>> get_circle_intersections_with_barrelLayer(shared_ptr<TrackBase> track, int volumeId, int layerId)
// Return the intersection of a fitted circle with the detector layer 
{
    Circle fitCircle = CircleFitByTaubin(track);
    x0 = fitCircle.a;
    y0 = fitCircle.b;
    r0 = fitCircle.r;

    x1 = 0;
    y1 = 0;
    auto detector = make_pair(volId, layerId);
    r1 = m_barrelLoc[detector];

    float d = sqrt(pow(x1-x0,2) + pow(y1-y0,2));
    if (d > r0 + r1){
        cout<<"non intersection"<<endl;
        exit(1);
    }
    else if (d < abs(r0 - r1)){
        cout<<"nested circles"<<endl;
        exit(1);
    }
    else if (d==0 && r0 = r1){
        cout<<"coincident circles"<<endl;
        exit(1);
    }
    else{
        float a=(r0*r0-r1*r1+d**2)/(2*d)
        float h = sqrt(r0*r0 - a*a);
        float x2=x0+a*(x1-x0)/d 
        float y2=y0+a*(y1-y0)/d
        float x3=x2+h*(y1-y0)/d
        float y3=y2-h*(x1-x0)/d
        float x4=x2-h*(y1-y0)/d
        float y4=y2+h*(x1-x0)/d

        pair<float, float> p1 = make_pair(x3,y3);
        pair<float, float> p2 = make_pair(x4,y4);
        pair<pair<float,float>, pair<float,float>> points = make_pair(p1,p2);
        return points;
    }
}

pair<float,float>> get_circle_intersection_with_barrelLayer(shared_ptr<TrackBase> track, int volumeId, int layerId)
{
    // Find the intersection point closest to our hits //

    auto points = get_circle_intersections_with_barrelLayer(track, volumeId, layerId);
    p1 = points.first;
    p2 = points.second;
    p1x = p1.first;
    p1y = p1.second;
    p2x = p2.first;
    p2y = p2.second;

    float meanX = 0;
    float meanY = 0;
    auto hits = track->getHitsList();
    for(const auto& hit: hits)
    {
        meanX += hit->getX();
        meanY += hit->getY();
    }
    meanX /= hits.size();
    meanY /= hits.size();

    d1 = sqrt(pow(p1x - meanX,2) + pow(p1y - meanY,2));
    d2 = sqrt(pow(p2x - meanX,2) + pow(p2y - meanY,2));

    if (d1 < d2) return p1;
    else return p2;
    

}


#endif