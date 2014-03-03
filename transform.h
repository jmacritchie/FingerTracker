/*! \file transform.h 
 * \brief file describing class transform 
 */
#ifndef TRANSFORM_H
#define TRANSFORM_H

// Include files
#include <stdlib.h>
#include <stdio.h>
// OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

/*! \brief A vector containing CvPoint structures (an x,y coordinate) */
typedef std::vector <CvPoint> CoorVec;
/*! \brief An iterator to point towards the CoorVec vector*/
typedef std::vector <CvPoint>::iterator CoorVecIt;
/*! \brief A vector containing doubles */
typedef std::vector <double> DistVec;

/*!\class Transform
 * \brief class enabling the calculation of the transformation and rotation of all points in a hand set from one frame to the next
 */
class Transform{

public:
	/*!\fn Transform(CoorVec fingers1, DistVec dist1,CoorVec fingers2, DistVec dist2, Fullhand &either_hand)
	 * \brief Constructor intialising the set of points for the first frame, \a fingers1 and the second frame \a fingers2. Also calls the calculate_trans function.
	 * \param[in] fingers1 Vector of coordinates for the first frame
	 * \param[in] dist1 Distances calculated for the first frame
	 * \param[in] fingers2 Vector of coordinates for the second frame
	 * \param[in] dist1 Distances calculated for the second frame
	 * 
	 */
	Transform(CoorVec fingers1, DistVec dist1,CoorVec fingers2, DistVec dist2, Fullhand &either_hand);
	~Transform();
protected:
	DistVec dist_current;/*!vector of distances calculated for the first frame */
	DistVec dist_next;/*!vector of distances calculated for the next frame */
	CoorVec point_current;/*!vector of coordinates for the first frame */
	CoorVec point_next;/*!vector of coordinates for the next frame */

	double dist_thumbs;/*!distance between thumb metacarpal and wrist point */
	double dist_fsts;/*!distance between first finger metacarpal and wrist point */
	double dist_scnds;/*!distance between second finger metacarpal and wrist point */
	double dist_thrds;/*!distance between third finger metacarpal and wrist point */
	double dist_pinkies;/*!distance between fourth finger/pinkie metacarpal and wrist point */
	double dist_thumbs_b, dist_fsts_b, dist_scnds_b, dist_thrds_b, dist_pinkies_b;/*!distance between proximal joint and metacarpal for corresponding finger */
	double dist_thumbs_c, dist_fsts_c, dist_scnds_c, dist_thrds_c, dist_pinkies_c;/*!distance between distal joint and proximal joint for corresponding finger */

	bool same_thumb;/*!value denoting if the thumb point is the same as in the previous frame */
	bool same_first;/*!value denoting if the first finger metacarpal point is the same as in the previous frame */
	bool same_second;/*!value denoting if the second finger metacarpal point is the same as in the previous frame */
	bool same_third;/*!value denoting if the third finger metacarpal point is the same as in the previous frame */
	bool same_pinkie;/*!value denoting if the fourth finger/pinkie metacarpal point is the same as in the previous frame */
	bool same_thumb_b, same_first_b, same_second_b, same_third_b, same_pinkie_b;/*!value denoting if the proximal point is the same as in the previous frame */
	bool same_thumb_c, same_first_c, same_second_c, same_third_c, same_pinkie_c;/*!value denoting if the distal point is the same as in the previous frame */

