/*! \file correlation.h
 * \brief Documentation for correlation namespace
 */

#ifndef CORRELATION_H
#define CORRELATION_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

/*! \def DEPTH_8_BIT 
 * \brief depth in bits of the grayscale images
 */
/*! \def DEPTH_32_FLOAT
 * \brief depth in bits of the colour images
 */
/*! \def CHANNELS_1
 * \brief for 1 channel images i.e. grayscale
 */
/*! \def CHANNELS_3
 * \brief for 3 channel images i.e. colour
 */
#define DEPTH_8_BIT 8
#define DEPTH_32_FLOAT IPL_DEPTH_32F
#define CHANNELS_1 1
#define CHANNELS_3 3

/*! \namespace correlation
 *  \brief functions used to create search areas/templates in the image, and perform correlation with blocks of proceeding images
 */

/*! \fn void search_and_template(CvPoint pt, IplImage *picture,IplImage *corrblock, IplImage *mask)
 * \brief Function creates search template of surrounding block \a corrblock of point \a pt from original image \a picture and creates mask \mask determining the search area in the new image.
 *  \param [in] pt A feature point coordinate that is to be tracked in subsequent images
 * \param [in] picture Image that the feature point is taken from
 * \param [out] corrblock A 3 channel 8 bit image which contains the feature point to be searched for, and a surrounding block of the original image
 * \param [out] mask A binary image where the block to be searched for is entirely white, with the rest of the image black
 */

/*! \fn void masktemplate(CvPoint pt,IplImage *picture, IplImage *mask)
 * \relates search_and_template
 * \brief Recalculates the mask image for a new search area to locate feature point \a pt in image \a picture 
 */

/*! \fn CvPoint max_correlation(CvPoint point,IplImage *corrblock, IplImage *maskarea, IplImage *frame)
 * \brief Correlation function which attempts to detect the feature point \a point in the subsequent image \a frame, returning the coordinates of the highest point of correlation as a CvPoint. 
 * \param [in] point A feature point coordinate that is to be tracked in subsequent images
 * \param [in] corrblock Part of the original image that contains the feature point and the area surrounding it
 * \param [in] maskarea The mask image that denotes which part of the new frame to search
 * \param [in] frame The new frame where the feature point is to be searched for
 * \return The coordinate which represents the maximum correlation in the search block.
 *  
 */
namespace Correlation{

  // creates search template block around point pt and mask for the search area to be put into the max_correlation function to perform correlation
  void search_and_template(CvPoint pt, IplImage *picture,IplImage *corrblock, IplImage *mask);

  //recalculates just mask search area
  void masktemplate(CvPoint pt,IplImage *picture, IplImage *mask);

  //finds the coordinate in the new image with the highest correlation to the feature point point within the old image
  CvPoint max_correlation(CvPoint point,IplImage *corrblock, IplImage *maskarea, IplImage *frame);

}

#endif

