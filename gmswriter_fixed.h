/* This code is nicked in near entirety from the gms website http.acroe.imag.fr/gms/docs */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
/*! \file gmswriter_fixed.h
 * \brief Documentation for gmswriter_fixed.h
 */
#include <vector>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <sstream>

#include "gms.h"
#include "fullhand.h"


/*! \def  LHWRIST_L_x Integer value for Left hand wrist point (left) x coordinate 0*/
#define LHWRIST_L_x	0
/*! \def  LHWRIST_L_y Integer value for Left hand wrist point (left) y coordinate 1*/
#define LHWRIST_L_y	1
/*! \def  LHWRIST_R_x Integer value for Left hand wrist point (right) x coordinate 2*/
#define LHWRIST_R_x	2
/*! \def  LHWRIST_R_y Integer value for Left hand wrist point (right) y coordinate 3*/
#define LHWRIST_R_y	3
/*! \def  LHTHUMB_M_x Integer value for Left hand thumb metacarpal x coordinate 4*/
#define LHTHUMB_M_x	4
/*! \def  LHTHUMB_M_y Integer value for Left hand thumb metacarpal y coordinate 5*/
#define LHTHUMB_M_y	5
/*! \def  LHFST_M_x Integer value for Left hand first finger metacarpal x coordinate 6*/
#define LHFST_M_x	6
/*! \def  LHFST_M_y Integer value for Left hand first finger metacarpal y coordinate 7*/
#define LHFST_M_y	7
/*! \def  LHSCND_M_x Integer value for Left hand second finger metacarpal x coordinate 8*/
#define LHSCND_M_x	8
/*! \def  LHSCND_M_y Integer value for Left hand second finger metacarpal y coordinate 9*/
#define LHSCND_M_y	9
/*! \def  LHTHRD_M_x Integer value for Left hand third finger metacarpal x coordinate 10*/
#define LHTHRD_M_x	10
/*! \def  LHTHRD_M_y Integer value for Left hand third finger metacarpal y coordinate 11*/
#define LHTHRD_M_y	11
/*! \def  LHPNK_M_x Integer value for Left hand fourth/pinkie finger metacarpal x coordinate 12*/
#define LHPNK_M_x	12
/*! \def  LHPNK_M_y Integer value for Left hand fourth/pinkie finger metacarpal y coordinate 13*/
#define LHPNK_M_y	13
/*! \def  LHTHUMB_P_x Integer value for Left hand thumb proximal joint x coordinate 14*/
#define LHTHUMB_P_x	14
/*! \def  LHTHUMB_P_y Integer value for Left hand thumb proximal joint x coordinate 15*/
#define LHTHUMB_P_y	15
/*! \def  LHFST_P_x Integer value for Left hand first finger proximal joint x coordinate 16*/
#define LHFST_P_x	16
/*! \def  LHFST_P_y Integer value for Left hand first finger proximal joint y coordinate 17*/
#define LHFST_P_y	17
/*! \def  LHSCND_P_x Integer value for Left hand second finger proximal joint x coordinate 18*/
#define LHSCND_P_x	18
/*! \def  LHSCND_P_y Integer value for Left hand second finger proximal joint y coordinate 19*/
#define LHSCND_P_y	19
/*! \def  LHTHRD_P_x Integer value for Left hand third finger proximal joint x coordinate 20*/
#define LHTHRD_P_x	20
/*! \def  LHTHRD_P_y Integer value for Left hand third finger proximal joint x coordinate 21*/
#define LHTHRD_P_y	21
/*! \def  LHPNK_P_x Integer value for Left hand fourth/pinkie finger proximal joint x coordinate 22*/
#define LHPNK_P_x	22
/*! \def  LHPNK_P_y Integer value for Left hand fourth/pinkie finger proximal joint x coordinate 23*/
#define LHPNK_P_y	23
/*! \def  LHFST_D_x Integer value for Left hand first finger distal joint x coordinate 24*/
#define LHFST_D_x	24
/*! \def  LHFST_D_y Integer value for Left hand first finger distal joint y coordinate 25*/
#define LHFST_D_y	25
/*! \def  LHSCND_D_x Integer value for Left hand second finger distal joint x coordinate 26*/
#define LHSCND_D_x	26
/*! \def  LHSCND_D_y Integer value for Left hand second finger distal joint y coordinate 27*/
#define LHSCND_D_y	27
/*! \def  LHTHRD_D_x Integer value for Left hand third finger distal joint x coordinate 28*/
#define LHTHRD_D_x	28
/*! \def  LHTHRD_D_y Integer value for Left hand third finger distal joint y coordinate 29*/
#define LHTHRD_D_y	29
/*! \def  LHPNK_D_x Integer value for Left hand fourth/pinkie finger distal joint x coordinate 30*/
#define LHPNK_D_x	30
/*! \def  LHPNK_D_y Integer value for Left hand fourth/pinkie finger distal joint y coordinate 31*/
#define LHPNK_D_y	31

