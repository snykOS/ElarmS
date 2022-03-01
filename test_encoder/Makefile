# Makefile for test_encoder

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

DMLIB_INCL = -I../../libs/dmlib
DMLIB_LIB = ../../libs/dmlib/libdm.a

TNSTD_INCL = -I../../third_party/libtnstd/include
TNSTD_LIB = ../../third_party/libtnstd/libtnstd.a
BIN	= decoder

INCL	= $(DMLIB_INCL) $(QLIB2_INCL) $(ACTIVEMQ_INCL) $(APR_INCL) $(XERCES_INCL) $(TNSTD_INCL)

LIBS	= $(DMLIB_LIB) $(QLIB2_LIB) $(ACTIVEMQ_LIB) $(APR_LIB) $(XERCES_LIB) $(TNSTD_LIB) -lpthread

OBJS	= TestEncoder.cpp

CCFLAGS	+= -O0
CCFLAGS	+= $(DEBUG)

all: $(BIN)

decoder: $(OBJS)
	$(CXX) $(CCFLAGS) $(INCL) -o $@ $(OBJS) $(LIBS)

%.o:	%.cpp
	$(CXX) -c $(CCFLAGS) $(INCL) $<

ids:    *.cpp *.h
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cpp *.h
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

clean:	
	-rm -f *.o $(BIN)

veryclean:	clean rm-ids

depend:	# not supported

install: $(BIN) | $(INSTALL_DM_BIN_DIR)
	cp -p $(BIN) $(INSTALL_DM_BIN_DIR)/
