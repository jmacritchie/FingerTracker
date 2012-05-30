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
#include "organise.h"
#include "correlation.h"
#include <iostream>

using namespace std;

typedef std::vector <CvPoint> CoorVec;
typedef std::vector <CvPoint>::iterator CoorVecIt;
typedef std::vector <double> DistVec;

#define draw_cross( centre, color, d )                                 \
                cvLine( fgr_grp_img, cvPoint( centre.x - d, centre.y - d ),                \
                             cvPoint( centre.x + d, centre.y + d ), color, 1, 0 ); \
                cvLine( fgr_grp_img, cvPoint( centre.x + d, centre.y - d ),                \
                             cvPoint( centre.x - d, centre.y + d ), color, 1, 0 );

#define draw_circle( image,point,color,radius)				\
		cvCircle (image, cvPoint(point.x,point.y), radius, color, 1);

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

Fullhand::Fullhand(){
}

Fullhand::Fullhand(bool whichhand){
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
}

/* This function is fed in a vector of detected blobs (stored as coordinates), the height of the image and both images so that correlation can be performed
*/ //TODO: 
void Fullhand::setallbases(CoorVec &rightgroup, int height, IplImage* prev_frame, IplImage* now_frame, int r_or_l){

	//for first frame, take lowest y coordinate point in pointset to be the base, for all consecutive frames - calculate the correlation between the previous base point and the three lowest y coordinate points.
	CoorVec ordered;
	CoorVec baseordered;
	baseordered = Organise::order_y(rightgroup, height);
	int rsize = baseordered.size();
	CoorVecIt bases_it = baseordered.begin();

	if(baseptL.x==0 && baseptL.y==0 && baseptR.x==0 && baseptR.y==0){ 	//if the base point hasnt been set since its initialisation in constructor
		if(rsize>1&&r_or_l){			//search through all the points and find the point with highest y coordinate (remember the top left corner of the image is coordinate 0,0) 
			if(baseordered[0].x>baseordered[1].x)
				{baseptL = baseordered[1];
				baseptR = baseordered[0];
				}
			else
				{baseptL = baseordered[0];
				baseptR = baseordered[1];}
			baseordered.erase(bases_it);
			baseordered.erase(bases_it++);
		}	
		if(rsize>1 && !r_or_l){			//search through all the points and find the point with highest y coordinate (remember the top left corner of the image is coordinate 0,0) 
			if(baseordered[0].x>baseordered[1].x)
				{baseptR = baseordered[1];
				baseptL = baseordered[0];
				}
			else
				{baseptR = baseordered[0];
				baseptL = baseordered[1];}
			baseordered.erase(bases_it);
			baseordered.erase(bases_it++);
		}	
	}
	else{	//if base point has previously been set i.e. if this is not the first frame!
		//initialise images for use in correlation algorithms
		IplImage* corr_block = cvCreateImage( cvSize(9,9),DEPTH_8_BIT,CHANNELS_3);
		IplImage* new_mask = cvCreateImage( cvSize(now_frame->width, now_frame->height), DEPTH_8_BIT, CHANNELS_1 );
		IplImage* prev_gray = cvCreateImage( cvSize(now_frame->width, now_frame->height), DEPTH_8_BIT, CHANNELS_1 );
		IplImage* now_gray = cvCreateImage( cvSize(now_frame->width, now_frame->height), DEPTH_8_BIT, CHANNELS_1 );
		//convert picture frames to grey colour
		cvCvtColor(prev_frame,prev_gray,CV_RGB2GRAY);
		cvCvtColor(now_frame,now_gray,CV_RGB2GRAY);

		//calculate correlation of last base point and compare to the first three elements of ordered
		Correlation::search_and_template(baseptL,prev_gray, corr_block, new_mask);
		CvPoint poss_base_L = Correlation::max_correlation(baseptL, corr_block,new_mask, now_gray);

		Correlation::search_and_template(baseptR,prev_gray, corr_block, new_mask);
		CvPoint poss_base_R = Correlation::max_correlation(baseptR, corr_block,new_mask, now_gray);

		//compare poss_base to first three elements of vector 'ordered' NB: MAKE SURE VECTOR 'ORDERED' ACTUALLY HAS 3 ELEMENTS!!
		CoorVecIt iter = baseordered.begin();
		CoorVecIt itbase = baseordered.begin();
		double max_distance = 100.0;
		CvPoint l_base = cvPoint(0,0);
		for(int b = 0; b<3; b++){ //only checking the three lowest y-value coordinates
			if(baseordered.size()>b){
				if(distance(poss_base_L,baseordered[b])<max_distance){
					max_distance = distance(poss_base_L,baseordered[b]);
					l_base = baseordered[b];
					itbase = iter;
					}
				else{
					l_base = poss_base_L;
				}
			iter++;
			}
			else{
				l_base = poss_base_L;
			}
		}
		baseptL = l_base;
		if(baseordered.size()>0)baseordered.erase(itbase);
		//delete this element from rightgroup

		//same again for right basepoint
		CoorVecIt iterR = baseordered.begin();
		CoorVecIt itbaseR = baseordered.begin();
		double max_distanceR = 100.0;
		CvPoint r_base = cvPoint(0,0);
		for(int c = 0; c<3; c++){ //only checking the three lowest y-value coordinates
			if(baseordered.size()>c){
				if(distance(poss_base_R,baseordered[c])<max_distanceR){
					max_distanceR = distance(poss_base_R,baseordered[c]);
					r_base = baseordered[c];
					itbaseR = iterR;
					}
				else{
					r_base = poss_base_R;
				}
			iterR++;
			}
			else{
				r_base = poss_base_R;
			}
		}
		baseptR = r_base;
		if(baseordered.size()>0)baseordered.erase(itbaseR);
		//delete this element from rightgroup

	}
	
	//put the remaining points back into the vector rightgroup
	rightgroup.clear();
	rightgroup.assign(baseordered.begin(),baseordered.end());
	baseordered.clear();
}