/*! \def  RHWRIST_L_x Integer value for Right hand wrist point (left) x coordinate 32*/
#define RHWRIST_L_x	32
/*! \def  RHWRIST_L_y Integer value for Right hand wrist point (left) y coordinate 33*/
#define RHWRIST_L_y	33
/*! \def  RHWRIST_R_x Integer value for Right hand wrist point (right) x coordinate 34*/
#define RHWRIST_R_x	34
/*! \def  RHWRIST_R_y Integer value for Right hand wrist point (right) y coordinate 35*/
#define RHWRIST_R_y	35
/*! \def  RHTHUMB_M_x Integer value for Right hand thumb metacarpal x coordinate 36*/
#define RHTHUMB_M_x	36
/*! \def  RHTHUMB_M_y Integer value for Right hand thumb metacarpal y coordinate 37*/
#define RHTHUMB_M_y	37
/*! \def  RHFST_M_x Integer value for Right hand first finger metacarpal x coordinate 38*/
#define RHFST_M_x	38
/*! \def  RHFST_M_y Integer value for Right hand first finger metacarpal y coordinate 39*/
#define RHFST_M_y	39
/*! \def  RHSCND_M_x Integer value for Right hand second finger metacarpal x coordinate 40*/
#define RHSCND_M_x	40
/*! \def  RHSCND_M_y Integer value for Right hand second finger metacarpal y coordinate 41*/
#define RHSCND_M_y	41
/*! \def  RHTHRD_M_x Integer value for Right hand third finger metacarpal x coordinate 42*/
#define RHTHRD_M_x	42
/*! \def  RHTHRD_M_y Integer value for Right hand third finger metacarpal y coordinate 43*/
#define RHTHRD_M_y	43
/*! \def  RHPNK_M_x Integer value for Right hand fourth/pinkie finger metacarpal x coordinate 44*/
#define RHPNK_M_x	44
/*! \def  RHPNK_M_y Integer value for Right hand fourth/pinkie finger metacarpal y coordinate 45*/
#define RHPNK_M_y	45
/*! \def  RHTHUMB_P_x Integer value for Right hand thumb proximal joint x coordinate 46*/
#define RHTHUMB_P_x	46
/*! \def  RHTHUMB_P_y Integer value for Right hand thumb proximal joint y coordinate 47*/
#define RHTHUMB_P_y	47
/*! \def  RHFST_P_x Integer value for Right hand first finger proximal joint x coordinate 48*/
#define RHFST_P_x	48
/*! \def  RHFST_P_y Integer value for Right hand first finger proximal joint y coordinate 49*/
#define RHFST_P_y	49
/*! \def  RHSCND_P_x Integer value for Right hand second finger proximal joint x coordinate 50*/
#define RHSCND_P_x	50
/*! \def  RHSCND_P_y Integer value for Right hand second finger proximal joint y coordinate 51*/
#define RHSCND_P_y	51
/*! \def  RHTHRD_P_x Integer value for Right hand third finger proximal joint x coordinate 52*/
#define RHTHRD_P_x	52
/*! \def  RHTHRD_P_y Integer value for Right hand third finger proximal joint y coordinate 53*/
#define RHTHRD_P_y	53
/*! \def  RHPNK_P_x Integer value for Right hand fourth/pinkie finger proximal joint x coordinate 54*/
#define RHPNK_P_x	54
/*! \def  RHPNK_P_y Integer value for Right hand fourth/pinkie finger proximal joint y coordinate 55*/
#define RHPNK_P_y	55
/*! \def  RHFST_D_x Integer value for Right hand first finger distal joint x coordinate 56*/
#define RHFST_D_x	56
/*! \def  RHFST_D_y Integer value for Right hand first finger distal joint y coordinate 57*/
#define RHFST_D_y	57
/*! \def  RHSCND_D_x Integer value for Right hand second finger distal joint x coordinate 58*/
#define RHSCND_D_x	58
/*! \def  RHSCND_D_y Integer value for Right hand second finger distal joint y coordinate 59*/
#define RHSCND_D_y	59
/*! \def  RHTHRD_D_x Integer value for Right hand third finger distal joint x coordinate 60*/
#define RHTHRD_D_x	60
/*! \def  RHTHRD_D_y Integer value for Right hand third finger distal joint y coordinate 61*/
#define RHTHRD_D_y	61
/*! \def  RHPNK_D_x Integer value for Right hand fourth/pinkie finger distal joint x coordinate 62*/
#define RHPNK_D_x	62
/*! \def  RHPNK_D_y Integer value for Right hand fourth/pinkie finger distal joint y coordinate 63*/
#define RHPNK_D_y	63

