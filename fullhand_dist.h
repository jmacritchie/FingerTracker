/*! \file fullhand_dist.h
 * \brief file describing class fullhand_dist
 * */
#ifndef FULLHANDIST_H
#define FULLHANDIST_H
// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>
#include "fullhand.h"
/*! \brief A vector containing CvPoint structures (an x,y coordinate) */
typedef std::vector <CvPoint> CoorVec;
/*! \brief An iterator to point towards the CoorVec vector*/
typedef std::vector <CvPoint>::iterator CoorVecIt;
/*! \brief A vector containing doubles */
typedef std::vector <double> DistVec;

/*! \class Fullhand_dist
 * \brief class enabling calculating distances between all the detected joints of the fullhand object. Inherits from the fullhand class
 * */
class Fullhand_dist:public Fullhand{
public:
	/*!\fn Fullhand_dist(bool whichhand, double * handframe)
	 * \brief Constructor for Fullhand_dist object. Sets all members to inital values and assign
	 * \param[in] whichhand Boolean value denoting if hand is right (true) or left (false)
	 * \param[in] handframe Array of doubles which contains the ordered points of the hand.
	 */
	Fullhand_dist(bool whichhand, double * handframe);
	/*!\fn void calc_distances()
	 * \brief Function that calculates all the distances a,b,c,d,e,fthumb to fpinkie,g1 to gpinkie
	 */
	void calc_distances();
	/*! \fn void print_distances()
	 * \brief Prints values of all distance members of fullhand_dist object to command line
	 */
	void print_distances();
	/*! \fn double geta_dist()
	 * \brief Function to return the value of the distance A
	 * \return distance A
	 */
	double geta_dist(){return a_dist;};
	/*! \fn double getb_dist()
	 * \brief Function to return the value of the distance B
	 * \return distance B
	 */
	double getb_dist(){return b_dist;};
	/*! \fn double getc_dist()
	 * \brief Function to return the value of the distance C
	 * \return distance C
	 */
	double getc_dist(){return c_dist;};
	/*! \fn double getd_dist()
	 * \brief Function to return the value of the distance D
	 * \return distance D
	 */
	double getd_dist(){return d_dist;};
	/*! \fn double getf_thumb()
	 * \brief Function to return the value of the distance F (thumb)
	 * \return distance F (thumb)
	 */
	double getf_thumb(){return f_thumb;};
	/*! \fn double getf_1st()
	 * \brief Function to return the value of the distance F (1st finger)
	 * \return distance F (1st finger)
	 */
	double getf_1st(){return f_1st;};
	/*! \fn double getf_2nd()
	 * \brief Function to return the value of the distance F (2nd finger)
	 * \return distance F (2nd finger)
	 */
	double getf_2nd(){return f_2nd;};
	/*! \fn double getf_3rd()
	 * \brief Function to return the value of the distance F (3rd finger)
	 * \return distance F (3rd finger)
	 */
	double getf_3rd(){return f_3rd;};
	/*! \fn double getf_pinkie()
	 * \brief Function to return the value of the distance F (pinkie finger)
	 * \return distance F (pinkie finger)
	 */
	double getf_pinkie(){return f_pinkie;};
	/*! \fn double getg_1st()
	 * \brief Function to return the value of the distance G (1st finger)
	 * \return distance G (1st finger)
	 */
	double getg_1st(){return g_1st;};
	/*! \fn double getg_2nd()
	 * \brief Function to return the value of the distance G (second finger)
	 * \return distance G (second finger)
	 */
	double getg_2nd(){return g_2nd;};
	/*! \fn double getg_3rd()
	 * \brief Function to return the value of the distance G (third finger)
	 * \return distance G (third finger)
	 */
	double getg_3rd(){return g_3rd;};
	/*! \fn double getg_pinkie()
	 * \brief Function to return the value of the distance G (pinkie finger)
	 * \return distance G (pinkie finger)
	 */
	double getg_pinkie(){return g_pinkie;};
	/*! \fn double geth_dist()
	 * \brief Function to return the value of the distance H
	 * \return distance H
	 */
	double geth_dist(){return h_dist;};

	double a_dist;/*!Distance A measured between the left and right wrist points */
	double b_dist;/*!Distance B measured between the left wrist point and the 1st finger metacarpal */
	double c_dist;/*!Distance C measured between the right wrist point and the 4th/pinkie finger metacarpal */
	double d_dist;/*!Distance D measured between the 1st and 4th/pinkie finger metacarpals */
	double f_thumb;
	double f_1st;/*!Distance F (1st finger) measured between the metacarpal and proximal points */
	double f_2nd;/*!Distance F (2nd finger) measured between the metacarpal and proximal points */
	double f_3rd;/*!Distance F (3rd finger) measured between the metacarpal and proximal points */
	double f_pinkie;/*!Distance F (4th/pinkie finger) measured between the metacarpal and proximal points */
	double g_1st;/*!Distance G (1st finger) measured between the metacarpal and proximal points */
	double g_2nd;/*!Distance G (2nd finger) measured between the metacarpal and proximal points */
	double g_3rd;/*!Distance G (3rd finger) measured between the metacarpal and proximal points */
	double g_pinkie;/*!Distance G (4th/pinkie finger) measured between the metacarpal and proximal points */
	double h_dist;/*!Distance H measured between the thumb metacarpal and the nearest wrist point(depending on whether left or right hand) */

	double LR_zest_wrista;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_wristb;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_meta_thumb;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_meta_1;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_meta_2;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_meta_3;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_meta_4;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_prox_thumb;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_prox_1;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_prox_2;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_prox_3;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_prox_4;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_dist_1;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_dist_2;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_dist_3;/*!Estimation of z axis coordinate for this point  */
	double LR_zest_dist_4;/*!Estimation of z axis coordinate for this point  */



};

#endif
