/* Jennifer MacRitchie
Small program designed to ascertain thresholding values for H, S and V channels for two colours of markers.
In this case we have used red and green markers, so variable names reflect this.

*/
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include "cvblob.h"

using namespace cv;
using namespace cvb;
using namespace std;

int HRed_trackbar = 0;
int maxH_trackbar = 180;
int SRed_trackbar = 0;
int maxS_trackbar = 255;
int VRed_trackbar = 0;
int maxV_trackbar = 255;
int HRedmax_trackbar = 180;
int SRedmax_trackbar = 255;
int VRedmax_trackbar = 255;

int HGreen_trackbar = 0;
int SGreen_trackbar = 0;
int VGreen_trackbar = 0;
int HGreenmax_trackbar = 180;
int SGreenmax_trackbar = 255;
int VGreenmax_trackbar = 255;

int Hred, Sred, Vred, Hredmax, Sredmax, Vredmax = 0;
int Hgreen, Sgreen, Vgreen, Hgreenmax, Sgreenmax, Vgreenmax = 0;

IplImage *hsvimg;
IplImage *hsvoriginalimg;

void ThresholdHSVRed(int,void* = NULL){
  IplImage* imgThreshRed=cvCreateImage(cvGetSize(hsvimg),IPL_DEPTH_8U, 1);
  cvInRangeS (hsvoriginalimg, cvScalar (HRed_trackbar,SRed_trackbar,VRed_trackbar), cvScalar(HRedmax_trackbar,SRedmax_trackbar,VRedmax_trackbar), imgThreshRed); 
  cvShowImage("Threshold for Red Dots", imgThreshRed);
  Hred = HRed_trackbar;
  Hredmax = HRedmax_trackbar;
  Sred = SRed_trackbar;
  Sredmax = SRedmax_trackbar;
  Vred = VRed_trackbar;
  Vredmax = VRedmax_trackbar;
  cout<< "Labels are at: "<<Hred<<","<<Hredmax<<","<<Sred<<","<<Sredmax<<","<<Vred<<","<<Vredmax<<endl;
}


void ThresholdHSVGreen(int,void* = NULL){
  IplImage* imgThreshGreen=cvCreateImage(cvGetSize(hsvimg),IPL_DEPTH_8U, 1);
  cvInRangeS (hsvimg, cvScalar (HGreen_trackbar,SGreen_trackbar,VGreen_trackbar), cvScalar(HGreenmax_trackbar,SGreenmax_trackbar,VGreenmax_trackbar), imgThreshGreen); 
  cvShowImage("Threshold for Green Dots", imgThreshGreen);
  Hgreen = HGreen_trackbar;
  Hgreenmax = HGreenmax_trackbar;
  Sgreen = SGreen_trackbar;
  Sgreenmax = SGreenmax_trackbar;
  Vgreen = VGreen_trackbar;
  Vgreenmax = VGreenmax_trackbar;
  cout<< "Labels are at: "<<Hgreen<<","<<Hgreenmax<<","<<Sgreen<<","<<Sgreenmax<<","<<Vgreen<<","<<Vgreenmax<<endl;
}

void usage(){
  cout << "Threshold finder program for two colours of markers\n"
       << "\nUsage:\n"
       << "\t\tthresholdfinder <test frame> <background frame> <output.yml>\n"
       << "Use a background frame where the markers are not visible and use a test frame where all markers should be detected.\n"
       << "Output file is in .yml format and lists minimum and maximum threshold values for H, S and V channels\n\n";
  exit(1);
  
}
	