using namespace std;
/*! \namespace Gmswriter Functions to create new gmsfiles and data structures, and to fill these with data, also for reading data files.
 */
namespace Gmswriter{
/*! \brief A vector containing CvPoint structures (an x,y coordinate)*/
typedef std::vector <CvPoint> PointVec;

/*!\fn  gms_uct_struct_t * create( const char *name, int nb_units, int nb_channels, gms_channel_type_t info_type ,gms_dimension_t dimension)
 *\brief create new gmsfile
 *\param[in] name filename
 *\param[in] nb_units number of units
 *\param[in] nb_channels number of channels
 *\param[in] info_type what type of data will be stored, options are GMS_POSITION or GMS_FORCE
 *\param[in] dimension dimensions of data to be stored, options are GMS_X, GMS_Y, GMS_Z, GMS_XY, GMS_XZ, GMS_YZ, GMS_XYZ 
 *\return gms structure describing how data is encoded in the file
 For more information on gmsfiles and their structures, see http://acroe.imag.fr/gms/
 */
gms_uct_struct_t * create( const char *name, int nb_units, int nb_channels, gms_channel_type_t info_type ,gms_dimension_t dimension);
/*!\fn gms_storage_info_t storenew(gms_uct_struct_t *uct,gms_file_t* gmsfile,const char *scenename,double *info_xy ,double frame_rate, int nb_frames,gms_data_type_t data_type)
 * \brief Function to write data in array \a info_xy into \a gmsfile
 * \param[in] uct gms structure created from the "create" function
 * \param[in,out] gmsfile Name of gmsfile
 * \param[in] scenename Name of this particular "scene"
 * \param[in] info_xy Array of doubles containing data ordered in correct way
 * \param[in] frame_rate Frame rate of the images
 * \param[in] nb_frames Number of frames that has been recorded
 * \param[in] data_type Encoding of the tracks, options are GMS_FLOAT32, GMS_FLOAT64 or GMS_INT32
 * \return gms structure describing how the data is stored
 */
gms_storage_info_t storenew(gms_uct_struct_t *uct,gms_file_t* gmsfile,const char *scenename,double *info_xy ,double frame_rate, int nb_frames,gms_data_type_t data_type);
/*!\fn gms_storage_info_t storeold(int startframe, gms_uct_struct_t *uct,gms_file_t* gmsfile,const char *scenename,double *info_xy ,double frame_rate, int nb_frames,gms_data_type_t data_type)
 * \brief Function to write data in array \a info_xy into \a gmsfile from frame number \a startframe
 * \param[in] startframe number of starting frame
 * \param[in] uct gms structure created from the "create" function
 * \param[in,out] gmsfile Name of gmsfile
 * \param[in] scenename Name of this particular "scene"
 * \param[in] info_xy Array containing data ordered in correct way
 * \param[in] frame_rate Frame rate of the images
 * \param[in] nb_frames Number of frames that has been recorded
 * \param[in] data_type Encoding of the tracks, options are GMS_FLOAT32, GMS_FLOAT64 or GMS_INT32
 * \return structure that describes how the frames are encoded in the file
 */
gms_storage_info_t storeold(int startframe, gms_uct_struct_t *uct,gms_file_t* gmsfile,const char *scenename,double *info_xy ,double frame_rate, int nb_frames,gms_data_type_t data_type);
/*!\fn void writedata(vector<PointVec> left, vector<PointVec> right, double *info_xy, int startframe_num);
 * \brief Function rearranges data from Left hand object \a left and Right hand object \a right into an array of doubles
 * \param[in] left Vector of CVpoints from left hand object
 * \param[in] right Vector of CVpoints from right hand object
 * \param[out] info_xy array of doubles where data will be arranged
 * \param[in] startframe_num number of frame the data starts from
 */
void writedata(vector<PointVec> left, vector<PointVec> right, double *info_xy, int startframe_num);
/*!\fn void readdata(gms_uct_struct_t *uct, gms_file_t* gms_file, const gms_storage_info_t storage_info)
 * \brief Function to read data from gmsfile. Prints information such as frame rate and structure to the screen
 * \param[in] uct Structure of gms file
 * \param[in] gms_file Name of the gms file to be read
 * \param[in] storage_info structure created in GMSwriter::store
 */
void readdata(gms_uct_struct_t *uct, gms_file_t* gms_file, const gms_storage_info_t storage_info);
/*!\fn void read_all_frames(char *file_name)
 * \brief Function to read particular data from the file \a filename
 */
void read_all_frames(char *file_name);
/*!\fn void read_frames(int start, int end, char *file_name)
 * \brief Function to read particular data from frames \a start to \a end
 * \param[in] start Number of start frame
 * \param[in] end Number of end frame
 * \param[in] file_name Name of file containing data
 */
void read_frames(int start, int end, char *file_name);
/*!\fn void read_velocity(int start, int end, char *file_name)
 * \brief Function to read particular data from frames \a start to \a end
 * \param[in] start Number of start frame
 * \param[in] end Number of end frame
 * \param[in] file_name Name of file containing data
 */
void read_velocity(int start, int end, char *file_name);
/*!\fn void read_movements(int start, int end, char *file_name)
 * \brief Function to read particular data from frames \a start to \a end
 * \param[in] start Number of start frame
 * \param[in] end Number of end frame
 * \param[in] file_name Name of file containing data
 */
void read_movements(int start, int end, char * file_name);
/*!\fn void read_curves(int start, int end, char *file_name)
 * \brief Function to read particular data from frames \a start to \a end
 * \param[in] start Number of start frame
 * \param[in] end Number of end frame
 * \param[in] file_name Name of file containing data
 */
void read_curves(int start, int end,char *file_name); 
/*!\fn calc_vel(CvPoint point_one, CvPoint point_two, double frametime)
 * \brief Function to calculate velocity of particular point
 * \param[in] point_one Point coordinate from first frame
 * \param[in] point_two Point coordinate from second frame
 * \param[in] frametime Time between frames
 */
double calc_vel(CvPoint point_one, CvPoint point_two, double frametime);
/*!\fn calc_dist(CvPoint point_one, CvPoint point_two)
 * \brief Function to calculate distance between two points
 * \param[in] point_one Point coordinates of first point
 * \param[in] point_two Point coordinates of second point
 */
double calc_dist(CvPoint point_one, CvPoint point_two);
}

class GnuStream : public std::ostringstream {
  
    FILE *gnuplot;

public:
  GnuStream();
  ~GnuStream();



};