/*setfingers function takes the detected blobs and first orders them in terms of distance to be separated into three groups: metacarpal joints, proximal joints and distal joints. These separate groups are ordered in terms of x coordinate. Angles between corresponding joints are then calculated: i.e. first element of metacarpal, proximal and distal vectors are taken to determine if all these points are on the same finger.*/
void Fullhand::setfingers(CoorVec hand, int r_or_l, int width, int height){

	//integer r_or_l will be 1 for right hand and 0 for left hand
	if (r_or_l){cout<< "setting fingers for right hand"<<endl;}
	else{cout<< "setting fingers for left hand"<<endl;}
	//the left hand - furthest coordinate in x axis will be pinkie
	//the right hand - furthest coordinate in x axis will be thumb if it is indeed there!!

	int min_dist = 30;
	int m_joint = 70;
	int p_joint = 120;
	int d_joint = 200; 
	CoorVec meta;
	CoorVec prox;	
	CoorVec distal;

	int shrinksize = hand.size();

 	for (int element=0;element<shrinksize;element++){
		//for the amount of elements existing, go through and determine smallest distance from wrist marker.
		CoorVecIt start = hand.begin();
		CoorVecIt position = hand.begin();
		CvPoint thispoint = cvPoint(0,0);
		double distance = 200;   //only want blobs within 500 pixel distance of the base point to be considered
		//determine size of vector of elements to be re-assigned
		int newsize = hand.size();
			for(int existing=0;existing<newsize;existing++){
				int xd, yd =0;
 				xd = abs(hand[existing].x-(baseptL.x + ((baseptR.x - baseptL.x)/2))); // distance to the middle x point between the two basepts
 				yd = abs(hand[existing].y-baseptL.y);
				double newdistance = sqrt(xd*xd + yd*yd);
				if(newdistance<distance)
					{distance = newdistance;
					thispoint = hand[existing];
					start = position;
					}	
				position++;
			}
	//find in which range of distance i.e which joint this point belongs to and push back the appropriate vector
		if(distance>min_dist){ //make sure point isnt right next to base point e.g. when the base point blob has eroded into two parts
			if(distance<m_joint)
				{meta.push_back(thispoint);
				hand.erase(start);}
			else if(distance<p_joint)
				{prox.push_back(thispoint);
				hand.erase(start);}
			else {distal.push_back(thispoint);
				hand.erase(start);}
		}
		else{hand.erase(start);}
	}
	//order each vector in terms of x depending on right or left hand
	/*CoorVec sort_meta = Organise::order_x(meta,width,r_or_l);
	CoorVec sort_prox = Organise::order_x(prox,width,r_or_l);
	CoorVec sort_dist = Organise::order_x(distal,width,r_or_l);*/
	//check that blobs have not been split in two...
	CoorVec sort_meta = Organise::checkblobs(meta);
	CoorVec sort_prox = Organise::checkblobs(prox);
	CoorVec sort_dist = Organise::checkblobs(distal);
	//for interphalangeal joints i.e. proxal and distal vectors, check points arent in wrong vector!
	//test order of points to make sure that first three are in acsending y order and the angles of lines between each set of two points are similar.	
	checkgroups(sort_prox,sort_dist,width);
	//cout<<"meta size is "<<sort_meta.size()<<", prox is "<<sort_prox.size()<<", distal is "<<sort_dist.size()<<endl; cout.flush();
	//make just one big set function so that corresponding points of other vectors can be used to check angles and determine exactly which point is missing and not just well if theres only four points it must be the thumb thats gone.
	metaset(sort_meta,r_or_l);
	proxset(sort_prox,r_or_l);
	distset(sort_dist,r_or_l);

 	
	//make sure proxal and distal vectors have the right elements in them. done by checking angles from basept 
	//ordered.clear();
	meta.clear();
	//meta_x.clear();
	sort_meta.clear();
	prox.clear();
	//prox_x.clear();
	sort_prox.clear();
	distal.clear();
	//distal_x.clear();
	sort_dist.clear();
}
/* getfingers function draws the points and their joint connections onto the screen */
IplImage* Fullhand::getfingers(IplImage *fgr_grp_img)
{
	draw_cross(baseptL, CV_RGB(255,255,255),4);
	if(!lost_fst){
		draw_cross(fstfinger, CV_RGB(255,0,0),4);
		if(!lost_scnd){ 
			draw_cross(scndfinger, CV_RGB(0,255,0),4);
			cvLine( fgr_grp_img, fstfinger,scndfinger,CV_RGB(255,255,60), 1, 0 );
		}
		if (!occ_thumb)
			{
			draw_cross(thumb,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb,CV_RGB(255,255,60), 1, 0 );
			cvLine( fgr_grp_img, thumb,fstfinger,CV_RGB(255,255,60), 1, 0 );}
		else
			{cvLine( fgr_grp_img, baseptL,fstfinger,CV_RGB(255,255,60), 1, 0 );}
		if (!hid_pinkie)
			{
			draw_cross(pinkie, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie,baseptL,CV_RGB(255,255,60), 1, 0 );
			if(!lost_thrd)
				{
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger,pinkie,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );}
			else
				{cvLine( fgr_grp_img, scndfinger,pinkie,CV_RGB(255,255,60), 1, 0 );}
			}
		else
			{if(!lost_thrd){
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger,baseptL,CV_RGB(255,255,60), 1, 0 );
				}
			}
	}
	else
	{
		if(!lost_scnd){ 
			draw_cross(scndfinger, CV_RGB(0,255,0),4);
		}
		if (!occ_thumb)
			{
			draw_cross(thumb,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb,CV_RGB(255,255,60), 1, 0 );
			cvLine( fgr_grp_img, thumb,scndfinger,CV_RGB(255,255,60), 1, 0 );}
		else
			{cvLine( fgr_grp_img, baseptL,scndfinger,CV_RGB(255,255,60), 1, 0 );}
		if (!hid_pinkie)
			{
			draw_cross(pinkie, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie,baseptL,CV_RGB(255,255,60), 1, 0 );
			if(!lost_thrd)
				{
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger,pinkie,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );}
			else
				{cvLine( fgr_grp_img, scndfinger,pinkie,CV_RGB(255,255,60), 1, 0 );}
			}
		else
			{if(!lost_thrd){
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger,baseptL,CV_RGB(255,255,60), 1, 0 );
				}
			}
	}	

//for second set of knuckles
	if(!lost_fst_b){ 
		draw_cross(fstfinger_b, CV_RGB(255,0,0),4);
		if(!lost_scnd_b){ 
			draw_cross(scndfinger_b, CV_RGB(0,255,0),4);
			cvLine( fgr_grp_img, fstfinger_b,scndfinger_b,CV_RGB(255,60,255), 1, 0 );
		}	
		if (!occ_thumb_b)
			{
			draw_cross(thumb_b,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb_b,CV_RGB(255,60,255), 1, 0 );
			cvLine( fgr_grp_img, thumb_b,fstfinger_b,CV_RGB(255,60,255), 1, 0 );}
		else
			{cvLine( fgr_grp_img, baseptL,fstfinger_b,CV_RGB(255,60,255), 1, 0 );}
		if (!hid_pinkie_b)
			{
			draw_cross(pinkie_b, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_b,baseptL,CV_RGB(255,60,255), 1, 0 );
			if(!lost_thrd_b)
				{
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );}
			else
				{cvLine( fgr_grp_img, scndfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );}
			}
		else
			{if(!lost_thrd_b){
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger_b,baseptL,CV_RGB(255,60,255), 1, 0 );
				}
			}
	}	
	else
	{
		if(!lost_scnd_b){ 
			draw_cross(scndfinger_b, CV_RGB(0,255,0),4);
		}
		if (!occ_thumb_b)
			{
			draw_cross(thumb_b,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb_b,CV_RGB(255,60,255), 1, 0 );
			cvLine( fgr_grp_img, thumb_b,scndfinger_b,CV_RGB(255,60,255), 1, 0 );}
		else
			{cvLine( fgr_grp_img, baseptL,scndfinger_b,CV_RGB(255,255,60), 1, 0 );}
		
		if (!hid_pinkie_b)
			{
			draw_cross(pinkie_b, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_b,baseptL,CV_RGB(255,60,255), 1, 0 );
			if(!lost_thrd_b)
				{
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );}
			else
				{cvLine( fgr_grp_img, scndfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );}
			}
		else
			{if(!lost_thrd_b){
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger_b,baseptL,CV_RGB(255,60,255), 1, 0 );
				}
			}
	}
	
//for third set of knuckles
	if(!lost_fst_c){ 
		draw_cross(fstfinger_c, CV_RGB(255,0,0),4);
		if(!lost_scnd_c){ 
			draw_cross(scndfinger_c, CV_RGB(0,255,0),4);
			cvLine( fgr_grp_img, fstfinger_c,scndfinger_c,CV_RGB(60,60,255), 1, 0 );
		}
		if (!lost_fst_c)
			{cvLine( fgr_grp_img, baseptL,fstfinger_c,CV_RGB(60,60,255), 1, 0 );}
		if (!hid_pinkie_c)
			{
			draw_cross(pinkie_c, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_c,baseptL,CV_RGB(60,60,255), 1, 0 );
			if(!lost_thrd_c)
				{
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );}
			else
				{cvLine( fgr_grp_img, scndfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );}
			}
		else
			{if(!lost_thrd_c){
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger_c,baseptL,CV_RGB(60,60,255), 1, 0 );
				}
			}
	}
	else
	{
		if(!lost_scnd_c){
			draw_cross(scndfinger_c, CV_RGB(0,255,0),4);
			cvLine( fgr_grp_img, baseptL,scndfinger_c,CV_RGB(60,60,255), 1, 0 );
		}
		if (!hid_pinkie_c)
			{
			draw_cross(pinkie_c, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_c,baseptL,CV_RGB(60,60,255), 1, 0 );
			if(!lost_thrd_c)
				{
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );}
			else
				{cvLine( fgr_grp_img, scndfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );}
			}
		else
			{if(!lost_thrd_c){
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger_c,baseptL,CV_RGB(60,60,255), 1, 0 );
				}
			}
	}	
	return(fgr_grp_img);
}

IplImage* Fullhand::getfingerlines(IplImage *fgr_grp_img)
{
	draw_cross(baseptL, CV_RGB(255,255,255),4);
	draw_cross(baseptR, CV_RGB(255,255,255),4);
	cvLine( fgr_grp_img, baseptL,baseptR,CV_RGB(255,255,255), 1, 0 );
	draw_cross(fstfinger, CV_RGB(255,0,0),4);
	cvLine( fgr_grp_img, baseptL,fstfinger,CV_RGB(255,255,60), 1, 0 );
	draw_cross(fstfinger_b, CV_RGB(255,0,0),4);
	cvLine( fgr_grp_img, fstfinger,fstfinger_b,CV_RGB(255,255,60), 1, 0 );
	draw_cross(fstfinger_c, CV_RGB(255,0,0),4);
	cvLine( fgr_grp_img, fstfinger_b,fstfinger_c,CV_RGB(255,255,60), 1, 0 );
			
	draw_cross(scndfinger, CV_RGB(0,255,0),4);
	cvLine( fgr_grp_img, baseptL,scndfinger,CV_RGB(60,60,255), 1, 0 );
	draw_cross(scndfinger_b, CV_RGB(0,255,0),4);
	cvLine( fgr_grp_img, scndfinger,scndfinger_b,CV_RGB(60,60,255), 1, 0 );
	draw_cross(scndfinger_c, CV_RGB(0,255,0),4);
	cvLine( fgr_grp_img, scndfinger_b,scndfinger_c,CV_RGB(60,60,255), 1, 0 );

	draw_cross(thrdfinger, CV_RGB(0,0,255),4);
	cvLine( fgr_grp_img, baseptR,thrdfinger,CV_RGB(60,255,60), 1, 0 );
	draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
	cvLine( fgr_grp_img, thrdfinger,thrdfinger_b,CV_RGB(60,255,60), 1, 0 );
	draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
	cvLine( fgr_grp_img, thrdfinger_b,thrdfinger_c,CV_RGB(60,255,60), 1, 0 );

	draw_cross(pinkie, CV_RGB(255,160,0),4);
	cvLine( fgr_grp_img, baseptR,pinkie,CV_RGB(255,255,255), 1, 0 );
	draw_cross(pinkie_b, CV_RGB(255,160,0),4);
	cvLine( fgr_grp_img, pinkie,pinkie_b,CV_RGB(255,255,255), 1, 0 );
	draw_cross(pinkie_c, CV_RGB(255,160,0),4);
	cvLine( fgr_grp_img, pinkie_b,pinkie_c,CV_RGB(255,255,255), 1, 0 );

	draw_cross(thumb, CV_RGB(0,255,255),4);
	cvLine( fgr_grp_img, baseptL,thumb,CV_RGB(0,255,255), 1, 0 );
	draw_cross(thumb_b, CV_RGB(0,255,255),4);
	cvLine( fgr_grp_img, thumb,thumb_b,CV_RGB(0,255,255), 1, 0 );		

	
	return(fgr_grp_img);
}


