#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <vector>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include "gms.h"
#include "gmswriter_fixed.h"
#include "fullhand.h"
#include "fullhand_dist.h"

#define NO_OF_HANDS	2
#define NO_OF_DIMENSIONS	3
#define HAND_MARKERS_Z	16


using namespace std;

//opens 2D coordinates from gms file output from FingerDance program. Uses particular reference markers in the hand to create a z value for each marker - a z norm. A new gms file is written incorporating the 3D information with the same name as original file but with '_z' appended to the name.

int ref_frame_no = 0;
int marker = 0;
ofstream fout("SCov_distances.dat");

double L_zest_wrista_P = 0;
double L_zest_wristb_P = 0;
double L_zest_meta_thumb_P = 0;
double L_zest_meta_1_P = 0;
double L_zest_meta_2_P = 0;
double L_zest_meta_3_P = 0;
double L_zest_meta_4_P = 0;
double L_zest_prox_thumb_P = 0;
double L_zest_prox_1_P = 0;
double L_zest_prox_2_P = 0;
double L_zest_prox_3_P = 0;
double L_zest_prox_4_P = 0;
double L_zest_dist_1_P = 0;
double L_zest_dist_2_P = 0;
double L_zest_dist_3_P = 0;
double L_zest_dist_4_P = 0;
double R_zest_wrista_P = 0;
double R_zest_wristb_P = 0;
double R_zest_meta_thumb_P = 0;
double R_zest_meta_1_P = 0;
double R_zest_meta_2_P = 0;
double R_zest_meta_3_P = 0;
double R_zest_meta_4_P = 0;
double R_zest_prox_thumb_P = 0;
double R_zest_prox_1_P = 0;
double R_zest_prox_2_P = 0;
double R_zest_prox_3_P = 0;
double R_zest_prox_4_P = 0;
double R_zest_dist_1_P = 0;
double R_zest_dist_2_P = 0;
double R_zest_dist_3_P = 0;
double R_zest_dist_4_P = 0;


double L_a_dist0 = 0;
double L_b_dist0 = 0;
double L_c_dist0 = 0;
double L_d_dist0 = 0;
double L_f1_dist0 = 0;
double L_f2_dist0 = 0;
double L_f3_dist0 = 0;
double L_f4_dist0 = 0;
double L_g1_dist0 = 0;
double L_g2_dist0 = 0;
double L_g3_dist0 = 0;
double L_g4_dist0 = 0;
double L_h_dist0 = 0;

double R_a_dist0 = 0;
double R_b_dist0 = 0;
double R_c_dist0 = 0;
double R_d_dist0 = 0;
double R_f1_dist0 = 0;
double R_f2_dist0 = 0;
double R_f3_dist0 = 0;
double R_f4_dist0 = 0;
double R_g1_dist0 = 0;
double R_g2_dist0 = 0;
double R_g3_dist0 = 0;
double R_g4_dist0 = 0;
double R_h_dist0 = 0;


bool custom_isnan(double var)
{
	volatile double d = var;
	return d!=d;


}

