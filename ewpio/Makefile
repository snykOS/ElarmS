# Makefile for ewpio

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

LIB_NAME = libewpio

AQMS_INCL = -I$(EEWDIR)/third_party/libtntime/include \
	    -I$(EEWDIR)/third_party/libtnchnl/include \
	    -I$(EEWDIR)/third_party/librtseis/include \
	    -I$(EEWDIR)/third_party/libtnstd/include

QLIB2_INCL = -I$(EEWDIR)/third_party/qlib2

WPLIB_INCL = -I$(EEWDIR)/libs/datapkt -I$(EEWDIR)/libs/utils

INCL	= $(QLIB2_INCL) $(AQMS_INCL) $(WPLIB_INCL)

CC_SRCS = \
	EWPacket.cc \
	EWTrigger.cc \
	FilterBank.cc \
	GMPeak.cc \
	MaxAmplitudes.cc \
	Measurement.cc \
	NEtoZ.cc \
	PrintLock.cc \
	RangePostTrig.cc \
	TimeString.cc \
	TriggerParams.cc \
	ZeroCrossings.cc

C_SRCS	=

CC_OBJS	= $(CC_SRCS:%.cc=%.o) 
C_OBJS	= $(C_SRCS:%.c=%.o) 
OBJS	= $(CC_OBJS) $(C_OBJS)

CCFLAGS	+= -g -O0 -pg -Wextra
CFLAGS  += -g -O0

all:	$(LIB_NAME).a

$(LIB_NAME).a: $(OBJS)
	ar r $(LIB_NAME).a $(OBJS)

%.o:	%.cc
	$(CXX) $(CCFLAGS) $(INCL) -c $<

%.o:	%.c
	$(CC) $(CFLAGS) $(INCL) -c $<

ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cc

cleandepend:
	rm -rf $(DEPENDFILES)

clean:
	-rm *.o *.a

veryclean: clean rm-ids cleandepend

install:
