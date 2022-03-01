# Makefile for qmcast2ew

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)

BIN	= qmcast2ew

AQMS_INCL	= -I$(EEWDIR)/third_party/libtnchnl/include \
		  -I$(EEWDIR)/third_party/libtntime/include \
		  -I$(EEWDIR)/third_party/librtseis/include \
		  -I$(EEWDIR)/third_party/libtnstd/include

AQMS_LIB	= -L$(EEWDIR)/third_party/librtseis -lrtseis \
		  -L$(EEWDIR)/third_party/libtnchnl -ltnchnl \
		  -L$(EEWDIR)/third_party/libtntime -ltntime \
		  -L$(EEWDIR)/third_party/libtnstd -ltnstd

INCL	= $(AQMS_INCL) $(EW_INCL) $(QLIB2_INCL)
LIBS	= $(AQMS_LIB) $(EW_LIB) $(QLIB2_LIB) -lpthread

EWLIB   = $(EWDIR)/lib
EW_INCL = -I$(EWDIR)/include


EWOBJS =$(EWLIB)/brent.o	\
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

# comment this next line out to TURN OFF debugging print statments
DEBUG =-g # -DDEBUG

########################################################################

BINOBJS = qmcast2ew.o StationMcast.o EWConfig.o  mcastutil.o CSUtils.o

all:$(BIN)

$(BIN):$(BINOBJS)
	$(CXX) $(DEBUG) $(CFLAGS) $(INCL) $(BINOBJS) $(EWOBJS) -o $@ $(LIBS)

.C.o: 
	echo Compiling...
	$(CXX) $< -c $(DEBUG) $(CFLAGS) $(INCL) 

clean:
	-rm -f *.o *~ core $(BIN)

veryclean:  clean

depend:	# no-op

ids: # no-op

rm-ids: # no-op

docs: # no-op

install: $(BIN) | $(INSTALL_EEWDATA_BIN_DIR)
	cp -p $(BIN) $(INSTALL_EEWDATA_BIN_DIR)/
