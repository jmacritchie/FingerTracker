/* Finger Tracking Software for Pianists' Finger Movements 
Also includes a user verification program which allows user to change recorded positions by clicking on the screen.

Ver1.0 includes use of correlation to have better tracking of the base point - a huge problem before. 
Ver 2.0 uses two different colours for segmentation of left and right hands. This eliminates the need for the k-means clustering steps but adds in some more thresholding functions.
Ver 3.0 changes the base markers on the hands to two base markers. New implementations of the fullhand modeal are included.
Ver 4.0 uses the cvblob library and updated opencv functions

*/
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <vector>
#include <math.h>
#include <getopt.h>

#include "blobby.h"
#include "correlation.h"
#include "fullhand.h"
#include "transform.h"
#include "gmswriter_fixed.h"
using namespace std;
using namespace cv;

#define DEPTH_8_BIT 8
#define DEPTH_32_FLOAT IPL_DEPTH_32F
#define CHANNELS_1 1
#define CHANNELS_3 3
#define HAND_MARKERS 16

#define draw_cross( centre, color, d )                                 \
                cvLine( thresh_r, cvPoint( centre.x - d, centre.y - d ),                \
                             cvPoint( centre.x + d, centre.y + d ), color, 1, 0 ); \
                cvLine( thresh_r, cvPoint( centre.x + d, centre.y - d ),                \
                             cvPoint( centre.x - d, centre.y + d ), color, 1, 0 );

#define draw_circle( image,point,color,radius)				\
		cvCircle (image, cvPoint(point.x,point.y), radius, color, 1);


//2D vector created to store point with present coordinate at element [0] and history of coordinates stored up to 50 frames
//vector<CvPoint>points;

typedef std::vector <CvPoint> CoorVec;
typedef std::vector <CvPoint>::iterator CoorVecIt;
typedef std::vector <double> DistVec;
typedef std::vector <IplImage*> ImageVec;


CvPoint marker;
int add_point = 0;
Fullhand lh(false); //false bool argument declares the hand is left
Fullhand rh(true); //true bool argument declares the hand is right
CoorVec c_block;
IplImage *correctimg = NULL;
IplImage *prev_frame = NULL;
int image_width = 1024;
int image_height = 268;

/****************************************************************
VARIABLE WHICHAND:
		0 = LEFT
		1 = RIGHT
VARIABLE r_or_l:
		0 = FALSE = LEFT
		1 = TRUE = RIGHT
*****************************************************************/

//defining action for when a mouse event is detected. In this case, we want to store the x and y coordinates
void on_mouse( int event, int x, int y, int flags, void* param )
{
	switch( event ){
   	 case CV_EVENT_LBUTTONDOWN:
			marker = cvPoint(x,y);
			c_block.push_back(marker);
			add_point = 1;
			cout<< "point is at "<<x<<","<<y<<endl;
			break;
	}
}

