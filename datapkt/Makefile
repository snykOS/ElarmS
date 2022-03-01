# Makefile for wplib on Linux and SunOS

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

APP = datapkt
BIN	= libwp.a

META_SOURCES = WPProperties.meta
GENERATED_HEADERS = $(notdir $(META_SOURCES:.meta=.h))
GENERATED_SOURCES = $(notdir $(META_SOURCES:.meta=.cc))
GENERATED_FILES = $(GENERATED_HEADERS) $(GENERATED_SOURCES) EXAMPLE_WP.cfg

# waveform specific modules
SOURCES = $(GENERATED_SOURCES)

SOURCES += wp.cc \
		   PacketBuffer.cc \
		   readraw.cc DataChannel.cc WaveformFeeder.cc

INCL = -I. \
    -I$(EEWDIR)/third_party \
	$(AQMS_INCL) \
	$(OTL_INCL) $(ORACLE_INCL) \
	-I$(EWDIR)/include \
	-I$(EEWDIR)/libs/utils \
	$(QLIB2_INCL) \
	$(ACTIVEMQ_INCL) $(APR_INCL) \
	$(QMASERV_INCL) \
	$(XERCES_INCL)

EWLIB = $(EWDIR)/lib
EWOBJS = $(EWLIB)/brent.o	\
	$(EWLIB)/chron3.o	\
	$(EWLIB)/getutil.o	\
	$(EWLIB)/kom.o	\
	$(EWLIB)/logit.o	\
	$(EWLIB)/mnbrak.o	\
	$(EWLIB)/swap.o	\
	$(EWLIB)/tlay.o	\
	$(EWLIB)/dirops_ew.o	\
	$(EWLIB)/sleep_ew.o	\
	$(EWLIB)/time_ew.o	\
	$(EWLIB)/transport.o	

LIBS+=$(AQMS_LIB) \
	$(QLIB2_LIB) \
	$(EEWDIR)/libs/utils/libutils.a

# additional libraries for notify-eew
LIBS+= \
	$(AQMS_LIB) \
	$(XERCES_LIB) \
	$(ACTIVEMQ_LIB) \


# support for earthworm
ifdef ENABLE_EARTHWORM
CCFLAGS += -DENABLE_EARTHWORM
SOURCES += ewfeeder.cc
endif # ENABLE_EARTHWORM

# support for multicast
ifdef ENABLE_MCAST
CCFLAGS+=$(ENABLE_MCAST)
INCL+=-I$(PCAP)/include
LIBS+=-L$(PCAP)/lib -lpcap
SOURCES += mcastfeeder.cc
endif # ENABLE_MCAST

# support for static file feed
ifdef ENABLE_FLFEED
CCFLAGS+=$(ENABLE_FLFEED)
LIBS+=-L$(EWDIR)/lib -lmseed
SOURCES += flfeeder.cc
endif # ENABLE_FLFEED

# support for mass recenter notifications
ifdef NOTIFY_EEW
SOURCES += notify_eew.cc
endif #NOTIFY_EEW

# comment this next line out to TURN OFF debugging print statments
DEBUG =-g 

UFLAGS=-D_REENTRANT
UFLAGS += -DBUILDER=\"$(BUILDER)\"

ifeq ($(UNAME), SunOS)
UFLAGS+=-mt
NOWARN= -erroff=wvarhidemem,temwhilespec,temspecend,hidevf
CCFLAGS+=$(NOWARN)
endif


CMNOBJS = $(notdir $(SOURCES:.cc=.o) )

BINOBJS = $(EWOBJS) $(CMNOBJS) $(CBINOBJS)


# first target is the default
all: $(BIN)

$(BIN): $(BINOBJS)
	$(AR) $@ $^


# generic rules
.cc.o: 
	$(CC) -c $< $(DEBUG) $(UFLAGS) $(CCFLAGS) $(INCL)
.c.o: 
	$(cc) -c $< $(DEBUG) $(UFLAGS) $(CFLAGS) $(INCL) 


# rules for generating property files
gen: $(GENERATED_FILES)
$(GENERATED_FILES): $(PROPGENNEW) $(META_SOURCES)
	$(PROPGENNEW) WP

cleangen:
	-rm -f $(GENERATED_FILES)

# Id expansion
ID_SOURCES = $(filter-out $(GENERATED_FILES), $(wildcard $(META_SOURCES) *.h $(SOURCES)))

ids: $(ID_SOURCES)
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids: $(ID_SOURCES)
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)


# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cc

cleandepend:
	rm -rf $(DEPENDFILES)



# documentation
DOCDIR=$(EEWDIR)/docs/$(APP)
docs: doxygen.conf $(SOURCES) $(GENERATED_FILES)
	mkdir -p $(DOCDIR)
	doxygen doxygen.conf

cleandocs:
	rm -rf $(DOCDIR) *.tag


# cleanup rules
clean:
	-rm -f *.o core $(BIN) $(UNITTESTS)

veryclean: rm-ids cleandepend clean cleangen cleandocs


printvars:
	@echo MAKEFILE=$(MAKEFILE)
	@echo MAKEDEPEND=$(MAKEDEPEND)
	@echo DEPENDFILES=$(DEPENDFILES)
	@echo UNAME=$(UNAME)
	@echo BUILDER=$(BUILDER)
	@echo EEWDIR=$(EEWDIR)
	@echo PROPGENNEW=$(PROPGENNEW)
	@echo AQMS=$(AQMS)
	@echo APP=$(APP)
	@echo BIN=$(BIN)
	@echo META_SOURCES=$(META_SOURCES)
	@echo GENERATED_HEADERS=$(GENERATED_HEADERS)
	@echo GENERATED_SOURCES=$(GENERATED_SOURCES)
	@echo GENERATED_FILES=$(GENERATED_FILES)
	@echo SOURCES=$(SOURCES)
	@echo cc=$(cc)
	@echo CC=$(CC)
	@echo CFLAGS=$(CFLAGS)
	@echo CCFLAGS=$(CCFLAGS)
	@echo UFLAGS=$(UFLAGS)
	@echo EWDIR=$(EWDIR)
	@echo QLIB2=$(QLIB2)
	@echo XERCES_INCL=$(XERCES_INCL)
	@echo APR_INCL=$(APR_INCL)
	@echo ACTIVEMQ=$(ACTIVEMQ)
	@echo ORACLE_INCL=$(ORACLE_INCL)
	@echo OTLDIR=$(OTLDIR)
	@echo PCAP=$(PCAP)

	@echo INCL=$(INCL)
	@echo BINOBJS=$(BINOBJS)
	@echo LIBS=$(LIBS)
