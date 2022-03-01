# Makefile for test_alg_receiver

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

DMLIB_INCL = -I../../libs/dmlib
DMLIB_LIB = ../../libs/dmlib/libdm.a
QLIB2_INCL = -I../../third_party/qlib2
QLIB2_LIB = ../../third_party/qlib2/lib64/libqlib2.a

TNSTD_INCL = -I../../third_party/libtnstd/include
TNSTD_LIB = ../../third_party/libtnstd/libtnstd.a
BIN	= test_alg_receiver

INCL	= $(DMLIB_INCL) $(QLIB2_INCL) $(ACTIVEMQ_INCL) $(APR_INCL) $(XERCES_INCL) $(TNSTD_INCL)

LIBS	= $(DMLIB_LIB) $(QLIB2_LIB) $(ACTIVEMQ_LIB) $(APR_LIB) $(XERCES_LIB) $(TNSTD_LIB) -lpthread

OBJS	= test_alg_receiver.o

CCFLAGS	+= -O0
CCFLAGS	+= $(DEBUG)

all: $(BIN)

test_alg_receiver: $(OBJS)
	$(CXX) $(CCFLAGS) $(INCL) -o $@ $(OBJS) $(LIBS)

%.o:	%.cpp
	$(CXX) -c $(CCFLAGS) $(INCL) $<

clean:	
	-rm -f *.o $(BIN)

veryclean:	clean

install: $(BIN) | $(INSTALL_DM_BIN_DIR)
	cp -p $(BIN) $(INSTALL_DM_BIN_DIR)/