//function to store corrected assignment of one marker
void correct_onept(int whichhand, char whichgroup, char whichpoint, IplImage *bckgnd)
{
	cout<<"Please click on the point in its correct position"<<endl;
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	cvWaitKey(0);
	if(add_point){
		if(whichhand)
			{
			switch(whichpoint){
			    case 'a':
				    if(whichgroup=='m'){rh.setthumb(marker);}
				    if(whichgroup=='p'){rh.setthumb_b(marker);}
				    break;
			    case 'b':
				    if(whichgroup=='m'){rh.set1stfing(marker);}
				    if(whichgroup=='p'){rh.set1stfing_b (marker);}
				    if(whichgroup=='d'){rh.set1stfing_c (marker);}
				    break;
			    case 'c':
				    if(whichgroup=='m'){rh.set2ndfing (marker);}
				    if(whichgroup=='p'){rh.set2ndfing_b (marker);}
				    if(whichgroup=='d'){rh.set2ndfing_c (marker);}
				    break;
			    case 'd':
				    if(whichgroup=='m'){rh.set3rdfing (marker);}
				    if(whichgroup=='p'){rh.set3rdfing_b (marker);}
				    if(whichgroup=='d'){rh.set3rdfing_c (marker);}
				    break;
			    case 'e':
				    if(whichgroup=='m'){rh.setpinkie (marker);}
				    if(whichgroup=='p'){rh.setpinkie_b (marker);}
				    if(whichgroup=='d'){rh.setpinkie_c (marker);}
				    break;
			    default:break;	
			}//end switch
		}
		else{
			switch(whichpoint){
			    case 'a':
				    if(whichgroup=='m'){lh.setthumb(marker);}
				    if(whichgroup=='p'){lh.setthumb_b(marker);}
				    break;
			    case 'b':
				    if(whichgroup=='m'){lh.set1stfing(marker);}
				    if(whichgroup=='p'){lh.set1stfing_b (marker);}
				    if(whichgroup=='d'){lh.set1stfing_c (marker);}
				    break;
			    case 'c':
				    if(whichgroup=='m'){lh.set2ndfing (marker);}
				    if(whichgroup=='p'){lh.set2ndfing_b (marker);}
				    if(whichgroup=='d'){lh.set2ndfing_c (marker);}
				    break;
			    case 'd':
				    if(whichgroup=='m'){lh.set3rdfing (marker);}
				    if(whichgroup=='p'){lh.set3rdfing_b (marker);}
				    if(whichgroup=='d'){lh.set3rdfing_c (marker);}
				    break;
			    case 'e':
				    if(whichgroup=='m'){lh.setpinkie (marker);}
				    if(whichgroup=='p'){lh.setpinkie_b (marker);}
				    if(whichgroup=='d'){lh.setpinkie_c (marker);}
				    break;
			    default:break;	
			}//end switch
		}
	}
	CoorVecIt cstart = c_block.begin();	
	int thesize = c_block.size();
	for (int s=0; s<thesize; s++){
		c_block.erase(cstart);
	}
	//show results of changing marker assignments....update image
	IplImage *correctone = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
	IplImage *correcttwo = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
	
	correctone = lh.getfingerlines(bckgnd);
	correcttwo = rh.getfingerlines(correctone);
	cvNamedWindow("lines",1);
	cvShowImage("lines", correcttwo);
	cvWaitKey(0);
	cvDestroyWindow("lines");
}

//correcting marker assignments in the metacarpal joints
void correct_meta(int whichhand, IplImage *bckgnd){
	//depending on LH or RH will determine how marker points are assigned
	cout<<"meta being corrected "; cout.flush();
	cout<<"Please specify which finger a-e to correct or z if all\n";
	int changeall = 0;
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	int fin = cvWaitKey(0);
	switch((char)fin){
	    case 'a': correct_onept(whichhand,'m','a', bckgnd);
		    break;
	    case 'b': correct_onept(whichhand,'m','b', bckgnd);
		    break;								
	    case 'c': correct_onept(whichhand,'m','c', bckgnd);
		    break;
	    case 'd': correct_onept(whichhand,'m','d', bckgnd);
		    break;		
	    case 'e': correct_onept(whichhand,'m','e', bckgnd);
		    break;		
	    default:
		    changeall = 1;
		    break;										
	}					

	if(changeall){
	    cvWaitKey(0);
	    if(add_point){
		cout<<"HUZZAH";
		add_point = 0;
	    }
	
	    if(c_block.size()>4){
	    cout <<"Assigned "<<c_block.size()<<" points"<<endl; cout.flush();
	    //assign corrected marker assignments in correct order based on whether right or left hand
		if(whichhand){
			rh.setthumb(c_block[0]);
			rh.set1stfing(c_block[1]);
			rh.set2ndfing(c_block[2]);
			rh.set3rdfing(c_block[3]);
			rh.setpinkie(c_block[4]);
		}
		else{
			lh.setthumb(c_block[0]);
			lh.set1stfing(c_block[1]);
			lh.set2ndfing(c_block[2]);
			lh.set3rdfing(c_block[3]);
			lh.setpinkie(c_block[4]);
		}
		
	    }
	    //after all markers have been assigned, clear vector
	    CoorVecIt cstart = c_block.begin();	
	    int thesize = c_block.size();
	    for (int s=0; s<thesize; s++){
		    c_block.erase(cstart);
	   }
	    //show results of changing points....update image
	    IplImage *correctone = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
	    IplImage *correcttwo = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
	    correctone = lh.getfingerlines(bckgnd);
	    correcttwo = rh.getfingerlines(correctone);
	    cvNamedWindow("lines",1);
	    cvShowImage("lines", correcttwo);
	    cvWaitKey(0);
	    cvDestroyWindow("lines");
	    changeall = 0;
	}
}

