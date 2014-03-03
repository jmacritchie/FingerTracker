// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "fullhand.h"
#include "transform.h"
#include "organise.h"
#include "correlation.h"
#include <iostream>

using namespace std;
using namespace cv;

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

void Fullhand::setallbases(CoorVec &rightgroup, int height, IplImage* prev_frame, IplImage* now_frame, int r_or_l){

	//for first frame, take lowest y coordinate point in pointset to be the base, for all consecutive frames - calculate the correlation between the previous base point and the three lowest y coordinate points.
	CoorVec ordered;
	CoorVec baseordered;
	baseordered = Organise::order_y(rightgroup, height);
	int rsize = baseordered.size();
	CoorVecIt bases_it = baseordered.begin();

	if(baseptL.x==0 && baseptL.y==0 && baseptR.x==0 && baseptR.y==0){ 	//if the base point hasnt been set since its initialisation in constructor
		if(rsize>1&&r_or_l){
			//search through all the points and find the point with highest y coordinate (remember the top left corner of the image is coordinate 0,0) 
			if(baseordered[0].x>baseordered[1].x){
				baseptL = baseordered[1];
				baseptR = baseordered[0];
			}
			else{
				baseptL = baseordered[0];
				baseptR = baseordered[1];
			}
			baseordered.erase(bases_it);
			baseordered.erase(bases_it++);
		}	
		if(rsize>1 && !r_or_l){			
			//search through all the points and find the point with highest y coordinate (remember the top left corner of the image is coordinate 0,0) 
			if(baseordered[0].x>baseordered[1].x){
				baseptR = baseordered[1];
				baseptL = baseordered[0];
			}
			else{
				baseptR = baseordered[0];
				baseptL = baseordered[1];
			}
			baseordered.erase(bases_it);
			baseordered.erase(bases_it++);
		}	
		cout<< "Basepoint Left for this hand is "<<baseptL.x<<","<<baseptL.y<<endl;
		cout<< "Basepoint Right for this hand is "<<baseptR.x<<","<<baseptR.y<<endl;
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
		double max_distance = 20.0;
		CvPoint l_base = cvPoint(0,0);
		int setbase = 0;
		for(int b = 0; b<3; b++){ //only checking the three lowest y-value coordinates
			if(baseordered.size()>b){
				if(distance(poss_base_L,baseordered[b])<max_distance){
					max_distance = distance(poss_base_L,baseordered[b]);
					l_base = baseordered[b];
					cout<<"l_base is now"<<l_base.x<<","<<l_base.y<<" : maxdistance:"<<max_distance<<endl;
					itbase = iter;
					setbase = 1;
				}
				iter++;
			}
		}
		if (setbase==0 && distance(poss_base_L,baseptL)<distance(l_base, baseptL)){
			//if distance between poss_baseL and baseptL is smaller than what l_base actually is.this will also apply if lbase has not yet been assigned a marker			
			l_base = poss_base_L;
			cout<<"gone with correlation point: l_base is now"<<l_base.x<<","<<l_base.y<<endl;
		}
		baseptL = l_base;
		cout<< "Correlated Basepoint Left for this hand is "<<baseptL.x<<","<<baseptL.y<<endl;
		if(baseordered.size()>0)baseordered.erase(itbase);
		//delete this element from rightgroup

		//same again for right basepoint
		CoorVecIt iterR = baseordered.begin();
		CoorVecIt itbaseR = baseordered.begin();
		double max_distanceR = 20.0;
		CvPoint r_base = cvPoint(0,0);
		int setbaseR=0;
		for(int c = 0; c<3; c++){ //only checking the three lowest y-value coordinates
			if(baseordered.size()>c){
				if(distance(poss_base_R,baseordered[c])<max_distanceR){
					max_distanceR = distance(poss_base_R,baseordered[c]);
					r_base = baseordered[c];
					itbaseR = iterR;
					setbaseR = 1;
					cout<<"r_base is now"<<r_base.x<<","<<r_base.y<<" : maxdistance:"<<max_distanceR<<endl;
				}
			iterR++;
			}
		}
		if (setbaseR==0 && distance(poss_base_R,baseptR)<distance(r_base, baseptR)){
			r_base = poss_base_R;
			cout<<"gone with correlation point: r_base is now"<<r_base.x<<","<<r_base.y<<endl;
		}

		baseptR = r_base;
		cout<< "Correlated Basepoint Right for this hand is "<<baseptR.x<<","<<baseptR.y<<endl;
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

	if (r_or_l){cout<< "setting fingers for right hand"<<endl;}
	else{cout<< "setting fingers for left hand"<<endl;}
	//the left hand - furthest coordinate in x axis will be 5th finger
	//the right hand - furthest coordinate in x axis will be thumb 

	if(!meta_mindistance){meta_mindistance = 25;} //these distances will change depending on the previous frame measurements. save these distances as members of the hand object.
	if(!meta_maxdistance){meta_maxdistance = 50;}
	if(!prox_maxdistance){prox_maxdistance = 75;}
	if(!dist_maxdistance){dist_maxdistance= 100;} 
	
	cout<<"Distances are:"<<meta_mindistance<<","<<meta_maxdistance<<","<<prox_maxdistance<<","<<dist_maxdistance<<endl;
	
	CoorVec meta;
	CoorVec prox;	
	CoorVec distal;
	CvPoint midbase;
	
	//all distances are calculated according to the mid point between the two wrist points
	midbase.x = min(baseptL.x,baseptR.x) + abs((baseptR.x - baseptL.x)/2);
	midbase.y = min(baseptL.y,baseptR.y) + abs((baseptR.y - baseptL.y)/2);

	if(r_or_l){
	  if(hand.size()>0){cout<< hand.size()<<":"<< hand[0].x<<","<<hand[0].y<<endl;}
	}
	int shrinksize = hand.size();
 	for (int element=0;element<shrinksize;element++){
		//for the amount of elements existing, go through and determine smallest distance from wrist marker.
		//initialise iterators
		CoorVecIt start = hand.begin();
		CoorVecIt position = hand.begin();
		CvPoint thispoint = cvPoint(0,0);
		double distance = 200;   //only want blobs within 200 pixel distance of the base point to be considered
		//determine size of vector of elements to be re-assigned
		int newsize = hand.size();
		//this loop finds the next nearest blob
		for(int existing=0;existing<newsize;existing++){
			int xd, yd =0;
			xd = abs(hand[existing].x-midbase.x); 
			yd = abs(hand[existing].y-midbase.y);
			double newdistance = sqrt(xd*xd + yd*yd);
			if(newdistance<distance){
				distance = newdistance;
				thispoint = hand[existing];
				start = position;
			}	
			position++;
		}
	//find in which range of distance i.e which joint this point belongs to and push back the appropriate vector
		if(distance>meta_mindistance && distance<dist_maxdistance){ //make sure point isnt right next to base point e.g. when the base point blob has eroded into two parts
			if(distance<meta_maxdistance){
				meta.push_back(thispoint);
				cout<<"META thispoint:"<< thispoint.x <<","<<thispoint.y<<" with distance "<<distance<<endl;
				hand.erase(start);}
			else if(distance<prox_maxdistance){
				prox.push_back(thispoint);
				cout<<"PROX thispoint:"<< thispoint.x <<","<<thispoint.y<<" with distance "<<distance<<endl;
				hand.erase(start);}
			else if (distance<dist_maxdistance){
				distal.push_back(thispoint);
				cout<<"DIST thispoint:"<< thispoint.x <<","<<thispoint.y<<" with distance "<<distance<<endl;
				hand.erase(start);}
		}
		else{hand.erase(start);}
	}
	//order each vector in terms of x depending on right or left hand
	cout<<"First Organise x meta: "<<r_or_l<<":";
	CoorVec sort_meta = Organise::order_x(meta,width,r_or_l);
	cout<<"First Organise x prox: "<<r_or_l<<":";
	CoorVec sort_prox = Organise::order_x(prox,width,r_or_l);
	cout<<"FirstOrganise x dist: "<<r_or_l<<":";
	CoorVec sort_dist = Organise::order_x(distal,width,r_or_l);
	//Checkblobs function checks that blobs have not been split in two, i.e. there is not a small distance between centroids.
	sort_meta = Organise::checkblobs(sort_meta);
	sort_prox = Organise::checkblobs(sort_prox);
	sort_dist = Organise::checkblobs(sort_dist);

	//checks the relationship between the proximal and distal markers so that points have not been assigned the wrong group.	
	checkgroups(sort_prox,sort_dist,width);
	//assign markers into groups
	metaset(sort_meta,r_or_l);
	proxset(sort_prox,r_or_l);
	distset(sort_dist,r_or_l);
	
	//clear vectors
	meta.clear();
	sort_meta.clear();
	prox.clear();
	sort_prox.clear();
	distal.clear();
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
		if (!occ_thumb){
			draw_cross(thumb,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb,CV_RGB(255,255,60), 1, 0 );
			cvLine( fgr_grp_img, thumb,fstfinger,CV_RGB(255,255,60), 1, 0 );
		}
		else{
			cvLine( fgr_grp_img, baseptL,fstfinger,CV_RGB(255,255,60), 1, 0 );
		}
		if (!hid_pinkie){
			draw_cross(pinkie, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie,baseptL,CV_RGB(255,255,60), 1, 0 );
			if(!lost_thrd) {
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger,pinkie,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );
			}
			else{
				cvLine( fgr_grp_img, scndfinger,pinkie,CV_RGB(255,255,60), 1, 0 );
			}
		}
		else{
			if(!lost_thrd){
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger,baseptL,CV_RGB(255,255,60), 1, 0 );
			}
		}
	}
	else{
		if(!lost_scnd){ 
			draw_cross(scndfinger, CV_RGB(0,255,0),4);
		}
		if (!occ_thumb){
			draw_cross(thumb,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb,CV_RGB(255,255,60), 1, 0 );
			cvLine( fgr_grp_img, thumb,scndfinger,CV_RGB(255,255,60), 1, 0 );
		}
		else {
			cvLine( fgr_grp_img, baseptL,scndfinger,CV_RGB(255,255,60), 1, 0 );
		}
		if (!hid_pinkie){
			draw_cross(pinkie, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie,baseptL,CV_RGB(255,255,60), 1, 0 );
			if(!lost_thrd){
				draw_cross(thrdfinger, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger,pinkie,CV_RGB(255,255,60), 1, 0 );
				cvLine( fgr_grp_img, scndfinger,thrdfinger,CV_RGB(255,255,60), 1, 0 );
			}
			else{
				cvLine( fgr_grp_img, scndfinger,pinkie,CV_RGB(255,255,60), 1, 0 );
			}
		}
		else{
			if(!lost_thrd){
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
		if (!occ_thumb_b){
			draw_cross(thumb_b,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb_b,CV_RGB(255,60,255), 1, 0 );
			cvLine( fgr_grp_img, thumb_b,fstfinger_b,CV_RGB(255,60,255), 1, 0 );
		}
		else{
			cvLine( fgr_grp_img, baseptL,fstfinger_b,CV_RGB(255,60,255), 1, 0 );
		}
		if (!hid_pinkie_b){
			draw_cross(pinkie_b, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_b,baseptL,CV_RGB(255,60,255), 1, 0 );
			if(!lost_thrd_b){
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );
			}
			else{
				cvLine( fgr_grp_img, scndfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );
			}
		}
		else{
			if(!lost_thrd_b){
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger_b,baseptL,CV_RGB(255,60,255), 1, 0 );
			}
		}
	}	
	else{
		if(!lost_scnd_b){ 
			draw_cross(scndfinger_b, CV_RGB(0,255,0),4);
		}
		if (!occ_thumb_b){
			draw_cross(thumb_b,CV_RGB(0,255,255),4);
			cvLine( fgr_grp_img, baseptL,thumb_b,CV_RGB(255,60,255), 1, 0 );
			cvLine( fgr_grp_img, thumb_b,scndfinger_b,CV_RGB(255,60,255), 1, 0 );
		}
		else{
			cvLine( fgr_grp_img, baseptL,scndfinger_b,CV_RGB(255,255,60), 1, 0 );
		}
		if (!hid_pinkie_b){
			draw_cross(pinkie_b, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_b,baseptL,CV_RGB(255,60,255), 1, 0 );
			if(!lost_thrd_b){
				draw_cross(thrdfinger_b, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_b,thrdfinger_b,CV_RGB(255,60,255), 1, 0 );
			}
			else{
				cvLine( fgr_grp_img, scndfinger_b,pinkie_b,CV_RGB(255,60,255), 1, 0 );
			}
		}
		else{
			if(!lost_thrd_b){
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
		if (!hid_pinkie_c){
			draw_cross(pinkie_c, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_c,baseptL,CV_RGB(60,60,255), 1, 0 );
			if(!lost_thrd_c){
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );
			}
			else{
				cvLine( fgr_grp_img, scndfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );
			}
		}
		else{
			if(!lost_thrd_c){
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, thrdfinger_c,baseptL,CV_RGB(60,60,255), 1, 0 );
			}
		}
	}
	else{
		if(!lost_scnd_c){
			draw_cross(scndfinger_c, CV_RGB(0,255,0),4);
			cvLine( fgr_grp_img, baseptL,scndfinger_c,CV_RGB(60,60,255), 1, 0 );
		}
		if (!hid_pinkie_c){
			draw_cross(pinkie_c, CV_RGB(255,255,0),4);
			cvLine( fgr_grp_img, pinkie_c,baseptL,CV_RGB(60,60,255), 1, 0 );
			if(!lost_thrd_c){
				draw_cross(thrdfinger_c, CV_RGB(0,0,255),4);
				cvLine( fgr_grp_img, thrdfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );
				cvLine( fgr_grp_img, scndfinger_c,thrdfinger_c,CV_RGB(60,60,255), 1, 0 );
			}
			else{
				cvLine( fgr_grp_img, scndfinger_c,pinkie_c,CV_RGB(60,60,255), 1, 0 );
			}
		}
		else{
			if(!lost_thrd_c){
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
  
	//for each finger draw lines between base marker and metacarpal, proximal and distal joint
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


IplImage* Fullhand::drawcrosses(IplImage *fgr_grp_img)
{
	//function to draw crosses at new estimated positions of markers. Still to be included in functionality of program.
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
	if(est_thumb.x || est_thumb.y){
		draw_circle(imahge, est_thumb, CV_RGB(255,0,0),3);
		 //set estimation as thumb point
		 setthumb(est_thumb);
		 occ_thumb = false;
		 est_thumb = cvPoint(0,0);}
	if(est_first.x || est_first.y){
		draw_circle(imahge, est_first, CV_RGB(255,0,0),3);
		 set1stfing(est_first);
		 lost_fst = false;
		 est_first = cvPoint(0,0);}
	if(est_second.x || est_second.y){
		draw_circle(imahge, est_second, CV_RGB(255,0,0),3);
		 set2ndfing(est_second);
		 lost_scnd = false;
		 est_second = cvPoint(0,0);}
	if(est_third.x || est_third.y){
		draw_circle(imahge, est_third, CV_RGB(255,0,0),3);
		 set3rdfing(est_third);
		 lost_thrd = false;
		 est_third = cvPoint(0,0);}
	if(est_pinkie.x || est_pinkie.y){
		draw_circle(imahge, est_pinkie, CV_RGB(255,0,0),3);
		 setpinkie(est_pinkie);
		 hid_pinkie = false;
		 est_pinkie = cvPoint(0,0);}
	if(est_thumb_b.x || est_thumb_b.y){
		draw_circle(imahge, est_thumb_b, CV_RGB(255,0,0),3);
		 setthumb_b(est_thumb_b);
		 occ_thumb_b = false;
		 est_thumb_b = cvPoint(0,0);}
	if(est_first_b.x || est_first_b.y){
		draw_circle(imahge, est_first_b, CV_RGB(255,0,0),3);
		 set1stfing_b(est_first_b);
		 lost_fst_b = false;
		 est_first_b = cvPoint(0,0);}
	if(est_second_b.x || est_second_b.y){
		draw_circle(imahge, est_second_b, CV_RGB(255,0,0),3);
		 set2ndfing_b(est_second_b);
		 lost_scnd_b = false;
		 est_second_b = cvPoint(0,0);}
	if(est_third_b.x || est_third_b.y){
		draw_circle(imahge, est_third_b, CV_RGB(255,0,0),3);
		 set3rdfing_b(est_third_b);
		 lost_thrd_b = false;
		 est_third_b = cvPoint(0,0);}
	if(est_pinkie_b.x || est_pinkie_b.y){
		draw_circle(imahge, est_pinkie_b, CV_RGB(255,0,0),3);
		 setpinkie_b(est_pinkie_b);
		 hid_pinkie_b = false;
		 est_pinkie_b = cvPoint(0,0);}
	if(est_first_c.x || est_first_c.y){
		draw_circle(imahge, est_first_c, CV_RGB(255,0,0),3);
		 set1stfing_c(est_first_c);
		 lost_fst_c = false;
		 est_first_c = cvPoint(0,0);}
	if(est_second_c.x || est_second_c.y){
		draw_circle(imahge, est_second_c, CV_RGB(255,0,0),3);
		 set2ndfing_c(est_second_c);
		 lost_scnd_c = false;
		 est_second_c = cvPoint(0,0);}
	if(est_third_c.x || est_third_c.y){
		draw_circle(imahge, est_third_c, CV_RGB(255,0,0),3);
		 set3rdfing_c(est_third_c);
		 lost_thrd_c = false;
		 est_third_c = cvPoint(0,0);}
	if(est_pinkie_c.x || est_pinkie_c.y){
		draw_circle(imahge, est_pinkie_c, CV_RGB(255,0,0),3);
		 setpinkie_c(est_pinkie_c);
		 hid_pinkie_c = false;
		 est_pinkie_c = cvPoint(0,0);}

	return (imahge);
}

CoorVec Fullhand::setpoints_wrt_org(){
	
	CoorVec fingers;
	//find middle of the two base points.
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
	// markers already in order either from lowest to highest x if RH or highest to lowest x if LH.

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

	bool assigned_thu=false; bool assigned_1st=false; bool assigned_2nd =false; bool assigned_3rd = false; bool assigned_pink = false;
	//if this is the first frame, calculate mid point of two bases and estimate positions from there using x coordinates.
	if(!pinkie.x && !thrdfinger.x && !scndfinger.x && !fstfinger.x && !thumb.x){
	  CoorVec scnd_candidates;
	  CvPoint midbase = cvPoint(0,0);
	  double basedist = 100.0;
	  int markerID;
	  midbase.x = min(baseptL.x,baseptR.x)+abs((baseptR.x-baseptL.x)/2);
	  for (int c = 0; c<ordered.size(); c++){ 
	    double dist = abs(ordered[c].x-midbase.x);
	    if((!r_or_l && dist<basedist && ordered[c].x>=baseptR.x && ordered[c].x<=baseptL.x)||(r_or_l && dist<basedist && ordered[c].x>=baseptL.x && ordered[c].x<=baseptR.x)){
	      basedist = dist;
	      markerID = c;
	    }
	  }
	    
	  if(r_or_l){ //if right hand
		if(markerID>=0 && markerID<=ordered.size() && ordered.size()>0){
		    scndfinger = ordered[markerID];
		    assigned_2nd = true;
		    if((markerID-1)>=0){fstfinger = ordered[markerID-1]; assigned_1st=true;}
		    if((markerID-2)>=0){thumb = ordered[markerID-2]; assigned_thu=true;}
		    if((markerID+1)<ordered.size()){thrdfinger = ordered[markerID+1]; assigned_3rd=true;}
		    if((markerID+2)<ordered.size()){pinkie = ordered[markerID+2]; assigned_pink=true;}
		}
	  }
	  else{ //if left hand
	      if(markerID>=0 && markerID<=ordered.size() && ordered.size()>0){
		    scndfinger = ordered[markerID];
		    assigned_2nd = true;
		    if((markerID-1)>=0){thrdfinger = ordered[markerID-1]; assigned_3rd=true;}
		    if((markerID-2)>=0){pinkie = ordered[markerID-2];assigned_pink = true;}
		    if((markerID+1)<ordered.size()){fstfinger = ordered[markerID+1];assigned_2nd = true;}
		    if((markerID+2)<ordered.size()){thumb = ordered[markerID+2]; assigned_thu=true;}
	      }
	  } 
	}
	else{ //if markers have been set before in a previous frame
	//otherwise, assess distance of previous position of each finger to marker, and assign based on smallest distance
	      vector<double> dist_ps;
	      vector<double> dist_3rds;
	      vector<double> dist_2nds;
	      vector<double> dist_1sts;
	      vector<double> dist_thus;
	      
	      int many = ordered.size();
	      //calculate distances between previous marker value, and the new detected blobs in ordered vector
	      for(int thismany=0; thismany<many; thismany++){
		      
		      dist_ps.push_back(distance(pinkie,ordered[thismany]));
		      dist_3rds.push_back(distance(thrdfinger,ordered[thismany]));
		      dist_2nds.push_back(distance(scndfinger,ordered[thismany]));
		      dist_1sts.push_back(distance(fstfinger,ordered[thismany]));
		      dist_thus.push_back(distance(thumb,ordered[thismany]));
	      }
	      //for each hand point (not all detected markers should be assumed to be hand points)
	      //find min of each vector, check that it is less than a standard distance of pixels, is not more in value than corresponding elements of other vectors, and the point hasnt already been assigned. If all these conditions are met, assign point.
	      double top_dist = 20.0;
	      int whichfinger = 60;
	      
	      double min_ps = 40.0; //this is arbitrary just to make sure it is more than top_dist
	      if(dist_ps.size()>1){min_ps = *std::min_element(dist_ps.begin(), dist_ps.end());}
	      else if (dist_ps.size()==1){min_ps = dist_ps[0];}
	      
	      if(min_ps<top_dist){
		  int min_ps_loc = std::distance(dist_ps.begin(), min_element(dist_ps.begin(), dist_ps.end()));
		  if(min_ps<dist_3rds[min_ps_loc] && min_ps<dist_2nds[min_ps_loc] && min_ps<dist_1sts[min_ps_loc] && min_ps<dist_thus[min_ps_loc]){
			//assign point
			pinkie = ordered[min_ps_loc];	
			assigned_pink = true;
		  }
	      }
	      
	      double min_3rds = 40.0; //this is arbitrary just to make sure it is more than top_dist
	      if(dist_3rds.size()>1){min_3rds = *std::min_element(dist_3rds.begin(), dist_3rds.end());}
	      else if (dist_3rds.size()==1){min_3rds = dist_3rds[0];}
	      
	      if(min_3rds<top_dist){
		  int min_3rds_loc = std::distance(dist_3rds.begin(), min_element(dist_3rds.begin(), dist_3rds.end()));
		  if(min_3rds<dist_ps[min_3rds_loc] && min_3rds<dist_2nds[min_3rds_loc] && min_3rds<dist_1sts[min_3rds_loc] && min_3rds<dist_thus[min_3rds_loc]){
			//assign point
			thrdfinger = ordered[min_3rds_loc];	
			assigned_3rd = true;
		  }
	      }

	      double min_2nds = 40.0; //this is arbitrary just to make sure it is more than top_dist
	      if(dist_2nds.size()>1){min_2nds = *std::min_element(dist_2nds.begin(), dist_2nds.end());}
	      else if (dist_2nds.size()==1){min_2nds = dist_2nds[0];}
	      
	      if(min_2nds<top_dist){
		  int min_2nds_loc = std::distance(dist_2nds.begin(), min_element(dist_2nds.begin(), dist_2nds.end()));
		  if(min_2nds<dist_ps[min_2nds_loc] && min_2nds<dist_3rds[min_2nds_loc] && min_2nds<dist_1sts[min_2nds_loc] && min_2nds<dist_thus[min_2nds_loc]){
			//assign point
			scndfinger = ordered[min_2nds_loc];	
			assigned_2nd = true;
		  }
	      }
	      
	      double min_1sts = 40.0; //this is arbitrary just to make sure it is more than top_dist
	      if(dist_1sts.size()>1){min_1sts = *std::min_element(dist_1sts.begin(), dist_1sts.end());}
	      else if (dist_1sts.size()==1){min_1sts = dist_1sts[0];}
	      
	      if(min_1sts<top_dist){
		  int min_1sts_loc = std::distance(dist_1sts.begin(), min_element(dist_1sts.begin(), dist_1sts.end()));
		  if(min_1sts<dist_ps[min_1sts_loc] && min_1sts<dist_3rds[min_1sts_loc] && min_1sts<dist_2nds[min_1sts_loc] && min_1sts<dist_thus[min_1sts_loc]){
			//assign point
			fstfinger = ordered[min_1sts_loc];	
			assigned_1st = true;
		  }
	      }
	      
	      double min_thus = 40.0; //this is arbitrary just to make sure it is more than top_dist
	      if(dist_thus.size()>1){min_thus = *std::min_element(dist_thus.begin(), dist_thus.end());}
	      else if (dist_thus.size()==1){min_thus = dist_thus[0];}
	      
	      if(min_thus<top_dist){
		  int min_thus_loc = std::distance(dist_thus.begin(), min_element(dist_thus.begin(), dist_thus.end()));
		  if(min_thus<dist_ps[min_thus_loc] && min_thus<dist_3rds[min_thus_loc] && min_thus<dist_2nds[min_thus_loc] && min_thus<dist_1sts[min_thus_loc]){
			//assign point
			thumb = ordered[min_thus_loc];	
			assigned_thu = true;
		  }
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

      //check the x coordinate order for the meta group - this should not change as it is impossible!
      if(r_or_l){ //right hand
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
      cout<<"meta set pinkie is "<<pinkie.x<<","<<pinkie.y<<endl;
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
	bool assigned_thu_b=false; bool assigned_1st_b=false; bool assigned_2nd_b=false; bool assigned_3rd_b=false; bool assigned_pink_b = false;
	if(!pinkie_b.x && !thrdfinger_b.x && !scndfinger_b.x && !fstfinger_b.x && !thumb_b.x){
	    CoorVec scnd_candidates;
	    CvPoint midbase = cvPoint(0,0);
	    double basedist = 100.0;
	    int markerID;
	    midbase.x = min(baseptL.x,baseptR.x)+abs((baseptR.x-baseptL.x)/2);
	    for (int c = 0; c<ordered.size(); c++){
	      double dist = abs(ordered[c].x-midbase.x);
	      if((!r_or_l && dist<basedist && ordered[c].x>=baseptR.x && ordered[c].x<=baseptL.x)||(r_or_l && dist<basedist && ordered[c].x>=baseptL.x && ordered[c].x<=baseptR.x)){
		basedist = dist;
		markerID = c;
	      }
	    }
	    if(r_or_l){
		  if(markerID>=0 && markerID<=ordered.size() && ordered.size()>0){
		    scndfinger_b = ordered[markerID];
		    assigned_2nd_b = true;
		    if((markerID-1)>=0){fstfinger_b = ordered[markerID-1]; assigned_1st_b=true;}
		    if((markerID-2)>=0){thumb_b = ordered[markerID-2]; assigned_thu_b=true;}
		    if((markerID+1)<ordered.size()){thrdfinger_b = ordered[markerID+1]; assigned_3rd_b=true;}
		    if((markerID+2)<ordered.size()){pinkie_b = ordered[markerID+2]; assigned_pink_b=true;}
		  }
	    }
	    else{
		if(markerID>=0 && markerID<=ordered.size() && ordered.size()>0){
		      scndfinger_b = ordered[markerID];
		      assigned_2nd_b = true;
		      if((markerID-1)>=0){thrdfinger_b = ordered[markerID-1]; assigned_3rd_b=true;}
		      if((markerID-2)>=0){pinkie_b = ordered[markerID-2];assigned_pink_b = true;}
		      if((markerID+1)<ordered.size()){fstfinger_b = ordered[markerID+1];assigned_2nd_b = true;}
		      if((markerID+2)<ordered.size()){thumb_b = ordered[markerID+2]; assigned_thu_b=true;}
		}
	    }
	  }
	  else{
	    //otherwise, assess distance of previous position of each finger to marker, and assign based on smallest distance
	    vector<double> dist_ps_b;
	    vector<double> dist_3rds_b;
	    vector<double> dist_2nds_b;
	    vector<double> dist_1sts_b;
	    vector<double> dist_thus_b;
	    
	    int many = ordered.size();
	    for(int thismany=0; thismany<many; thismany++){    
		    dist_ps_b.push_back(distance(pinkie_b,ordered[thismany]));
		    dist_3rds_b.push_back(distance(thrdfinger_b,ordered[thismany]));
		    dist_2nds_b.push_back(distance(scndfinger_b,ordered[thismany]));
		    dist_1sts_b.push_back(distance(fstfinger_b,ordered[thismany]));
		    dist_thus_b.push_back(distance(thumb_b,ordered[thismany]));
	    }
	    //for each hand point (not all detected markers should be assumed to be hand points)
	    //find min of each vector, check that it is less than a standard distance of pixels, is not more in value than corresponding elements of other vectors, and the point hasnt already been assigned. If all these conditions are met, assign point.
	    double top_dist = 20.0;
	    int whichfinger = 60;
	    
	    double min_ps_b = 40.0; //this is arbitrary just to make sure it is more than top_dist
	    if(dist_ps_b.size()>1){min_ps_b = *std::min_element(dist_ps_b.begin(), dist_ps_b.end());}
	    else if (dist_ps_b.size()==1){min_ps_b = dist_ps_b[0];}
	    
	    if(min_ps_b<top_dist){
		int min_ps_b_loc = std::distance(dist_ps_b.begin(), min_element(dist_ps_b.begin(), dist_ps_b.end()));
		if(min_ps_b<dist_3rds_b[min_ps_b_loc] && min_ps_b<dist_2nds_b[min_ps_b_loc] && min_ps_b<dist_1sts_b[min_ps_b_loc] && min_ps_b<dist_thus_b[min_ps_b_loc]){
		      //assign point
		      pinkie_b = ordered[min_ps_b_loc];	
		      assigned_pink_b = true;
		}
	    }
	    
	    double min_3rds_b = 40.0; //this is arbitrary just to make sure it is more than top_dist
	    if(dist_3rds_b.size()>1){min_3rds_b = *std::min_element(dist_3rds_b.begin(), dist_3rds_b.end());}
	    else if (dist_3rds_b.size()==1){min_3rds_b = dist_3rds_b[0];}
	    
	    if(min_3rds_b<top_dist){
		int min_3rds_b_loc = std::distance(dist_3rds_b.begin(), min_element(dist_3rds_b.begin(), dist_3rds_b.end()));
		if(min_3rds_b<dist_ps_b[min_3rds_b_loc] && min_3rds_b<dist_2nds_b[min_3rds_b_loc] && min_3rds_b<dist_1sts_b[min_3rds_b_loc] && min_3rds_b<dist_thus_b[min_3rds_b_loc]){
		      //assign point
		      thrdfinger_b = ordered[min_3rds_b_loc];	
		      assigned_3rd_b = true;
		}
	    }

	    double min_2nds_b = 40.0; //this is arbitrary just to make sure it is more than top_dist
	    if(dist_2nds_b.size()>1){min_2nds_b = *std::min_element(dist_2nds_b.begin(), dist_2nds_b.end());}
	    else if (dist_2nds_b.size()==1){min_2nds_b = dist_2nds_b[0];}
	    
	    if(min_2nds_b<top_dist){
		int min_2nds_b_loc = std::distance(dist_2nds_b.begin(), min_element(dist_2nds_b.begin(), dist_2nds_b.end()));
		if(min_2nds_b<dist_ps_b[min_2nds_b_loc] && min_2nds_b<dist_3rds_b[min_2nds_b_loc] && min_2nds_b<dist_1sts_b[min_2nds_b_loc] && min_2nds_b<dist_thus_b[min_2nds_b_loc]){
		      //assign point
		      scndfinger_b = ordered[min_2nds_b_loc];	
		      assigned_2nd_b = true;
		}
	    }
	    
	    double min_1sts_b = 40.0; //this is arbitrary just to make sure it is more than top_dist
	    if(dist_1sts_b.size()>1){min_1sts_b = *std::min_element(dist_1sts_b.begin(), dist_1sts_b.end());}
	    else if (dist_1sts_b.size()==1){min_1sts_b = dist_1sts_b[0];}
	    
	    if(min_1sts_b<top_dist){
		int min_1sts_b_loc = std::distance(dist_1sts_b.begin(), min_element(dist_1sts_b.begin(), dist_1sts_b.end()));
		if(min_1sts_b<dist_ps_b[min_1sts_b_loc] && min_1sts_b<dist_3rds_b[min_1sts_b_loc] && min_1sts_b<dist_2nds_b[min_1sts_b_loc] && min_1sts_b<dist_thus_b[min_1sts_b_loc]){
		      //assign point
		      fstfinger_b = ordered[min_1sts_b_loc];	
		      assigned_1st_b = true;
		}
	    }
	    
	    double min_thus_b = 40.0; //this is arbitrary just to make sure it is more than top_dist
	    if(dist_thus_b.size()>1){min_thus_b = *std::min_element(dist_thus_b.begin(), dist_thus_b.end());}
	    else if (dist_thus_b.size()==1){min_thus_b = dist_thus_b[0];}
	    
	    if(min_thus_b<top_dist){
		int min_thus_b_loc = std::distance(dist_thus_b.begin(), min_element(dist_thus_b.begin(), dist_thus_b.end()));
		if(min_thus_b<dist_ps_b[min_thus_b_loc] && min_thus_b<dist_3rds_b[min_thus_b_loc] && min_thus_b<dist_2nds_b[min_thus_b_loc] && min_thus_b<dist_1sts_b[min_thus_b_loc]){
		      //assign point
		      thumb_b = ordered[min_thus_b_loc];	
		      assigned_thu_b = true;
		}
	    }
	    
    }		//determine which are missing....
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
      cout.flush();

      //check the x coordinate order for the proxal group
      if(r_or_l){ //right hand
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
      else{  //left hand
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
	bool assigned_1st_c = false; bool assigned_2nd_c=false; bool assigned_3rd_c=false; bool assigned_pink_c = false;
	int many_c = ordered.size();
	
	if(!pinkie_c.x && !thrdfinger_c.x && !scndfinger_c.x && !fstfinger_c.x){
		    CoorVec scnd_candidates;
		    CvPoint midbase = cvPoint(0,0);
		    double basedist = 100.0;
		    int markerID;
		    midbase.x = min(baseptL.x,baseptR.x)+abs((baseptR.x-baseptL.x)/2);
		    for (int c = 0; c<ordered.size(); c++){
		      double dist = abs(ordered[c].x-midbase.x);
		      if((!r_or_l && dist<basedist && ordered[c].x>=baseptR.x && ordered[c].x<=baseptL.x)||(r_or_l && dist<basedist && ordered[c].x>=baseptL.x && ordered[c].x<=baseptR.x)){
			basedist = dist;
			markerID = c;
		      }
		    }
		    if(r_or_l){
			  if(markerID>=0 && markerID<=ordered.size() && ordered.size()>0){
			      scndfinger_c = ordered[markerID];
			      assigned_2nd_c = true;
			      if((markerID-1)>=0){fstfinger_c = ordered[markerID-1]; assigned_1st_c=true;}
			      if((markerID+1)<ordered.size()){thrdfinger_c = ordered[markerID+1]; assigned_3rd_c=true;}
			      if((markerID+2)<ordered.size()){pinkie_c = ordered[markerID+2]; assigned_pink_c=true;}
			  }
			  //if not been able to locate thrd point, use distances from x coordinate of bases to set the points, even if wrong
		    }
		    else{
			    if(markerID>=0 && markerID<=ordered.size() && ordered.size()>0){
				  scndfinger_c = ordered[markerID];
				  assigned_2nd_c = true;
				  cout<< "2nddist";
				  if((markerID-1)>=0){thrdfinger_c = ordered[markerID-1]; assigned_3rd_c=true; cout<<"3rddist\t";}
				  if((markerID-2)>=0){pinkie_c = ordered[markerID-2];assigned_pink_c = true; cout<<"pinkiedist\t";}
				  if((markerID+1)<ordered.size()){fstfinger_c = ordered[markerID+1];assigned_1st_c = true; cout<<"1stdist\t";}
			    }
			    //if not been able to locate thrd point see above
		    }
		    //if not been able to locate thrd point, use distances from x coordinate of bases to set the points, even if wrong
	}
	else{
	//otherwise, assess distance of previous position of each finger to marker, and assign based on smallest distance
		vector<double> dist_ps_c;
		vector<double> dist_3rds_c;
		vector<double> dist_2nds_c;
		vector<double> dist_1sts_c;
		
		int many = ordered.size();
		for(int thismany=0; thismany<many; thismany++){		
			dist_ps_c.push_back(distance(pinkie_c,ordered[thismany]));
			dist_3rds_c.push_back(distance(thrdfinger_c,ordered[thismany]));
			dist_2nds_c.push_back(distance(scndfinger_c,ordered[thismany]));
			dist_1sts_c.push_back(distance(fstfinger_c,ordered[thismany]));
		}
		//for each hand point (not all detected markers should be assumed to be hand points)
		//find min of each vector, check that it is less than a standard distance of pixels, is not more in value than corresponding elements of other vectors, and the point hasnt already been assigned. If all these conditions are met, assign point.
		double top_dist = 20.0;
		int whichfinger = 60;
		
		double min_ps_c = 40.0; //this is arbitrary just to make sure it is more than top_dist
		if(dist_ps_c.size()>1){min_ps_c = *std::min_element(dist_ps_c.begin(), dist_ps_c.end());}
		else if (dist_ps_c.size()==1){min_ps_c = dist_ps_c[0];}
		
		if(min_ps_c<top_dist){
		    int min_ps_c_loc = std::distance(dist_ps_c.begin(), min_element(dist_ps_c.begin(), dist_ps_c.end()));
		    if(min_ps_c<dist_3rds_c[min_ps_c_loc] && min_ps_c<dist_2nds_c[min_ps_c_loc] && min_ps_c<dist_1sts_c[min_ps_c_loc]){
			  //assign point
			  pinkie_c = ordered[min_ps_c_loc];	
			  assigned_pink_c = true;
		    }
		}
		
		double min_3rds_c = 40.0; //this is arbitrary just to make sure it is more than top_dist
		if(dist_3rds_c.size()>1){min_3rds_c = *std::min_element(dist_3rds_c.begin(), dist_3rds_c.end());}
		else if (dist_3rds_c.size()==1){min_3rds_c = dist_3rds_c[0];}
		
		if(min_3rds_c<top_dist){
		    int min_3rds_c_loc = std::distance(dist_3rds_c.begin(), min_element(dist_3rds_c.begin(), dist_3rds_c.end()));
		    if(min_3rds_c<dist_ps_c[min_3rds_c_loc] && min_3rds_c<dist_2nds_c[min_3rds_c_loc] && min_3rds_c<dist_1sts_c[min_3rds_c_loc]){
			  //assign point
			  thrdfinger_c = ordered[min_3rds_c_loc];	
			  assigned_3rd_c = true;
		    }
		}

		double min_2nds_c = 40.0; //this is arbitrary just to make sure it is more than top_dist
		if(dist_2nds_c.size()>1){min_2nds_c = *std::min_element(dist_2nds_c.begin(), dist_2nds_c.end());}
		else if (dist_2nds_c.size()==1){min_2nds_c = dist_2nds_c[0];}
		
		if(min_2nds_c<top_dist){
		    int min_2nds_c_loc = std::distance(dist_2nds_c.begin(), min_element(dist_2nds_c.begin(), dist_2nds_c.end()));
		    if(min_2nds_c<dist_ps_c[min_2nds_c_loc] && min_2nds_c<dist_3rds_c[min_2nds_c_loc] && min_2nds_c<dist_1sts_c[min_2nds_c_loc]){
			  //assign point
			  scndfinger_c = ordered[min_2nds_c_loc];	
			  assigned_2nd_c = true;
		    }
		}
		
		double min_1sts_c = 40.0; //this is arbitrary just to make sure it is more than top_dist
		if(dist_1sts_c.size()>1){min_1sts_c = *std::min_element(dist_1sts_c.begin(), dist_1sts_c.end());}
		else if (dist_1sts_c.size()==1){min_1sts_c = dist_1sts_c[0];}
		
		if(min_1sts_c<top_dist){
		    int min_1sts_c_loc = std::distance(dist_1sts_c.begin(), min_element(dist_1sts_c.begin(), dist_1sts_c.end()));
		    if(min_1sts_c<dist_ps_c[min_1sts_c_loc] && min_1sts_c<dist_3rds_c[min_1sts_c_loc] && min_1sts_c<dist_2nds_c[min_1sts_c_loc]){
			  //assign point
			  fstfinger_c = ordered[min_1sts_c_loc];	
			  assigned_1st_c = true;
		    }
		}
		
	}	//determine which are missing....
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
		

	//check the x coordinate order for the distal group
	if(r_or_l){ //right hand
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
	else{  //left hand
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
	for (int num = 0; num<newsize;num++){	//calculate all distances between fingers and elements of ordered. smallest distance is the winner!
		double fingerdist = distance(finger,list[num]);
		if(fingerdist<topdist){
			topdist=fingerdist;
			no_p = no;
			number=num;
		}
		no++;
	}
	if(number<(newsize+1)){
	    finger = list[number];
	    list.erase(no_p);
	    return (true);
	}
	else{
	return (false);
	}

}

void Fullhand::recordGroupDistances(void){
    //calculate maximum and minimum distances between each point in the three groups and the virtual base point.
    CvPoint midbase = cvPoint(0,0);
    midbase.x = min(baseptL.x,baseptR.x)+abs((baseptR.x-baseptL.x)/2);
    midbase.y = min(baseptL.y,baseptR.y)+abs((baseptR.y-baseptL.y)/2);
    //meta
    vector <double> meta_distances;
    if(thumb.x!=0 &&thumb.y!=0){meta_distances.push_back(distance(thumb,midbase));}
    if(fstfinger.x!=0 && fstfinger.y!=0){meta_distances.push_back(distance(fstfinger,midbase));}
    if(scndfinger.x!=0 && scndfinger.y!=0){meta_distances.push_back(distance(scndfinger,midbase));}
    if(thrdfinger.x!=0 && thrdfinger.y!=0){meta_distances.push_back(distance(thrdfinger,midbase));}
    if(pinkie.x!=0 && pinkie.y!=0){meta_distances.push_back(distance(pinkie,midbase));}
    if (meta_distances.size()>1){
	std::vector <double>::iterator vecIt = meta_distances.begin();   
	std::vector <double>::iterator vecIt_end = meta_distances.end();
	double meta_minvalue = *std::min_element(vecIt, vecIt_end);
	double meta_maxvalue = *std::max_element(vecIt, vecIt_end);
	meta_mindistance = meta_minvalue - (0.1*meta_minvalue);
	meta_maxdistance = meta_maxvalue + (0.1*meta_maxvalue);
	cout<<"META maxdistance = "<<meta_maxvalue<<","<<meta_distances.size()<<endl;
	cout<<"META mindistance = "<<meta_minvalue<<","<<meta_distances.size()<<endl;
    }
    else {meta_mindistance = 0.0; meta_maxdistance=0.0;}
    
    //proxal
    vector <double> prox_distances;
    if(thumb_b.x!=0 && thumb_b.y!=0){double d = distance(thumb_b,midbase); prox_distances.push_back(d); }
    if(fstfinger_b.x!=0 && fstfinger_b.y!=0){double d = distance(fstfinger_b,midbase); prox_distances.push_back(d); }
    if(scndfinger_b.x!=0 && scndfinger_b.y!=0){double d = distance(scndfinger_b,midbase); prox_distances.push_back(d);}
    if(thrdfinger_b.x!=0 && thrdfinger_b.y!=0){double d = distance(thrdfinger_b,midbase); prox_distances.push_back(d); }
    if(pinkie_b.x!=0 && pinkie_b.y!=0){double d = distance(pinkie_b,midbase); prox_distances.push_back(d); }
    if (prox_distances.size()>1){
	std::vector <double>::iterator vecIt_prox = prox_distances.begin();   
	std::vector <double>::iterator vecIt_end_prox = prox_distances.end();
	double prox_maxvalue = *std::max_element(vecIt_prox, vecIt_end_prox);
	cout<<"PROX maxdistance = "<<prox_maxvalue<<","<<prox_distances.size()<<endl;
	prox_maxdistance = prox_maxvalue + (0.1*prox_maxvalue);
    }
    else{prox_maxdistance=0.0;}
    //distal
    vector <double> dist_distances;
    if(fstfinger_c.x!=0 && fstfinger_c.y!=0){double d = distance(fstfinger_c,midbase); dist_distances.push_back(d);}
    if(scndfinger_c.x!=0 && scndfinger_c.y!=0){double d = distance(scndfinger_c,midbase); dist_distances.push_back(d);}
    if(thrdfinger_c.x!=0 && thrdfinger_c.y!=0){double d = distance(thrdfinger_c,midbase); dist_distances.push_back(d);}
    if(pinkie_c.x!=0 && pinkie_c.y!=0){double d = distance(pinkie_c,midbase);prox_distances.push_back(d); }
    if (dist_distances.size()>1){
	std::vector <double>::iterator vecIt_dist = dist_distances.begin();   
	std::vector <double>::iterator vecIt_end_dist = dist_distances.end();
	double dist_maxvalue = *std::max_element(vecIt_dist, vecIt_end_dist);
	cout<<"DIST maxdistance = "<<dist_maxvalue<<","<<dist_distances.size()<<endl;
	dist_maxdistance = dist_maxvalue + (0.1*dist_maxvalue);
    }
    else{dist_maxdistance = 0.0;}
    
    cout<<"Calculated Distances are:"<<meta_mindistance<<","<<meta_maxdistance<<","<<prox_maxdistance<<","<<dist_maxdistance<<endl;
}
 
void Fullhand::checkgroups(CoorVec &proxal, CoorVec &distal, int width){

	/*go through the x ordered vectors and for each pair of elements, check angles corresponding to basept. when checking proxal vector,if angles too similar, delete element with larger distance from basept and place it in distal vector and rearrange for x order. same goes vice versa. at the end of function rearrange both for xorder.*/
	CoorVec copy_proxal;
	CoorVec copy_distal;
	copy_proxal.assign(proxal.begin(),proxal.end());
	copy_distal.assign(distal.begin(),distal.end());
	
	//proxal vector checking
	CvPoint p = cvPoint(0,0);
	CvPoint q = cvPoint(0,0);
	int proxsize = copy_proxal.size();
	cout<<"Checkgroups: Proxsize is "<<proxsize<<endl;
	CoorVecIt proxit = copy_proxal.begin();
	CoorVecIt deleteprox = copy_proxal.begin();
	for (int points = 1; points<proxsize;points++){
			//calculate angles of copy_proxal[points] and [points-1]
			double angle1 = angle(copy_proxal[points-1], baseptL);
			double angle2 = angle(copy_proxal[points],baseptL);
			int diff = (int)angle1-angle2;
			diff = abs(diff);
			cout<<"Angle difference of elements "<<points-1<<":"<<angle1<<" and"<<points<<":"<<angle2<<" is "<<diff<<endl;
			//if angle1-angle2 < 5 then the furthest away point belongs to the distal vector - these vectors are of points ordered by x position : two next to each other could be distal and proxal as long as the angles between the two arent massive.
			if(diff<10){
				double distance1 = distance(copy_proxal[points-1],baseptL);
				double distance2 = distance(copy_proxal[points],baseptL);
				if(distance2>distance1){
					p = copy_proxal[points];
					q = copy_proxal[points-1];
					cout<<"keep "<<q.x<<","<<q.y<<endl;
					cout<<"rid of second "<<p.x<<","<<p.y<<endl;
					copy_distal.push_back(p);
					proxit++;
					deleteprox = proxit;
					copy_proxal.erase(deleteprox);
					proxsize--;
					proxit--;
					cout<<"proxsize is "<<proxsize<<endl;
					points = points-1;
				}else{
					p = copy_proxal[points-1];
					q = copy_proxal[points];
					cout<<"keep "<<q.x<<","<<q.y<<endl;
					cout<<"rid of first "<<p.x<<","<<p.y<<endl;
					copy_distal.push_back(p);
					deleteprox = proxit;
					copy_proxal.erase(deleteprox);
					proxsize--;
					cout<<"proxsize is "<<proxsize<<endl;
					points = points-1;
					if(points==0){points++;proxit=copy_proxal.begin();}
				}
			}
			else{proxit++;}	
	}

	//reorder for x the distal vector with any new elements
	cout<<"Second Organise Distal Checkgroups:"<<r_or_l<<":";
	CoorVec newdistal_copy = Organise::order_x(copy_distal,width,r_or_l);

	//distal vector checking
	CvPoint d = cvPoint(0,0);
	int distsize = newdistal_copy.size();
	CoorVecIt distit = newdistal_copy.begin();
	CoorVecIt deletedist = newdistal_copy.begin();
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
					cout<< (*distit).x <<","<< (*distit).y <<endl;
					distit++;
					deletedist = distit;
					cout<< (*deletedist).x <<","<< (*deletedist).y <<endl;
					newdistal_copy.erase(deletedist);
					distsize--;
					distit--;
					cout<< (*distit).x <<","<< (*distit).y <<endl;
					d_points = d_points-1;
				}else{
					d = newdistal_copy[d_points-1];
					copy_proxal.push_back(d);
					deletedist = distit;
					newdistal_copy.erase(deletedist);
					distsize--;
					d_points=d_points-1;
					if(d_points==0){d_points++;distit=newdistal_copy.begin();}
					
				}
			}
			else{distit++;}	
	}
// 	//reorder both proxal and distal vectors for x
	cout<<"Third Organise x Proxal Checkgroups:"<<r_or_l<<":";
 	CoorVec newprox = Organise::order_x(copy_proxal,width,r_or_l);
	cout<<"Third Organise x Distal Checkgroups:"<<r_or_l<<":";
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

