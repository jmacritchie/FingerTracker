
// Include files
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cvblob.h>
#include "blobby.h"

using namespace cv;
using namespace cvb;
using namespace std;


CoorVec Blobby::detect(IplImage *orig_img, IplImage *img, CoorVec hands, int &numblobs)
{
	//clear hands vector
	hands.clear();
	// delcare a set of blob results and image for blobs to be drawn onto
	CvBlobs blobs;
	IplImage *labelImg = cvCreateImage(cvGetSize(img),IPL_DEPTH_LABEL, 1);
	// detect the blobs from the image
	unsigned int result = cvLabel(img, labelImg, blobs);
	//filter blobs so only those with an area between 7 and 100 are left
	cvFilterByArea(blobs, 7, 100);
	vector < pair<CvLabel, CvBlob*> > redbloblist;
	copy (blobs.begin(),blobs.end(), back_inserter(redbloblist));
	for (int i=0; i<redbloblist.size();i++){
	  cout << "[" <<redbloblist[i].first <<"] -> " << (*redbloblist[i].second) <<endl;
	}
	// mark the blobs on the image
	cvRenderBlobs (labelImg, blobs, orig_img, orig_img);
	
	// For each blob detected, the centroid is pushed back into vector hands
	numblobs = blobs.size()+1;
	for  (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		int iMeanx = it->second->centroid.x;
		int iMeany = it->second->centroid.y;
		//add the centre of the blob to the vector hands
		CvPoint centrepoint = cvPoint(iMeanx,iMeany);
		hands.push_back(centrepoint);
	}
	cvZero(img);
	return (hands);
}

