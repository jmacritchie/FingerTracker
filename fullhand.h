/*! \file fullhand.h 
 * \brief file describing class fullhand 
 */

#ifndef FULLHAND_H
#define FULLHAND_H


/*this is the two hands:
			 	LEFT				   RIGHT
		           3    2    1 				1   2    3
		    pinkie o    o    o				o   o    o      pinkie 
		(c)    o  							o  (c)
					      
			    o   o   o	      thumb	thumb	 o   o   o
		(b)	o						      o    (b)

			     o  o  o  	   o 		   o	  o  o  o
		(a)	  o	          				    o     (a)
				       o		       o
		(bases)       o   o				   o   o	(bases)



for fingers 2-5, there are three sets of knuckles named (a)-(c) (**and where the numbers are placed are the fingertip points**NOT DONE).
for finger 1 (i.e the thumb) there are just two knuckles and then the fingertip.

*/

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>

/*! \brief A vector containing CvPoint structures (an x,y coordinate) */
typedef std::vector <CvPoint> CoorVec;
/*! \brief An iterator to point towards the CoorVec vector*/
typedef std::vector <CvPoint>::iterator CoorVecIt;
/*! \brief A vector containing doubles */
typedef std::vector <double> DistVec;

/*! \class Fullhand
 * \brief class enabling the setting and retrieval of all points on the hand as well as various functions to group points into metacarpals, proximal and distal points
 * */
class Fullhand{

public:
	/*! \fn Fullhand()
	 * \brief Constructor function for Fullhand object */
	Fullhand();
	/*! \fn Fullhand(bool whichhand)
	 * \param [in] whichhand Boolean value determining either Left or Right hand object to be constructed. True denotes Right hand and False denotes Left hand
	 * \brief Constructor function for Fullhand object which initialises all variables for left or right hand
	 */
	Fullhand(bool whichhand);
	/*! \fn ~Fullhand()
	 * \brief Destructor function for Fullhand object
	 */
	~Fullhand();
	/*! \fn void setallbases(CoorVec &rightgroup, int height, IplImage* prev, IplImage* now, int r_or_l)
	 * \brief Determines the base wrist points from the detected group of blobs \a rightgroup by performing correlation between the previous image \a prev and the current image \a now
	 * \param [in] rightgroup Vector containing coordinates of all detected blobs for this hand object
	 * \param [in] height Height of the image in pixels
	 * \param [in] prev The previous frame
	 * \param [in] now The current frame
	 * \param [in] r_or_l Boolean value denoting whether this is for the right or left hand. 1 is right and 0 is left
	 */
	void setallbases(CoorVec &rightgroup, int height, IplImage* prev, IplImage* now, int r_or_l);
	/*! \fn void setfingers(CoorVec hand, int r_or_l, int width, int height)
	 * \brief Orders the detected blobs in vector \a hand into groups by distance from the detected wrist points. These groups are then themselves ordered by x coordinate and angle from wrist point in order to determine which hand point they belong to
	 * \param [in] hand Vector containing coordinates of detected blobs after wrist points have been detected and removed from this vector
	 * \param [in] r_or_l Integer value denoting Right (1) hand or Left (0) hands
	 * \param [in] width Width of the image
	 * \param [in] height Height of the image
	 */
	void setfingers(CoorVec hand, int r_or_l, int width, int height);
	/*! \fn IplImage* getfingers(IplImage *fgr_grp_img)
	 * \brief Function to draw detected points and their joint connections on an image, including functions in case points are missing
	 * \param [in] fgr_grp_img Image to be drawn on
	 * \return Image with drawings
	 */
	IplImage* getfingers(IplImage *fgr_grp_img);
	/*! \fn IplImage* getfingerlines(IplImage *fgr_grp_img)
	 * \brief Function to draw detected points and their joint connections on an image
	 * \param [in] fgr_grp_img Image to be drawn on
	 * \return Image with drawings
	 */
	IplImage* getfingerlines(IplImage *fgr_grp_img);
	/*! \fn IplImage* drawcrosses(IplImage *fgr_grp_img)
	 * \brief Function to draw detected points on an image
	 * \param [in] fgr_grp_img Image to be drawn on
	 * \return Image with drawings
	 */
	IplImage* drawcrosses(IplImage *fgr_grp_img);
	/*! \fn IplImage* getestimations(IplImage *imahge)
	 * \brief Function to draw circles on estimated points for any lost or occluded markers
	 * \param [in] fgr_grp_img Image to be drawn on
	 * \return Image with drawings
	 */
	IplImage* getestimations(IplImage *imahge);
	/*! \fn CvPoint getbaseL()
	 * \brief Function to return the value of the Left wrist point baseptL
	 * \return CvPoint coordinate of baseptL
	 */
	CvPoint getbaseL(){return baseptL;};
	/*! \fn CvPoint getbaseR()
	 * \brief Function to return the value of the Right wrist point baseptR
	 * \return CvPoint coordinate of baseptR
	 */
	CvPoint getbaseR(){return baseptR;};
	/*! \fn CvPoint getthumb()
	 * \brief Function to return the value of the thumb point
	 * \return CvPoint coordinate of thumb
	 */
	CvPoint getthumb(){return thumb;};
	/*! \fn CvPoint get1stfing()
	 * \brief Function to return the value of the 1st finger metacarpal
	 * \return CvPoint coordinate of fstfinger
	 */
	CvPoint get1stfing(){return fstfinger;};
	/*! \fn CvPoint get2ndfing()
	 * \brief Function to return the value of the 2nd finger metacarpal
	 * \return CvPoint coordinate of scndfinger
	 */
	CvPoint get2ndfing(){return scndfinger;};
	/*! \fn CvPoint get3rdfing()
	 * \brief Function to return the value of the 3rd finger metacarpal
	 * \return CvPoint coordinate of thrdfinger
	 */
	CvPoint get3rdfing(){return thrdfinger;};
	/*! \fn CvPoint getpinkie()
	 * \brief Function to return the value of the pinkie finger metacarpal
	 * \return CvPoint coordinate of pinkie
	 */
	CvPoint getpinkie(){return pinkie;};
	/*! \fn CvPoint getthumb_b()
	 * \brief Function to return the value of the thumb proximal joint
	 * \return CvPoint coordinate of thumb_b
	 */
	CvPoint getthumb_b(){return thumb_b;};
	/*! \fn CvPoint get1stfing_b()
	 * \brief Function to return the value of the 1st finger proximal joint
	 * \return CvPoint coordinate of fstfinger_b
	 */
	CvPoint get1stfing_b(){return fstfinger_b;};
	/*! \fn CvPoint get2ndfing_b()
	 * \brief Function to return the value of the 2nd finger proximal joint
	 * \return CvPoint coordinate of scndfinger_b
	 */
	CvPoint get2ndfing_b(){return scndfinger_b;};
	/*! \fn CvPoint get3rdfing_b()
	 * \brief Function to return the value of the 3rd finger proximal joint
	 * \return CvPoint coordinate of thrdfinger_b
	 */
	CvPoint get3rdfing_b(){return thrdfinger_b;};
	/*! \fn CvPoint getpinkie_b()
	 * \brief Function to return the value of the pinkie proximal joint
	 * \return CvPoint coordinate of pinkie_b
	 */
	CvPoint getpinkie_b(){return pinkie_b;};
	/*! \fn CvPoint get1stfing_c()
	 * \brief Function to return the value of the 1st finger distal joint
	 * \return CvPoint coordinate of fstfinger_c
	 */
	CvPoint get1stfing_c(){return fstfinger_c;};
	/*! \fn CvPoint get2ndfing_c()
	 * \brief Function to return the value of the 2nd finger distal joint
	 * \return CvPoint coordinate of scndfinger_c
	 */
	CvPoint get2ndfing_c(){return scndfinger_c;};
	/*! \fn CvPoint get3rdfing_c()
	 * \brief Function to return the value of the 3rd finger distal joint
	 * \return CvPoint coordinate of thrdfinger_c
	 */
	CvPoint get3rdfing_c(){return thrdfinger_c;};
	/*! \fn CvPoint getpinkie_c()
	 * \brief Function to return the value of the pinkie distal joint
	 * \return CvPoint coordinate of pinkie_c
	 */
	CvPoint getpinkie_c(){return pinkie_c;};
	/*! \fn void setbaseptL(CvPoint a1)
	 * \brief Function to set the value of the Left wrist point baseptL
	 * \param [in] a1 coordinate to set baseptL to
	 */
	void setbaseptL(CvPoint a1){baseptL = a1;};;
	/*! \fn void setbaseptR(CvPoint a2)
	 * \brief Function to set the value of the Right wrist point baseptR
	 * \param [in] a1 coordinate to set baseptR to
	 */
	void setbaseptR(CvPoint a2){baseptR = a2;};;
	/*! \fn void setthumb(CvPoint b)
	 * \brief Function to set the value of the thumb point thumb
	 * \param [in] a1 coordinate to set thumb to
	 */
	void setthumb(CvPoint b){thumb = b;};
	/*! \fn void set1stfing(CvPoint c)
	 * \brief Function to set the value of the 1st finger metacarpal
	 * \param [in] a1 coordinate to set fstfinger to
	 */
	void set1stfing(CvPoint c){fstfinger = c;};
	/*! \fn void set2ndfing(CvPoint d)
	 * \brief Function to set the value of the 2nd finger metacarpal
	 * \param [in] a1 coordinate to set scndfinger to
	 */
	void set2ndfing(CvPoint d){scndfinger = d;};
	/*! \fn void set3rdfing(CvPoint e)
	 * \brief Function to set the value of the 3rd finger metacarpal
	 * \param [in] a1 coordinate to set thrdfinger to
	 */
	void set3rdfing(CvPoint e){thrdfinger = e;};
	/*! \fn void setpinkie(CvPoint f)
	 * \brief Function to set the value of the pinkie metacarpal
	 * \param [in] a1 coordinate to set pinkie to
	 */
	void setpinkie(CvPoint f){pinkie = f;};
	/*! \fn void setthumb_b(CvPoint g)
	 * \brief Function to set the value of the thumb proximal joint
	 * \param [in] a1 coordinate to set thumb_b to
	 */
	void setthumb_b(CvPoint g){thumb_b = g;};
	/*! \fn void set1stfing_b(CvPoint h)
	 * \brief Function to set the value of the 1st finger proximal joint
	 * \param [in] a1 coordinate to set fstfinger_b to
	 */
	void set1stfing_b(CvPoint h){fstfinger_b = h;};
	/*! \fn void set2ndfing_b(CvPoint i)
	 * \brief Function to set the value of the 2nd finger proximal joint
	 * \param [in] a1 coordinate to set scndfinger_b to
	 */
	void set2ndfing_b(CvPoint i){scndfinger_b = i;};
	/*! \fn void set3rdfing_b(CvPoint j)
	 * \brief Function to set the value of the 3rd finger proximal joint
	 * \param [in] a1 coordinate to set thrdfinger_b to
	 */
	void set3rdfing_b(CvPoint j){thrdfinger_b = j;};
	/*! \fn void setpinkie_b(CvPoint k)
	 * \brief Function to set the value of the pinkie proximal joint
	 * \param [in] a1 coordinate to set pinkie_b to
	 */
	void setpinkie_b(CvPoint k){pinkie_b = k;};
	/*! \fn void set1stfing_c(CvPoint l)
	 * \brief Function to set the value of the 1st finger distal joint
	 * \param [in] a1 coordinate to set fstfinger_c to
	 */
	void set1stfing_c(CvPoint l){fstfinger_c = l;};
	/*! \fn void set2ndfing_c(CvPoint m)
	 * \brief Function to set the value of the 2nd finger distal joint
	 * \param [in] a1 coordinate to set scndfinger_c to
	 */
	void set2ndfing_c(CvPoint m){scndfinger_c = m;};
	/*! \fn void set3rdfing_c(CvPoint n)
	 * \brief Function to set the value of the 3rd finger distal joint
	 * \param [in] a1 coordinate to set thrdfinger_c to
	 */
	void set3rdfing_c(CvPoint n){thrdfinger_c = n;};
	/*! \fn void setpinkie_c(CvPoint o)
	 * \brief Function to set the value of the pinkie distal joint
	 * \param [in] a1 coordinate to set pinkie_c to
	 */
	void setpinkie_c(CvPoint o){pinkie_c = o;};
	/*! \fn CoorVec setpoints_wrt_org()
	 * \brief Function to set all the member coordinates of the Fullhand object with respect to an origin point midway between the two wrist markers set to (0,0)
	 * \return vector containing all recalculated member coordinates Fullhand object 
	 */
	CoorVec setpoints_wrt_org();
	/*! \fn DistVec distances_to_org(CoorVec fingers)
	 * \brief Function to calculate distances between each member point and the origin
	 * \param [in] fingers Vector containing recalculated positions of each member point (output from setpoints_wrt_org function)
	 * \return Vector \a DistVec containing distances between each member point and the origin.
	 */
	DistVec distances_to_org(CoorVec fingers);
	