//correcting marker assignments in the proximal joints
void correct_prox(int whichhand, IplImage *bckgnd){
	int changeall = 0;
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	cout<<"Please specify which finger a-e to correct or z if all\n";
	int fin = cvWaitKey(0);
	switch((char)fin){
	    case 'a': correct_onept(whichhand,'p','a', bckgnd);
		    break;
	    case 'b': correct_onept(whichhand,'p','b', bckgnd);
		    break;								
	    case 'c': correct_onept(whichhand,'p','c', bckgnd);
		    break;
	    case 'd': correct_onept(whichhand,'p','d', bckgnd);
		    break;		
	    case 'e': correct_onept(whichhand,'p','e', bckgnd);
		    break;		
	    default:
		    changeall = 1;
		    break;										
	}					

	if(changeall){
		cvWaitKey(0);
		if(add_point){
			cout<<"HUZZAH";
			add_point = 0;
		}
		if(c_block.size()>4){
			//assign corrected points to hand vectors...
			if(whichhand){
				rh.setthumb_b(c_block[0]);
				rh.set1stfing_b(c_block[1]);
				rh.set2ndfing_b(c_block[2]);
				rh.set3rdfing_b(c_block[3]);
				rh.setpinkie_b(c_block[4]);
			}
			else{
				lh.setthumb_b(c_block[0]);
				lh.set1stfing_b(c_block[1]);
				lh.set2ndfing_b(c_block[2]);
				lh.set3rdfing_b(c_block[3]);
				lh.setpinkie_b(c_block[4]);
			}	
		}
	
		CoorVecIt cstart = c_block.begin();	
		int thesize = c_block.size();
		for (int s=0; s<thesize; s++){
			c_block.erase(cstart);
		}
		//show results of changing points....update image
		IplImage *correctone = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
		IplImage *correcttwo = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);	
		correctone = lh.getfingerlines(bckgnd);
		correcttwo = rh.getfingerlines(correctone);
		cvNamedWindow("lines",1);
		cvShowImage("lines", correcttwo);
		cvWaitKey(0);
		cvDestroyWindow("lines");
		changeall = 0;
	}
}

//correcting marker assignments in the distal joints
void correct_dist(int whichhand, IplImage *bckgnd)
{
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	int changeall = 0;
	cout<<"Please specify which finger b-e to correct or z if all\n";
	int fin = cvWaitKey(0);
	switch((char)fin){
	    case 'b': correct_onept(whichhand,'d','b', bckgnd);
		    break;
	    case 'c': correct_onept(whichhand,'d','c', bckgnd);
		    break;								
	    case 'd': correct_onept(whichhand,'d','d', bckgnd);
		    break;
	    case 'e': correct_onept(whichhand,'d','e', bckgnd);
		    break;		
	    default:
		    changeall = 1;
		    break;										
	}					

	if(changeall){
		cvWaitKey(0);
		if(add_point){
			cout<<"HUZZAH";
			add_point = 0;
		}
		if(c_block.size()>3){
			if(whichhand){
				rh.set1stfing_c(c_block[0]);
				rh.set2ndfing_c(c_block[1]);
				rh.set3rdfing_c(c_block[2]);
				rh.setpinkie_c(c_block[3]);
			}
			else{
				lh.set1stfing_c(c_block[0]);
				lh.set2ndfing_c(c_block[1]);
				lh.set3rdfing_c(c_block[2]);
				lh.setpinkie_c(c_block[3]);
			}
		}
		CoorVecIt cstart = c_block.begin();	
		int thesize = c_block.size();
		for (int s=0; s<thesize; s++){
			c_block.erase(cstart);
		}
		//show results of changing points....update image
		IplImage *correctone = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
		IplImage *correcttwo = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
		correctone = lh.getfingerlines(bckgnd);
		correcttwo = rh.getfingerlines(correctone);
		cvNamedWindow("lines",1);
		cvShowImage("lines", correcttwo);
		cvWaitKey(0);
		cvDestroyWindow("lines");
		changeall = 0;
	}
}

