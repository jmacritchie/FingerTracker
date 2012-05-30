/*! \file blobby.h
 * \brief Documentation for blobby.h
 */

#ifndef BLOBBY_H
#define BLOBBY_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <cv.h>
#include <highgui.h>
#include <cxcore.h>
#include <math.h>
#include <vector>

#include <Blob.h>
#include <BlobResult.h>

using namespace std;

/*! \brief A vector containing CvPoint structures (an x,y coordinate)*/
typedef std::vector<CvPoint> CoorVec; 
/*! \brief An iterator to point towards the CoorVec vector*/
typedef std::vector<CvPoint>::iterator CoorVecIt;

/*! \namespace Blobby
 \brief contains functions concerning the detection of blobs from images
*/

/*! \fn CoorVec detect(IplImage *img, CoorVec hands, int &numblobs) 
    \brief Detect blobs from image \a img and stores the centre coordinates of these blobs in coordinate vector \a hands. 
    \param [in] img The grayscale input image in which to detect the blobs.
    \param [out] hands The output vector which stores the coordinates of the centres of each detected blob.
    \param [out] numblobs The number of blobs detected in the image.
*/
namespace Blobby{

CoorVec detect(IplImage *img, CoorVec hands, int &numblobs);

}

#endif