//TODO: draw crosses for other parts of hand
IplImage* Fullhand::drawcrosses(IplImage *fgr_grp_img)
{
	draw_cross(baseptL, CV_RGB(255,255,255),4);
	if(!lost_fst){
		draw_cross(fstfinger, CV_RGB(255,0,0),4);
		}
	if(!lost_scnd){
		draw_cross(scndfinger, CV_RGB(0,255,0),4);
		}
	if (!occ_thumb)
		{draw_cross(thumb,CV_RGB(0,255,255),4);}
	if (!hid_pinkie)
		{draw_cross(pinkie, CV_RGB(255,255,0),4);}
	if(!lost_thrd)
		{draw_cross(thrdfinger, CV_RGB(0,0,255),4);}	
	
	return(fgr_grp_img);
}

IplImage* Fullhand::getestimations(IplImage *imahge)
{
	if(est_thumb.x || est_thumb.y)
		{draw_circle(imahge, est_thumb, CV_RGB(255,0,0),3);
		 //set estimation as thumb point
		 setthumb(est_thumb);
		 occ_thumb = false;
		 est_thumb = cvPoint(0,0);}
	if(est_first.x || est_first.y)
		{draw_circle(imahge, est_first, CV_RGB(255,0,0),3);
		 set1stfing(est_first);
		 lost_fst = false;
		 est_first = cvPoint(0,0);}
	if(est_second.x || est_second.y)
		{draw_circle(imahge, est_second, CV_RGB(255,0,0),3);
		 set2ndfing(est_second);
		 lost_scnd = false;
		 est_second = cvPoint(0,0);}
	if(est_third.x || est_third.y)
		{draw_circle(imahge, est_third, CV_RGB(255,0,0),3);
		 set3rdfing(est_third);
		 lost_thrd = false;
		 est_third = cvPoint(0,0);}
	if(est_pinkie.x || est_pinkie.y)
		{draw_circle(imahge, est_pinkie, CV_RGB(255,0,0),3);
		 setpinkie(est_pinkie);
		 hid_pinkie = false;
		 est_pinkie = cvPoint(0,0);}
	if(est_thumb_b.x || est_thumb_b.y)
		{draw_circle(imahge, est_thumb_b, CV_RGB(255,0,0),3);
		 setthumb_b(est_thumb_b);
		 occ_thumb_b = false;
		 est_thumb_b = cvPoint(0,0);}
	if(est_first_b.x || est_first_b.y)
		{draw_circle(imahge, est_first_b, CV_RGB(255,0,0),3);
		 set1stfing_b(est_first_b);
		 lost_fst_b = false;
		 est_first_b = cvPoint(0,0);}
	if(est_second_b.x || est_second_b.y)
		{draw_circle(imahge, est_second_b, CV_RGB(255,0,0),3);
		 set2ndfing_b(est_second_b);
		 lost_scnd_b = false;
		 est_second_b = cvPoint(0,0);}
	if(est_third_b.x || est_third_b.y)
		{draw_circle(imahge, est_third_b, CV_RGB(255,0,0),3);
		 set3rdfing_b(est_third_b);
		 lost_thrd_b = false;
		 est_third_b = cvPoint(0,0);}
	if(est_pinkie_b.x || est_pinkie_b.y)
		{draw_circle(imahge, est_pinkie_b, CV_RGB(255,0,0),3);
		 setpinkie_b(est_pinkie_b);
		 hid_pinkie_b = false;
		 est_pinkie_b = cvPoint(0,0);}
	if(est_first_c.x || est_first_c.y)
		{draw_circle(imahge, est_first_c, CV_RGB(255,0,0),3);
		 set1stfing_c(est_first_c);
		 lost_fst_c = false;
		 est_first_c = cvPoint(0,0);}
	if(est_second_c.x || est_second_c.y)
		{draw_circle(imahge, est_second_c, CV_RGB(255,0,0),3);
		 set2ndfing_c(est_second_c);
		 lost_scnd_c = false;
		 est_second_c = cvPoint(0,0);}
	if(est_third_c.x || est_third_c.y)
		{draw_circle(imahge, est_third_c, CV_RGB(255,0,0),3);
		 set3rdfing_c(est_third_c);
		 lost_thrd_c = false;
		 est_third_c = cvPoint(0,0);}
	if(est_pinkie_c.x || est_pinkie_c.y)
		{draw_circle(imahge, est_pinkie_c, CV_RGB(255,0,0),3);
		 setpinkie_c(est_pinkie_c);
		 hid_pinkie_c = false;
		 est_pinkie_c = cvPoint(0,0);}

	return (imahge);
}

CoorVec Fullhand::setpoints_wrt_org(){
	
	CoorVec fingers;
	CvPoint ref = cvPoint((baseptL.x + ((baseptR.x-baseptL.x)/2)), (min(baseptL.y, baseptR.y)));
	org_basept = cvPoint(0,0);

	//initialise all points to 0,0
	org_baseptL, org_baseptR, org_thumb,org_fstfinger, org_scndfinger, org_thrdfinger, org_pinkie, org_thumb_b, org_fst_b, org_scnd_b, org_thrd_b, org_pinkie_b, org_fst_c, org_scnd_c, org_thrd_c, org_pinkie_c =  cvPoint(0,0); 

	if(!lost_baseL)
		org_baseptL = cvPoint (baseptL.x-ref.x,baseptL.y-ref.y);
	if(!lost_baseR)
		org_baseptR = cvPoint (baseptR.x-ref.x,baseptR.y-ref.y);
	if(!occ_thumb)
		org_thumb = cvPoint (thumb.x-ref.x,thumb.y-ref.y);
	if(!lost_fst)
		org_fstfinger = cvPoint (fstfinger.x-ref.x,fstfinger.y-ref.y);
	if(!lost_scnd)
		org_scndfinger = cvPoint (scndfinger.x-ref.x,scndfinger.y-ref.y);
	if(!lost_thrd)	
		org_thrdfinger = cvPoint (thrdfinger.x-ref.x,thrdfinger.y-ref.y);
	if(!hid_pinkie)
		org_pinkie = cvPoint (pinkie.x-ref.x,pinkie.y-ref.y);
	if(!occ_thumb_b)
		org_thumb_b = cvPoint (thumb_b.x-ref.x,thumb_b.y-ref.y);
	if(!lost_fst_b)
		org_fst_b = cvPoint (fstfinger_b.x-ref.x,fstfinger_b.y-ref.y);
	if(!lost_scnd_b)
		org_scnd_b = cvPoint (scndfinger_b.x-ref.x,scndfinger_b.y-ref.y);
	if(!lost_thrd_b)	
		org_thrd_b = cvPoint (thrdfinger_b.x-ref.x,thrdfinger_b.y-ref.y);
	if(!hid_pinkie_b)
		org_pinkie_b = cvPoint (pinkie_b.x-ref.x,pinkie_b.y-ref.y);
	if(!lost_fst_c)
		org_fst_c = cvPoint (fstfinger_c.x-ref.x,fstfinger_c.y-ref.y);
	if(!lost_scnd_c)
		org_scnd_c = cvPoint (scndfinger_c.x-ref.x,scndfinger_c.y-ref.y);
	if(!lost_thrd_c)	
		org_thrd_c = cvPoint (thrdfinger_c.x-ref.x,thrdfinger_c.y-ref.y);
	if(!hid_pinkie_c)
		org_pinkie_c = cvPoint (pinkie_c.x-ref.x,pinkie_c.y-ref.y);
	
	fingers.push_back(org_baseptL);
	fingers.push_back(org_baseptR);
	fingers.push_back(org_thumb);
	fingers.push_back(org_fstfinger);
	fingers.push_back(org_scndfinger);
	fingers.push_back(org_thrdfinger);
	fingers.push_back(org_pinkie);
	fingers.push_back(org_thumb_b);
	fingers.push_back(org_fst_b);
	fingers.push_back(org_scnd_b);
	fingers.push_back(org_thrd_b);
	fingers.push_back(org_pinkie_b);
	fingers.push_back(org_fst_c);
	fingers.push_back(org_scnd_c);
	fingers.push_back(org_thrd_c);
	fingers.push_back(org_pinkie_c);

	return(fingers);
	fingers.clear();	
}