//correcting marker assignments in the base points
void correct_base(int whichhand, IplImage *bckgnd)
{
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	cvWaitKey(0);
	if(add_point){
			cout<<"HUZZAH";
			add_point = 0;
	}
	if(c_block.size()>1){
			//assign corrected points to hand vectors...
			if(whichhand){
				rh.setbaseptL(c_block[0]);
				rh.setbaseptR(c_block[1]);
				cout<<"set right hand\n";
			}
			else{
				lh.setbaseptL(c_block[0]);
				lh.setbaseptR(c_block[1]);
				cout<<"set left hand\n";	
			}
	}
	CoorVecIt cstart = c_block.begin();	
	int thesize = c_block.size();
	cout << "size of c_block starts as "<<c_block.size()<<endl;
	for (int s=0; s<thesize; s++){
		c_block.erase(cstart);
	}
	//show results of changing points....update image
	IplImage *correctone = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
	IplImage *correcttwo = cvCreateImage(cvSize(image_width,image_height),DEPTH_8_BIT, CHANNELS_3);
	correctone = lh.getfingerlines(bckgnd);
	correcttwo = rh.getfingerlines(correctone);
	cout<<"got finger lines\n";
	cvNamedWindow("lines",1);
	cvShowImage("lines", correcttwo);
	cvWaitKey(0);
	cvDestroyWindow("lines");
}

void usage(){
  cout << "\nUsage:\n"
       << "\tTo track blobs in videos for two hands in a piano performance\n"
       << "\t\ttwobase <input video> <starting frame number> <gms output file> <frame offset> <output video> <background frame file> <threshold values file>\n\n"
       << "Frame offset is used in order to skip the video on until the pianists hands are in view \n Starting frame number refers to which frame number you wish to start the program from AFTER this frame offset, and will consequently refer to the number of frame the program will write to in the gms file\n\n\n"
       << "Instructions:\n"
       << "The program will present each frame onscreen, with the tracked markers and their estimated assignments into the various joint groups of each hand.\n"
       << "If you agree with all the marker assignments, press 't' for true\n"
       << "If you wish to change one or more of the marker assignments, press 'f' for false. This will take you to the editing menu where you will be asked to choose:\n"
       << "\t - which hand ('l' left hand or 'r' right hand)\n" 
       << "\t - which joint group ('m', 'p' or 'd')\n"
       << "\t - which finger ('a' for the thumb through to 'e' for the 5th finger, or 'z' for all fingers which have markers in this joint group)\n"
       << "You will be asked to click on the correct marker positions in the 'corrections' window and press Enter. Your corrections will be visualised in the 'lines' window, at which point you will need to press Enter to return to the editing menu."
       << "You will keep being returned to the editing menu until you select 'q' for quit when asked to choose a hand. Quitting will move the program onto the next frame\n"
       << "When you want to exit the program, make sure you are not in the editing menu, and press Esc. This will write the tracked information to your output gms file.\n\n";
  exit(1);
}	

