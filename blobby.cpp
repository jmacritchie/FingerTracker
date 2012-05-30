
// Include files
#include <stdlib.h>
#include <stdio.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>
#include <Blob.h>
#include <BlobResult.h>

#include "blobby.h"

using namespace std;

CoorVec Blobby::detect(IplImage *img, CoorVec hands, int &numblobs)
{
	//clear hands vector
	hands.clear();
	// delcare a set of blob results
	CBlobResult blobs;
	// detect the blobs from the image, with no mask, using a threshold of 100
	blobs = CBlobResult( img, NULL, 100, false );
	//filter blobs so only those with an area between 2 and 150, and with length below 50 are left
	blobs.Filter( blobs, B_INCLUDE, CBlobGetArea(), B_INSIDE,2,150 );
	blobs.Filter( blobs, B_INCLUDE, CBlobGetLength(), B_INSIDE,0,50 );
	
	
	// mark the blobs on the image
	int i;
	// declare a single blob
	CBlob Blob;
	int iMaxx, iMinx, iMaxy, iMiny, iMeanx, iMeany;
	
	// For each blob detected, the coordinates of the centre are calculated and registered into the vector hands, as well as being marked on the image.
	numblobs = blobs.GetNumBlobs();
	for  (i=0; i<blobs.GetNumBlobs(); ++i)
	{
		// get the blob info
		Blob = blobs.GetBlob(i);
		// get max, and min co-ordinates
		iMaxx=(int)Blob.MaxX();
		iMinx=(int)Blob.MinX();
		iMaxy=(int)Blob.MaxY();
		iMiny=(int)Blob.MinY();
		// find the average of the blob (i.e. estimate its centre)
		iMeanx=(iMinx+iMaxx)/2;
		iMeany=(iMiny+iMaxy)/2;
		// mark centre on the image
		cvLine( img, cvPoint(iMeanx, iMeany), cvPoint(iMeanx, iMeany), CV_RGB(50, 50 , 50), 4, 8, 0 );
		// mark box around blob on the image
		cvRectangle( img, cvPoint(iMinx , iMiny ), cvPoint ( iMaxx, iMaxy ), CV_RGB(150, 150, 150), 1, 8, 0);
		//add the centre of the blob to the vector hands
		CvPoint centrepoint = cvPoint(iMeanx,iMeany);
		hands.push_back(centrepoint);

	}

	// display the image
	cvNamedWindow("image",1);
	cvShowImage("image", img);
	cvZero(img);
	return (hands);
}

