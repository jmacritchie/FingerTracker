/*! \file organise.h 
 * \brief file describing namespace Organise 
 */
/* A bunch of functions that organises the finger data into ascending x coordinate etc.*/
// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include "fullhand.h"

using namespace cv;
using namespace std;
/*! \brief A vector containing CvPoint structures (an x,y coordinate) */
typedef std::vector <CvPoint> CoorVec;
/*! \brief An iterator to point towards the CoorVec vector*/
typedef std::vector <CvPoint>::iterator CoorVecIt;
/*! \brief A vector containing doubles */
typedef std::vector <double> DistVec;

/*!\namespace Organise
 * \brief Namespace containing functions that order vectors depending on x and y coordinates
 */
namespace Organise{
	/*!\fn CoorVec order_x(CoorVec handvec, int width, int r_or_l)
	 * \brief Function that orders the coordinate points in vector \a handvec based on their x coordinate
	 * \param[in] handvec Vector of CVpoints
	 * \param[in] width Width in pixels of the image
	 * \param[in] r_or_l integer declaring whether this is the right(1) or left hand(0)
	 * \return Vector of coordinate points ordered in increasing (right hand) or decreasing (left hand) order
	 */
	CoorVec order_x(CoorVec handvec, int width, int r_or_l);
	/*!\fn CoorVec order_y(CoorVec handvec, int height)
	 * \brief Function that orders the coordinate points in vector \a handvec based on their y coordinate
	 * \param[in] handvec Vector of CVpoints
	 * \param[in] height Height in pixels of the image
	 * \return Vector of coordinate points ordered in increasing order
	 */
	CoorVec order_y(CoorVec handvec, int height);
	/*!\fn CoorVec checkblobs(CoorVec ordered)
	 * \brief Function to check that no two blobs are in fact right next to each other and could be considered the same blob
	 * \param[in] ordered Vector containing ordered list (by x coordinate) of points
	 * \return Vector containing ordered list of points, with any double points removed and replaced by a single point
	 */
	CoorVec checkblobs(CoorVec ordered);
}


