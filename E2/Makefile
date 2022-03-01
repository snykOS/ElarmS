# Makefile for epic
#
# From Make.include:
#   ACTIVEMQ_INCL, ACTIVEMQ_LIB, APR_INCL, APR_LIB, XERCES_INCL, XERCES_LIB, BUILD_UTILS
#   CCFLAGS
#

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)

BIN	= epic

OBJS	= E2Main.o E2AMQReader.o E2XMLParser.o E2Prop.o E2ModuleManager.o E2Trigger.o \
	  E2TriggerManager.o E2Event.o E2Region.o E2EventManager.o E2Location.o \
	  E2Magnitude.o E2Alert.o E2Associator.o deltaz.o

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

WPLIB_INCL	= -I$(EEWDIR)/libs/datapkt -I$(EEWDIR)/libs/utils
WPLIB_LIB	= -L$(EEWDIR)/libs/utils -lutils

EWPIO_INCL	= -I../ewpio
EWPIO_LIB	= -L../ewpio -lewpio

DMLIB_INCL	= -I$(EEWDIR)/libs/dmlib
DMLIB_LIB	= -L$(EEWDIR)/libs/dmlib -ldm

AQMS_INCL	= -I$(EEWDIR)/third_party/libtnchnl/include \
		  -I$(EEWDIR)/third_party/libtntime/include \
		  -I$(EEWDIR)/third_party/librtseis/include \
		  -I$(EEWDIR)/third_party/libtnstd/include \
		  -I$(EEWDIR)/third_party

AQMS_LIB	= -L$(EEWDIR)/third_party/libtntime -ltntime \
		  -L$(EEWDIR)/third_party/libtnstd -ltnstd

INCL		= $(WPLIB_INCL) $(EWPIO_INCL) $(DMLIB_INCL) $(AQMS_INCL) $(QLIB2_INCL) \
		  $(ACTIVEMQ_INCL) $(APR_INCL) $(XERCES_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

LIBS		= $(WPLIB_LIB) $(EWPIO_LIB) $(DMLIB_LIB) $(AQMS_LIB) $(QLIB2_LIB) \
		  $(XERCES_LIB) $(ACTIVEMQ_LIB) $(APR_LIB) $(SYSLIBS)


all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) $(DEBUG) $(INCL) $(OBJS) $(LIBS)

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
	-rm -f *.o $(BIN)

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
