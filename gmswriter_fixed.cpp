/* This code is nicked in near entirety from the gms website http.acroe.imag.fr/gms/docs */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <vector>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <fstream>

#include "gms.h"
#include "gmswriter_fixed.h"

using namespace std;


gms_uct_struct_t * Gmswriter::create(const char *name, int nb_units, int nb_channels, gms_channel_type_t info_type ,gms_dimension_t dimension){

//allocate a new and empty scene
gms_uct_struct_t* uct;
uct = gms_alloc_uct (name );

//add unit to the scene
for (int i=0;i<nb_units;i++){//Units are each hand object
	const char* UnitName;
	if(i==0){
		if (gms_add_unit (uct,"Unit 1 Hand 1")) {
		cout<< "error adding unit "<<i<<"\n";}
		}
	if(i==1){
		if (gms_add_unit (uct,"Unit 2 Hand 2")) {
		cout<< "error adding unit "<<i<<"\n";}
		}
	if(i==2){
		if (gms_add_unit (uct,"Unit 3 Hand 3")) {
		cout<< "error adding unit "<<i<<"\n";}
		}

	for (int j=0;j<nb_channels;j++){		//Channels will be fingers and each channel will have two tracks for x and y coordinates
	const char* ChannelName;

	if(j==0){
		if (gms_add_channel (uct,"Channel 1 Base point L", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==1){
		if (gms_add_channel (uct,"Channel 2 Base point R", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==2){
		if (gms_add_channel (uct,"Channel 3 Thumb ", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==3){
		if (gms_add_channel (uct,"Channel 4 First finger", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==4){
		if (gms_add_channel (uct,"Channel 5 Second finger", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==5){
		if (gms_add_channel (uct,"Channel 6 Third finger", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==6){
		if (gms_add_channel (uct,"Channel 7 Pinkie", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==7){
		if (gms_add_channel (uct,"Channel 8 Thumb_b ", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==8){
		if (gms_add_channel (uct,"Channel 9 First finger_b", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==9){
		if (gms_add_channel (uct,"Channel 10 Second finger_b", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==10){
		if (gms_add_channel (uct,"Channel 11 Third finger_b", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==11){
		if (gms_add_channel (uct,"Channel 12 Pinkie_b", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==12){
		if (gms_add_channel (uct,"Channel 13 First finger_c", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==13){
		if (gms_add_channel (uct,"Channel 14 Second finger_c", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==14){
		if (gms_add_channel (uct,"Channel 15 Third finger_c", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}		
	if(j==15){
		if (gms_add_channel (uct,"Channel 16 Pinkie_c", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==16){
		if (gms_add_channel (uct,"Channel 17 Unknown", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}
	if(j==17){
		if (gms_add_channel (uct,"Channel 18 Unknown", info_type, dimension)) {
		cout<< "error adding channel "<<j<<"\n";}
		}

	}
  }
  return (uct);
}

gms_storage_info_t Gmswriter::storenew(gms_uct_struct_t *uct,gms_file_t* gmsfile,const char *scenename,double *info_xy,double frame_rate, int nb_frames, gms_data_type_t data_type){

	//set gms_storage_info struct information
	gms_storage_info_t storage_info;
	storage_info.frame_rate = frame_rate;
	storage_info.data_type = data_type;
	storage_info.scale_factor = 1;
	storage_info.block_size = 0;
	storage_info.nb_tracks_per_frame = 2;


	//create a gms file - these must be closed with gms_close()
	gmsfile = gms_create (scenename, uct, &storage_info) ;
	gms_fcount_t nb_frames_to_write=nb_frames;

	//write a given number of frames using a buffer of doubles
	gms_fcount_t nb_frames_written = gms_write_double (gmsfile, info_xy, nb_frames_to_write ) ;
	cout << "Number of frames written is "<< nb_frames_written <<"\n";cout.flush();
	
	gms_close(gmsfile);

	return (storage_info);

}

gms_storage_info_t Gmswriter::storeold(int startingframe,gms_uct_struct_t *uct,gms_file_t* gmsfile,const char *scenename,double *info_xy,double frame_rate, int nb_frames, gms_data_type_t data_type){

	//set gms_storage_info struct information
	gms_storage_info_t storage_info;
	storage_info.frame_rate = frame_rate;
	storage_info.data_type = data_type;
	storage_info.scale_factor = 1;
	storage_info.block_size = 0;
	storage_info.nb_tracks_per_frame = 2;


	gms_uct_struct_t *uct2;
	gms_file_t* hands;
	//create a gms file - these must be closed with gms_close()
	if (hands = gms_open(scenename, NULL, GMS_READWRITE)) {cout<<"hooray! file is opened!\n";}
	else {cout<<"boo!";} cout.flush();

	gms_fcount_t nb_frames_to_write=nb_frames;
	//write a given number of frames using a buffer of doubles	
	//move file pointer to startframe
	gms_fcount_t offset = gms_seek_frame(hands, startingframe, SEEK_SET);
	cout <<offset; cout.flush();
	if(offset==startingframe){ 
	  cout<< "successfully moved file pointer\n"; cout.flush();
	  gms_fcount_t nb_frames_written = gms_write_double (hands, info_xy, nb_frames_to_write ) ;
	  cout << "Number of frames written is "<< nb_frames_written <<"\n";cout.flush();
	}
	else
	{cout << "problem moving file pointer"; }
	
	gms_close(hands);

	return (storage_info);

}


//data function takes vector of cvpoints and converts to streams of doubles in such a way the info will be encoded as two tracks (x,y) per channel (each point). it takes the startframe that the program starts tracking so it can align the frames written in the gms file with the frames of the video - i.e. the first number of frames will be stored as 0 or NULL? until the track is initiated. eventually each unit will represent a hand each...

void Gmswriter::writedata(vector<PointVec > left, vector<PointVec > right,double *info_xy, int startframe_nums){

	//gms file buffers need to be written frame by frame. Frame (unit(channel(track)))

	//assuming start frame is 0
	/*number of units is number of hands = 2
	number of channels is number of points = 6
	tracks is 2 for x and y coordinates*/

	int nb_units = 2;
	int nb_channels = left.size(); cout<<"channels is "<<nb_channels;
	int tracks = 2;
	int frames = left[0].size(); cout<<"frames is "<<frames<<endl;
	int marker = 0;
	for(int f=0; f<frames;f++){
		for(int u=0; u<nb_units;u++){
			for(int ch=0; ch<nb_channels; ch++){
				for(int tr=0; tr<tracks; tr++){
				//what hand info is taken from depends on which unit it is u=0 is left hand u=1 is right hand
				CvPoint coor = cvPoint(0,0);
				int coor_part = 0;
				if(u==0){
					coor = left[ch][f];
				}
				else{
					coor = right[ch][f];
				}
				//what specific part of the coordinate taken depends on track number. tr=0 is x and tr=1 is y
				if(tr==0){
					coor_part = coor.x;
				}
				else{
					coor_part = coor.y;
				}

				//setting points to array
				info_xy[marker]=coor_part;
				marker++;
				}
			}
		}
	}
}

void Gmswriter::readdata(gms_uct_struct_t *uct, gms_file_t * gms_file,const gms_storage_info_t storage_info){

	gms_dump_uct(uct);
	cout << "\n\n";
	gms_dump_storage_info(&storage_info );
	cout << "\n\n";
	//for a better read out of a gms file use program gmsread.
}


void Gmswriter::read_frames(int start, int end, char * file_name){

	int frames2read = end-start;
	gms_uct_struct_t * uct = gms_alloc_uct("no name");
	gms_file_t * gms_file = gms_open(file_name, uct, GMS_READ);
		
	gms_fcount_t frames = gms_get_nb_frames(gms_file);
	int nb_tracks = gms_get_storage_info(gms_file)->nb_tracks_per_frame;

	double buf_read [100000];

	if(gms_seek_frame(gms_file, start, SEEK_SET) == -1) {
         	printf("%s\n", gms_strerror());
        	 gms_close(gms_file);
        }

	//runs through each tracks positional data for all frames recorded e.g. 1st track frame 0 - 30, 2nd track frame 0-30. newlines are put in so that gnuplot "lifts the pen" between different tracks' data.
	ofstream fout("JChan_finale_LHwristL_3Db.dat");
	//for(int k=5;k<nb_tracks+1;k++){
	for(int k=2;k<4;k++){//seems to not enjoy reading several tracks of data - only reads accurately when it is one or two streams?!
		
		for(int i = 0; i < frames2read; i++) {
        	 gms_fcount_t  nb_frames_read;
        	 nb_frames_read = gms_read_double(gms_file, buf_read, 1);
            		 for(int j = k; j <(k+1); j++){
				int offset = 0;
				fout<<i <<"\t"<<buf_read[j-2] <<"\t" << buf_read[j-1]<<"\t" << buf_read[j];		
            		 } fout<<"\n";
        	 }k++; fout<<endl<<endl;
	}fout.close();

	gms_close(gms_file);
}

void Gmswriter::read_velocity(int start, int end, char * file_name){

	int frames2read = end-start;
	double duration = 0.01818182; //this value should be changed depending on the frame rate

	gms_uct_struct_t * uct = gms_alloc_uct("no name");
	gms_file_t * gms_file = gms_open(file_name, uct, GMS_READ);
	
	gms_dump_uct(uct);
	gms_dump_storage_info(gms_get_storage_info(gms_file));	
	
	gms_fcount_t frames = gms_get_nb_frames(gms_file);
	int nb_tracks = gms_get_storage_info(gms_file)->nb_tracks_per_frame;

	double buf_read [10000];

	if(gms_seek_frame(gms_file, start, SEEK_SET) == -1) {
         	printf("%s\n", gms_strerror());
        	 gms_close(gms_file);
        	//return EXIT_FAILURE;
     	}

	ofstream fout("velocities.dat");
	//store position one and position two for every frame. at the end of each frame, position two becomes position one
	//for the first frame dont calculate velocity
	CvPoint first_pos = cvPoint(0,0);
	CvPoint second_pos = cvPoint(0,0);

	for(int i = 0; i < frames2read; i++) {
	double frametime = i*duration;
	first_pos = second_pos;
         gms_fcount_t  nb_frames_read;
         nb_frames_read = gms_read_double(gms_file, buf_read, 1);
 
        if(nb_frames_read != 1) {
             printf("ERROR reading frame %d\n", i);
         } else {
 
             for(int j = 17; j < 18; j++){
		//store position in second_pos
		//fout<< buf_read[j-1] <<"\t" << buf_read[j]<<endl;
		second_pos = cvPoint((int)buf_read[j-1],(int)buf_read[j]);
             }
         }
	//for each frame, calculate the velocity and put to the fstream
	if(i>0){
		double velocity = calc_vel(first_pos, second_pos, frametime);
		fout << i << "\t" << velocity<<endl;
		}	
	} 

	fout.close();
	
	string outfilename = "vel_graph";
	FILE *gnu=popen( "/usr/bin/gnuplot", "w" );
	fprintf(gnu,"set terminal png\n");
	fprintf(gnu,"set output \"vel_graph.png\" \n");
	fprintf(gnu,"plot 'velocities.dat' using 1:2 with lines\n");
	fflush (gnu);
	while(1);

	gms_close(gms_file);


}

void Gmswriter::read_movements(int start, int end, char * file_name){

	int frames2read = end-start;
	double duration = 0.04; //again this should be changed depending on frame rate

	gms_uct_struct_t * uct = gms_alloc_uct("no name");
	gms_file_t * gms_file = gms_open(file_name, uct, GMS_READ);
	
	gms_dump_uct(uct);
	gms_dump_storage_info(gms_get_storage_info(gms_file));	
	
	gms_fcount_t frames = gms_get_nb_frames(gms_file);
	int nb_tracks = gms_get_storage_info(gms_file)->nb_tracks_per_frame;

	double buf_read [10000];

	if(gms_seek_frame(gms_file, start, SEEK_SET) == -1) {
         	printf("%s\n", gms_strerror());
        	 gms_close(gms_file);
        	//return EXIT_FAILURE;
     	}

	ofstream fout("movements.dat");
	//store position one and position two for every frame. at the end of each frame, position two becomes position one
	//for the first frame dont calculate velocity
	CvPoint first_pos = cvPoint(0,0);
	CvPoint second_pos = cvPoint(0,0);

	for(int i = 0; i < frames2read; i++) {
	double frametime = i*duration;
	first_pos = second_pos;
         gms_fcount_t  nb_frames_read;
         nb_frames_read = gms_read_double(gms_file, buf_read, 1);
 
        if(nb_frames_read != 1) {
             printf("ERROR reading frame %d\n", i);
         } else {
 
             for(int j = 17; j < 18; j++){
		//store position in second_pos
		//fout<< buf_read[j-1] <<"\t" << buf_read[j]<<endl;
		second_pos = cvPoint((int)buf_read[j-1],(int)buf_read[j]);
             }
         }
	//for each frame, calculate the velocity and put to the fstream
	if(i>0){
		double dist = calc_dist(first_pos, second_pos);
		//cout<<"vel "<<frametime<<endl;
		fout << i << "\t" << dist<<endl;
		}	

	} 

	fout.close();
	
	string outfilename = "move_graph";
	FILE *gnu=popen( "/usr/bin/gnuplot", "w" );
	fprintf(gnu,"set terminal png\n");
	fprintf(gnu,"set output \"move_graph.png\" \n");
	fprintf(gnu,"plot 'movements.dat' using 1:2 with lines\n");
	fflush (gnu);
	while(1);

	gms_close(gms_file);


}

void Gmswriter::read_curves(int start, int end, char *file_name){
//plotting the relative curvature between the different parts of whichfinger
	int frames2read = end-start;
	gms_uct_struct_t * uct = gms_alloc_uct("no name");
	gms_file_t * gms_file = gms_open(file_name, uct, GMS_READ);
	
	gms_dump_uct(uct);
	gms_dump_storage_info(gms_get_storage_info(gms_file));	
	
	gms_fcount_t frames = gms_get_nb_frames(gms_file);
	int nb_tracks = gms_get_storage_info(gms_file)->nb_tracks_per_frame;

	double buf_read [10000];

	if(gms_seek_frame(gms_file, start, SEEK_SET) == -1) {
         	printf("%s\n", gms_strerror());
        	 gms_close(gms_file);
        	//return EXIT_FAILURE;
     	}

	ofstream fout("curves.dat");

	for(int i = 0; i < frames2read; i++) {
	
         gms_fcount_t  nb_frames_read;
         nb_frames_read = gms_read_double(gms_file, buf_read, 1);
 
	CvPoint base = cvPoint(0,0);
	CvPoint meta = cvPoint(0,0);
	CvPoint prox = cvPoint(0,0);
	CvPoint dist = cvPoint(0,0);

        if(nb_frames_read != 1) {
             printf("ERROR reading frame %d\n", i);
         } else {
 
		base = cvPoint(buf_read[0],buf_read[1]);
		meta = cvPoint(buf_read[2],buf_read[3]);
 		prox = cvPoint(buf_read[12],buf_read[13]);
         	}
	//for each frame, calculate distances between joints and put to the fstream
	
		double dist_bm = calc_dist(base,meta);
		double dist_mp = calc_dist(meta,prox);
		double dist_pd = calc_dist(prox,dist);
		fout << i << "\t" << dist_bm <<endl;
		

	} 

	fout.close();
	
	string outfilename = "move_graph";
	FILE *gnu=popen( "/usr/bin/gnuplot", "w" );
	fprintf(gnu,"set terminal png\n");
	fprintf(gnu,"set output \"curve_graph.png\" \n");
	fprintf(gnu,"plot 'curves.dat' using 1:2 with lines\n");
	fflush (gnu);
	while(1);

	gms_close(gms_file);


}

void Gmswriter::read_all_frames(char *file_name){

	gms_uct_struct_t * uct = gms_alloc_uct("no name");
	gms_file_t * gms_file = gms_open(file_name, uct, GMS_READ);
	//gms_file_t * handposition = argv[1];
	gms_dump_uct(uct);
	gms_dump_storage_info(gms_get_storage_info(gms_file));	
	//gms_dump_storage_info(&storage_info );
	//Gmswriter::readdata(uct,handposition,storage);
	gms_fcount_t frames = gms_get_nb_frames(gms_file);
	cout<<frames<<endl;
	int nb_tracks = gms_get_storage_info(gms_file)->nb_tracks_per_frame;
	//cout <<" number of tracks is "<<nb_tracks<< "\t number of frames is "<<frames<<endl; cout.flush();

	double buf_read [100000];
	ofstream fout("hand_positions.dat");
	if(gms_seek_frame(gms_file, 0, SEEK_SET) == -1) {
         	printf("%s\n", gms_strerror());
        	 gms_close(gms_file);
        	//return EXIT_FAILURE;
     	}

	 for(int i = 0; i < frames; i++) {
         gms_fcount_t  nb_frames_read;
         nb_frames_read = gms_read_double(gms_file, buf_read, 1);
	
 
        if(nb_frames_read != 1) {
             printf("ERROR reading frame %d\n", i);
         } else {
             for(int j = 0; j < (nb_tracks-1); j++){
		fout << buf_read[j] << "\t" ;
             }
		fout << buf_read[nb_tracks-1] << endl;
         }
	}
	fout.close();
	gms_close(gms_file);

}

double Gmswriter::calc_vel(CvPoint point_one, CvPoint point_two, double frametime){

//velocity is distance divided by time...
		double distance = 0.0;
		double vel = 0.0;
		distance = calc_dist(point_one, point_two);
		vel = distance/frametime;
		return(vel);

}

double Gmswriter::calc_dist(CvPoint point_one, CvPoint point_two){

		int xd, yd =0;
		double distance = 0.0;
		xd = abs(point_two.x-point_one.x);
		yd = abs(point_two.y-point_one.y);
		distance = sqrt(xd*xd + yd*yd);
		return (distance);

}

GnuStream::GnuStream(){
    //open gnuplot
    gnuplot = popen( "gnuplot -persist", "w" );
    if( gnuplot == NULL ){
      cerr << "Unable to open gnuplot pipe!\n";
    }
}

GnuStream::~GnuStream(){
  flush();
  if( gnuplot ){
    fprintf( gnuplot, rdbuf()->str().c_str() ); while(1);
    fclose( gnuplot );
  }
}