	/*! \fn void metaset(CoorVec joints, bool r_or_l)
	 * \brief Function to set the points of the metacarpals of the hand from the input vector \a joints. If input vector has more elements than the number expected, function calculates distance between previous point and each point in the vector to find the detected point for this frame.
	 * \param [in,out] joints Vector containing suspected members of metacarpals of the hand, unordered
	 * \param [in] r_or_l Boolean value denoting which hand the points belong to. Left(false) or right(true).
	 */
	void metaset(CoorVec joints, bool r_or_l);
	/*! \fn void proxset(CoorVec joints, bool r_or_l)
	 * \brief Function to set the points of the proximal joints of the hand from the input vector \a joints. If input vector has more elements than the number expected, function calculates distance between previous point and each point in the vector to find the detected point for this frame.
	 * \param [in,out] joints Vector containing suspected members of proximal joints of the hand, unordered
	 * \param [in] r_or_l Boolean value denoting which hand the points belong to. Left(false) or right(true).
	 */
	void proxset(CoorVec joints, bool r_or_l);
	/*! \fn void distset(CoorVec joints, bool r_or_l)
	 * \brief Function to set the points of the distal joints of the hand from the input vector \a joints. If input vector has more elements than the number expected, function calculates distance between previous point and each point in the vector to find the detected point for this frame.
	 * \param [in,out] joints Vector containing suspected members of distal joints of the hand, unordered
	 * \param [in] r_or_l Boolean value denoting which hand the points belong to. Left(false) or right(true).
	 */
	void distset(CoorVec joints, bool r_or_l);
	/*!\fn bool sort(CoorVec &list, CvPoint &finger)
	 * \brief Calculates distances between each point in vector \a list and the point \a finger. The point in vector \a list with the shortest distance calculated is assigned as the newly detected point of \a finger. 
	 * \param [in] list Vector containing suspected points which would be the new value of \a finger.
	 * \param [in] finger Previous detected value of specific finger points
	 * \return Boolean value true = new point has been assigned. false = no new point has been assigned.
	 */
	bool sort(CoorVec &list, CvPoint &finger);