//function uses reference markers provided by the two wrists markers from each hand and the meta markers for first finger and pinkies to make an estimation of the z value for each marker. frame_buffer is a buffer listing the x,y positions of each marker, frame_buffer_prev is the same thing but for the previous frame of video and gmsfile_buffer is the output array which lists the x, y and z positions of each marker
void zest(int frame_no, double * frame_buffer, double * frame_buffer_prev, double * gmsfile_buffer, int f){

//use very first frame as a starter reference of 250 - as other pixel values are not negative... bear in mind the first frame is usually just after a clap so z values will be very high.
double focus_2 = 830.0 * 830.0 ; 

fout << R_a_dist0 << L_c_dist0<< "\n"; 
//create fullhand_dist objects using first half of buffer for LH and second half for RH
Fullhand_dist rh(true, frame_buffer);
Fullhand_dist lh(false, frame_buffer);
Fullhand_dist rh_prev(true, frame_buffer_prev);
Fullhand_dist lh_prev(false, frame_buffer_prev);

// calculate distances a,b,c and d for new and old buffer frame
rh.calc_distances(); //new frame distances RH
lh.calc_distances(); //new frame distances LH
rh_prev.calc_distances(); //old frame distances RH
lh_prev.calc_distances(); //old frame distances LH
//compare to distances a,b,c,and d for old buffer frame
//rh.print_distances(); 
//lh.print_distances(); 
fout<< rh.a_dist <<"\t"<<rh.b_dist<< "\t"<<rh.c_dist <<"\t"<<rh.d_dist<< "\t"<<rh.f_1st<<"\t"<<rh.f_2nd <<"\t"<<rh.f_3rd<<endl;
fout<< lh.a_dist <<"\t"<<lh.b_dist<< "\t"<<lh.c_dist <<"\t"<<lh.d_dist<< "\t"<<lh.f_1st<<"\t"<<lh.f_2nd <<"\t"<<lh.f_3rd<<endl;

fout<< rh.get1stfing().x <<"\t"<<rh.get1stfing().y<< "\t"<<rh.get2ndfing().x<<"\t"<<rh.get2ndfing().y<< "\t"<<rh.get3rdfing().x<<"\t"<<rh.get3rdfing().y <<endl;

fout<< lh.get1stfing().x <<"\t"<<lh.get1stfing().y<< "\t"<<lh.get2ndfing().x<<"\t"<<lh.get2ndfing().y<< "\t"<<lh.get3rdfing().x<<"\t"<<lh.get3rdfing().y <<endl;
//calculate z position for each meta marker and wrist markers from these distances

/*
for meta markers:
take distances a, b, c and d
1st and 2nd finger : a/b+d/c
3rd and 4th finger: b/a+d/c
left wrist marker: a/b+c/d
right wrist marker: b/a+c/d

*/
double lhmeta1 = (lh.a_dist+lh.d_dist)/2 ;
if (lhmeta1==0)
	{L_zest_meta_1_P = lh.LR_zest_meta_1;}
else
	{ double lha = focus_2 * tan(((((L_a_dist0+L_d_dist0)/2) /1.1) *0.076)* M_PI / 180) / lhmeta1;
	if(!custom_isnan(lha)) {lh.LR_zest_meta_1 = lha;}
	L_zest_meta_1_P = lh.LR_zest_meta_1;}

double lhmeta2 =(lh.a_dist+lh.d_dist)/2 ;
if (lhmeta2==0)
	{L_zest_meta_2_P = lh.LR_zest_meta_2;}
else
	{double lhb =  focus_2 *  tan(((((L_a_dist0+L_d_dist0)/2) /1.1) *0.076)* M_PI / 180)/ lhmeta2 ;
	if(!custom_isnan(lhb)) {lh.LR_zest_meta_2 = lhb;}
	L_zest_meta_2_P = lh.LR_zest_meta_2;}

double lhmeta3 = (lh.b_dist+lh.d_dist)/2;
if (lhmeta3==0)
	{L_zest_meta_3_P = lh.LR_zest_meta_3;}
else
	{double lhc =  focus_2 *  tan(((((L_b_dist0+L_d_dist0)/2) /1.1) *0.076)*M_PI / 180)/ lhmeta3;
	if(!custom_isnan(lhc)) {lh.LR_zest_meta_3 = lhc;}
	L_zest_meta_3_P = lh.LR_zest_meta_3;}

double lhmeta4 = (lh.b_dist+lh.d_dist)/2;
if (lhmeta4==0)
	{L_zest_meta_4_P = lh.LR_zest_meta_4;}
else
	{lh.LR_zest_meta_4 = focus_2 * tan(((((L_b_dist0+L_d_dist0)/2) /1.1) *0.076)* M_PI / 180)/ lhmeta4;
	L_zest_meta_4_P = lh.LR_zest_meta_4;}

double lhmetathumb = lh.h_dist;
if (lhmetathumb==0)
	{L_zest_meta_thumb_P = lh.LR_zest_meta_1;}
else
	{lh.LR_zest_meta_thumb = focus_2 * tan(((L_h_dist0 /1.1) *0.076)* M_PI / 180)/ lhmetathumb;
	L_zest_meta_thumb_P = lh.LR_zest_meta_thumb;}
fout<< "BOOOOOOO"<<lh.LR_zest_meta_1 <<"\t"<<lh.LR_zest_meta_2<< "\t"<< lh.LR_zest_meta_3<<"\t"<<lh.LR_zest_meta_4<<endl;
//cout<< "BOOOOOOO"<<lh.LR_zest_meta_1 <<"\t"<<lh.LR_zest_meta_2<< "\t"<< lh.LR_zest_meta_3<<"\t"<<lh.LR_zest_meta_4<<endl;
double rhmeta1 = (rh.a_dist+rh.d_dist) /2;
if (rhmeta1==0)
	{R_zest_meta_1_P = rh.LR_zest_meta_1;}
else
	{rh.LR_zest_meta_1 = focus_2 *  tan(((((R_a_dist0+R_d_dist0)/2) /1.1) *0.076)* M_PI / 180) / rhmeta1;
	R_zest_meta_1_P = rh.LR_zest_meta_1;}

double rhmeta2 = (rh.a_dist+rh.d_dist) /2;
if (rhmeta2==0)
	{R_zest_meta_2_P = rh.LR_zest_meta_2;}
else
	{rh.LR_zest_meta_2 = focus_2 * tan(((((R_a_dist0+R_d_dist0)/2) /1.1) *0.076)* M_PI / 180) / rhmeta2;
	R_zest_meta_2_P = rh.LR_zest_meta_2;}

double rhmeta3 = (rh.b_dist+rh.d_dist) /2;
if (rhmeta3==0)
	{R_zest_meta_3_P = rh.LR_zest_meta_3;}
else
	{rh.LR_zest_meta_3 = focus_2 * tan((( ((R_b_dist0+R_d_dist0)/2)/1.1) *0.076)* M_PI / 180)/ rhmeta3;
	R_zest_meta_3_P = rh.LR_zest_meta_3;}

double rhmeta4 = (rh.b_dist+rh.d_dist) /2;
if (rhmeta4==0)
	{R_zest_meta_4_P = rh.LR_zest_meta_4;}
else
	{rh.LR_zest_meta_4 = focus_2 *  tan(((((R_b_dist0+R_d_dist0)/2) /1.1) *0.076)* M_PI / 180) /rhmeta4;
	R_zest_meta_4_P = rh.LR_zest_meta_4;}

double rhmetathumb = rh.h_dist ;
if (rhmetathumb==0)
	{R_zest_meta_thumb_P = rh.LR_zest_meta_thumb;}
else
	{rh.LR_zest_meta_thumb = focus_2 *  tan(((R_h_dist0 /1.1) *0.076)* M_PI / 180) / rhmetathumb;
	R_zest_meta_thumb_P = rh.LR_zest_meta_thumb;}
fout<< rh.LR_zest_meta_1 <<"\t"<<rh.LR_zest_meta_2<< "\t"<< rh.LR_zest_meta_3<<"\t"<<rh.LR_zest_meta_4<<endl;

fout<<"\n\n";
/*
for proxal and distal markers:
meta z + change in meta-prox distance or change in prox-dist distance
for each finger, the distance of meta-prox and prox-dist doesnt change except for a change in height.
*/
double lhprox1 = lh.f_1st ;
if(lhprox1!=0) {lh.LR_zest_prox_1 = focus_2 * tan(((L_f1_dist0/1.1)*0.076)* M_PI / 180) / lhprox1;}
double lhprox2 = lh.f_2nd ;
if(lhprox2!=0) {lh.LR_zest_prox_2 = focus_2 * tan(((L_f2_dist0/1.1)*0.076)* M_PI / 180) / lhprox2;}
double lhprox3 = lh.f_3rd ;
if(lhprox3!=0) {lh.LR_zest_prox_3 = focus_2 * tan(((L_f3_dist0/1.1)*0.076)* M_PI / 180) / lhprox3;}
double lhproxp = lh.f_pinkie ;
if(lhproxp!=0) {lh.LR_zest_prox_4 = focus_2 *  tan(((L_f4_dist0/1.1)*0.076)* M_PI / 180) / lhproxp;}
double lhproxt = lh.f_thumb ;
if(lhproxt!=0) {lh.LR_zest_prox_thumb = focus_2 * tan(((L_h_dist0/1.1)*0.076)* M_PI / 180) / lhproxt;}

double rhprox1 = rh.f_1st;
if(rhprox1!=0) {rh.LR_zest_prox_1 = focus_2 *  tan(((R_f1_dist0/1.1)*0.076)* M_PI / 180) / rhprox1;}
double rhprox2 = rh.f_2nd ;
if(rhprox2!=0) {rh.LR_zest_prox_2 = focus_2 *  tan(((R_f2_dist0/1.1)*0.076)* M_PI / 180) / rhprox2;}
double rhprox3 = rh.f_3rd ;
if(rhprox3!=0) {rh.LR_zest_prox_3 = focus_2 * tan(((R_f3_dist0/1.1)*0.076)* M_PI / 180) / rhprox3;}
double rhproxp = rh.f_pinkie;
if(rhproxp!=0) {rh.LR_zest_prox_4 = focus_2 * tan(((R_f4_dist0/1.1)*0.076)* M_PI / 180) / rhproxp;}
double rhproxt = rh.f_thumb ;
if(rhproxt!=0) {rh.LR_zest_prox_thumb = focus_2 * tan(((R_h_dist0/1.1)*0.076)* M_PI / 180) / rhproxt;}

double lhdist1 = lh.g_1st;
if(lhdist1!=0) {lh.LR_zest_dist_1 = focus_2 * tan(((L_g1_dist0/1.1)*0.076)* M_PI / 180) / lhdist1;}
double lhdist2 = lh.g_2nd ;
if(lhdist2!=0) {lh.LR_zest_dist_2 = focus_2 * tan(((L_g2_dist0/1.1)*0.076)* M_PI / 180)/ lhdist2;}
double lhdist3 = lh.g_3rd ;
if(lhdist3!=0) {lh.LR_zest_dist_3 = focus_2 *  tan(((L_g3_dist0/1.1)*0.076)* M_PI / 180) / lhdist3;}
double lhdistp = lh.g_pinkie ;
if(lhdistp!=0) {lh.LR_zest_dist_4 = focus_2 *  tan(((L_g4_dist0/1.1)*0.076)* M_PI / 180) / lhdistp;}

double rhdist1 = rh.g_1st;
if(rhdist1!=0) {rh.LR_zest_dist_1 = focus_2 *  tan(((R_g1_dist0/1.1)*0.076)* M_PI / 180) / rhdist1;}
double rhdist2 = rh.g_2nd ;
if(rhdist2!=0) {rh.LR_zest_dist_2 = focus_2 *  tan(((R_g2_dist0/1.1)*0.076)* M_PI / 180) / rhdist2;}
double rhdist3 = rh.g_3rd ;
if(rhdist3!=0) {rh.LR_zest_dist_3 = focus_2 *  tan(((R_g3_dist0/1.1)*0.076)* M_PI / 180) / rhdist3;}
double rhdistp = rh.g_pinkie ;
if(rhdistp!=0) {rh.LR_zest_dist_4 = focus_2 *  tan(((R_g4_dist0/1.1)*0.076)* M_PI / 180) / rhdistp;}


/*
thumb estimations - meta should depend on the distance between thumb meta and 1st meta as this is a possible physical change.
rule for proxal still stands as the distance meta-prox cannot change except for in height.
*/

//use distances between meta joints and proxal and distal joints to determine their z values. Use z values estimated for meta values.
double lhwrista = (lh.a_dist+lh.c_dist) /2;
if(lhwrista!=0) {lh.LR_zest_wrista = focus_2 * tan(((((L_a_dist0+L_c_dist0)/2)/1.1)*0.076)* M_PI / 180) /lhwrista ;}
double lhwristb = (lh.b_dist+lh.c_dist) /2;
if(lhwristb!=0) {lh.LR_zest_wristb = focus_2 *tan(((((L_b_dist0+L_c_dist0)/2)/1.1)*0.076)* M_PI / 180) / lhwristb ;}
L_zest_wrista_P = lh.LR_zest_wrista;
L_zest_wristb_P = lh.LR_zest_wristb;

double rhwrista = (rh.a_dist+rh.c_dist) /2;
if(rhwrista!=0) {rh.LR_zest_wrista =  focus_2 *tan(((((R_a_dist0+R_c_dist0)/2)/1.1)*0.076)* M_PI / 180) /rhwrista;}
double rhwristb = (rh.b_dist+rh.c_dist) /2;
if(rhwristb!=0) {rh.LR_zest_wristb =  focus_2 *tan(((((R_b_dist0+R_c_dist0)/2)/1.1)*0.076) * M_PI / 180) / rhwristb;}
R_zest_wrista_P = rh.LR_zest_wrista;
R_zest_wristb_P = rh.LR_zest_wristb;


//write new information to gmsfile_buffer info_xyz

	gmsfile_buffer[marker] = lh.getbaseL().x;
	gmsfile_buffer[marker+1] = lh.getbaseL().y;
	gmsfile_buffer[marker+2] = lh.LR_zest_wrista;
	gmsfile_buffer[marker+3] = lh.getbaseR().x;
	gmsfile_buffer[marker+4] = lh.getbaseR().y;
	gmsfile_buffer[marker+5] = lh.LR_zest_wristb;
	gmsfile_buffer[marker+6] = lh.getthumb().x;
	gmsfile_buffer[marker+7] = lh.getthumb().y;
	gmsfile_buffer[marker+8] = lh.LR_zest_meta_thumb;
	gmsfile_buffer[marker+9] = lh.get1stfing().x;
	gmsfile_buffer[marker+10] = lh.get1stfing().y;
	gmsfile_buffer[marker+11] = lh.LR_zest_meta_1;
	gmsfile_buffer[marker+12] = lh.get2ndfing().x;
	gmsfile_buffer[marker+13] = lh.get2ndfing().y;
	gmsfile_buffer[marker+14] = lh.LR_zest_meta_2;
	gmsfile_buffer[marker+15] = lh.get3rdfing().x;
	gmsfile_buffer[marker+16] = lh.get3rdfing().y;
	gmsfile_buffer[marker+17] = lh.LR_zest_meta_3;
	gmsfile_buffer[marker+18] = lh.getpinkie().x;
	gmsfile_buffer[marker+19] = lh.getpinkie().y;
	gmsfile_buffer[marker+20] = lh.LR_zest_meta_4;
	gmsfile_buffer[marker+21] = lh.getthumb_b().x;
	gmsfile_buffer[marker+22] = lh.getthumb_b().y;
	gmsfile_buffer[marker+23] = lh.LR_zest_prox_thumb;
	gmsfile_buffer[marker+24] = lh.get1stfing_b().x;
	gmsfile_buffer[marker+25] = lh.get1stfing_b().y;
	gmsfile_buffer[marker+26] = lh.LR_zest_prox_1;
	gmsfile_buffer[marker+27] = lh.get2ndfing_b().x;
	gmsfile_buffer[marker+28] = lh.get2ndfing_b().y;
	gmsfile_buffer[marker+29] = lh.LR_zest_prox_2;
	gmsfile_buffer[marker+30] = lh.get3rdfing_b().x;
	gmsfile_buffer[marker+31] = lh.get3rdfing_b().y;
	gmsfile_buffer[marker+32] = lh.LR_zest_prox_3;
	gmsfile_buffer[marker+33] = lh.getpinkie_b().x;
	gmsfile_buffer[marker+34] = lh.getpinkie_b().y;
	gmsfile_buffer[marker+35] = lh.LR_zest_prox_4;
	gmsfile_buffer[marker+36] = lh.get1stfing_c().x;
	gmsfile_buffer[marker+37] = lh.get1stfing_c().y;
	gmsfile_buffer[marker+38] = lh.LR_zest_dist_1;
	gmsfile_buffer[marker+39] = lh.get2ndfing_c().x;
	gmsfile_buffer[marker+40] = lh.get2ndfing_c().y;
	gmsfile_buffer[marker+41] = lh.LR_zest_dist_2;
	gmsfile_buffer[marker+42] = lh.get3rdfing_c().x;
	gmsfile_buffer[marker+43] = lh.get3rdfing_c().y;
	gmsfile_buffer[marker+44] = lh.LR_zest_dist_3;
	gmsfile_buffer[marker+45] = lh.getpinkie_c().x;
	gmsfile_buffer[marker+46] = lh.getpinkie_c().y;
	gmsfile_buffer[marker+47] = lh.LR_zest_dist_4;

	gmsfile_buffer[marker+48] = rh.getbaseL().x;
	gmsfile_buffer[marker+49] = rh.getbaseL().y;
	gmsfile_buffer[marker+50] = rh.LR_zest_wrista;
	gmsfile_buffer[marker+51] = rh.getbaseR().x;
	gmsfile_buffer[marker+52] = rh.getbaseR().y;
	gmsfile_buffer[marker+53] = rh.LR_zest_wristb;
	gmsfile_buffer[marker+54] = rh.getthumb().x;
	gmsfile_buffer[marker+55] = rh.getthumb().y;
	gmsfile_buffer[marker+56] = rh.LR_zest_meta_thumb;
	gmsfile_buffer[marker+57] = rh.get1stfing().x;
	gmsfile_buffer[marker+58] = rh.get1stfing().y;
	gmsfile_buffer[marker+59] = rh.LR_zest_meta_1;
	gmsfile_buffer[marker+60] = rh.get2ndfing().x;
	gmsfile_buffer[marker+61] = rh.get2ndfing().y;
	gmsfile_buffer[marker+62] = rh.LR_zest_meta_2;
	gmsfile_buffer[marker+63] = rh.get3rdfing().x;
	gmsfile_buffer[marker+64] = rh.get3rdfing().y;
	gmsfile_buffer[marker+65] = rh.LR_zest_meta_3;
	gmsfile_buffer[marker+66] = rh.getpinkie().x;
	gmsfile_buffer[marker+67] = rh.getpinkie().y;
	gmsfile_buffer[marker+68] = rh.LR_zest_meta_4;
	gmsfile_buffer[marker+69] = rh.getthumb_b().x;
	gmsfile_buffer[marker+70] = rh.getthumb_b().y;
	gmsfile_buffer[marker+71] = rh.LR_zest_prox_thumb;
	gmsfile_buffer[marker+72] = rh.get1stfing_b().x;
	gmsfile_buffer[marker+73] = rh.get1stfing_b().y;
	gmsfile_buffer[marker+74] = rh.LR_zest_prox_1;
	gmsfile_buffer[marker+75] = rh.get2ndfing_b().x;
	gmsfile_buffer[marker+76] = rh.get2ndfing_b().y;
	gmsfile_buffer[marker+77] = rh.LR_zest_prox_2;
	gmsfile_buffer[marker+78] = rh.get3rdfing_b().x;
	gmsfile_buffer[marker+79] = rh.get3rdfing_b().y;
	gmsfile_buffer[marker+80] = rh.LR_zest_prox_3;
	gmsfile_buffer[marker+81] = rh.getpinkie_b().x;
	gmsfile_buffer[marker+82] = rh.getpinkie_b().y;
	gmsfile_buffer[marker+83] = rh.LR_zest_prox_4;
	gmsfile_buffer[marker+84] = rh.get1stfing_c().x;
	gmsfile_buffer[marker+85] = rh.get1stfing_c().y;
	gmsfile_buffer[marker+86] = rh.LR_zest_dist_1;
	gmsfile_buffer[marker+87] = rh.get2ndfing_c().x;
	gmsfile_buffer[marker+88] = rh.get2ndfing_c().y;
	gmsfile_buffer[marker+89] = rh.LR_zest_dist_2;
	gmsfile_buffer[marker+90] = rh.get3rdfing_c().x;
	gmsfile_buffer[marker+91] = rh.get3rdfing_c().y;
	gmsfile_buffer[marker+92] = rh.LR_zest_dist_3;
	gmsfile_buffer[marker+93] = rh.getpinkie_c().x;
	gmsfile_buffer[marker+94] = rh.getpinkie_c().y;
	gmsfile_buffer[marker+95] = rh.LR_zest_dist_4;
	
}



