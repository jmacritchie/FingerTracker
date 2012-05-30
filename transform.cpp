// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>
#include "fullhand.h"
#include "transform.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
using namespace std;

typedef std::vector <CvPoint> CoorVec;
typedef std::vector <CvPoint>::iterator CoorVecIt;
typedef std::vector <double> DistVec;

#define THUMB 0
#define FSTFING 1
#define SCNDFING 2
#define THRDFING 3
#define PINKIE 4
#define THUMB_B 5
#define FSTFING_B 6
#define SCNDFING_B 7
#define THRDFING_B 8
#define PINKIE_B 9
#define FSTFING_C 10
#define SCNDFING_C 11
#define THRDFING_C 12
#define PINKIE_C 13

//creates Transform object with the coordinates of the hand for the current and the next frame as well as the distances calculated between each finger point and the base point. The hand object is included by reference to be able to manipulate its values and also to provide info if it is the left or right hand.

Transform::Transform(CoorVec fingers1, DistVec dist1, CoorVec fingers2, DistVec dist2, Fullhand &either_hand)
{
	point_current = fingers1;
	point_next = fingers2;
	dist_current=dist1;
	dist_next = dist2;
	argh = either_hand;
	argh = calculate_trans();
	either_hand =argh;
}

Fullhand Transform::calculate_trans()
{
	int number_detected=0;
	//initialise members to be zero
	scale_thumb, scale_fst, scale_scnd, scale_thrd, scale_pinkie = 0;
	scale_thumb_b, scale_fst_b, scale_scnd_b, scale_thrd_b, scale_pinkie_b = 0;
	scale_thumb_c, scale_fst_c, scale_scnd_c, scale_thrd_c, scale_pinkie_c = 0;
	rot_thumb, rot_fst, rot_scnd, rot_thrd, rot_pinkie=0;
	rot_thumb_b, rot_fst_b, rot_scnd_b, rot_thrd_b, rot_pinkie_b=0;
	rot_thumb_c, rot_fst_c, rot_scnd_c, rot_thrd_c, rot_pinkie_c=0;
	same_thumb, same_first, same_second, same_third, same_pinkie=false;
	same_thumb_b, same_first_b, same_second_b, same_third_b, same_pinkie_b=false;
	same_thumb_c, same_first_c, same_second_c, same_third_c, same_pinkie_c=false;

	//calculate distance between point in current frame and next frame.If distance is zero, it hasnt moved! i.e.no scaling or rotation
	calc_distances(point_current[THUMB], point_next[THUMB], argh.occ_thumb, dist_thumbs, same_thumb, scale_thumb, rot_thumb, number_detected);
	calc_distances(point_current[FSTFING], point_next[FSTFING], argh.lost_fst, dist_fsts, same_first, scale_fst, rot_fst, number_detected);
	calc_distances(point_current[SCNDFING], point_next[SCNDFING], argh.lost_scnd, dist_scnds, same_second, scale_scnd, rot_scnd, number_detected);
	calc_distances(point_current[THRDFING], point_next[THRDFING], argh.lost_thrd, dist_thrds, same_third, scale_thrd, rot_thrd, number_detected);
	calc_distances(point_current[PINKIE], point_next[PINKIE], argh.hid_pinkie, dist_pinkies, same_pinkie, scale_pinkie, rot_pinkie, number_detected);

	calc_distances(point_current[THUMB_B], point_next[THUMB_B], argh.occ_thumb_b, dist_thumbs_b, same_thumb_b, scale_thumb_b, rot_thumb_b, number_detected);
	calc_distances(point_current[FSTFING_B], point_next[FSTFING_B], argh.lost_fst_b, dist_fsts_b, same_first_b, scale_fst_b, rot_fst_b, number_detected);
	calc_distances(point_current[SCNDFING_B], point_next[SCNDFING_B], argh.lost_scnd_b, dist_scnds_b, same_second_b, scale_scnd_b, rot_scnd_b, number_detected);
	calc_distances(point_current[THRDFING_B], point_next[THRDFING_B], argh.lost_thrd_b, dist_thrds_b, same_third_b, scale_thrd_b, rot_thrd_b, number_detected);
	calc_distances(point_current[PINKIE_B], point_next[PINKIE_B], argh.hid_pinkie_b, dist_pinkies_b, same_pinkie_b, scale_pinkie_b, rot_pinkie_b, number_detected);

	calc_distances(point_current[FSTFING_C], point_next[FSTFING_C], argh.lost_fst_c, dist_fsts_c, same_first_c, scale_fst_c, rot_fst_c, number_detected);
	calc_distances(point_current[SCNDFING_C], point_next[SCNDFING_C], argh.lost_scnd_c, dist_scnds_c, same_second_c, scale_scnd_c, rot_scnd_c, number_detected);
	calc_distances(point_current[THRDFING_C], point_next[THRDFING_C], argh.lost_thrd_c, dist_thrds_c, same_third_c, scale_thrd_c, rot_thrd_c, number_detected);
	calc_distances(point_current[PINKIE_C], point_next[PINKIE_C], argh.hid_pinkie_c, dist_pinkies_c, same_pinkie_c, scale_pinkie_c, rot_pinkie_c, number_detected);

	//calculate (if applicable) scaling and rotation
	calc_rotation(point_current[THUMB], point_next[THUMB], argh.occ_thumb, same_thumb, dist_current[THUMB], dist_next[THUMB], rot_thumb, scale_thumb);
	calc_rotation(point_current[FSTFING], point_next[FSTFING], argh.lost_fst, same_first, dist_current[FSTFING], dist_next[FSTFING], rot_fst, scale_fst);
	calc_rotation(point_current[SCNDFING], point_next[SCNDFING], argh.lost_scnd, same_second, dist_current[SCNDFING], dist_next[SCNDFING], rot_scnd, scale_scnd);
	calc_rotation(point_current[THRDFING], point_next[THRDFING], argh.lost_thrd, same_third, dist_current[THRDFING], dist_next[THRDFING], rot_thrd, scale_thrd);
	calc_rotation(point_current[PINKIE], point_next[PINKIE], argh.hid_pinkie, same_pinkie, dist_current[PINKIE], dist_next[PINKIE], rot_pinkie, scale_pinkie);
	
	calc_rotation(point_current[THUMB_B], point_next[THUMB_B], argh.occ_thumb_b, same_thumb_b, dist_current[THUMB_B], dist_next[THUMB_B], rot_thumb_b, scale_thumb_b);
	calc_rotation(point_current[FSTFING_B], point_next[FSTFING_B], argh.lost_fst_b, same_first_b, dist_current[FSTFING_B], dist_next[FSTFING_B], rot_fst_b, scale_fst_b);
	calc_rotation(point_current[SCNDFING_B], point_next[SCNDFING_B], argh.lost_scnd_b, same_second_b, dist_current[SCNDFING_B], dist_next[SCNDFING_B], rot_scnd_b, scale_scnd_b);
	calc_rotation(point_current[THRDFING_B], point_next[THRDFING_B], argh.lost_thrd_b, same_third_b, dist_current[THRDFING_B], dist_next[THRDFING_B], rot_thrd_b, scale_thrd_b);
	calc_rotation(point_current[PINKIE_B], point_next[PINKIE_B], argh.hid_pinkie_b, same_pinkie_b, dist_current[PINKIE_B], dist_next[PINKIE_B], rot_pinkie_b, scale_pinkie_b);

	calc_rotation(point_current[FSTFING_C], point_next[FSTFING_C], argh.lost_fst_c, same_first_c, dist_current[FSTFING_C], dist_next[FSTFING_C], rot_fst_c, scale_fst_c);
	calc_rotation(point_current[SCNDFING_C], point_next[SCNDFING_C], argh.lost_scnd_c, same_second_c, dist_current[SCNDFING_C], dist_next[SCNDFING_C], rot_scnd_c, scale_scnd_c);
	calc_rotation(point_current[THRDFING_C], point_next[THRDFING_C], argh.lost_thrd_c, same_third_c, dist_current[THRDFING_C], dist_next[THRDFING_C], rot_thrd_c, scale_thrd_c);
	calc_rotation(point_current[PINKIE_C], point_next[PINKIE_C], argh.hid_pinkie_c, same_pinkie_c, dist_current[PINKIE_C], dist_next[PINKIE_C], rot_pinkie_c, scale_pinkie_c);
	
	
	
	if(argh.occ_thumb && point_current[THUMB].x && point_current[THUMB].y)
		{argh = find_fingers(THUMB, number_detected);}
	if(argh.lost_fst && point_current[FSTFING].x && point_current[FSTFING].y)
		{argh = find_fingers(FSTFING, number_detected);}
	if(argh.lost_scnd && point_current[SCNDFING].x && point_current[SCNDFING].y)
		{argh = find_fingers(SCNDFING, number_detected);}
	if(argh.lost_thrd && point_current[THRDFING].x && point_current[THRDFING].y)
		{argh = find_fingers(THRDFING, number_detected);}
	if(argh.hid_pinkie && point_current[PINKIE].x && point_current[PINKIE].y)
		{argh = find_fingers(PINKIE, number_detected);}

	if(argh.occ_thumb_b && point_current[THUMB_B].x && point_current[THUMB_B].y)
		{argh = find_fingers(THUMB_B, number_detected);}
	if(argh.lost_fst_b && point_current[FSTFING_B].x && point_current[FSTFING_B].y)
		{argh = find_fingers(FSTFING_B, number_detected);}
	if(argh.lost_scnd_b && point_current[SCNDFING_B].x && point_current[SCNDFING_B].y)
		{argh = find_fingers(SCNDFING_B, number_detected);}
	if(argh.lost_thrd_b && point_current[THRDFING_B].x && point_current[THRDFING_B].y)
		{argh = find_fingers(THRDFING_B, number_detected);}
	if(argh.hid_pinkie_b && point_current[PINKIE_B].x && point_current[PINKIE_B].y)
		{argh = find_fingers(PINKIE_B, number_detected);}

	if(argh.lost_fst_c && point_current[FSTFING_C].x && point_current[FSTFING_C].y)
		{argh = find_fingers(FSTFING_C, number_detected);}
	if(argh.lost_scnd_c && point_current[SCNDFING_C].x && point_current[SCNDFING_C].y)
		{argh = find_fingers(SCNDFING_C, number_detected);}
	if(argh.lost_thrd_b && point_current[THRDFING_C].x && point_current[THRDFING_C].y)
		{argh = find_fingers(THRDFING_C, number_detected);}
	if(argh.hid_pinkie_b && point_current[PINKIE_C].x && point_current[PINKIE_C].y)
		{argh = find_fingers(PINKIE_C, number_detected);}

	return(argh);
}

