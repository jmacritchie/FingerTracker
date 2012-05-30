/* Geometric shape matching for pentagon of hands
Program to take in a video and detect blobs for each frame using the blobby functions and store the coordinates of the bounding boxes in a vector to be used when determining which hand each dot tracked is on. This is done by calculating pentagons relating to the base point of the wrist connected to the five knuckles

Also includes a user verification program which allows user to change recorded positions by clicking on the screen.

Ver1.0 includes use of correlation to have better tracking of the base point - a huge problem before. 
Ver 2.0 uses two different colours for segmentation of left and right hands. This eliminates the need for the k-means clustering steps but adds in some more thresholding functions.
Ver 3.0 changes the base markers on the hands to two base markers. New implementations of the fullhand modeal are included.

*/
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>
#include <math.h>
#include <getopt.h>

#include "blobby.h"
#include "correlation.h"
#include "fullhand.h"
#include "transform.h"
#include "gmswriter_fixed.h"
using namespace std;

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


//defining action for when a mouse event is detected. In this case, we want to store the x and y coordinates
void on_mouse( int event, int x, int y, int flags, void* param )
{
	switch( event )
   	 {
   	 case CV_EVENT_LBUTTONDOWN:
			marker = cvPoint(x,y);
			c_block.push_back(marker);
			add_point = 1;
			cout<< "point is at "<<x<<","<<y<<endl;
			break;
		}
	
}

/****************************************************************
VARIABLE WHICHAND:
		0 = LEFT
		1 = RIGHT
VARIABLE r_or_l:
		0 = FALSE = LEFT
		1 = TRUE = RIGHT
*****************************************************************/



