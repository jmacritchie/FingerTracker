// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "fullhand.h"
#include "organise.h"
#include <iostream>
using namespace std;
using namespace cv;

typedef std::vector <CvPoint> CoorVec;
typedef std::vector <CvPoint>::iterator CoorVecIt;
typedef std::vector <double> DistVec;


CoorVec Organise::order_x(CoorVec handvec, int width, int r_or_l){
//orders elements of vector handvec in terms of x coordinates. depending on whether it is of the right or left hand will determine whether it is highest or lowest x value first. LH - highest first, RH - lowest first. first element marks the thumb marker.
	int msize = handvec.size();
	CoorVec xordered;
	for(int m = 0;m<msize;m++){
		int rh_x=0;
		int lh_x = width;
		int r_l_y = 0;
		CvPoint base = cvPoint(0,0);
		CoorVecIt it = handvec.begin();
		CoorVecIt base_it = handvec.begin();
		int shrinksize = handvec.size();
		for (int int_r=0;int_r<shrinksize;int_r++){
				if(handvec[int_r].x<lh_x){
					lh_x = handvec[int_r].x;
					base= handvec[int_r];
					base_it = it;
				}
			it++;
		}
		xordered.push_back(base);
		handvec.erase(base_it);
	}
	cout<< "X ordered:"<<r_or_l<<":";
	for (int j=0; j<xordered.size(); j++){
	 cout<< xordered[j].x<<","<<xordered[j].y<<"\t"; 
	}
	cout<<endl;
	return(xordered);
	xordered.clear();
}


CoorVec Organise::order_y(CoorVec handvec, int height){

//orders the vector in terms of y coordinates - lowest first
	int msize = handvec.size();
	CoorVec yordered;
	for(int m = 0;m<msize;m++){
		int rh_y=0;
		CvPoint base = cvPoint(0,0);
		CoorVecIt it = handvec.begin();
		CoorVecIt base_it = handvec.begin();
		int shrinksize = handvec.size();
		for (int int_r=0;int_r<shrinksize;int_r++){
			if(handvec[int_r].y>rh_y){
				rh_y = handvec[int_r].y;
				base= handvec[int_r];
				base_it = it;
			}
			it++;
		}
		handvec.erase(base_it);
		yordered.push_back(base);
	}
	return(yordered);
	yordered.clear();
}

CoorVec Organise::checkblobs(CoorVec ordered){

	CoorVec checked;
	cout<< "checking blobs\t";
	CvPoint blob = cvPoint(0,0);
	int newsize = ordered.size();
	for (int count=0;count<newsize; count++){
		cout<<ordered[count].x<<","<<ordered[count].y<<"\t";
	} cout<<"end of blobs"<<endl;

	CoorVecIt blobit = ordered.begin();
	CoorVecIt deleteblob = ordered.begin();
	cout<<"newsize is "<<newsize<<endl;
	//check that no blobs are right next to each other i.e. no blobs are split in two.
	for (int blobs = 1; blobs<newsize;blobs++){
			cout<<"loopsize is "<<newsize;
			int xd, yd =0;
			blobit++;
 			xd = abs(ordered[blobs].x-ordered[blobs-1].x);
 			yd = abs(ordered[blobs].y-ordered[blobs-1].y);
			double blobdistance = sqrt(xd*xd + yd*yd);
			if(blobdistance<6){
				deleteblob = blobit;
				int midx = int((ordered[blobs].x+ordered[blobs-1].x)/2);
				int midy = int((ordered[blobs].y+ordered[blobs-1].y)/2);
				ordered[blobs] = cvPoint(midx,midy);
				cout<<"size now is "<<ordered.size()<<"\t";

				if(!blobs==newsize-1){
					checked.push_back(cvPoint(midx,midy));
				}
			}
			else{
				blob = ordered[blobs-1];
				checked.push_back(blob);
				if(blobs==newsize-1){
					CvPoint nextblob = ordered[blobs];
					checked.push_back(nextblob);
				}
			}
	}
	int checksize = checked.size();
	cout<<"begin checked blobs\t";
	for(int c=0;c<checksize;c++){
		cout<<checked[c].x<<","<<checked[c].y<<"\t";
	} cout<<"checked blobs\n";

	return(checked);
}