DistVec Fullhand::distances_to_org(CoorVec fingers){

	org_baseptL = fingers[0];
	org_baseptR = fingers[1];
	org_thumb = fingers[2];
	org_fstfinger = fingers[3];
	org_scndfinger = fingers[4];
	org_thrdfinger = fingers[5];
	org_pinkie = fingers[6];

	//calculates distances from each point to the origin (i.e. 0,0)
	double Dist_a, Dist_b, Dist_c, Dist_d, Dist_e, Dist_f, Dist_g, Dist_h, Dist_i, Dist_j, Dist_k, Dist_l, Dist_m, Dist_n = 0.0;
	DistVec handdist;
	//since the distance is to point 0,0 - dont need to calculate x and y values
	if(!occ_thumb)
		{Dist_a = sqrt(org_thumb.x*org_thumb.x + org_thumb.y*org_thumb.y);}
	if(!lost_fst)
		{Dist_b = sqrt(org_fstfinger.x*org_fstfinger.x + org_fstfinger.y*org_fstfinger.y);}
	if(!lost_scnd)
		{Dist_c = sqrt(org_scndfinger.x*org_scndfinger.x + org_scndfinger.y*org_scndfinger.y);}
	if(!lost_thrd)
		{Dist_d = sqrt(org_thrdfinger.x*org_thrdfinger.x + org_thrdfinger.y*org_thrdfinger.y);}
	if(!hid_pinkie)
		{Dist_e = sqrt(org_pinkie.x*org_pinkie.x + org_pinkie.y*org_pinkie.y);}
	if(!occ_thumb_b)
		{Dist_f = sqrt(org_thumb_b.x*org_thumb_b.x + org_thumb_b.y*org_thumb_b.y);}
	if(!lost_fst_b)
		{Dist_g = sqrt(org_fst_b.x*org_fst_b.x + org_fst_b.y*org_fst_b.y);}
	if(!lost_scnd_b)
		{Dist_h = sqrt(org_scnd_b.x*org_scnd_b.x + org_scnd_b.y*org_scnd_b.y);}
	if(!lost_thrd_b)
		{Dist_i = sqrt(org_thrd_b.x*org_thrd_b.x + org_thrd_b.y*org_thrd_b.y);}
	if(!hid_pinkie_b)
		{Dist_j = sqrt(org_pinkie_b.x*org_pinkie_b.x + org_pinkie_b.y*org_pinkie_b.y);}
	if(!lost_fst_c)
		{Dist_k = sqrt(org_fst_c.x*org_fst_c.x + org_fst_c.y*org_fst_c.y);}
	if(!lost_scnd_c)
		{Dist_l = sqrt(org_scnd_c.x*org_scnd_c.x + org_scnd_c.y*org_scnd_c.y);}
	if(!lost_thrd_c)
		{Dist_m = sqrt(org_thrd_c.x*org_thrd_c.x + org_thrd_c.y*org_thrd_c.y);}
	if(!hid_pinkie_c)
		{Dist_n = sqrt(org_pinkie_c.x*org_pinkie_c.x + org_pinkie_c.y*org_pinkie_c.y);}

	//put distances into a vector of distances for each hand stored in order of thumb to pinkie
	//distances are zero for occluded fingers
	handdist.push_back(Dist_a);
	handdist.push_back(Dist_b);
	handdist.push_back(Dist_c);
	handdist.push_back(Dist_d);
	handdist.push_back(Dist_e);
	handdist.push_back(Dist_f);
	handdist.push_back(Dist_g);
	handdist.push_back(Dist_h);
	handdist.push_back(Dist_i);
	handdist.push_back(Dist_j);
	handdist.push_back(Dist_k);
	handdist.push_back(Dist_l);
	handdist.push_back(Dist_m);
	handdist.push_back(Dist_n);	

	return (handdist);	

	handdist.clear();

}

//prob better to have one setting function for all vectors so then can consider which points are lost by comparison to other vectors
//compare by looking at corresponding finger joints in the other groupings.

void Fullhand::metaset(CoorVec joints, bool r_or_l){

	CoorVec ordered = joints;
	// fingers now in order f lowest x axis value to highest.

	//initialise boolean arguments for missing fingers
	occ_thumb = false;
	hid_pinkie = false;
	lost_thrd = false;
	lost_scnd = false;
	lost_fst = false;

	//calculate which fingers are missing
	int newsize = ordered.size();
	cout<< "meta size is "<<newsize<<endl;

	for (int count=0; count<newsize; count++){
		cout<< ordered[count].x <<","<<ordered[count].y<<"\t" ; 
	}
	cout<<endl;

	if(newsize==5 && !r_or_l)
		{
		if((!pinkie.x && !pinkie.y)){pinkie = ordered[0];}
		if((!thrdfinger.x && !thrdfinger.y)){thrdfinger = ordered[1]; }
		if((!scndfinger.x && !scndfinger.y)){scndfinger = ordered[2];}
		if((!fstfinger.x && !fstfinger.y)){fstfinger = ordered[3];}
		if((!thumb.x && !thumb.y)){thumb = ordered[4];}
		if(!sort(ordered,pinkie))
			{hid_pinkie=true;}
		if(!sort(ordered,thrdfinger))
			{lost_thrd=true;}
		if(!sort(ordered,scndfinger))
			{lost_scnd=true;}
		if(!sort(ordered,fstfinger))
			{lost_fst=true;}
		if(!sort(ordered,thumb))
			{occ_thumb=true;}
		
		}
	else if (newsize==5 && !r_or_l){
		if((!pinkie.x && !pinkie.y)){pinkie = ordered[4];}
		if((!thrdfinger.x && !thrdfinger.y)){thrdfinger = ordered[3];}
		if((!scndfinger.x && !scndfinger.y)){scndfinger = ordered[2];}
		if((!fstfinger.x && !fstfinger.y)){fstfinger = ordered[1];}
		if((!thumb.x && !thumb.y)){thumb = ordered[0];}
		if(!sort(ordered,pinkie))
			{hid_pinkie=true;}
		if(!sort(ordered,thrdfinger))
			{lost_thrd=true;}
		if(!sort(ordered,scndfinger))
			{lost_scnd=true;}
		if(!sort(ordered,fstfinger))
			{lost_fst=true;}
		if(!sort(ordered,thumb))
			{occ_thumb=true;}
		}
	else if(newsize>5) //if more than 5 points, go through them all and see which is closest to previous points
		{
		if(!sort(ordered,pinkie))
			{hid_pinkie=true;}
		if(!sort(ordered,thrdfinger))
			{lost_thrd=true;}
		if(!sort(ordered,scndfinger))
			{lost_scnd=true;}
		if(!sort(ordered,fstfinger))
			{lost_fst=true;}
		if(!sort(ordered,thumb))
			{occ_thumb=true;}	
		}
	else{//if less than required number of points, have to go through calculating which previous point is closest to each element of ordered
		bool assigned_thu=false; bool assigned_1st=false; bool assigned_2nd =false; bool assigned_3rd = false; bool assigned_pink = false;
		int many = ordered.size();
		for(int thismany=0; thismany<many; thismany++){
			int whichfinger = 60;
			double topdist = 40.0;
			double dist_p = distance(pinkie,ordered[thismany]);
			double dist_3rd = distance(thrdfinger,ordered[thismany]);
			double dist_2nd = distance(scndfinger,ordered[thismany]);
			double dist_1st = distance(fstfinger,ordered[thismany]);
			double dist_thu = distance(thumb,ordered[thismany]);

			if(dist_p<topdist && !assigned_pink)
				{topdist=dist_p;
				whichfinger = PINKIE;}
			
			if(dist_3rd<topdist && !assigned_3rd)
				{topdist=dist_3rd;
				whichfinger = THRDFING;}
			
			if(dist_2nd<topdist && !assigned_2nd)
				{topdist=dist_2nd;
				whichfinger = SCNDFING;}
			
			if(dist_1st<topdist && !assigned_1st)
				{topdist=dist_1st;
				whichfinger = FSTFING;}
			
			if(dist_thu<topdist && !assigned_thu)
				{topdist=dist_thu;
				whichfinger = THUMB;}

			int thisfinger = whichfinger;
			switch(thisfinger){
			case THUMB :
					thumb = ordered[thismany];	
					assigned_thu = true;
					break;
			case FSTFING:
					fstfinger = ordered[thismany];
					assigned_1st = true;
					break;
			case SCNDFING:
					scndfinger = ordered[thismany];
					//cout<<"second finger assigned ";
					assigned_2nd = true;
					break;
			case THRDFING:
					thrdfinger = ordered[thismany];
					//cout<<"third finger assigned ";
					assigned_3rd = true;
					break;
			case PINKIE:
					pinkie = ordered[thismany];
					//cout<<"pinkie assigned ";
					assigned_pink = true;
					break;
			default:
					;
			}
		}
		//determine which are missing....
		if(!assigned_thu)
			{occ_thumb = true;}
		if(!assigned_1st)
			{lost_fst = true;}
		if(!assigned_2nd)
			{lost_scnd = true;}
		if(!assigned_3rd)
			{lost_thrd = true;}
			//cout<<"third finger NOW is "<<thrdfinger.x<<","<<thrdfinger.y<<"\t";}
		if(!assigned_pink)
			{//cout<<"pinkie ";cout.flush();
			hid_pinkie = true;}
		}


	//check the x coordinate order for the meta group - this should not change as it is impossible!
	if(r_or_l){ //right hand!
		if (thumb.x > fstfinger.x)
			{ CvPoint tempfing = thumb ;
			 thumb = fstfinger;
			fstfinger = tempfing; }
		if (fstfinger.x > scndfinger.x)
			{ CvPoint tempfing = fstfinger ;
			 fstfinger = scndfinger;
			scndfinger = tempfing; }
		if (scndfinger.x > thrdfinger.x)
			{ CvPoint tempfing = scndfinger ;
			 scndfinger = thrdfinger;
			thrdfinger = tempfing;   }
		if (thrdfinger.x > pinkie.x)
			{  CvPoint tempfing = thrdfinger ;
			 thrdfinger = pinkie;
			pinkie = tempfing;   }

	}
	else{  //left hand!
		if (thumb.x < fstfinger.x)
			{ CvPoint tempfing = thumb ;
			 thumb = fstfinger;
			fstfinger = tempfing; }
		if (fstfinger.x < scndfinger.x)
			{ CvPoint tempfing = fstfinger ;
			 fstfinger = scndfinger;
			scndfinger = tempfing;    }
		if (scndfinger.x < thrdfinger.x)
			{ CvPoint tempfing = scndfinger ;
			 scndfinger = thrdfinger;
			thrdfinger = tempfing;   }
		if (thrdfinger.x < pinkie.x)
			{ CvPoint tempfing = thrdfinger ;
			 thrdfinger = pinkie;
			pinkie = tempfing; }

	}
	

	cout<<"meta set thumb is "<<thumb.x<<","<<thumb.y<<"\t";
	cout<<"meta set fst is "<<fstfinger.x<<","<<fstfinger.y<<"\t";
	cout<<"meta set scnd is "<<scndfinger.x<<","<<scndfinger.y<<"\t";
	cout<<"meta set thrd is "<<thrdfinger.x<<","<<thrdfinger.y<<"\t";
	//cout<<endl;
	cout.flush();
}