int main(int argc , char** argv)
{

	if( argc < 7 ) {usage();}

	// this ofstream will store all the marker assignments in case of a crash
	ofstream outputbackup;
	outputbackup.open("backup.txt");
	
	//initialise vector for coordinates of hands
	CoorVec hands;
	CoorVec rhblobs;
	CoorVec lhblobs;
	
	DistVec lh_current; //vector to record lh distances to origin at current run
	DistVec rh_current;
	DistVec lh_next;  //vector to record lh distances to origin at next run	
	DistVec rh_next;
	vector <CoorVec> lhVec; //lh matrix for gms writing
	vector <CoorVec> rhVec; //rh matrix for gms writing
	cout<< "File will be started from frame " << argv[2] <<endl;
	// calculate the offset frame from which to start the video.
	int startframe = atoi(argv[2]);
	int clapframe = atoi(argv[4]);
	int blobframe = clapframe+startframe;
	//create memory for the hand vectors
	for(int p=0; p<HAND_MARKERS;p++){
		lhVec.push_back(CoorVec());  //loop creates 6 columns for each hand vector for gms writing
		rhVec.push_back(CoorVec());
	}
	//this correct flag monitors whether the user has agreed with all marker assignments.
	bool correct = false;	
	// check video capture
	IplImage tmp_frame;
	IplImage* vid = 0; 
	VideoCapture cap(argv[1]);
	if(!cap.isOpened()) {
	  cout << "bad video"<<endl;
		exit(0);
	}
	
	int frameH    = (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	int frameW    = (int) cap.get( CV_CAP_PROP_FRAME_WIDTH);
	int fps = (int) cap.get(CV_CAP_PROP_FPS);
	//creating output video writer
	CvVideoWriter *writer = 0;
	int isColor = 1;
	writer=cvCreateVideoWriter(argv[5],CV_FOURCC('D','I','V','X'),
				fps,cvSize(frameW,frameH),isColor);



	//creating and initialising IplImages
	IplImage *channels = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT,CHANNELS_3);
	IplImage *twothresh = cvCreateImage( cvSize(frameW,frameH), DEPTH_8_BIT, CHANNELS_1 );

	IplImage *separated = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *one_hand = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *two_hands = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *est = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *prev_frame = NULL;
	
	//load background image to subtract
	IplImage *background;
	if((background=cvLoadImage(argv[6]))==NULL)
		printf( "error loading file \n");
	IplImage * hsvimg_background=cvCreateImage(cvGetSize(background),8, 3);
	cvCvtColor(background, hsvimg_background, CV_BGR2HSV);
	
	int width =  frameW; 
	int height = frameH; 
	int HRed, HRedmax, SRed, SRedmax, VRed, VRedmax, HGreen, HGreenmax, SGreen, SGreenmax, VGreen, VGreenmax;

	//retrieve thresholding values:
	FileStorage fs(argv[7], FileStorage::READ);

	fs["HRed"] >> HRed ;
	fs["HRedmax"] >> HRedmax;
	fs["SRed"] >> SRed ;
	fs["SRedmax"] >> SRedmax;
	fs["VRed"] >> VRed ;
	fs["VRedmax"] >> VRedmax;
	
	fs["HGreen"] >> HGreen ;
	fs["HGreenmax"] >> HGreenmax;
	fs["SGreen"] >> SGreen ;
	fs["SGreenmax"] >> SGreenmax;
	fs["VGreen"] >> VGreen ;
	fs["VGreenmax"] >> VGreenmax;
  
	fs.release();
	int fr = 1;
	Mat tmp_mat;

	//Enter frame processing loop. 'Esc' exits the program and saves the data via gms file.
	while(1) {
		cap >> tmp_mat;
		tmp_frame = tmp_mat;
		if(fr>blobframe){		
		  cvNamedWindow("hands",1);
		  cvNamedWindow("video",1);
		  cout<<"frame "<<fr<<endl;
		  //set up the vectors of coordinates for each hand
		  CoorVec lhfingers_current;
		  CoorVec rhfingers_current;	
		  if(tmp_mat.empty()){cout<<"MISSING FRAME"<<endl;}
		  IplImage *channels = cvCloneImage(&tmp_frame);
		  if (fr>(blobframe+1)){//calculating distances etc from hands calculated on previous frame 
			  lhfingers_current = lh.setpoints_wrt_org();
			  rhfingers_current = rh.setpoints_wrt_org();
			  lh_current = lh.distances_to_org(lhfingers_current);
			  rh_current = rh.distances_to_org(rhfingers_current);
		  }
		  //creating and initialising IplImages and cvMats
		  Mat background_mat(background);
		  Mat loadimg_mat(channels);
		  Mat loadimg_matHSV;
		  Mat background_matHSV;
		  Mat diff;
		  Mat diff_image;
		  IplImage *hsvimg=cvCreateImage(cvGetSize(channels),8, 3);
		  Mat diff_mask;
		  // conduct background subtraction using the input background frame. Above a certain threshold of difference, keep the pixels.
		  if (background_mat.channels()== loadimg_mat.channels() && background_mat.rows==loadimg_mat.rows && background_mat.cols==loadimg_mat.cols){
		    absdiff(loadimg_mat, background_mat, diff);
		    diff_mask = diff > 15; // make diff image a mask by thresholding for values above 0
		    loadimg_mat.copyTo(diff_image, diff_mask);
		  }
		  
		  IplImage diff_imgIPL = diff_image;
		  cvCvtColor(&diff_imgIPL, hsvimg, CV_BGR2HSV);
		  
		  //threshold images based on the thresholding values for the two colours (red and green in this case)
		  IplImage* imgThreshGreen=cvCreateImage(cvGetSize(hsvimg),IPL_DEPTH_8U, 1);
		  cvInRangeS (hsvimg, cvScalar(HGreen,SGreen,VGreen), cvScalar(HGreenmax,SGreenmax,VGreenmax), imgThreshGreen); 
		  IplImage* imgThreshRed=cvCreateImage(cvGetSize(hsvimg),IPL_DEPTH_8U, 1);
		  cvInRangeS (hsvimg, cvScalar (HRed,SRed,VRed), cvScalar(HRedmax,SRedmax,VRedmax), imgThreshRed); 
		  
		  //run blob detection on each thresholded image
		  int numblobs_lh, numblobs_rh=0;
		  rhblobs = Blobby::detect(channels,imgThreshGreen,rhblobs,numblobs_rh);
		  lhblobs = Blobby::detect(channels,imgThreshRed,lhblobs,numblobs_lh);
		  
		  if(!prev_frame){ prev_frame = cvCloneImage(&tmp_frame);}
		  IplImage *bgnd = cvCloneImage(&tmp_frame);
		  //assign base points of each hand object from the detected blobs
		  lh.setallbases(lhblobs,height, prev_frame, &tmp_frame, 0);
		  rh.setallbases(rhblobs,height, prev_frame, &tmp_frame, 1);
	  
		  //assign the rest of the fingers from the remaining detected blobs
		  lh.setfingers(lhblobs, 0, width,height);
		  rh.setfingers(rhblobs, 1, width,height);

		  //drawing the marker crosses and lines between each joint group on the output image	
		  one_hand = lh.getfingerlines(bgnd);
		  two_hands = rh.getfingerlines(one_hand);
		  cvShowImage("hands",two_hands);
		
		  //TODO: this loop will use the transform functions in transform.cpp to calculate the affine transforms or each pointer from the previous frame. Using this information, estimations will be made for any unassigned markers.  
		if(fr>(blobframe+1)){
			/*CoorVec lhfingers_next;
			CoorVec rhfingers_next;
			lhfingers_next = lh.setpoints_wrt_org();
			rhfingers_next = rh.setpoints_wrt_org();
			lh_next = lh.distances_to_org(lhfingers_next);
			rh_next = rh.distances_to_org(rhfingers_next);
			//now we have a current and next set of distances, the transformations between each point between two images can be calculated
			Transform lhtransform(lhfingers_current,lh_current,lhfingers_next,lh_next, lh );
			Transform rhtransform(rhfingers_current,rh_current,rhfingers_next,rh_next, rh);
			//est = lh.getestimations(two_hands); 
			//final = rh.getestimations(est);	

			lhfingers_current.clear();
			rhfingers_current.clear();
			lhfingers_next.clear();
			rhfingers_next.clear();	*/	
		}
			  
		cvShowImage("hands",two_hands); 
		int k = cvWaitKey(0); // wait for user to confirm or contest marker assignments
		if( (char)k == 27 ) break;
		switch( (char) k) {
				case 'f':
				  correct = false;
				  break;
				case 't':
				  correct = true;
				  break;		  
		} //endswitch
		
		while(!correct){ //move to Editing Menu. Will keep inside this menu until user presses 'q' for quit
			IplImage *correctimg = cvCloneImage(&tmp_frame);
			//create a new image showing correcting points.
			cvNamedWindow("corrections",1);
			cvMoveWindow("corrections", 0,300);
			cvShowImage("corrections",correctimg);

			int whichhand=2;
			int meta, prox, dist = 0;
			cout<<"Please specify which hand is wrong l for left and r for right or q for quit:\n";
			int p = cvWaitKey(0);
			switch( (char) p) { //based on user input specifying which hand contains a wrong marker assignment. 
			    case 'l':
				    whichhand = 0;
				    break;
			    case 'r':
				    whichhand = 1;
				    break;
			    case 'q':
				    correct = true;
				    break;
			    
			    default :
				    break;
			}
			
			if(!correct){
			  
			    cout<<"Please specify which group has wrong elements:\n b\tbase\n m\tmeta\n p\tproxal\n or d\t distal\n";
			    int m = cvWaitKey(0);
			    switch((char)m) { // based on user input specifying which joint group contains a wrong marker assignment
				case 'b':
					correct_base(whichhand, correctimg);
					break;
				case 'm':
					correct_meta(whichhand, correctimg);
					break;
				case 'p':
					correct_prox(whichhand, correctimg);
					break;
				case 'd':
					correct_dist(whichhand, correctimg);
					break;
				case 'q':
					correct = true;
					cvZero(correctimg);
					break;
				default:	
					break;
		
			    } //endswitch
			}
			//destory corrections window and image
			cvDestroyWindow("corrections");
			cvReleaseImage(&correctimg);
		} //end of Editing Menu

		prev_frame=cvCloneImage(&tmp_frame);	  
		correct = false;
		cvZero(separated);
		//record distances of marker joint groups from base points. These will be used for the next frame to assign joint groups.
		lh.recordGroupDistances();
		rh.recordGroupDistances();
		
		//write video of correct positions
		IplImage *lh_lines = lh.getfingerlines(prev_frame);
		IplImage *both_lines = rh.getfingerlines(lh_lines);
		cvShowImage("video",both_lines);
		cvWriteFrame(writer,both_lines); 

		//zero images
		cvZero(est);
		cvZero(two_hands);
		cvZero(one_hand);
		cvZero(separated);
		
		//append coordinates of correctly assigned markers into appropriate vectors for writing to gms files. Store these values in backup txt file.
		lhVec[0].push_back(lh.getbaseL()); outputbackup<<lh.getbaseL().x<<","<<lh.getbaseL().y<<"\t";
		lhVec[1].push_back(lh.getbaseR()); outputbackup<<lh.getbaseR().x<<","<<lh.getbaseR().y<<"\t";
		lhVec[2].push_back(lh.getthumb());outputbackup<<lh.getthumb().x<<","<<lh.getthumb().y<<"\t";
		lhVec[3].push_back(lh.get1stfing());outputbackup<<lh.get1stfing().x<<","<<lh.get1stfing().y<<"\t";
		lhVec[4].push_back(lh.get2ndfing());outputbackup<<lh.get2ndfing().x<<","<<lh.get2ndfing().y<<"\t";
		lhVec[5].push_back(lh.get3rdfing());outputbackup<<lh.get3rdfing().x<<","<<lh.get3rdfing().y<<"\t";
		lhVec[6].push_back(lh.getpinkie());outputbackup<<lh.getpinkie().x<<","<<lh.getpinkie().y<<"\t";
		lhVec[7].push_back(lh.getthumb_b());outputbackup<<lh.getthumb_b().x<<","<<lh.getthumb_b().y<<"\t";
		lhVec[8].push_back(lh.get1stfing_b());outputbackup<<lh.get1stfing_b().x<<","<<lh.get1stfing_b().y<<"\t";
		lhVec[9].push_back(lh.get2ndfing_b());outputbackup<<lh.get2ndfing_b().x<<","<<lh.get2ndfing_b().y<<"\t";
		lhVec[10].push_back(lh.get3rdfing_b());outputbackup<<lh.get3rdfing_b().x<<","<<lh.get3rdfing_b().y<<"\t";
		lhVec[11].push_back(lh.getpinkie_b());outputbackup<<lh.getpinkie_b().x<<","<<lh.getpinkie_b().y<<"\t";
		lhVec[12].push_back(lh.get1stfing_c());outputbackup<<lh.get1stfing_c().x<<","<<lh.get1stfing_c().y<<"\t";
		lhVec[13].push_back(lh.get2ndfing_c());outputbackup<<lh.get2ndfing_c().x<<","<<lh.get2ndfing_c().y<<"\t";
		lhVec[14].push_back(lh.get3rdfing_c());outputbackup<<lh.get3rdfing_c().x<<","<<lh.get3rdfing_c().y<<"\t";
		lhVec[15].push_back(lh.getpinkie_c());outputbackup<<lh.getpinkie_c().x<<","<<lh.getpinkie_c().y<<"\t";

		rhVec[0].push_back(rh.getbaseL()); outputbackup<<rh.getbaseL().x<<","<<rh.getbaseL().y<<"\t";
		rhVec[1].push_back(rh.getbaseR()); outputbackup<<rh.getbaseR().x<<","<<rh.getbaseR().y<<"\t";
		rhVec[2].push_back(rh.getthumb()); outputbackup<<rh.getthumb().x<<","<<rh.getthumb().y<<"\t";
		rhVec[3].push_back(rh.get1stfing()); outputbackup<<rh.get1stfing().x<<","<<rh.get1stfing().y<<"\t";
		rhVec[4].push_back(rh.get2ndfing()); outputbackup<<rh.get2ndfing().x<<","<<rh.get2ndfing().y<<"\t";
		rhVec[5].push_back(rh.get3rdfing()); outputbackup<<rh.get3rdfing().x<<","<<rh.get3rdfing().y<<"\t";
		rhVec[6].push_back(rh.getpinkie()); outputbackup<<rh.getpinkie().x<<","<<rh.getpinkie().y<<"\t";
		rhVec[7].push_back(rh.getthumb_b());outputbackup<<rh.getthumb_b().x<<","<<rh.getthumb_b().y<<"\t";
		rhVec[8].push_back(rh.get1stfing_b());outputbackup<<rh.get1stfing_b().x<<","<<rh.get1stfing_b().y<<"\t";
		rhVec[9].push_back(rh.get2ndfing_b());outputbackup<<rh.get2ndfing_b().x<<","<<rh.get2ndfing_b().y<<"\t";
		rhVec[10].push_back(rh.get3rdfing_b());outputbackup<<rh.get3rdfing_b().x<<","<<rh.get3rdfing_b().y<<"\t";
		rhVec[11].push_back(rh.getpinkie_b());outputbackup<<rh.getpinkie_b().x<<","<<rh.getpinkie_b().y<<"\t";
		rhVec[12].push_back(rh.get1stfing_c());outputbackup<<rh.get1stfing_c().x<<","<<rh.get1stfing_c().y<<"\t";
		rhVec[13].push_back(rh.get2ndfing_c());outputbackup<<rh.get2ndfing_c().x<<","<<rh.get2ndfing_c().y<<"\t";
		rhVec[14].push_back(rh.get3rdfing_c());outputbackup<<rh.get3rdfing_c().x<<","<<rh.get3rdfing_c().y<<"\t";
		rhVec[15].push_back(rh.getpinkie_c());outputbackup<<rh.getpinkie_c().x<<","<<rh.getpinkie_c().y<<"\n";

		cvReleaseImage(&channels);
					  
		}
	  fr++;
		
	}
	//close backup stream
	outputbackup.close();

	cout <<" writing gms files "<< endl;
	cout.flush();
	//write vectors to gms file
	int nb_frames = lhVec[0].size();
	double info_xy[((nb_frames+1)*HAND_MARKERS*2*2)];
	//if starting from the beginning of a new gms file, write a new gms file
	if (startframe==0){
		cout<< " starting from frame 0 "<<endl; cout.flush();
		const char *scenename = argv[3];
		Gmswriter::writedata(lhVec,rhVec,info_xy, 0);
		gms_file_t *handposition;
		int nb_channels = HAND_MARKERS;
		gms_uct_struct_t* uct = Gmswriter::create (scenename, 2, nb_channels, GMS_POSITION, GMS_XY);
		gms_storage_info_t storage = Gmswriter::storenew (uct, handposition,scenename,info_xy,fps,nb_frames, GMS_FLOAT32);
	}
	else{ //otherwise append existing gms file
		cout << "starting from frame "<< startframe << endl; cout.flush();
		const char *scenename = argv[3];
		//open exisiting gms file
		Gmswriter::writedata(lhVec,rhVec,info_xy, 0);
		gms_file_t *handposition;
		gms_uct_struct_t* uct;
		gms_storage_info_t storage = Gmswriter::storeold (startframe, uct, handposition,scenename,info_xy,fps,nb_frames, GMS_FLOAT32);
	}

	//clear vectors
	hands.clear();
	lh_current.clear();
	rh_current.clear();
	lh_next.clear();
	rh_next.clear();
	lhVec.clear();
	rhVec.clear();

	cvReleaseImage(&vid);
	cvReleaseImage(&twothresh);
	cvDestroyAllWindows();

	return 0;
}//end main	
