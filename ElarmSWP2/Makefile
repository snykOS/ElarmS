# Makefile for epicWP
#
# From Make.include:
#   ACTIVEMQ_INCL, ACTIVEMQ_LIB, APR_INCL, APR_LIB, XERCES_INCL, XERCES_LIB, BUILD_UTILS
#   EWDIR, CCFLAGS
#

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)

BIN	= epicWP
BIN2	= epicWPReceiver

EWLIB	= $(EWDIR)/lib

EWOBJS =$(EWLIB)/brent.o \
	$(EWLIB)/chron3.o \
	$(EWLIB)/getutil.o \
	$(EWLIB)/kom.o \
	$(EWLIB)/logit.o \
	$(EWLIB)/mnbrak.o \
	$(EWLIB)/swap.o \
	$(EWLIB)/tlay.o \
	$(EWLIB)/dirops_ew.o \
	$(EWLIB)/sleep_ew.o \
	$(EWLIB)/time_ew.o \
	$(EWLIB)/transport.o

OBJS	= ElarmSWP2.o EWP2.o EWP2Sender.o FilterBands.o FilterBankMeasure.o \
	  GMPeakAmpsTSPModule.o MaxAmpMeasure.o Measure.o \
	  NEtoZMeasure.o PickerTSPModule.o ProcessBuffer.o \
          RangePostTrigMeasure.o ToGroundTSPModule.o TriggerParamsTSPModule.o \
          ZeroCrossingsMeasure.o $(EWOBJS)

#OBJS2 = $(BIN2).o
OBJS2 = EWP2Receiver.o	  

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

WPLIB_INCL	= -I$(EEWDIR)/libs/datapkt -I$(EEWDIR)/libs/utils
WPLIB_LIB	= -L$(EEWDIR)/libs/datapkt -lwp -L$(EEWDIR)/libs/utils -lutils

EWPIO_INCL	= -I../ewpio
EWPIO_LIB	= -L../ewpio -lewpio

DMLIB_INCL	= -I$(EEWDIR)/libs/dmlib
DMLIB_LIB	= -L$(EEWDIR)/libs/dmlib -ldm

AQMS_INCL	= -I$(EEWDIR)/third_party/libtnchnl/include \
		  -I$(EEWDIR)/third_party/libtntime/include \
		  -I$(EEWDIR)/third_party/librtseis/include \
		  -I$(EEWDIR)/third_party/libtnstd/include \
		  -I$(EEWDIR)/third_party

AQMS_LIB	= \
		  -L$(EEWDIR)/third_party/libamq -lamq \
		  -L$(EEWDIR)/third_party/librtseis -lrtseis \
		  -L$(EEWDIR)/third_party/libtnchnl -ltnchnl \
		  -L$(EEWDIR)/third_party/libtntime -ltntime \
		  -L$(EEWDIR)/third_party/libtnstd -ltnstd

MSEED_LIB	= -L$(EWLIB) -lmseed

INCL		= $(EWPIO_INCL) $(DMLIB_INCL) $(AQMS_INCL) $(QLIB2_INCL) $(WPLIB_INCL) \
		  $(ACTIVEMQ_INCL) $(APR_INCL) $(XERCES_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

LIBS		= $(EWPIO_LIB) $(DMLIB_LIB) $(WPLIB_LIB) $(AQMS_LIB) $(QLIB2_LIB) \
		  $(XERCES_LIB) $(ACTIVEMQ_LIB) $(APR_LIB) $(MSEED_LIB) $(SYSLIBS)


all:	$(BIN) $(BIN2)

$(BIN): $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) $(DEBUG) $(INCL) $(OBJS) $(LIBS)

$(BIN2): $(OBJS2)
	$(CC) -o $@ $(CCFLAGS) $(DEBUG) $(OBJS2) $(LIBS)

%.o:	%.cc
	$(CXX) -c $< $(CCFLAGS) $(DEBUG) $(INCL)

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
	-rm -f *.o $(BIN) $(BIN2)

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
	cp $(BIN2) $(INSTALL_ELARMS_BIN_DIR)/