void Fullhand::proxset(CoorVec joints, bool r_or_l){

	//by this point, the meta joints have been set so we can use them just as thumb, fstfinger etc. IT IS ASSUMED that these meta joints are correct. dangerous, i know...

	CoorVec ordered = joints;
	// fingers now in order of lowest x axis value to highest.

	//initialise boolean arguments for missing fingers
	occ_thumb_b = false;
	hid_pinkie_b = false;
	lost_thrd_b = false;
	lost_scnd_b = false;
	lost_fst_b = false;

	//calculate which fingers are missing
	int newsize = ordered.size();
	if(newsize==5 && !r_or_l)
		{
		if((!pinkie_b.x && !pinkie_b.y)){pinkie_b = ordered[0];}
		if(!thrdfinger_b.x && !thrdfinger_b.y){thrdfinger_b = ordered[1]; }
		if(!scndfinger_b.x && !scndfinger_b.y){scndfinger_b = ordered[2];}
		if(!fstfinger_b.x && !fstfinger_b.y){fstfinger_b = ordered[3];}
		if(!thumb_b.x && !thumb_b.y){thumb_b = ordered[4];}
		if(!sort(ordered,pinkie_b))
			{hid_pinkie_b=true;}
		if(!sort(ordered,thrdfinger_b))
			{lost_thrd_b=true;}
		if(!sort(ordered,scndfinger_b))
			{lost_scnd_b=true;}
		if(!sort(ordered,fstfinger_b))
			{lost_fst_b=true;}
		if(!sort(ordered,thumb_b))
			{occ_thumb_b=true;}
	
		}
	else if(newsize==5 && r_or_l)
		{
		if((!pinkie_b.x && !pinkie_b.y)){pinkie_b = ordered[4];}
		if(!thrdfinger_b.x && !thrdfinger_b.y){thrdfinger_b = ordered[3]; }
		if(!scndfinger_b.x && !scndfinger_b.y){scndfinger_b = ordered[2];}
		if(!fstfinger_b.x && !fstfinger_b.y){fstfinger_b = ordered[1];}
		if(!thumb_b.x && !thumb_b.y){thumb_b = ordered[0];}
		if(!sort(ordered,pinkie_b))
			{hid_pinkie_b=true;}
		if(!sort(ordered,thrdfinger_b))
			{lost_thrd_b=true;}
		if(!sort(ordered,scndfinger_b))
			{lost_scnd_b=true;}
		if(!sort(ordered,fstfinger_b))
			{lost_fst_b=true;}
		if(!sort(ordered,thumb_b))
			{occ_thumb_b=true;}
	
		}
	else if(newsize>5 && !r_or_l) //if more than 5 points, go through them all and see which is closest to previous points
			{
			//if((!pinkie_b.x && !pinkie_b.y)){pinkie_b = ordered[0];}
		//if(!thrdfinger_b.x && !thrdfinger_b.y){thrdfinger_b = ordered[1]; }
		//if(!scndfinger_b.x && !scndfinger_b.y){scndfinger_b = ordered[2];}
		//if(!fstfinger_b.x && !fstfinger_b.y){fstfinger_b = ordered[3];}
		//if(!thumb_b.x && !thumb_b.y){thumb_b = ordered[4];}
			if(!sort(ordered,pinkie_b))
				{hid_pinkie_b=true;}
			if(!sort(ordered,thrdfinger_b))
				{lost_thrd_b=true;}
			if(!sort(ordered,scndfinger_b))
				{lost_scnd_b=true;}
			if(!sort(ordered,fstfinger_b))
				{lost_fst_b=true;}
			if(!sort(ordered,thumb_b))
				{occ_thumb_b=true;}
			}
	else if(newsize>5 && r_or_l) //if more than 5 points, go through them all and see which is closest to previous points
			{
			//if((!pinkie_b.x && !pinkie_b.y)){pinkie_b = ordered[4];}
		//if(!thrdfinger_b.x && !thrdfinger_b.y){thrdfinger_b = ordered[3]; }
		//if(!scndfinger_b.x && !scndfinger_b.y){scndfinger_b = ordered[2];}
		//if(!fstfinger_b.x && !fstfinger_b.y){fstfinger_b = ordered[1];}
		//if(!thumb_b.x && !thumb_b.y){thumb_b = ordered[0];}
			if(!sort(ordered,pinkie_b))
				{hid_pinkie_b=true;}
			if(!sort(ordered,thrdfinger_b))
				{lost_thrd_b=true;}
			if(!sort(ordered,scndfinger_b))
				{lost_scnd_b=true;}
			if(!sort(ordered,fstfinger_b))
				{lost_fst_b=true;}
			if(!sort(ordered,thumb_b))
				{occ_thumb_b=true;}			
			}
	
	else
		{//cout<<"less";
		bool assigned_thu_b=false; bool assigned_1st_b=false; bool assigned_2nd_b=false; bool assigned_3rd_b=false; bool assigned_pink_b = false;
		int many_b = ordered.size();
		
		for(int thismany_b=0; thismany_b<many_b; thismany_b++){
			double topdist_b = 20.0;
			int whichfinger_b = 0;
			double dist_p = distance(pinkie_b,ordered[thismany_b]);
			double dist_3rd = distance(thrdfinger_b,ordered[thismany_b]);
			double dist_2nd = distance(scndfinger_b,ordered[thismany_b]);
			double dist_1st = distance(fstfinger_b,ordered[thismany_b]);
			double dist_thu = distance(thumb_b,ordered[thismany_b]);
			if(dist_p<dist_3rd && dist_p<dist_2nd && dist_p<dist_1st && dist_p<dist_thu && !assigned_pink_b)
				{//topdist_b=dist_p;
				whichfinger_b = PINKIE_B;}
			else if(dist_3rd<dist_p && dist_3rd<dist_2nd && dist_3rd<dist_1st && dist_3rd<dist_thu && !assigned_3rd_b)
				{//topdist_b=dist_3rd;
				whichfinger_b = THRDFING_B;}
			
			else if(dist_2nd<dist_p && dist_2nd<dist_3rd && dist_2nd<dist_1st && dist_2nd<dist_thu && !assigned_2nd_b)
				{//topdist_b=dist_2nd;
				whichfinger_b = SCNDFING_B;}
			
			else if(dist_1st<dist_p && dist_1st<dist_3rd && dist_1st<dist_2nd && dist_1st<dist_thu && !assigned_1st_b)
				{//topdist_b=dist_1st;
				whichfinger_b = FSTFING_B;}
			
			if((dist_thu<dist_p && dist_thu<dist_3rd && dist_thu<dist_2nd && dist_thu<dist_1st) && (!assigned_thu_b))
				{//topdist_b=dist_thu;
				whichfinger_b = THUMB_B;}
			int thisfinger = whichfinger_b;
			//cout<<"this finger "<<thisfinger<<endl;
			switch(thisfinger){
			case THUMB_B :
					thumb_b = ordered[thismany_b];	
					cout<<"thumb is "<<thumb_b.x<<","<<thumb_b.y<<"\t";
					assigned_thu_b = true;
					break;
			case FSTFING_B:
					fstfinger_b = ordered[thismany_b];
					assigned_1st_b=true;
					cout<<"fst is "<<fstfinger_b.x<<","<<fstfinger_b.y<<"\t";
					break;
			case SCNDFING_B:
					scndfinger_b = ordered[thismany_b];
					assigned_2nd_b = true;
					cout<<"scnd is "<<scndfinger_b.x<<","<<scndfinger_b.y<<"\t";
					break;
			case THRDFING_B:
					thrdfinger_b = ordered[thismany_b];
					assigned_3rd_b = true;
					cout<<"thrd is "<<thrdfinger_b.x<<","<<thrdfinger_b.y<<"\t";
					break;
			case PINKIE_B:
					pinkie_b = ordered[thismany_b];
					assigned_pink_b = true;
					cout<<"pinkie is "<<pinkie_b.x<<","<<pinkie_b.y<<"\t";
					break;
			default:
					;
			}

		}
		//determine which are missing....
		if(!assigned_thu_b)
			{occ_thumb_b = true;
			cout<<"LOST thumb is "<<thumb_b.x<<","<<thumb_b.y<<"\t";}
		if(!assigned_1st_b)
			{lost_fst_b = true;
			cout<<"LOST fst is "<<fstfinger_b.x<<","<<fstfinger_b.y<<"\t";}
		if(!assigned_2nd_b)
			{lost_scnd_b = true;
			cout<<"LOST scnd is "<<scndfinger_b.x<<","<<scndfinger_b.y<<"\t";}
		if(!assigned_3rd_b)
			{lost_thrd_b = true;
			cout<<"LOST thrd is "<<thrdfinger_b.x<<","<<thrdfinger_b.y<<"\t";}
		if(!assigned_pink_b)
			{hid_pinkie_b = true;
			cout<<"LOST pinkie is "<<pinkie_b.x<<","<<pinkie_b.y<<"\t";}
		}
	
	cout.flush();

//separate section to re-evaluate the assignments based on angles.
	//calculate angles from meta joint base point
	/*double ang_thumb = angle(thumb, baseptL);
	double ang_fst = angle(fstfinger, baseptL);
	double ang_scnd = angle(scndfinger, baseptL);
	double ang_thrd = angle(thrdfinger, baseptR);
	double ang_pink = angle(pinkie, baseptR);
	//calculate angles from proxal joint to corresponding meta joint
	double ang_thumb_b = angle(thumb_b, thumb);
	double ang_fst_b = angle(fstfinger_b, fstfinger);
	double ang_scnd_b = angle(scndfinger_b, scndfinger);
	double ang_thrd_b = angle(thrdfinger_b, thrdfinger);
	double ang_pink_b = angle(pinkie_b, pinkie);

	double diff_thumb = ang_thumb_b - ang_thumb;
	double diff_fst = ang_fst_b - ang_fst;
	double diff_scnd = ang_scnd_b - ang_scnd;
	double diff_thrd = ang_thrd_b - ang_thrd;
	double diff_pink = ang_pink_b - ang_pink;
	//second set of angles shouldnt differ so much from the first set! also, for left hand the angles should be in decreasing order wheras for the right hand they should be in increasing order. only exception may be the thumb for both cases so will leave out the thumb for now.

//TODO: only look for crazy angles if the angles seem specifically large compared to the lower joint..

	if(r_or_l) //if its a right hand
		{
			if((diff_fst> 15) && (diff_scnd>15))
				{if(ang_fst_b>ang_scnd_b)
					{//change fst and scnd fingers about
					CvPoint int_fst = fstfinger_b;
					CvPoint int_scnd = scndfinger_b;
					fstfinger_b = int_scnd;
					scndfinger_b = int_fst;
					//recalculate angles for first and second fingers
					ang_fst_b = angle(fstfinger_b, fstfinger);
					ang_scnd_b = angle(scndfinger_b, scndfinger);
					diff_fst = ang_fst_b - ang_fst;
					diff_scnd = ang_scnd_b - ang_scnd;
					}
				}
			if((diff_scnd>15) && (diff_thrd>15))
				{if(ang_scnd_b> ang_thrd_b)
					{//change scnd and thrd fingers about
					CvPoint int_scnd = scndfinger_b;
					CvPoint int_thrd = thrdfinger_b;
					scndfinger_b = int_thrd;
					thrdfinger_b = int_scnd;
					//recalculate angles for scnd and thrd fingers
					ang_scnd_b = angle(scndfinger_b, scndfinger);
					ang_thrd_b = angle(thrdfinger_b, thrdfinger);
					diff_scnd = ang_scnd_b - ang_scnd;
					diff_thrd = ang_thrd_b - ang_thrd;
					}
				}
			if((diff_thrd>15) && (diff_pink>15))
				{if(ang_thrd_b> ang_pink_b)
					{//change thrd and pinkie fingers about
					CvPoint int_thrd = thrdfinger_b;
					CvPoint int_pinkie = pinkie_b;
					thrdfinger_b = int_pinkie;
					pinkie_b = int_thrd;
					//recalculate angles for thrd and pinkie fingers
					ang_thrd_b = angle(thrdfinger_b, thrdfinger);
					ang_pink_b = angle(pinkie_b, pinkie);
					diff_thrd = ang_thrd_b - ang_thrd;
					diff_pink = ang_pink_b - ang_pink;
					}
				}
		}
	else	  //if its a left hand
		{
			if((diff_fst> 15) && (diff_scnd>15))
				{if(ang_fst_b<ang_scnd_b)
					{//change fst and scnd fingers about
					CvPoint int_fst = fstfinger_b;
					CvPoint int_scnd = scndfinger_b;
					fstfinger_b = int_scnd;
					scndfinger_b = int_fst;
					//recalculate angles for first and second fingers
					ang_fst_b = angle(fstfinger_b, fstfinger);
					ang_scnd_b = angle(scndfinger_b, scndfinger);
					diff_fst = ang_fst_b - ang_fst;
					diff_scnd = ang_scnd_b - ang_scnd;
					}
				}
			if((diff_scnd>15) && (diff_thrd>15))
				{if(ang_scnd_b< ang_thrd_b)
					{//change scnd and thrd fingers about
					CvPoint int_scnd = scndfinger_b;
					CvPoint int_thrd = thrdfinger_b;
					scndfinger_b = int_thrd;
					thrdfinger_b = int_scnd;
					//recalculate angles for scnd and thrd fingers
					ang_scnd_b = angle(scndfinger_b, scndfinger);
					ang_thrd_b = angle(thrdfinger_b, thrdfinger);
					diff_scnd = ang_scnd_b - ang_scnd;
					diff_thrd = ang_thrd_b - ang_thrd;
					}
				}
			if((diff_thrd>15) && (diff_pink>15))
				{if(ang_thrd_b< ang_pink_b)
					{//change thrd and pinkie fingers about
					CvPoint int_thrd = thrdfinger_b;
					CvPoint int_pinkie = pinkie_b;
					thrdfinger_b = int_pinkie;
					pinkie_b = int_thrd;
					//recalculate angles for thrd and pinkie fingers
					ang_thrd_b = angle(thrdfinger_b, thrdfinger);
					ang_pink_b = angle(pinkie_b, pinkie);
					diff_thrd = ang_thrd_b - ang_thrd;
					diff_pink = ang_pink_b - ang_pink;
					}
				}
		} */


	//check the x coordinate order for the meta group - this should not change as it is impossible!
	if(r_or_l){ //right hand!
		if (thumb_b.x > fstfinger_b.x)
			{ CvPoint tempfing = thumb_b ;
			 thumb_b = fstfinger_b;
			fstfinger_b = tempfing; }
		if (fstfinger_b.x > scndfinger_b.x)
			{ CvPoint tempfing = fstfinger_b ;
			 fstfinger_b = scndfinger_b;
			scndfinger_b = tempfing; }
		if (scndfinger_b.x > thrdfinger_b.x)
			{ CvPoint tempfing = scndfinger_b ;
			 scndfinger_b = thrdfinger_b;
			thrdfinger_b = tempfing;   }
		if (thrdfinger_b.x > pinkie_b.x)
			{  CvPoint tempfing = thrdfinger_b ;
			 thrdfinger_b = pinkie_b;
			pinkie_b = tempfing;   }

	}
	else{  //left hand!
		if (thumb_b.x < fstfinger_b.x)
			{ CvPoint tempfing = thumb_b ;
			 thumb_b = fstfinger_b;
			fstfinger_b = tempfing; }
		if (fstfinger_b.x < scndfinger_b.x)
			{ CvPoint tempfing = fstfinger_b ;
			 fstfinger_b = scndfinger_b;
			scndfinger_b = tempfing;    }
		if (scndfinger_b.x < thrdfinger_b.x)
			{ CvPoint tempfing = scndfinger_b ;
			 scndfinger_b = thrdfinger_b;
			thrdfinger_b = tempfing;   }
		if (thrdfinger_b.x < pinkie_b.x)
			{ CvPoint tempfing = thrdfinger_b ;
			 thrdfinger_b = pinkie_b;
			pinkie_b = tempfing; }

	}
 
}

