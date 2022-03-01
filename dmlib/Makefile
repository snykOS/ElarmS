# Makefile for dmlib

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

QLIB2_INCL = -I../../third_party/qlib2

INCL	= $(QLIB2_INCL) $(XERCES_INCL) $(ACTIVEMQ_INCL) $(APR_INCL) -I$(EEWDIR)/third_party

CC_SRCS = \
	AlgMessage.cpp \
	CoreEventInfo.cpp \
	DMMessage.cpp \
	DMMessageDecoder.cpp \
	DMMessageEncoder.cpp \
	DMMessageReceiver.cpp \
	DMMessageSender.cpp \
	DMReviewMessage.cpp \
	EpicMessage.cpp \
	ElarmsMessage.cpp \
	FinDerMessage.cpp \
	FiniteFaultMessage.cpp \
	GMContour.cpp \
	GMMap.cpp \
	GMMessage.cpp \
	GMObservation.cpp \
	GMPrediction.cpp \
	GPSlipMessage.cpp \
	HBConsumer.cpp \
	HBForward.cpp \
	HBProducer.cpp \
	OnSiteMessage.cpp \
	StaNetChanLoc.cpp \
	VSMessage.cpp

C_SRCS	=
CC_OBJS	= $(CC_SRCS:%.cpp=%.o) 
C_OBJS	= $(C_SRCS:%.c=%.o) 
OBJS	= $(CC_OBJS) $(C_OBJS)

CCFLAGS	+=  -DGNU_COMPILER
CCFLAGS	+= -O0
CCFLAGS	+= $(DEBUG)

DM_LIB	= libdm.a

all: $(DM_LIB)

$(DM_LIB): $(OBJS)
	ar r $(DM_LIB) $(OBJS)

%.o:	%.cpp
	$(CXX) $(CCFLAGS) $(INCL) -c $<

%.o:	%.c
	$(CC) $(CFLAGS) $(INCL) -c $<

ids:    *.cpp *.h
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cpp *.h
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

install: $(DM_LIB) | $(INSTALL_EEW_LIB_DIR) $(INSTALL_EEW_INCL_DIR)
	cp *.a $(INSTALL_EEW_LIB_DIR)/
	cp *.h $(INSTALL_EEW_INCL_DIR)/

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cpp

cleandepend:
	rm -rf $(DEPENDFILES)

# documentation
DOCDIR=$(EEWDIR)/docs/dmlib
docs:
	mkdir -p $(DOCDIR)
	doxygen doxygen.conf

cleandocs:
	rm -rf $(DOCDIR) *.tag
clean:
	-rm -f *.o *.a

veryclean: rm-ids cleandepend clean cleandocs

test: # no-op