Fullhand Transform::find_fingers(int fingerNo, int number_detected)
{
	//takes last appearance of finger coordinates and with average of calculated rotations and scalings from other fingers, works out estimated position
	CvPoint lastKnownPos = point_current[fingerNo];
	double last_dist = dist_current[fingerNo];
	double last_ang = angle_cart2polar(point_current[fingerNo],dist_current[fingerNo]);
	//calculate average scaling
	double avg_scale = (scale_thumb + scale_fst + scale_scnd + scale_thrd + scale_pinkie + scale_thumb_b + scale_fst_b + scale_scnd_b + scale_thrd_b + scale_pinkie_b + scale_fst_c + scale_scnd_c + scale_thrd_c + scale_pinkie_c)/number_detected;
	double avg_rot = (rot_thumb + rot_fst + rot_scnd + rot_thrd + rot_pinkie + rot_thumb_b + rot_fst_b + rot_scnd_b + rot_thrd_b + rot_pinkie_b + rot_fst_c + rot_scnd_c + rot_thrd_c + rot_pinkie_c)/number_detected;

	//cout<< "Average scale and rotation "<<avg_scale<<"\t"<<avg_rot<<endl;

	//apply scaling and rotation to provide estimated position of lost point
	double new_dist = last_dist * avg_scale;
	double new_ang = last_ang + avg_rot;
	CvPoint orgPos = polar2cart(new_dist,new_ang);

	//find estimated point by adding to original value of base point (i.e. move point back to normal place)
	CvPoint base = argh.getbaseL();
	CvPoint newPos;
	newPos.x = base.x + orgPos.x;
	newPos.y = base.y + orgPos.y; 

	double dist = distance(base, newPos);
	if (dist>300)
		{newPos = lastKnownPos;}

	
	//find out which finger is estimated and set it in hand object
	switch(fingerNo) {
	case THUMB :
			argh.est_thumb = newPos;
			break;
	case FSTFING:
			argh.est_first = newPos;
			break;
	case SCNDFING: 	
			argh.est_second = newPos;
			break;
	case THRDFING:
			argh.est_third = newPos;
			break;
	case PINKIE:	
			argh.est_pinkie = newPos;
			break;
	case THUMB_B:
			argh.est_thumb_b = newPos;
			break;
	case FSTFING_B:
			argh.est_first_b = newPos;
			break;
	case SCNDFING_B:
			argh.est_second_b = newPos;
			break;
	case THRDFING_B:
			argh.est_third_b = newPos;
			break;
	case FSTFING_C:
			argh.est_first_c = newPos;
			break;
	case SCNDFING_C:
			argh.est_second_c = newPos;
			break;
	case THRDFING_C:
			argh.est_third_c = newPos;
			break;
	case PINKIE_C:
			argh.est_pinkie_c = newPos;
			break;
	default:
		;
	}
		
	return (argh);

}

