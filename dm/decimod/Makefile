# Makefile for decimod

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

DMLIB_INCL = -I../../libs/dmlib
DMLIB_LIB = ../../libs/dmlib/libdm.a

AQMS_INCL = -I../../third_party/libtntime/include -I../../third_party/libtnstd/include
AQMS_LIB = ../../third_party/libtntime/libtntime.a ../../third_party/libtnstd/libtnstd.a
BIN	= dm

INCL	= $(DMLIB_INCL) $(QLIB2_INCL) $(ACTIVEMQ_INCL) $(APR_INCL) $(XERCES_INCL) $(AQMS_INCL)
INCL	+= -I../../third_party -I../../libs/utils

LIBS	= $(DMLIB_LIB) $(QLIB2_LIB) $(ACTIVEMQ_LIB) $(APR_LIB) $(XERCES_LIB) $(AQMS_LIB) -lpthread

OBJS	= AlgorithmEvent.o DMEvent.o DecisionModule.o decimod_main.o GetProp.o

CCFLAGS	+= -O0
CCFLAGS	+= $(DEBUG)
CCFLAGS += -DBUILDER=\"$(BUILDER)\"

all: $(BIN)

dm: $(OBJS) $(DMLIB_LIB)
	$(CXX) $(CCFLAGS) $(INCL) -o $@ $(OBJS) $(LIBS)

%.o:	%.cpp
	$(CXX) -c $(CCFLAGS) $(INCL) $<

ids:    *.cpp *.h
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cpp *.h
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

docs: # done one level up

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cpp

cleandepend:
	rm -rf $(DEPENDFILES)

clean:	
	-rm -f *.o $(BIN)

veryclean:	clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_DM_BIN_DIR)
	cp -p $(BIN) $(INSTALL_DM_BIN_DIR)/
