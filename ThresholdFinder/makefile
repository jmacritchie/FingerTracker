CXX = g++
LINKCC = $(CXX)
PROGNAME := thresholdfinder		
SRCS := thresholdfinder.cpp 

OBJS := $(SRCS:.cpp=.o)

CFLAGS +=-g 
INCLUDES +=-I/usr/local/include/ 
LDFLAGS +=-L/usr/local/lib
	
LIBS += -lm -lopencv_core -lopencv_highgui -lopencv_imgproc -lcvblob 

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
