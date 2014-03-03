CXX = g++
LINKCC = $(CXX)
PROGNAME := twobase		
SRCS := vid_corr.cpp blobby.cpp fullhand.cpp correlation.cpp transform.cpp gmswriter_fixed.cpp organise.cpp 

OBJS := $(SRCS:.cpp=.o)

CFLAGS +=-g 
INCLUDES +=-I/usr/local/include/opencv2 -I/usr/local/include/opencv -I/home/jenni/cvblobslib_v6p1/ 
LDFLAGS +=-L/home/jenni/cvblobslib_v6p1/ -L/usr/local/lib
	







LIBS += -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lgms -lblob
 

$(PROGNAME): $(OBJS) 
	$(LINKCC) $(CFLAGS) -o $@ $(OBJS) $(INCLUDES) $(LDFLAGS) $(LIBS)

$(OBJS): $(SRCS)
	$(LINKCC) $(CFLAGS) $(INCLUDES) -c $*.cpp


.PHONY: clean
clean:
	$(RM) \#*\# *.o *~ core .depend $(PROGNAME)

.PHONY: depend
depend:
	$(CPP) -M $(SRCS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