	double scale_thumb;/*! scaling value for thumb metacarpal between previous and current frame */
	double scale_fst;/*! scaling value for first finger metacarpal between previous and current frame */
	double scale_scnd;/*! scaling value for second finger metacarpal between previous and current frame */
	double scale_thrd;/*! scaling value for third finger metacarpal between previous and current frame */
	double scale_pinkie;/*! scaling value for fourth finger/pinkie metacarpal between previous and current frame */
	double scale_thumb_b, scale_fst_b, scale_scnd_b, scale_thrd_b, scale_pinkie_b;/*! scaling value for proximal point between previous and current frame */
	double scale_thumb_c, scale_fst_c, scale_scnd_c, scale_thrd_c, scale_pinkie_c;/*! scaling value for distal point between previous and current frame */
	double rot_thumb;/*! rotation value for thumb metacarpal between previous and current frame */
	double rot_fst;/*! rotation value for first finger metacarpal between previous and current frame */
	double rot_scnd;/*! rotation value for second finger metacarpal between previous and current frame */
	double rot_thrd;/*! rotation value for third finger metacarpal between previous and current frame */
	double rot_pinkie;/*! rotation value for fourth finger/pinkie metacarpal between previous and current frame */
	double rot_thumb_b, rot_fst_b, rot_scnd_b, rot_thrd_b, rot_pinkie_b;/*! rotation value for proximal point between previous and current frame */
	double rot_thumb_c, rot_fst_c, rot_scnd_c, rot_thrd_c, rot_pinkie_c;/*! rotation value for distal point between previous and current frame */
	int number_detected;/*!integer value reflecting number of detected points out of possible 32 */
	Fullhand argh;/*!fullhand object*/
private:
	/*!\fn Fullhand calculate_trans()
	 * \brief Function calculates the scaling and rotation of each point in the Fullhand object between the previous and current frame
	 * \return Fullhand object with detected points from current frame
	 */
	Fullhand calculate_trans();
	/*!\fn Fullhand find_fingers(int fingerNo, int number)
	 * \param[in] fingerNo finger number denoting the finger/joint point to be detected
	 * \param[in] number number of points detected
	 * \return Fullhand object
	 */
	Fullhand find_fingers(int fingerNo, int number);
	/*!\fn double distance(CvPoint one, CvPoint two)
	 * \brief Function calculating distance between two coordinate points
	 * \param[in] one first coordinate point
	 * \param[out] two second coordinate point
	 * \return distance between the two points
	 */
	double distance(CvPoint one, CvPoint two);
	/*!\fn double length_cart2polar(CvPoint cart)
	 * \brief Function to calculate the distance of the coordinate \a cart from the origin 0,0
	 * \param[in] cart the coordinate point
	 * \return distance from coordinate point to the origin 0,0
	 */
	double length_cart2polar(CvPoint cart);
	/*!\fn angle_cart2polar(CvPoint cart, double length)
	 * \brief function to calculate the angle between the origin and the coordinate point \a cart
	 * \param[in] cart coordinate point
	 * \param[in] length distance between the coordinate point and the origin
	 * \return angle 
	 */
	double angle_cart2polar(CvPoint cart, double length);
	/*!\fn CvPoint polar2cart(double pol_length, double pol_angle)
	 * \brief function to convert values of length and angle from origin into a coordinate point
	 * \param[in] pol_length distance from origin
	 * \param[in] pol_angle angle from origin
	 * \return coordinate point
	 */
	CvPoint polar2cart(double pol_length, double pol_angle);
	/*!\fn int sgn(int number)
	 * \brief function to return an integer based on the sign of the input number \a number
	 * \param[in] number an integer number of any sign
	 * \return integer value, 0 if number is 0, -1 if number is negative, 1 if number is positive
	 */
	int sgn(int number);
	/*!\fn void calc_distances(CvPoint current, CvPoint next, bool occ, double &dist, bool &same, double &scale,double &rot, int &detected)
	 * \brief function to calculate all distances for each detected point from frame \a current to frame \a next
	 * \param[in] current current detected coordinate point
	 * \param[in] next detected coordinate point in next frame
	 * \param[in] occ boolean value to determine if point is occluded
	 * \param[in,out] dist distance calculated between two points \a current and \a next
	 * \param[in,out] same boolean value to determine if the points \a current and \a next are exactly the same
	 * \param[in,out] scale scaling value between two frames
	 * \param[in,out] rotation calculated rotation angle betwen two frames
	 * \param[in,out] detected integer value stating whether point has been detected in both frames
	 */
	void calc_distances(CvPoint current, CvPoint next, bool occ, double &dist, bool &same, double &scale,double &rot, int &detected);
	/*!\fn void calc_rotation(CvPoint current, CvPoint next, bool occ, bool same, double dist_current, double dist_next, double &rot,double &scale)
	 * \brief Function to calculate the rotational angles between a point in two consecutive frames
	 * \param[in] current coordinate for current frame
	 * \param[in] next coordinate for next frame
	 * \param[in] occ boolean value to determine if the point is occluded in either frame
	 * \param[in] same boolean value to determine if points \a current and \a next are exactly the same 
	 * \param[in] dist_current distance from coordinate \a current to the origin
	 * \param[in] dist_next distance from coordinate \a next to the origin
	 * \param[in] rot calculated rotation from current frame to next frame between coordinates \a current and \a next
	 * \param[in] scale calculated scaling from current frame to next frame between coordinates \a current and \a next
	 */
	void calc_rotation(CvPoint current, CvPoint next, bool occ, bool same, double dist_current, double dist_next, double &rot,double &scale);
};

#endif