void Fullhand::distset(CoorVec joints, bool r_or_l){

	CoorVec ordered = joints;
	// fingers now in order f lowest x axis value to highest.
	//initialise boolean arguments for missing fingers
	hid_pinkie_c = false;
	lost_thrd_c = false;
	lost_scnd_c = false;
	lost_fst_c = false;

	//calculate which fingers are missing
	int newsize = ordered.size();
	if(newsize==4 && !r_or_l)
		{ 
		if(!pinkie_c.x && !pinkie_c.y){pinkie_c = ordered[0];}
		if(!thrdfinger_c.x && !thrdfinger_c.y){thrdfinger_c = ordered[1]; }
		if(!scndfinger_c.x && !scndfinger_c.y){scndfinger_c = ordered[2];}
		if(!fstfinger_c.x && !fstfinger_c.y){fstfinger_c = ordered[3];}
		if(!sort(ordered,pinkie_c))
			{hid_pinkie_c=true;}
		if(!sort(ordered,thrdfinger_c))
			{lost_thrd_c=true;}
		if(!sort(ordered,scndfinger_c))
			{lost_scnd_c=true;}
		if(!sort(ordered,fstfinger_c))
			{lost_fst_c=true;}
		}
	if(newsize==4 && r_or_l)
		{ 
		if(!pinkie_c.x && !pinkie_c.y){pinkie_c = ordered[3];}
		if(!thrdfinger_c.x && !thrdfinger_c.y){thrdfinger_c = ordered[2]; }
		if(!scndfinger_c.x && !scndfinger_c.y){scndfinger_c = ordered[1];}
		if(!fstfinger_c.x && !fstfinger_c.y){fstfinger_c = ordered[0];}
		if(!sort(ordered,pinkie_c))
			{hid_pinkie_c=true;}
		if(!sort(ordered,thrdfinger_c))
			{lost_thrd_c=true;}
		if(!sort(ordered,scndfinger_c))
			{lost_scnd_c=true;}
		if(!sort(ordered,fstfinger_c))
			{lost_fst_c=true;}
		}
	else if(newsize>4) //if more than 5 points, go through them all and see which is closest to previous points
			{ 
			if(!sort(ordered,pinkie_c))
				{hid_pinkie_c=true;}
			if(!sort(ordered,thrdfinger_c))
				{lost_thrd_c=true;}
			if(!sort(ordered,scndfinger_c))
				{lost_scnd_c=true;}
			if(!sort(ordered,fstfinger_c))
				{lost_fst_c=true;}
			}
	else
		{ 
		bool assigned_1st_c = false; bool assigned_2nd_c=false; bool assigned_3rd_c=false; bool assigned_pink_c = false;
		int many_c = ordered.size();
		
		for(int thismany_c=0; thismany_c<many_c; thismany_c++){
			double topdist_c = 40.0;
			int whichfinger_c = 6;
			double dist_p = distance(pinkie_c,ordered[thismany_c]);
			double dist_3rd = distance(thrdfinger_c,ordered[thismany_c]);
			double dist_2nd = distance(scndfinger_c,ordered[thismany_c]);
			double dist_1st = distance(fstfinger_c,ordered[thismany_c]);
			if(dist_p<dist_3rd && dist_p<dist_2nd && dist_p<dist_1st && !assigned_pink_c)
				{topdist_c=dist_p;
				whichfinger_c = PINKIE_C;}
			
			else if(dist_3rd<dist_p && dist_3rd<dist_2nd && dist_3rd<dist_1st && !assigned_3rd_c)
				{topdist_c=dist_3rd;
				whichfinger_c = THRDFING_C;}
			
			else if(dist_2nd<dist_p && dist_2nd<dist_3rd && dist_2nd<dist_1st && !assigned_2nd_c)
				{topdist_c=dist_2nd;
				whichfinger_c = SCNDFING_C;}
			
			else if(dist_1st<dist_p && dist_1st<dist_3rd && dist_1st<dist_2nd && !assigned_1st_c)
				{topdist_c=dist_1st;
				whichfinger_c = FSTFING_C;}
			int thisfinger = whichfinger_c;

			switch(thisfinger){
			
			case FSTFING_C:
					fstfinger_c = ordered[thismany_c];
					assigned_1st_c=true;
					break;
			case SCNDFING_C:
					scndfinger_c = ordered[thismany_c];
					assigned_2nd_c = true;
					break;
			case THRDFING_C:
					thrdfinger_c = ordered[thismany_c];
					assigned_3rd_c = true;
					break;
			case PINKIE_C:
					pinkie_c = ordered[thismany_c];
					assigned_pink_c = true;
					break;
			default:
					;
			}
		}
		//determine which are missing....
		if(!assigned_1st_c)
			{lost_fst_c = true;
			fstfinger_c = fstfinger_c;}
		if(!assigned_2nd_c)
			{lost_scnd_c = true;
			scndfinger_c = scndfinger_c;}
		if(!assigned_3rd_c)
			{lost_thrd_c = true;
			thrdfinger_c = thrdfinger_c;}
		if(!assigned_pink_c)
			{hid_pinkie_c = true;
			pinkie_c=pinkie_c;}
		}
	
//separate section to re-evaluate the assignments based on angles.
	//calculate angles from meta joint base point

	/*double ang_fst_b = angle(fstfinger_b, fstfinger);
	double ang_scnd_b = angle(scndfinger_b, scndfinger);
	double ang_thrd_b = angle(thrdfinger_b, thrdfinger);
	double ang_pink_b = angle(pinkie_b, pinkie);
	//calculate angles from proxal joint to corresponding meta joint
	double ang_fst_c = angle(fstfinger_c, fstfinger_b);
	double ang_scnd_c = angle(scndfinger_c, scndfinger_b);
	double ang_thrd_c = angle(thrdfinger_c, thrdfinger_b);
	double ang_pink_c = angle(pinkie_c, pinkie_b);

	double diff_fst_c = ang_fst_c - ang_fst_b;
	double diff_scnd_c = ang_scnd_c - ang_scnd_b;
	double diff_thrd_c = ang_thrd_c - ang_thrd_b;
	double diff_pink_c = ang_pink_c - ang_pink_b;
	//second set of angles shouldnt differ so much from the first set! also, for left hand the angles should be in decreasing order wheras for the right hand they should be in increasing order. only exception may be the thumb for both cases so will leave out the thumb for now.

//TODO: only look for crazy angles if the angles seem specifically large compared to the lower joint..

	if(r_or_l) //if its a right hand
		{
			if((diff_fst_c> 15) || (diff_scnd_c>15))
			{if(ang_fst_c>ang_scnd_c)
				{//change fst and scnd fingers about
				CvPoint int_fst = fstfinger_c;
				CvPoint int_scnd = scndfinger_c;
				fstfinger_c = int_scnd;
				scndfinger_c = int_fst;
				//recalculate angles for first and second fingers
				ang_fst_c = angle(fstfinger_c, fstfinger_b);
				ang_scnd_c = angle(scndfinger_c, scndfinger_b);
				}
			}
			if((diff_scnd_c>15) || (diff_thrd_c>15))
			{if(ang_scnd_c> ang_thrd_c)
				{//change scnd and thrd fingers about
				CvPoint int_scnd = scndfinger_c;
				CvPoint int_thrd = thrdfinger_c;
				scndfinger_c = int_thrd;
				thrdfinger_c = int_scnd;
				//recalculate angles for scnd and thrd fingers
				ang_scnd_c = angle(scndfinger_c, scndfinger_b);
				ang_thrd_c = angle(thrdfinger_c, thrdfinger_b);
				}
			}
			if((diff_thrd_c>15) || (diff_pink_c>15))
			{if(ang_thrd_c> ang_pink_c)
				{//change thrd and pinkie fingers about
				CvPoint int_thrd = thrdfinger_c;
				CvPoint int_pinkie = pinkie_c;
				thrdfinger_c = int_pinkie;
				pinkie_c = int_thrd;
				//recalculate angles for thrd and pinkie fingers
				ang_thrd_c = angle(thrdfinger_c, thrdfinger_b);
				ang_pink_c = angle(pinkie_c, pinkie_b);
				}
			}
		}
	else	  //if its a left hand
		{
			if((diff_fst_c> 15) || (diff_scnd_c>15))
			{if(ang_fst_c<ang_scnd_c)
				{//change fst and scnd fingers about
				CvPoint int_fst = fstfinger_c;
				CvPoint int_scnd = scndfinger_c;
				fstfinger_c = int_scnd;
				scndfinger_c = int_fst;
				//recalculate angles for first and second fingers
				ang_fst_c = angle(fstfinger_c, fstfinger_b);
				ang_scnd_c = angle(scndfinger_c, scndfinger_b);
				}
			}
			if((diff_scnd_c>15) || (diff_thrd_c>15))
			{if(ang_scnd_c< ang_thrd_c)
				{//change scnd and thrd fingers about
				CvPoint int_scnd = scndfinger_c;
				CvPoint int_thrd = thrdfinger_c;
				scndfinger_c = int_thrd;
				thrdfinger_c = int_scnd;
				//recalculate angles for scnd and thrd fingers
				ang_scnd_c = angle(scndfinger_c, scndfinger_b);
				ang_thrd_c = angle(thrdfinger_c, thrdfinger_b);
				}
			}
			if((diff_thrd_c>15) || (diff_pink_c>15))
			{if(ang_thrd_c< ang_pink_c)
				{//change thrd and pinkie fingers about
				CvPoint int_thrd = thrdfinger_c;
				CvPoint int_pinkie = pinkie_c;
				thrdfinger_c = int_pinkie;
				pinkie_c = int_thrd;
				//recalculate angles for thrd and pinkie fingers
				ang_thrd_c = angle(thrdfinger_c, thrdfinger_b);
				ang_pink_c = angle(pinkie_c, pinkie_b);
				}
			}
		}*/


	//check the x coordinate order for the meta group - this should not change as it is impossible!
	if(r_or_l){ //right hand!
		
		if (fstfinger_c.x > scndfinger_c.x)
			{ CvPoint tempfing = fstfinger_c ;
			 fstfinger_c = scndfinger_c;
			scndfinger_c = tempfing; }
		if (scndfinger_c.x > thrdfinger_c.x)
			{ CvPoint tempfing = scndfinger_c ;
			 scndfinger_c = thrdfinger_c;
			thrdfinger_c= tempfing;   }
		if (thrdfinger_c.x > pinkie_c.x)
			{  CvPoint tempfing = thrdfinger_c ;
			 thrdfinger_c = pinkie_c;
			pinkie_c = tempfing;   }

	}
	else{  //left hand!
	
		if (fstfinger_c.x < scndfinger_c.x)
			{ CvPoint tempfing = fstfinger_c ;
			 fstfinger_c = scndfinger_c;
			scndfinger_c = tempfing;    }
		if (scndfinger_c.x < thrdfinger_c.x)
			{ CvPoint tempfing = scndfinger_c ;
			 scndfinger_c = thrdfinger_c;
			thrdfinger_c = tempfing;   }
		if (thrdfinger_c.x < pinkie_c.x)
			{ CvPoint tempfing = thrdfinger_c ;
			 thrdfinger_c = pinkie_c;
			pinkie_c = tempfing; }

	}


}

