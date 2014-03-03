#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cvblob.h>

#include "correlation.h"
using namespace cv;
using namespace cvb;
using namespace std;

void Correlation::search_and_template(CvPoint pt, IplImage *picture, IplImage *corrblock, IplImage *mask){
  int x=pt.x;
  int y=pt.y;
  int p,q,m,n,r,w=0;
  CvScalar a = cvScalar(0,0,0,0);
  CvScalar c = cvScalar(0,0,0,0);
  cvZero (corrblock);
  cvZero (mask);

  for (p=(x-8);p<=(x+8);p++) {
	  for (q=(y-8);q<=(y+8);q++) {		  
		  m=p;
		  n=q;
		  //look out for edges of picture
		  if (m<=0)
			  {m=0;}
		  if (m>=picture->width-8)
			  {m=picture->width -9;}
		  if (n<=0)
			  {n=0;}
		  if (n>=picture->height-8)
			  {n=picture->height -9;}

		  if(m>=(x-1) && m<=(x+1) && n>=(y-1) && n<=(y+1)) {
			  //check for edges in new picture
			  //get pixel from original image to create 9x9 block used as search template
			  
			  a = cvGet2D(picture,n,m); // picture referred to as height *width hence n then m
			  r=n-(y-1); //calculating correct y value for search block
			  
			  //ensure against picture edges
			  if (r<0)
				  r=0;
			  if (r>3)
				  r=3;
			  if (w<7)
				  w=7;
			  if (w>9)
				  w=9;
		  
			  cvSet2D(corrblock,r,w-7,a);

		  }
		  //set pixel values to white to create search mask around point pt
		  c=cvScalar(255,255,255,0);
		  cvSet2D(mask,n,m,c);
	  }
	  w++;
  }

}

void Correlation::masktemplate(CvPoint pt,IplImage *picture, IplImage *mask){
  int x=pt.x;
  int y=pt.y;
  int p,q,m,n=0;
  CvScalar c = cvScalar(0,0,0,0);
  cvZero(mask);

  //copy 8x8 block around point pt for use in correlation function
	  for (p=(x-8);p<=(x+8);p++) {
		  for (q=(y-8);q<=(y+8);q++) {
			  m=p;
			  n=q;
			  //look out for edges of picture
			  if (m<=0)
				  m=0;
			  if (m>=picture->width-8)
				  m=picture->width -9;
			  if (n<=0)
				  n=0;
			  if (n>=picture->height-8)
				  n=picture->height-9;

			  //set pixel values to white to create search mask around point pt
			  c=cvScalar(255,255,255,0);
			  cvSet2D(mask,n,m,c);
		  }
	  } 
}

CvPoint Correlation::max_correlation(CvPoint point,IplImage *corrblock, IplImage *maskarea, IplImage *frame){

    IplImage *dest = cvCreateImage( cvGetSize(frame), DEPTH_8_BIT, CHANNELS_1 );
    //copy maskarea size of image from frame image. use this as search area
    cvCopy (frame,dest,maskarea);
    //perform correlation using the corrblock against the calculated area
    int x = point.x;
    int y = point.y;
    int p,q,r,s=0;
    int inner_wid, inner_length,out_wid,out_len, w, l =0;
    double corrvalue=0.0;
    CvPoint newpoint = point;
    int z=0;
    double maxvalue=0.0; 	
    
    //calculate energy of correlation block i.e. block*block - use this to normalise results
    double energy = 0.0;
    CvScalar pix;
    int i,j,il,jw;
    double normalise = 0.0;
    //correlation area
	    for (i=0;i<corrblock->height;i++){
		    for(j=0; j<corrblock->width;j++){
		      //calculate correlation and store value
		      il=i;
		      jw=j;

		      //check for edges
		      if(jw<0)
			      jw=0;
		      if (jw>=dest->width)
			      jw=dest->width-1;
		      if (il<0)
			      il=0;
		      if (il>=dest->height)
			      il=dest->height-1;
		      
		      pix = cvGet2D(corrblock, il,jw);
		      energy += pix.val[0]*pix.val[0];
		    }
	    }
    normalise = energy;

  //search area
  for (p=x-8;p<x+8;p++){
	  for(q=y-8;q<y+8;q++){
	      r=p;
	      s=q;
	      //look out for edges of picture
	      if (r<=0)
		      r=0;
	      if (r>=dest->width)
		      r=dest->width-1;
	      if (s<=0)
		      s=0;
	      if (s>=dest->height)
		      s=dest->height-1;

	      //if reaches ends of picture - better to break and terminate loop?
	      double correlation=0.0;
	      //correlation area
	      for (inner_length=0;inner_length<corrblock->height;inner_length++){
		   for(inner_wid=0; inner_wid<corrblock->width;inner_wid++){
	      
		      //calculate correlation and store value
		      w = inner_wid;
		      l = inner_length;		
		      out_len = s+l;
		      out_wid = r+w;

		      //check for edges
		      if(out_wid<0)
			      out_wid=0;
		      if (out_wid>=dest->width)
			      out_wid=dest->width-1;
		      if (out_len<0)
			      out_len=0;
		      if (out_len>=dest->height)
			      out_len=dest->height-1;
		      
		      //multiply value of corrblock with corresponding value of dest and add to corrvalue
		      CvScalar c1,c2;
		      c1 = cvGet2D(corrblock, l,w);
		      c2 = cvGet2D(frame,out_len,out_wid);
		      correlation += c1.val[0] * c2.val[0];
		    }
	      } 
      //if value of corrvalue is bigger than previous value, make new value and also return corresponding values of r and s at same time
	      z++;
	      double number = correlation/normalise;
	      if (number>maxvalue){
		      maxvalue=number;
		      //if maxvalue is under a specific threshold point has been lost and return newpoint as NULL
		      newpoint = cvPoint(r+1,s+1);
		      corrvalue=number;
	      }

	 }
    }
    //return the maximum point as a CvPoint
    return newpoint;
}