int main(int argc, char**argv){

	 if(argc != 4) {
         fprintf(stderr, "usage : %s <infile name> <ref_frame_no> <output file>\n", argv[0]);
         return EXIT_FAILURE;
     }

	//open file and read contents into a buffer (this part of the program is taken from gms_read.cpp provided on the GMS website
	
	char * file_name = argv[1];
	ref_frame_no = atoi(argv[2]);
	cout<<ref_frame_no<<endl;
 
     gms_uct_struct_t * uct = gms_alloc_uct("no name");
     if(uct == NULL) {
         printf("%s\n", gms_strerror());
         return EXIT_FAILURE;
    }
     gms_file_t * gms_file = gms_open(file_name, uct, GMS_READ);
     if(gms_file == NULL) {
         gms_delete_uct(uct);
         printf("ERROR opening file %s: %s\n", file_name, gms_strerror());
        return EXIT_FAILURE;
     }
 
     puts("SCENE DUMP");
     gms_dump_uct(uct);
     gms_delete_uct(uct);
 
     puts("\nSTORAGE INFO DUMP");
     gms_dump_storage_info(gms_get_storage_info(gms_file));
 
    puts("\nDATA DUMP");
     gms_fcount_t nb_frames = gms_get_nb_frames(gms_file);
    int nb_tracks = gms_get_storage_info(gms_file)->nb_tracks_per_frame;
     // one-frame buffer
     double* buf_read = (double*)calloc(nb_tracks, sizeof(double));
	double* buf_read_prev = (double*)calloc(nb_tracks, sizeof(double));
 
     if(buf_read == NULL) {
        puts("ERROR alloc");
        gms_close(gms_file);
         return EXIT_FAILURE;
     }
 
     if(gms_seek_frame(gms_file, 0, SEEK_SET) == -1) {
        printf("%s\n", gms_strerror());
         gms_close(gms_file);
         return EXIT_FAILURE;
    }
     printf("nb frames = %d\n", nb_frames);

//declaring a buffer for storing all frames of 3d information
	double info_xyz[((nb_frames+1)*HAND_MARKERS_Z*NO_OF_HANDS*NO_OF_DIMENSIONS)];
 	int ref = (ref_frame_no *96) ;
	//marker = marker+ref;
	for(int j=0; j<ref_frame_no ;j++){
		info_xyz[j*3] = 0.0;
		info_xyz[j*3 +1] = 0.0;
		int r = j*3 + 2;
		info_xyz[r] = 830.0;
		//cout<<j+2;
		cout<<j<<"\t"<<r<<"\t";
	}

	
	

    for(int i = 0; i < nb_frames; i++) {
       if(i<ref_frame_no){cout<<"testing this "<<info_xyz[i*3 +2]<<"\n";}
	gms_fcount_t  nb_frames_read;
        nb_frames_read = gms_read_double(gms_file, buf_read, 1);
	if(i==ref_frame_no){
		gms_read_double(gms_file,buf_read_prev,1);
		}
 
       if(nb_frames_read != 1) {
           printf("ERROR reading frame %d\n", i);
         } else {
             printf("frame %d\n", i);
            // for(int j = 0; j < nb_tracks; j++){
              //   printf("   track %d = %g\n", j, buf_read[j]);
             }
        if(i<ref_frame_no){ 
		info_xyz[marker] = buf_read[0];
		info_xyz[marker+1] = buf_read[1];
		info_xyz[marker+2] = 830.0;
		info_xyz[marker+3] = buf_read[2];
		info_xyz[marker+4] = buf_read[3];
		info_xyz[marker+5] = 830.0;
		info_xyz[marker+6] = buf_read[4];
		info_xyz[marker+7] = buf_read[5];
		info_xyz[marker+8] = 830.0;
		info_xyz[marker+9] = buf_read[6];
		info_xyz[marker+10] = buf_read[7];
		info_xyz[marker+11] = 830.0;
		info_xyz[marker+12] = buf_read[8];
		info_xyz[marker+13] = buf_read[9];
		info_xyz[marker+14] = 830.0;
		info_xyz[marker+15] = buf_read[10];
		info_xyz[marker+16] = buf_read[11];
		info_xyz[marker+17] = 830.0;
		info_xyz[marker+18] = buf_read[12];
		info_xyz[marker+19] = buf_read[13];
		info_xyz[marker+20] = 830.0;
		info_xyz[marker+21] = buf_read[14];
		info_xyz[marker+22] = buf_read[15];
		info_xyz[marker+23] = 830.0;
		info_xyz[marker+24] = buf_read[16];
		info_xyz[marker+25] = buf_read[17];
		info_xyz[marker+26] = 830.0;
		info_xyz[marker+27] = buf_read[18];
		info_xyz[marker+28] = buf_read[19];
		info_xyz[marker+29] = 830.0;
		info_xyz[marker+30] = buf_read[20];
		info_xyz[marker+31] = buf_read[21];
		info_xyz[marker+32] = 830.0;
		info_xyz[marker+33] = buf_read[22];
		info_xyz[marker+34] = buf_read[23];
		info_xyz[marker+35] = 830.0;
		info_xyz[marker+36] = buf_read[24];
		info_xyz[marker+37] = buf_read[25];
		info_xyz[marker+38] = 830.0;
		info_xyz[marker+39] = buf_read[26];
		info_xyz[marker+40] = buf_read[27];
		info_xyz[marker+41] = 830.0;
		info_xyz[marker+42] = buf_read[28];
		info_xyz[marker+43] = buf_read[29];
		info_xyz[marker+44] = 830.0;
		info_xyz[marker+45] = buf_read[30];
		info_xyz[marker+46] = buf_read[31];
		info_xyz[marker+47] = 830.0;
	
		info_xyz[marker+48] = buf_read[32];
		info_xyz[marker+49] = buf_read[33];
		info_xyz[marker+50] = 830.0;
		info_xyz[marker+51] = buf_read[34];
		info_xyz[marker+52] = buf_read[35];
		info_xyz[marker+53] = 830.0;
		info_xyz[marker+54] = buf_read[36];
		info_xyz[marker+55] = buf_read[37];
		info_xyz[marker+56] = 830.0;
		info_xyz[marker+57] = buf_read[38];
		info_xyz[marker+58] = buf_read[39];
		info_xyz[marker+59] = 830.0;
		info_xyz[marker+60] = buf_read[40];
		info_xyz[marker+61] = buf_read[41];
		info_xyz[marker+62] = 830.0;
		info_xyz[marker+63] = buf_read[42];
		info_xyz[marker+64] = buf_read[43];
		info_xyz[marker+65] = 830.0;
		info_xyz[marker+66] = buf_read[44];
		info_xyz[marker+67] = buf_read[45];
		info_xyz[marker+68] = 830.0;
		info_xyz[marker+69] = buf_read[46];
		info_xyz[marker+70] = buf_read[47];
		info_xyz[marker+71] = 830.0;
		info_xyz[marker+72] = buf_read[48];
		info_xyz[marker+73] = buf_read[49];
		info_xyz[marker+74] = 830.0;
		info_xyz[marker+75] = buf_read[50];
		info_xyz[marker+76] = buf_read[51];
		info_xyz[marker+77] = 830.0;
		info_xyz[marker+78] = buf_read[52];
		info_xyz[marker+79] = buf_read[53];
		info_xyz[marker+80] = 830.0;
		info_xyz[marker+81] = buf_read[54];
		info_xyz[marker+82] = buf_read[55];
		info_xyz[marker+83] = 830.0;
		info_xyz[marker+84] = buf_read[56];
		info_xyz[marker+85] = buf_read[57];
		info_xyz[marker+86] = 830.0;
		info_xyz[marker+87] = buf_read[58];
		info_xyz[marker+88] = buf_read[59];
		info_xyz[marker+89] = 830.0;
		info_xyz[marker+90] = buf_read[60];
		info_xyz[marker+91] = buf_read[61];
		info_xyz[marker+92] = 830.0;
		info_xyz[marker+93] = buf_read[62];
		info_xyz[marker+94] = buf_read[63];
		info_xyz[marker+95] = 830.0;
		marker+=96;
	}
	//if(i==ref_frame_no)

	//takes one frame of 2D data and sends it to the z-est algorithm along with actual frame number and giant buffer to store everything in. Giant buffer is filled with each frame of data.This is in the format of marker1-x,y,z, marker2-x,y,z etc etc from lh to rh

	if(i==ref_frame_no)
		{cout<<buf_read[0]<<"\t"<<buf_read[1]<<endl;
		Fullhand_dist rh_ref(true, buf_read);
		Fullhand_dist lh_ref(false, buf_read);
		rh_ref.calc_distances();
		lh_ref.calc_distances();
		L_a_dist0 = lh_ref.a_dist;
		L_b_dist0 = lh_ref.b_dist;
		L_c_dist0 = lh_ref.c_dist;
		L_d_dist0 = lh_ref.d_dist;
		L_f1_dist0 = lh_ref.f_1st;
		L_f2_dist0 = lh_ref.f_2nd;
		L_f3_dist0 = lh_ref.f_3rd;
		L_f4_dist0 = lh_ref.f_pinkie;
		L_g1_dist0 = lh_ref.g_1st;
		L_g2_dist0 = lh_ref.g_2nd;
		L_g3_dist0 = lh_ref.g_3rd;
		L_g4_dist0 = lh_ref.g_pinkie;
		L_h_dist0 = lh_ref.h_dist;
	
		R_a_dist0 = rh_ref.a_dist;
		R_b_dist0 = rh_ref.b_dist;
		R_c_dist0 = rh_ref.c_dist;
		R_d_dist0 = rh_ref.d_dist;
		R_f1_dist0 = rh_ref.f_1st;
		R_f2_dist0 = rh_ref.f_2nd;
		R_f3_dist0 = rh_ref.f_3rd;
		R_f4_dist0 = rh_ref.f_pinkie;
		R_g1_dist0 = rh_ref.g_1st;
		R_g2_dist0 = rh_ref.g_2nd;
		R_g3_dist0 = rh_ref.g_3rd;
		R_g4_dist0 = rh_ref.g_pinkie;
		R_h_dist0 = rh_ref.h_dist;
		cout<< rh_ref.a_dist<<endl;
		
		}

	if(i>ref_frame_no)
		{zest(i,buf_read,buf_read_prev, info_xyz, i);
		marker += 96;}

	//stores present buffer as previous buffer
	for(int j=0; j<nb_tracks; j++){
		buf_read_prev[j] = buf_read[j]; 
		}

	}


	//Giant buffer is sent to gms function to convert it into a gms file.
	int fps = 25;
	//creates new gms file for 3D data so with dimension type GMS_XYZ
	const char *scenename = argv[3];
	gms_file_t *handposition_z;
	gms_uct_struct_t* uct2 = Gmswriter::create (scenename, 2, HAND_MARKERS_Z, GMS_POSITION, GMS_XYZ);
	gms_storage_info_t storage2 = Gmswriter::storenew (uct2, handposition_z,scenename,info_xyz,fps,nb_frames, GMS_FLOAT32);

	fout.close();
     free(buf_read);
     gms_close(gms_file);


}