bool Fullhand::sort(CoorVec &list, CvPoint &finger)
{
	int newsize = list.size();
	double topdist = 50.0;
	
	CoorVecIt no_p  = list.begin();
	CoorVecIt no = list.begin();
	int number = 800; // a random integer that starts off at a high number so that it isnt assigned to element 0 automatically.
	for (int num = 0; num<newsize;num++)
		{	//calculate all distances between fingers and elements of ordered. smallest distance is the winner!
		double fingerdist = distance(finger,list[num]);
		if(fingerdist<topdist)
			{topdist=fingerdist;
			no_p = no;
			number=num;
			}
		no++;
		}
	if(number<(newsize+1)){finger = list[number];
	list.erase(no_p);
	return (true);}
	else
	{return (false);}

}

 
void Fullhand::checkgroups(CoorVec &proxal, CoorVec &distal, int width){

	/*go through the x ordered vectors and for each pair of elements, check angles corresponding to basept. when checking proxal vector,if angles too similar, delete element with larger distance from basept and place it in distal vector and rearrange for x order. same goes vice versa. at the end of function rearrange both for xorder.*/
	CoorVec copy_proxal;
	CoorVec copy_distal;
	copy_proxal.assign(proxal.begin(),proxal.end());
	copy_distal.assign(distal.begin(),distal.end());
	
	//proxal vector checking
	CvPoint p = cvPoint(0,0);
	int proxsize = copy_proxal.size();
	CoorVecIt proxit = copy_proxal.begin();
	CoorVecIt deleteprox = copy_proxal.begin();
	//cout<<"inital prox size is "<<proxsize<<"\t";
	for (int points = 1; points<proxsize;points++){
			//calculate angles of copy_proxal[points] and [points-1]
			double angle1 = angle(copy_proxal[points-1], baseptL);
			double angle2 = angle(copy_proxal[points],baseptL);
			int diff = (int)angle1-angle2;
			diff = abs(diff);
			//if angle1-angle2 < 5 then the furthest away point belongs to the distal vector - these vectors are of points ordered by x position : two next to each other could be distal and proxal as long as the angles between the two arent massive.
			if(diff<10){
				double distance1 = distance(copy_proxal[points-1],baseptL);
				double distance2 = distance(copy_proxal[points],baseptL);
				if(distance2>distance1){
					//cout<<"rid of second ";
					p = copy_proxal[points];
					copy_distal.push_back(p);
					deleteprox = proxit+1;
					copy_proxal.erase(deleteprox);
					proxsize--;
					points = points-1;
				}else{
					//cout<<"rid of first ";
					p = copy_proxal[points-1];
					copy_distal.push_back(p);
					deleteprox = proxit;
					copy_proxal.erase(deleteprox);
					proxsize--;
					points = points-1;
				}
			}
		proxit++;	
	}

	//cout<<"new prox size is "<<copy_proxal.size()<<endl;
	//reorder for x the distal vector with any new elements
	CoorVec newdistal_copy = Organise::order_x(copy_distal,width,r_or_l);

	//distal vector checking
	CvPoint d = cvPoint(0,0);
	int distsize = newdistal_copy.size();
	CoorVecIt distit = newdistal_copy.begin();
	CoorVecIt deletedist = newdistal_copy.begin();
	//cout<<"inital dist size is "<<distsize<<"\t";
	for (int d_points = 1; d_points<distsize;d_points++){
			
			//calculate angles of copy_proxal[points] and [points-1]
			double d_angle1 = angle(newdistal_copy[d_points-1], baseptL);
			double d_angle2 = angle(newdistal_copy[d_points],baseptL);
			int d_diff = (int)d_angle1-d_angle2;
			d_diff = abs(d_diff);
			//if angle1-angle2 > 5 then the furthest away point belongs to the distal vector
			if(d_diff<3){
				double d_distance1 = distance(newdistal_copy[d_points-1],baseptL);
				double d_distance2 = distance(newdistal_copy[d_points],baseptL);
				if(d_distance2<d_distance1){
					//cout<<"remove second ";cout.flush();
					d = newdistal_copy[d_points];
					copy_proxal.push_back(d);
					deletedist = distit+1;
					newdistal_copy.erase(deletedist);
					distsize--;
					d_points = d_points-1;
				}else{
					//cout<<"remove first ";cout.flush();
					d = newdistal_copy[d_points-1];
					copy_proxal.push_back(d);
					deletedist = distit;
					newdistal_copy.erase(deletedist);
					distsize--;
					d_points=d_points-1;
				}
			}
		distit++;	
	}
	//cout<<"new dist size is "<<newdistal_copy.size()<<"\t"<<"distal vector: ";
// 	for(int test = 0; test<newdistal_copy.size();test++){
// 		cout<<newdistal_copy[test].x<<","<<newdistal_copy[test].y<<" : ";
// 	} cout<<endl;
// 	//reorder both proxal and distal vectors for x
 	CoorVec newprox = Organise::order_x(copy_proxal,width,r_or_l);
	CoorVec newdist = Organise::order_x(newdistal_copy,width,r_or_l);

	//copy back into original vectors
	proxal.assign(newprox.begin(),newprox.end());
	distal.assign(newdist.begin(),newdist.end());

}

double Fullhand::angle(CvPoint thepoint, CvPoint base)
{
	double length = distance(thepoint,base);
	double polar_angle = acos(((thepoint.x-base.x)/length)) * (180/M_PI);
	return (polar_angle);
}

double Fullhand::distance(CvPoint onepoint, CvPoint twopoint)
{
	int xa, ya =0;
	xa = abs(onepoint.x-twopoint.x);
 	ya = abs(onepoint.y-twopoint.y);
	double length = sqrt(xa*xa + ya*ya);
	return(length);
}


Fullhand::~Fullhand(){
	
}

