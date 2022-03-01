# Makefile for mcast2ew

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)

BIN	= mcast2ew

EWLIB   = $(EWDIR)/lib
EW_INCL = -I$(EWDIR)/include

EWOBJS =$(EWLIB)/sleep_ew.o \
	$(EWLIB)/logit.o \
	$(EWLIB)/getutil.o \
        $(EWLIB)/kom.o \
	$(EWLIB)/transport.o \
	$(EWLIB)/time_ew.o

AQMS_INCL	= -I$(EEWDIR)/third_party/libtnchnl/include \
		  -I$(EEWDIR)/third_party/libtntime/include \
		  -I$(EEWDIR)/third_party/librtseis/include \
		  -I$(EEWDIR)/third_party/libtnstd/include

AQMS_LIB	= -L$(EEWDIR)/third_party/librtseis -lrtseis \
		  -L$(EEWDIR)/third_party/libtnchnl -ltnchnl \
		  -L$(EEWDIR)/third_party/libtntime -ltntime \
		  -L$(EEWDIR)/third_party/libtnstd -ltnstd

INCL	= $(AQMS_INCL) $(EW_INCL) $(QLIB2_INCL)
LIBS	= $(AQMS_LIB) $(QLIB2_LIB) -lpthread

BUILDER=$(USER)@$(HOSTNAME)
CFLAGS += -DBUILDER=\"$(BUILDER)\"
CFLAGS += -DCHECK_TIMESTAMPS
CFLAGS += -DGEN_BAD_TIMESTAMPS

#DEBUG += -g -DDEBUG

BINOBJS = mcast2ew.o StationMcast.o EWConfig.o  mcastutil.o CSUtils.o

all:$(BIN)

mcast2ew:$(BINOBJS)
	$(CXX) $(DEBUG) $(CFLAGS) $(INCL) $(BINOBJS) $(EWOBJS) -o $@ $(LIBS)

.C.o: 
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