	/*! \fn void checkgroups(CoorVec &proxal, CoorVec &distal, int width)
	 * \brief Function to check whether any points in each joint group should belong to another based on their distance from the wrist points of the hand. Reassigns points to the other vector if necessary.
	 * \param[in,out] proxal Vector containing points believed to be in the proxal joint group
	 * \param[in,out] distal Vector containing points believed to be in the distal joint group
	 * \param[in] width Width of the frame in pixels e.g. 216
	 */
	void checkgroups(CoorVec &proxal, CoorVec &distal, int width);
	/*!\fn void recordGroupsDistances(void)
	 * \brief Function to record all distances from detected markers to virtual base points
	 */
	void recordGroupDistances(void);
	/*!\fn double angle(CvPoint thepoint, CvPoint base)
	 * \brief Function to calculate the angle between a detected point and the virtual base point
	 * \param[in] thepoint Detected point coordinates
	 * \param[in] base Virtual base point coordinates
	 * \return Calculated angle
	 */
	double angle(CvPoint thepoint, CvPoint base);
	/*!\fn double distance(CvPoint thepoint, CvPoint base)
	 * \brief Function to calculate the distance between two detected points
	 * \param[in] one First detected point coordinates
	 * \param[in] two Second detected point coordinates
	 * \return Calculated distance
	 */
	double distance(CvPoint one, CvPoint two);
	
	bool r_or_l; /*! Boolean value denoting right or left hand. Right=true, Left=false *///true is for right hand, false is for left hand 
	bool lost_baseL;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_baseR;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool occ_thumb;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool hid_pinkie;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_fst;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_scnd;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_thrd;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool occ_thumb_b;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool hid_pinkie_b;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_fst_b;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_scnd_b;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_thrd_b;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	bool lost_fst_c, lost_scnd_c, lost_thrd_c, hid_pinkie_c;/*!Boolean value denoting whether this point is occluded/lost true=lost, false=detected*/
	
	CvPoint est_thumb;/*!CvPoint for estimated position of this point */
	CvPoint est_first;/*!CvPoint for estimated position of this point */
	CvPoint est_second;/*!CvPoint for estimated position of this point */
	CvPoint est_third;/*!CvPoint for estimated position of this point */
	CvPoint est_pinkie;/*!CvPoint for estimated position of this point */
	CvPoint est_thumb_b;/*!CvPoint for estimated position of this point */
	CvPoint est_first_b;/*!CvPoint for estimated position of this point */
	CvPoint est_second_b;/*!CvPoint for estimated position of this point */
	CvPoint est_third_b;/*!CvPoint for estimated position of this point */
	CvPoint est_pinkie_b;/*!CvPoint for estimated position of this point */
	CvPoint est_thumb_c, est_first_c, est_second_c, est_third_c, est_pinkie_c;/*!CvPoint for estimated position of this point */
	
protected:

	CvPoint baseptL, baseptR;
	CvPoint thumb;
	CvPoint fstfinger;
	CvPoint scndfinger;
	CvPoint thrdfinger;
	CvPoint pinkie;
	CvPoint thumb_b, fstfinger_b, scndfinger_b, thrdfinger_b, pinkie_b;
	CvPoint fstfinger_c, scndfinger_c, thrdfinger_c, pinkie_c;

	CvPoint org_basept,org_baseptL, org_baseptR;
	CvPoint org_thumb;
	CvPoint org_fstfinger;
	CvPoint org_scndfinger;
	CvPoint org_thrdfinger;
	CvPoint org_pinkie;
	CvPoint org_thumb_b, org_fst_b, org_scnd_b, org_thrd_b, org_pinkie_b;
	CvPoint org_fst_c, org_scnd_c, org_thrd_c, org_pinkie_c;
	
	double meta_mindistance;
	double meta_maxdistance;
	double prox_maxdistance;
	double dist_maxdistance;

};
#endif