void correct_onept(int whichhand, char whichgroup, char whichpoint)
{
	cout<<"Please click on the point in its correct position"<<endl;
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	cvWaitKey(0);
	if(add_point)
		{
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
		else
			{switch(whichpoint){
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
			cstart++;
		}
	//show results of changing points....update image
	IplImage *correctone = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
	//cvZero(correctone);
	IplImage *correcttwo = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
	//cvZero(correcttwo);
	
	correctone = lh.getfingerlines(correcttwo);
	correcttwo = rh.getfingerlines(correctone);
	cvNamedWindow("lines",1);
	cvShowImage("lines", correctone);
	cvWaitKey(0);
	cvDestroyWindow("lines");
}

void correct_meta(int whichhand){
	//change round elements of meta group for hand 'whichhand'
	//if(whichhand) //then its the right hand
		//change round right hand points i.e. shift one to the right
		//either shift or could have user click on points instead of waiting by chance to get it right?
	cout<<"meta being corrected "; cout.flush();
	cout<<"Please specify which finger a-e to correct or z if all\n";
	int changeall = 0;
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	int fin = cvWaitKey(0);
	switch((char)fin){
	case 'a': correct_onept(whichhand,'m','a');
		break;
	case 'b': correct_onept(whichhand,'m','b');
		break;								
	case 'c': correct_onept(whichhand,'m','c');
		break;
	case 'd': correct_onept(whichhand,'m','d');
		break;		
	case 'e': correct_onept(whichhand,'m','e');
		break;		
	default:
		changeall = 1;
		break;										
		}					

	if(changeall){
			cvWaitKey(0);
			if(add_point)
			{
			cout<<"HUZZAH";
			add_point = 0;
	
			}
	
	
	
			if(c_block.size()>4)
			{
			cout <<"Assigned "<<c_block.size()<<" points"<<endl; cout.flush();
			//assign corrected points to hand vectors...
			if(whichhand) 
				{ rh.setthumb(c_block[0]);
				rh.set1stfing(c_block[1]);
				rh.set2ndfing(c_block[2]);
				rh.set3rdfing(c_block[3]);
				rh.setpinkie(c_block[4]);
				}
			else
				{lh.setthumb(c_block[0]);
				lh.set1stfing(c_block[1]);
				lh.set2ndfing(c_block[2]);
				lh.set3rdfing(c_block[3]);
				lh.setpinkie(c_block[4]);
				}
			
			}
		CoorVecIt cstart = c_block.begin();	
		int thesize = c_block.size();
		//cout << "size of c_block starts as "<<c_block.size()<<endl;
		for (int s=0; s<thesize; s++){
			c_block.erase(cstart);
			cstart++;
		}
		//cout << "size of c_block is now "<<c_block.size()<<endl;
	
		//show results of changing points....update image
		IplImage *correctone = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
		//cvZero(correctone);
		IplImage *correcttwo = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
		//cvZero(correcttwo);
		
		correctone = lh.getfingerlines(correcttwo);
		correcttwo = rh.getfingerlines(correctone);
		cvNamedWindow("lines",1);
		cvShowImage("lines", correctone);
		cvWaitKey(0);
		cvDestroyWindow("lines");
		changeall = 0;
	}


}

void correct_prox(int whichhand){
	int changeall = 0;
	//change round elements of proxal group for hand 'whichhand'
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	cout<<"Please specify which finger a-e to correct or z if all\n";
	int fin = cvWaitKey(0);
	switch((char)fin){
	case 'a': correct_onept(whichhand,'p','a');
		break;
	case 'b': correct_onept(whichhand,'p','b');
		break;								
	case 'c': correct_onept(whichhand,'p','c');
		break;
	case 'd': correct_onept(whichhand,'p','d');
		break;		
	case 'e': correct_onept(whichhand,'p','e');
		break;		
	default:
		changeall = 1;
		break;										
		}					

	if(changeall){
		cvWaitKey(0);
		if(add_point)
			{
			cout<<"HUZZAH";
			add_point = 0;
			//push back vector with the new point, do this until there are 5 points
	
			}
		if(c_block.size()>4)
			{
			//assign corrected points to hand vectors...
			if(whichhand)
				{
				cout<<rh.getthumb_b().x<<"\t"<<rh.get1stfing_b().x<<"\n";//<<rh.get2ndfing_b().x<<"\t"<<rh.get3rdfing_b().x<<"\t"<<rh.getpinkie_b()<<endl;
				rh.setthumb_b(c_block[0]);
				rh.set1stfing_b(c_block[1]);
				rh.set2ndfing_b(c_block[2]);
				rh.set3rdfing_b(c_block[3]);
				rh.setpinkie_b(c_block[4]);
				cout<<rh.getthumb_b().x<<"\t"<<rh.get1stfing_b().x<<"\n";//<<rh.get2ndfing_b().x<<"\t"<<rh.get3rdfing_b().x<<"\t"<<rh.getpinkie_b()<<endl;
				}
			else
				{
				cout<<lh.getthumb_b().x<<"\t"<<lh.get1stfing_b().x<<"\n";//<<lh.get2ndfing_b().x<<"\t"<<lh.get3rdfing_b().x<<"\t"<<lh.getpinkie_b()<<endl;
				lh.setthumb_b(c_block[0]);
				lh.set1stfing_b(c_block[1]);
				lh.set2ndfing_b(c_block[2]);
				lh.set3rdfing_b(c_block[3]);
				lh.setpinkie_b(c_block[4]);
				cout<<lh.getthumb_b().x<<"\t"<<lh.get1stfing_b().x<<"\n";//<<lh.get2ndfing_b().x<<"\t"<<lh.get3rdfing_b().x<<"\t"<<lh.getpinkie_b()<<endl;
				}
	
				
			
			}
	
		CoorVecIt cstart = c_block.begin();	
		int thesize = c_block.size();
		//cout << "size of c_block starts as "<<c_block.size()<<endl;
		for (int s=0; s<thesize; s++){
			c_block.erase(cstart);
			cstart++;
		}
		//ask user if they are finished with corrections, if not return to hand question, if so , destroy corrections window
		//show results of changing points....update image
		IplImage *correctone = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
		//cvZero(correctone);
		IplImage *correcttwo = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
		//cvZero(correcttwo);
		
		correctone = lh.getfingerlines(correcttwo);
		correcttwo = rh.getfingerlines(correctone);
		cvNamedWindow("lines",1);
		cvShowImage("lines", correctone);
		cvWaitKey(0);
		cvDestroyWindow("lines");
		changeall = 0;
	}
}

void correct_dist(int whichhand)
{
	//change round elements of distal group for hand 'whichhand'
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	int changeall = 0;
	cout<<"Please specify which finger b-e to correct or z if all\n";
	int fin = cvWaitKey(0);
	switch((char)fin){
	case 'b': correct_onept(whichhand,'d','b');
		break;
	case 'c': correct_onept(whichhand,'d','c');
		break;								
	case 'd': correct_onept(whichhand,'d','d');
		break;
	case 'e': correct_onept(whichhand,'d','e');
		break;		
	default:
		changeall = 1;
		break;										
		}					

	if(changeall){

		cvWaitKey(0);
		if(add_point)
			{
			cout<<"HUZZAH";
			add_point = 0;
			//push back vector with the new point, do this until there are 5 points
	
			}
		if(c_block.size()>3)
			{
			//assign corrected points to hand vectors...
			if(whichhand)
				{rh.set1stfing_c(c_block[0]);
				rh.set2ndfing_c(c_block[1]);
				rh.set3rdfing_c(c_block[2]);
				rh.setpinkie_c(c_block[3]);
				}
			else
				{lh.set1stfing_c(c_block[0]);
				lh.set2ndfing_c(c_block[1]);
				lh.set3rdfing_c(c_block[2]);
				lh.setpinkie_c(c_block[3]);
				}
	
			}
		CoorVecIt cstart = c_block.begin();	
		int thesize = c_block.size();
		//cout << "size of c_block starts as "<<c_block.size()<<endl;
		for (int s=0; s<thesize; s++){
			c_block.erase(cstart);
			cstart++;
		}
		//ask user if they are finished with corrections, if not return to hand question, if so , destroy corrections window
		//show results of changing points....update image
		IplImage *correctone = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
		//cvZero(correctone);
		IplImage *correcttwo = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
		//cvZero(correcttwo);
		
		correctone = lh.getfingerlines(correcttwo);
		correcttwo = rh.getfingerlines(correctone);
		cvNamedWindow("lines",1);
		cvShowImage("lines", correctone);
		cvWaitKey(0);
		cvDestroyWindow("lines");
		changeall = 0;
	}
}

void correct_base(int whichhand)
{
	cvShowImage("corrections",correctimg);
	cvSetMouseCallback( "corrections", on_mouse, 0 );
	cvWaitKey(0);
	if(add_point)
		{
			cout<<"HUZZAH";
			add_point = 0;
			//push back vector with the new point, do this until there are 5 points
			}
		if(c_block.size()>1)
			{
			//assign corrected points to hand vectors...
			if(whichhand)
				{rh.setbaseptL(c_block[0]);
				rh.setbaseptR(c_block[1]);
				}
			else
				{lh.setbaseptL(c_block[0]);
				lh.setbaseptR(c_block[1]);
				}
	
			}
		CoorVecIt cstart = c_block.begin();	
		int thesize = c_block.size();
		//cout << "size of c_block starts as "<<c_block.size()<<endl;
		for (int s=0; s<thesize; s++){
			c_block.erase(cstart);
			cstart++;
		}
	//show results of changing points....update image
	IplImage *correctone = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
	//cvZero(correctone);
	IplImage *correcttwo = cvCreateImage(cvSize(780,216),DEPTH_8_BIT, CHANNELS_3);
	//cvZero(correcttwo);
	
	correctone = lh.getfingerlines(correcttwo);
	correcttwo = rh.getfingerlines(correctone);
	cvNamedWindow("lines",1);
	cvShowImage("lines", correctone);
	cvWaitKey(0);
	cvDestroyWindow("lines");
}

void usage(){
  cout << "\nUsage:\n"
       << "\tTo track blobs in videos for two hands in a piano performance\n"
       << "\t\ttwobase <start frame number> <gms output file> <frame number just before clap> <output video>\n\n";

  exit(1);
}	

int main(int argc , char** argv)
{

	if( argc < 4 ) {usage();}

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
	cout<< "File will be started from frame " << argv[1] <<endl;
	int startframe = atoi(argv[1]);
	int clapframe = atoi(argv[3]);
	int blobframe = clapframe+startframe;
	for(int p=0; p<HAND_MARKERS;p++){
		lhVec.push_back(CoorVec());  //loop creates 6 columns for each hand vector for gms writing
		rhVec.push_back(CoorVec());
	}

	//ImageVec images;
	bool correct = false;
	
	IplImage *tmp_frame=NULL;
	IplImage* vid = 0; 
	
	CvCapture* cap = NULL;
	cap = cvCaptureFromFile("/home/jenni/Videos/Fingerdance_Videos/SCovfin4.avi"); //capturing video from test avi file
	tmp_frame = cvQueryFrame(cap);
	if(!tmp_frame) {
		printf("bad video \n");
		exit(0);
	}
	cvQueryFrame(cap);
	int frameH    = (int) cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);
	int frameW    = (int) cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
	int fps = (int) cvGetCaptureProperty(cap, CV_CAP_PROP_FPS);

	CvVideoWriter *writer = 0;
	int isColor = 1;
	writer=cvCreateVideoWriter(argv[4],CV_FOURCC('D','I','V','X'),
				fps,cvSize(frameW,frameH),isColor);



	//creating and initialising IplImages
	IplImage *display = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT,CHANNELS_3);
	IplImage *channels = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT,CHANNELS_3);
	IplImage *twothresh = cvCreateImage( cvSize(frameW,frameH), DEPTH_8_BIT, CHANNELS_1 );

	IplImage *separated = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *one_hand = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *two_hands = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *est = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *final = cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	IplImage *prev_frame = NULL;//cvCreateImage(cvSize(frameW,frameH),DEPTH_8_BIT, CHANNELS_3);
	
	IplImage *green = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *red = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *blue = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *red_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *blue_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_green = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *mid_green = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_g = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_red = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *mid_red = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_r = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_blue = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *mid_blue = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_red_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *mid_red_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_r_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_blue_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *mid_blue_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *thresh_b_b = cvCreateImage(cvSize(frameW,frameH),8,1);
	IplImage *b = cvCreateImage(cvSize(frameW,frameH),8,3);
	IplImage *b_b = cvCreateImage(cvSize(frameW,frameH),8,3);
	IplImage *g = cvCreateImage(cvSize(frameW,frameH),8,3);
	IplImage *r = cvCreateImage(cvSize(frameW,frameH),8,3);
	IplImage *r_b = cvCreateImage(cvSize(frameW,frameH),8,3);

	int width =  frameW; //780
	int height = frameH; //216

	// threshold values for yellow and turquoise paint
		//threshold values for yellow UV paint
		int lowred = 20 ;
		int highred = 120;
		int maxred = 255;
	
		int lowgreen = 110;
		int highgreen = 255;
		int maxgreen = 255;
	
		int lowblue = 0;
		int highblue = 40;
		int maxblue = 255;
	
		//threshold values for turquoise UV paint
		int lowred_b = 20 ;
		int highred_b = 100;
		int maxred_b = 255;

		int lowblue_b = 30 ;
		int highblue_b = 160;
		int maxblue_b = 255;


	for( int fr = 1; tmp_frame;tmp_frame = cvQueryFrame(cap),fr++ ) {
		if(fr>blobframe){		

		cout<<"frame "<<fr<<endl;
		CoorVec lhfingers_current;
		CoorVec rhfingers_current;	

		//creating and initialising IplImages
		IplImage *display=cvCloneImage(tmp_frame);
		IplImage *channels = cvCloneImage(tmp_frame);
		if (fr>(blobframe+1))
			{//calculating distances etc from hands calculated on previous run (i.e. for first time, for k=0)
			lhfingers_current = lh.setpoints_wrt_org();
			rhfingers_current = rh.setpoints_wrt_org();
			lh_current = lh.distances_to_org(lhfingers_current);
			rh_current = rh.distances_to_org(rhfingers_current);
			}
		
		
		cvZero(green);
		cvZero(thresh_green);
		cvZero(mid_green);
		cvZero(thresh_g);
		cvZero(g);
		cvZero(blue);
		cvZero(thresh_blue);
		cvZero(mid_blue);
		cvZero(thresh_b);
		cvZero(b);
		cvZero(red);
		cvZero(thresh_red);
		cvZero(mid_red);
		cvZero(thresh_r);
		cvZero(r);
		cvZero(blue_b);
		cvZero(thresh_blue_b);
		cvZero(mid_blue_b);
		cvZero(thresh_b_b);
		cvZero(b_b);
		cvZero(red_b);
		cvZero(thresh_red_b);
		cvZero(mid_red_b);
		cvZero(thresh_r_b);
		cvZero(r_b);

		IplImage *channels_g = cvCloneImage(channels);
		cvSetImageCOI(channels_g,2);
		cvCopy(channels_g,green);
	
		cvThreshold(green,thresh_green,highgreen,maxgreen,CV_THRESH_BINARY_INV);
		cvCopy (green,mid_green,thresh_green);
		cvThreshold(mid_green,thresh_g,lowgreen,maxgreen,CV_THRESH_BINARY);
		cvCopy(channels,g,thresh_g);
	
	
		//copy image for two sets of thresholds..... yellow and turquoise
	
		IplImage *channels_b = cvCloneImage(g);
		cvSetImageCOI(channels_b,1);
		cvCopy(channels_b,blue);
	
		cvThreshold(blue,thresh_blue,highblue,maxblue,CV_THRESH_BINARY_INV);
		cvCopy (blue,mid_blue,thresh_blue);
		cvThreshold(mid_blue,thresh_b,lowblue,maxblue,CV_THRESH_BINARY);
		cvCopy(channels,b,thresh_b);
	
		IplImage *channels_r = cvCloneImage(b);
		cvSetImageCOI(channels_r,3);
		cvCopy(channels_r,red);
	
		cvThreshold(red,thresh_red,highred,maxred,CV_THRESH_BINARY_INV);
		cvCopy (red,mid_red,thresh_red);
		cvThreshold(mid_red,thresh_r,lowred,maxred,CV_THRESH_BINARY);
		cvCopy(channels,r,thresh_r);
	
		IplImage *channels_b2 = cvCloneImage(g);
		cvSetImageCOI(channels_b2,1);
		cvCopy(channels_b2,blue_b);
	
		cvThreshold(blue_b,thresh_blue_b,highblue_b,maxblue_b,CV_THRESH_BINARY_INV);
		cvCopy (blue_b,mid_blue_b,thresh_blue_b);
		cvThreshold(mid_blue_b,thresh_b_b,lowblue_b,maxblue_b,CV_THRESH_BINARY);
		cvCopy(channels,b_b,thresh_b_b);
	
		IplImage *channels_r2 = cvCloneImage(b_b);
		cvSetImageCOI(channels_r2,3);
		cvCopy(channels_r2,red_b);
	
		cvThreshold(red_b,thresh_red_b,highred_b,maxred_b,CV_THRESH_BINARY_INV);
		cvCopy (red_b,mid_red_b,thresh_red_b);
		cvThreshold(mid_red_b,thresh_r_b,lowred_b,maxred_b,CV_THRESH_BINARY);
		cvCopy(channels,r_b,thresh_r_b);

		//run blob detection twice for two separate blob groups
		int numblobs_lh, numblobs_rh=0;
		
		rhblobs = Blobby::detect(thresh_r_b,rhblobs,numblobs_rh);
		lhblobs = Blobby::detect(thresh_r,lhblobs,numblobs_lh);
		
		cvNamedWindow("mean",1);
		cvNamedWindow("hands",1);
		cvNamedWindow("video",1);

		
		cvShowImage("mean",thresh_r);
		
		//if(numblobs_lh>1 || numblobs_rh>1 )
		//	{
			if(!prev_frame){ prev_frame = cvCloneImage(tmp_frame);}
			cvNamedWindow("prev frame",1);
			cvShowImage("prev frame", prev_frame);
			
			//group hands into two separate objects using hand-> setbase and -> setfingers
			//find base point and set this in hand object
			lh.setallbases(lhblobs,height, prev_frame, tmp_frame, 0);
			rh.setallbases(rhblobs,height, prev_frame, tmp_frame, 1);
	
			//for the rest of the fingers 
			lh.setfingers(lhblobs, 0, width,height);
			rh.setfingers(rhblobs, 1, width,height);

			//plotting points of each hand onto image	
			one_hand = lh.getfingerlines(separated);
			two_hands = rh.getfingerlines(one_hand);
			final = cvCloneImage(two_hands);
			CoorVec lhfingers_next;
			CoorVec rhfingers_next;
			cvShowImage("hands",final);
			if(fr>(blobframe+1))
				{//calculating new distances etc now hands have been reassigned on k=1 run
				/*lhfingers_next = lh.setpoints_wrt_org();
				rhfingers_next = rh.setpoints_wrt_org();
				lh_next = lh.distances_to_org(lhfingers_next);
				rh_next = rh.distances_to_org(rhfingers_next);
				//now we have a current and next set of distances, the transformations between each point between two images can be calculated
				Transform lhtransform(lhfingers_current,lh_current,lhfingers_next,lh_next, lh );
				Transform rhtransform(rhfingers_current,rh_current,rhfingers_next,rh_next, rh);
				//est = lh.getestimations(two_hands); //estimations are currently rubbish! keep returning non-valid numbers...and screwing up the pictures. //TODO: FIX THIS!!!
				//final = rh.getestimations(est);	
	
				lhfingers_current.clear();
				rhfingers_current.clear();
				lhfingers_next.clear();
				rhfingers_next.clear();	*/	
				}
			
 			cvShowImage("hands",final); 
			int k = cvWaitKey(0);
			if( (char)k == 27 ) break;
			switch( (char) k) {		//user input to check that points are correct before they are stored.
					//evaluate whether the frame is correct. 
					case 'f':
						correct = false;
						break;
					case 't':
						correct = true;
						break;
					default:
						correct=true;
						break;
				
				} //endswitch
			while(!correct){ //keep stuck in this loop for as long as there are errors. i.e. cvWaitKey(0);
						IplImage *correctimg = cvCloneImage(tmp_frame);
						//create a new image showing correcting points...
						cvNamedWindow("corrections",1);
						cvShowImage("corrections",correctimg);

						int whichhand=2;
						int meta, prox, dist = 0;
						cout<<"Please specify which hand is wrong l for left and r for right or q for quit:\n";
						//cin >> whichhand;
						int p = cvWaitKey(0);
						switch( (char) p) {		//user input to check that points are correct before they are stored.
						//evaluate whether the frame is correct. 
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
						
						if(!correct){cout<<"Please specify which group has wrong elements:\n b\tbase\n m\tmeta\n p\tproxal\n or d\t distal\n";


							int m = cvWaitKey(0);
							switch((char)m) {		//user input to check that points are correct before they are stored.
							//evaluate whether the frame is correct. 
							case 'b':
								correct_base(whichhand);
								break;
							case 'm':
								correct_meta(whichhand);
								break;
							case 'p':
								correct_prox(whichhand);
 								break;
							case 'd':
								correct_dist(whichhand);
								break;
							case 'q':
								correct = true;
								cvZero(correctimg);
								break;
							default:	
								break;
					
							} //endswitch
						}
				
			}

			prev_frame=cvCloneImage(tmp_frame);
			
			correct = false;
			cvZero(separated);
			//write video of correct positions
			IplImage *lh_lines = lh.getfingerlines(separated);
			IplImage *both_lines = rh.getfingerlines(lh_lines);
			cvShowImage("video",both_lines);
			cvWriteFrame(writer,both_lines); 

			cvZero(final);
			cvZero(est);
			cvZero(two_hands);
			cvZero(one_hand);
			cvZero(separated);
			cvZero(thresh_r); cvZero(thresh_g); cvZero(thresh_red); cvZero(thresh_green); cvZero(red); cvZero(green); cvZero(mid_red); cvZero(mid_green); cvZero(r); cvZero(g);
		
			
			//put coordinates of fingers into vectors for writing to gms files
			lhVec[0].push_back(lh.getbaseL()); cout<<lh.getbaseL().x<<","<<lh.getbaseL().y<<"\t";
			lhVec[1].push_back(lh.getbaseR()); cout<<lh.getbaseR().x<<","<<lh.getbaseR().y<<"\t";
			lhVec[2].push_back(lh.getthumb());cout<<lh.getthumb().x<<","<<lh.getthumb().y<<"\t";
			lhVec[3].push_back(lh.get1stfing());cout<<lh.get1stfing().x<<","<<lh.get1stfing().y<<"\t";
			lhVec[4].push_back(lh.get2ndfing());cout<<lh.get2ndfing().x<<","<<lh.get2ndfing().y<<"\t";
			lhVec[5].push_back(lh.get3rdfing());cout<<lh.get3rdfing().x<<","<<lh.get3rdfing().y<<"\t";
			lhVec[6].push_back(lh.getpinkie());cout<<lh.getpinkie().x<<","<<lh.getpinkie().y<<"\t";
			lhVec[7].push_back(lh.getthumb_b());cout<<lh.getthumb_b().x<<","<<lh.getthumb_b().y<<"\t";
			lhVec[8].push_back(lh.get1stfing_b());cout<<lh.get1stfing_b().x<<","<<lh.get1stfing_b().y<<"\t";
			lhVec[9].push_back(lh.get2ndfing_b());cout<<lh.get2ndfing_b().x<<","<<lh.get2ndfing_b().y<<"\t";
			lhVec[10].push_back(lh.get3rdfing_b());cout<<lh.get3rdfing_b().x<<","<<lh.get3rdfing_b().y<<"\t";
			lhVec[11].push_back(lh.getpinkie_b());cout<<lh.getpinkie_b().x<<","<<lh.getpinkie_b().y<<"\t";
			lhVec[12].push_back(lh.get1stfing_c());cout<<lh.get1stfing_c().x<<","<<lh.get1stfing_c().y<<"\t";
			lhVec[13].push_back(lh.get2ndfing_c());cout<<lh.get2ndfing_c().x<<","<<lh.get2ndfing_c().y<<"\t";
			lhVec[14].push_back(lh.get3rdfing_c());cout<<lh.get3rdfing_c().x<<","<<lh.get3rdfing_c().y<<"\t";
			lhVec[15].push_back(lh.getpinkie_c());cout<<lh.getpinkie_c().x<<","<<lh.getpinkie_c().y<<"\n";

			rhVec[0].push_back(rh.getbaseL()); cout<<rh.getbaseL().x<<","<<rh.getbaseL().y<<"\t";
			rhVec[1].push_back(rh.getbaseR()); cout<<rh.getbaseR().x<<","<<rh.getbaseR().y<<"\t";
			rhVec[2].push_back(rh.getthumb()); cout<<rh.getthumb().x<<","<<rh.getthumb().y<<"\t";
			rhVec[3].push_back(rh.get1stfing()); cout<<rh.get1stfing().x<<","<<rh.get1stfing().y<<"\t";
			rhVec[4].push_back(rh.get2ndfing()); cout<<rh.get2ndfing().x<<","<<rh.get2ndfing().y<<"\t";
			rhVec[5].push_back(rh.get3rdfing()); cout<<rh.get3rdfing().x<<","<<rh.get3rdfing().y<<"\t";
			rhVec[6].push_back(rh.getpinkie()); cout<<rh.getpinkie().x<<","<<rh.getpinkie().y<<"\t";
			rhVec[7].push_back(rh.getthumb_b());cout<<rh.getthumb_b().x<<","<<rh.getthumb_b().y<<"\t";
			rhVec[8].push_back(rh.get1stfing_b());cout<<rh.get1stfing_b().x<<","<<rh.get1stfing_b().y<<"\t";
			rhVec[9].push_back(rh.get2ndfing_b());cout<<rh.get2ndfing_b().x<<","<<rh.get2ndfing_b().y<<"\t";
			rhVec[10].push_back(rh.get3rdfing_b());cout<<rh.get3rdfing_b().x<<","<<rh.get3rdfing_b().y<<"\t";
			rhVec[11].push_back(rh.getpinkie_b());cout<<rh.getpinkie_b().x<<","<<rh.getpinkie_b().y<<"\t";
			rhVec[12].push_back(rh.get1stfing_c());cout<<rh.get1stfing_c().x<<","<<rh.get1stfing_c().y<<"\t";
			rhVec[13].push_back(rh.get2ndfing_c());//cout<<rh.get2ndfing_c().x<<","<<rh.get2ndfing_c().y<<"\t";
			rhVec[14].push_back(rh.get3rdfing_c());//cout<<rh.get3rdfing_c().x<<","<<rh.get3rdfing_c().y<<"\t";
			rhVec[15].push_back(rh.getpinkie_c());//cout<<rh.getpinkie_c().x<<","<<rh.getpinkie_c().y<<"\n";

			
			
		}	
	}

	cout <<" writing gms files "<< endl;
	cout.flush();
	//write vectors to gms file
	int nb_frames = lhVec[0].size();
	
	double info_xy[((nb_frames+1)*HAND_MARKERS*2*2)];
	if (startframe==0)
		{
		cout<< " starting from frame 0 "<<endl; cout.flush();
		const char *scenename = argv[2];
		Gmswriter::writedata(lhVec,rhVec,info_xy, 0);
		gms_file_t *handposition;
		//int frame_rate = 50;
		int nb_channels = HAND_MARKERS;
		gms_uct_struct_t* uct = Gmswriter::create (scenename, 2, nb_channels, GMS_POSITION, GMS_XY);
		gms_storage_info_t storage = Gmswriter::storenew (uct, handposition,scenename,info_xy,fps,nb_frames, GMS_FLOAT32);
		}
	else
		{
		cout << "starting from frame "<< startframe << endl; cout.flush();
		const char *scenename = argv[2];
		//open exisiting gms file
		Gmswriter::writedata(lhVec,rhVec,info_xy, 0);
		gms_file_t *handposition;
		gms_uct_struct_t* uct;
		gms_storage_info_t storage = Gmswriter::storeold (startframe, uct, handposition,scenename,info_xy,fps,nb_frames, GMS_FLOAT32);
		}

	hands.clear();
	lh_current.clear();
	rh_current.clear();
	lh_next.clear();
	rh_next.clear();
	lhVec.clear();
	rhVec.clear();

	cvReleaseImage(&vid);
	cvReleaseImage(&twothresh);
	cvReleaseImage(&display);
	cvReleaseImage(&green);
	cvReleaseImage(&red); 
	cvReleaseImage(&blue); 
	cvReleaseImage(&red_b);
	cvReleaseImage(&blue_b);
	cvReleaseImage(&thresh_green);
	cvReleaseImage(&mid_green);
	cvReleaseImage(&thresh_g); 
	cvReleaseImage(&thresh_red);
	cvReleaseImage(&mid_red);
	cvReleaseImage(&thresh_r);
	cvReleaseImage(&thresh_blue);
	cvReleaseImage(&mid_blue);
	cvReleaseImage(&thresh_b);
	cvReleaseImage(&thresh_red_b);
	cvReleaseImage(&mid_red_b);
	cvReleaseImage(&thresh_r_b);
	cvReleaseImage(&thresh_blue_b);
	cvReleaseImage(&mid_blue_b);
	cvReleaseImage(&thresh_b_b );
	cvReleaseImage(&b);
	cvReleaseImage(&b_b);
	cvReleaseImage(&g );
	cvReleaseImage(&r );
	cvReleaseImage(&r_b);
	cvReleaseImage(&channels);	
	cvReleaseImage(&green);
	cvReleaseCapture(&cap);

	return 0;
}//end main


				