int main(int argc, char *argv[])
{

	if(argc<3) {usage();}
	
	// delcare IplImages
	IplImage *loadimg;
	IplImage *background;
	//load test image and background image
	if((loadimg=cvLoadImage(argv[1]))==NULL)
		printf( "error loading file \n");
	if((background=cvLoadImage(argv[2]))==NULL)
		printf( "error loading file \n");
	//convert to Mat structures
	Mat background_mat(background);
	Mat loadimg_mat(loadimg);
	
	Mat loadimg_matHSV;
	Mat background_matHSV;
	Mat diff;
	Mat diff_image;
	//background subtraction and threshold values of the difference image
	absdiff(loadimg_mat, background_mat, diff);
	Mat diff_mask = diff > 15; // make diff image a mask by thresholding for values above 0
	loadimg_mat.copyTo(diff_image, diff_mask);
	IplImage diff_imgIPL = diff_image;
	IplImage nondiff_img = loadimg_mat;
	
	//show difference image
	cout<<"This is the background subtracted image. If you are satisfied please press Enter, otherwise Esc to exit the program and choose new images"<<endl;
	cvNamedWindow ("difference image", 1);
	cvShowImage ("difference image", &diff_imgIPL);
	cvWaitKey(0);
	
	//initialise IplImages and convert images to HSV colour space
	hsvimg=cvCreateImage(cvGetSize(loadimg),8, 3);
	hsvoriginalimg = cvCreateImage(cvGetSize(loadimg),8,3);
	IplImage* imgThreshRed=cvCreateImage(cvGetSize(loadimg),IPL_DEPTH_8U, 1);
	IplImage* imgThreshGreen=cvCreateImage(cvGetSize(loadimg),IPL_DEPTH_8U, 1);
	cvCvtColor(&diff_imgIPL, hsvimg, CV_BGR2HSV);
	cvCvtColor(&nondiff_img, hsvoriginalimg, CV_BGR2HSV);
	
	IplImage *thresh_green = cvCreateImage(cvGetSize(loadimg),8,3);
	IplImage *thresh_red = cvCreateImage(cvGetSize(loadimg),8,3);
	cvZero(thresh_green);
	cvZero(thresh_red);

	//set up slider labels
	char Hthresh_label[50];
	sprintf( Hthresh_label, "Hue value minimum", 0 );
	char Hthreshmax_label[50];
	sprintf( Hthreshmax_label, "Hue value maximum", 0 );
	char Sthresh_label[50];
	sprintf( Sthresh_label, "Saturation value minimum", 0 );
	char Sthreshmax_label[50];
	sprintf( Sthreshmax_label, "Saturation value maximum", 0 );
	char Vthresh_label[50];
	sprintf( Vthresh_label, "Value value minimum", 0 );
	char Vthreshmax_label[50];
	sprintf( Vthreshmax_label, "Value value maximum", 0 );
	cvNamedWindow("Threshold for Red Dots",1);
	cvNamedWindow("Threshold for Green Dots",1);
	
	cout<<"Please use the trackbars to vary the thresholding values of each channel. You want the markers to show up as white dots on an otherwise black image. Once you are satisfied with your selection, press Enter to set the values"<<endl;
	//create trackbars and update threshold values on loop until user presses Enter, at which point the final values of all three channels are set.
	createTrackbar( Hthresh_label, "Threshold for Red Dots", &HRed_trackbar, maxH_trackbar, ThresholdHSVRed);
	createTrackbar( Hthreshmax_label, "Threshold for Red Dots", &HRedmax_trackbar, maxH_trackbar, ThresholdHSVRed);
	createTrackbar( Sthresh_label, "Threshold for Red Dots", &SRed_trackbar, maxS_trackbar, ThresholdHSVRed);
	createTrackbar( Sthreshmax_label, "Threshold for Red Dots", &SRedmax_trackbar, maxS_trackbar, ThresholdHSVRed);
	createTrackbar( Vthresh_label, "Threshold for Red Dots", &VRed_trackbar, maxV_trackbar, ThresholdHSVRed);
	createTrackbar( Vthreshmax_label, "Threshold for Red Dots", &VRedmax_trackbar, maxV_trackbar, ThresholdHSVRed);
	
	createTrackbar( Hthresh_label, "Threshold for Green Dots", &HGreen_trackbar, maxH_trackbar, ThresholdHSVGreen);
	createTrackbar( Hthreshmax_label, "Threshold for Green Dots", &HGreenmax_trackbar, maxH_trackbar, ThresholdHSVGreen);
	createTrackbar( Sthresh_label, "Threshold for Green Dots", &SGreen_trackbar, maxS_trackbar, ThresholdHSVGreen);
	createTrackbar( Sthreshmax_label, "Threshold for Green Dots", &SGreenmax_trackbar, maxS_trackbar, ThresholdHSVGreen);
	createTrackbar( Vthresh_label, "Threshold for Green Dots", &VGreen_trackbar, maxV_trackbar, ThresholdHSVGreen);
	createTrackbar( Vthreshmax_label, "Threshold for Green Dots", &VGreenmax_trackbar, maxV_trackbar, ThresholdHSVGreen);
	
	cvWaitKey(0);
	cvDestroyWindow("Threshold for Green Dots");
	cvWaitKey(0);
	cout<< "Final Labels are at: "<<Hred<<","<<Hredmax<<","<<Sred<<","<<Sredmax<<","<<Vred<<","<<Vredmax<<endl;
	cout<< "Final Labels are at: "<<Hgreen<<","<<Hgreenmax<<","<<Sgreen<<","<<Sgreenmax<<","<<Vgreen<<","<<Vgreenmax<<endl;
	cvDestroyWindow("Threshold for Red Dots");
	
	cvInRangeS (hsvoriginalimg, cvScalar (Hred,Sred,Vred), cvScalar(Hredmax,Sredmax,Vredmax), imgThreshRed);
	cvInRangeS (hsvimg, cvScalar (Hgreen,Sgreen,Vgreen), cvScalar(Hgreenmax,Sgreenmax,Vgreenmax), imgThreshGreen);
	
	//write threshold values to output .yml file
	FileStorage fs(argv[3], FileStorage::WRITE);
	fs << "HRed" << Hred;
	fs << "HRedmax" << Hredmax;
	fs << "SRed" << Sred;
	fs << "SRedmax" << Sredmax;
	fs << "VRed" << Vred;
	fs << "VRedmax" << Vredmax;
	
	fs << "HGreen" << Hgreen;
	fs << "HGreenmax" << Hgreenmax;
	fs << "SGreen" << Sgreen;
	fs << "SGreenmax" << Sgreenmax;
	fs << "VGreen" << Vgreen;
	fs << "VGreenmax" << Vgreenmax;
	
	fs.release();
	
	//show blob detection for tracker program on red channel as example
	IplImage *img;
	IplImage *img2;
	img = cvCloneImage(imgThreshRed);
	img2 = cvCloneImage(imgThreshGreen);
	cvNamedWindow ("Blob Extraction", 1);
	cvShowImage ("Blob Extraction", imgThreshRed);
	
	//setup blob structures and detect blobs in thresholded image
	CvBlobs redblobs;
	IplImage *labelImg = cvCreateImage(cvGetSize(imgThreshRed),IPL_DEPTH_LABEL, 1);
	unsigned int result = cvLabel(imgThreshRed, labelImg, redblobs);
	cout << "Blobs found -> "<<redblobs.size() <<endl;
	cvFilterByArea(redblobs, 7, 100);
	cout << "Filtered Blobs found -> "<<redblobs.size() <<endl;
	vector < pair<CvLabel, CvBlob*> > redbloblist;
	copy (redblobs.begin(),redblobs.end(), back_inserter(redbloblist));
	for (int i=0; i<redbloblist.size();i++){
	  cout << "[" <<redbloblist[i].first <<"] -> " << (*redbloblist[i].second) <<endl;
	}
	cvRenderBlobs (labelImg, redblobs, loadimg, loadimg);

	//show blob detection
	cvNamedWindow ("Blobs Detected",1);
	cvShowImage("Blobs Detected", loadimg);
	cvWaitKey(0);
	// release the image
	cvReleaseImage(&img);

	return 0;
}