double Transform::distance(CvPoint one, CvPoint two)
{
		int xd, yd =0;
		double distance = 0.0;
		xd = abs(two.x-one.x);
		yd = abs(two.y-one.y);
		distance = sqrt(xd*xd + yd*yd);	
		return(distance);

}

double Transform::length_cart2polar(CvPoint cart)
{
	CvPoint org = cvPoint(0,0);
	double polar_len = Transform::distance(org,cart);
	return (polar_len);

}

double Transform::angle_cart2polar(CvPoint cart, double length)
{
	double polar_angle = acos((cart.x/length)) * (180/M_PI);
	return (polar_angle);

}

CvPoint Transform::polar2cart(double pol_length,double angle )
{
	int x, y = 0;
	double pol_angle =angle;

	x = round( pol_length * (cos(pol_angle*(M_PI/180))));
	y = round(pol_length * (sin(pol_angle*(M_PI/180))));
	
	return (cvPoint(x,y));
}

int Transform::sgn(int number)
{
	if(number<0)
		return (-1);
	if(number>0)
		return (1);
	if(number==0)
		return (0);
}

void Transform::calc_distances(CvPoint current, CvPoint next, bool occ, double &dist, bool &same, double &scale,double &rot, int &detected)
{
	if(!occ)
		{number_detected++;
		dist = distance(current, next);
		if (dist==0 && current.x==next.x && current.y==next.y)
			{same = true;
			scale = 1;
			rot = 0;}	
		}

}

void Transform::calc_rotation(CvPoint current, CvPoint next, bool occ, bool same, double dist_current, double dist_next, double &rot,double &scale)
{
	if(!same && !occ)
		{
		double current_angle = angle_cart2polar(current,dist_current);
		double next_angle = angle_cart2polar(next,dist_next);
		rot = next_angle - current_angle;
		
		if(dist_current==dist_next)
			{scale=1;}			
		else
			{scale = dist_next/dist_current; }
		}



}

Transform::~Transform()
{

}


