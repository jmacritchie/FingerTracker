#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <vector>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "gms.h"
#include "gmswriter_fixed.h"

using namespace std;


int main(int argc, char**argv){

	 if(argc != 2) {
         fprintf(stderr, "usage : %s <infile name>\n", argv[0]);
         return EXIT_FAILURE;
     }

	char * file_name = argv[1];
	//Gmswriter::readdata();
	Gmswriter::read_all_frames(file_name);
	//Gmswriter::read_frames(0,150 ,file_name);
	//Gmswriter::read_curves(0,150,file_name);

	return 0;

}



