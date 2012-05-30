#include "fullhand_dist.h"
// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>
#include <iostream>

using namespace std;

typedef std::vector <CvPoint> CoorVec;
typedef std::vector <CvPoint>::iterator CoorVecIt;
typedef std::vector <double> DistVec;

Fullhand_dist::Fullhand_dist(bool whichhand, double * handframe){
	r_or_l = whichhand;
	lost_baseL=false; lost_baseR=false;
	occ_thumb=false; hid_pinkie=false; lost_fst=false; lost_scnd=false; lost_thrd=false;
	occ_thumb_b=false; hid_pinkie_b=false; lost_fst_b=false; lost_scnd_b=false; lost_thrd_b=false;
	hid_pinkie_c=false; lost_fst_c=false; lost_scnd_c=false; lost_thrd_c=false;
	CvPoint start = cvPoint(0,0);
	baseptL = start; baseptR=start;
	thumb = start; thumb_b=start; fstfinger=start; fstfinger_b=start; fstfinger_c=start; scndfinger=start; scndfinger_b=start; scndfinger_c=start; thrdfinger=start; thrdfinger_b=start; thrdfinger_c=start; pinkie=start; pinkie_b=start; pinkie_c = start;
	est_thumb = start; est_first = start; est_second = start; est_third = start; est_pinkie = start;
	est_thumb_b = start; est_first_b = start; est_second_b = start; est_third_b = start; est_pinkie_b = start;
	est_thumb_c = start; est_first_c = start; est_second_c = start; est_third_c = start; est_pinkie_c = start;


	//if r_or_l is false, take first part of the buffer handframe and assign it to hand makers, if true, take second part of buffer
	if(!r_or_l){
		baseptL = cvPoint(handframe[0],handframe[1]);
		baseptR = cvPoint(handframe[2],handframe[3]);
		thumb = cvPoint(handframe[4],handframe[5]);
		fstfinger = cvPoint(handframe[6],handframe[7]) ;
		scndfinger = cvPoint(handframe[8],handframe[9]);
		thrdfinger = cvPoint(handframe[10],handframe[11]);
		pinkie = cvPoint(handframe[12],handframe[13]);
		thumb_b = cvPoint(handframe[14],handframe[15]);
		fstfinger_b = cvPoint(handframe[16],handframe[17]);
		scndfinger_b = cvPoint(handframe[18],handframe[19]);
 		thrdfinger_b = cvPoint(handframe[20],handframe[21]);
		pinkie_b = cvPoint(handframe[22],handframe[23]);
		fstfinger_c = cvPoint(handframe[24],handframe[25]);
		scndfinger_c = cvPoint(handframe[26],handframe[27]);
		thrdfinger_c = cvPoint(handframe[28],handframe[29]);
		pinkie_c = cvPoint(handframe[30],handframe[31]);

	}
	else{
		baseptL = cvPoint(handframe[32],handframe[33]);
		baseptR = cvPoint(handframe[34],handframe[35]);
		thumb = cvPoint(handframe[36],handframe[37]) ;
		fstfinger = cvPoint(handframe[38],handframe[39]);
		scndfinger = cvPoint(handframe[40],handframe[41]);
		thrdfinger = cvPoint(handframe[42],handframe[43]);
		pinkie = cvPoint(handframe[44],handframe[45]);
		thumb_b = cvPoint(handframe[46],handframe[47]);
		fstfinger_b = cvPoint(handframe[48],handframe[49]);
		scndfinger_b = cvPoint(handframe[50],handframe[51]);
 		thrdfinger_b = cvPoint(handframe[52],handframe[53]);
		pinkie_b = cvPoint(handframe[54],handframe[55]);
		fstfinger_c = cvPoint(handframe[56],handframe[57]);
		scndfinger_c = cvPoint(handframe[58],handframe[59]);
		thrdfinger_c = cvPoint(handframe[60],handframe[61]);
		pinkie_c = cvPoint(handframe[62],handframe[63]);

	}
	LR_zest_wrista = 0.0;
	LR_zest_wristb = 0.0;
	LR_zest_meta_thumb = 0.0;
	LR_zest_meta_1 = 0.0;
	LR_zest_meta_2 = 0.0;
	LR_zest_meta_3 = 0.0;
	LR_zest_meta_4 = 0.0;
	LR_zest_prox_thumb = 0.0;
	LR_zest_prox_1 = 0.0;
	LR_zest_prox_2 = 0.0;
	LR_zest_prox_3 = 0.0;
	LR_zest_prox_4 = 0.0;
	LR_zest_dist_1 = 0.0;
	LR_zest_dist_2 = 0.0;
	LR_zest_dist_3 = 0.0;
	LR_zest_dist_4 = 0.0;


}

void Fullhand_dist::calc_distances(){

	a_dist = distance (baseptL, fstfinger);
	b_dist = distance (baseptR, thrdfinger);
	c_dist = distance(baseptL, baseptR);
	d_dist = distance (fstfinger,thrdfinger);
	f_thumb = distance(thumb, thumb_b);
	f_1st = distance(fstfinger, fstfinger_b);
	f_2nd = distance(scndfinger, scndfinger_b);
	f_3rd = distance(thrdfinger,thrdfinger_b);
	f_pinkie = distance(pinkie,pinkie_b);
	g_1st = distance(fstfinger_b,fstfinger_c);
	g_2nd = distance(scndfinger_b,scndfinger_c);
	g_3rd = distance(thrdfinger_b,thrdfinger_c);
	g_pinkie = distance(pinkie_b,pinkie_c);

	h_dist = distance (baseptL, thumb);

}

void Fullhand_dist::print_distances(){

	cout<< a_dist <<"\t"<<b_dist<< "\t"<<c_dist <<"\t"<<d_dist<< "\t"<<f_1st<<"\t"<<f_2nd <<"\t"<<f_3rd<<endl;


}



