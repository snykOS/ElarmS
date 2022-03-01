# Makefile for spyring

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)

BIN	= spyring

EWLIB   = $(EWDIR)/lib
EW_INCL = -I$(EWDIR)/include

INCL	= $(EW_INCL) $(QLIB2_INCL)
LIBS	= $(EW_LIB) $(QLIB2_LIB) -lpthread

EWOBJS	= $(EWLIB)/mem_circ_queue.o \
	$(EWLIB)/kom.o \
	$(EWLIB)/getsysname_ew.o \
	$(EWLIB)/getutil.o \
	$(EWLIB)/logit.o \
	$(EWLIB)/transport.o \
	$(EWLIB)/sleep_ew.o \
	$(EWLIB)/socket_ew.o \
	$(EWLIB)/time_ew.o \
	$(EWLIB)/threads_ew.o \
	$(EWLIB)/sema_ew.o

# comment this next line out to TURN OFF debugging print statments
DEBUG =-g # -DDEBUG

########################################################################

BINOBJS = spyring.o

all:$(BIN)

$(BIN):$(BINOBJS)
	$(CXX) $(DEBUG) $(CFLAGS) $(INCL) $(BINOBJS) $(EWOBJS) -o $@ $(LIBS)

.c.o: 
	echo Compiling...
	gcc $< -c $(DEBUG) $(CFLAGS) $(INCL) 

clean:
	-rm -f *.o *~ core $(BIN)

veryclean:  clean

depend:	# no-op

ids: # no-op

rm-ids: # no-op

docs: #no-op

install: $(BIN) | $(INSTALL_EEWDATA_BIN_DIR)
	cp -p $(BIN) $(INSTALL_EEWDATA_BIN_DIR)/
