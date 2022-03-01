########################################################################
#
# Makefile     : conlog Program
#
# Author       : Patrick Small
#
# Last Revised : September 2, 1999
#
########################################################################


########################################################################
# Standard definitions and macros
#
########################################################################

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

BIN	= conlog

AQMS_INCL = -I$(EEWDIR)/third_party/libtnchnl/include \
            -I$(EEWDIR)/third_party/libtntime/include \
            -I$(EEWDIR)/third_party/librtseis/include \
            -I$(EEWDIR)/third_party/libtnstd/include \
            -I$(EEWDIR)/third_party/libtndb/include \
            -I$(EEWDIR)/third_party/libamq

AQMS_LIB = -L$(EEWDIR)/third_party/libtntime -ltntime \
		-L$(EEWDIR)/third_party/libtnstd -ltnstd

INCL	 = $(AQMS_INCL) $(QLIB2_LIB)

LIBS  = $(AQMS_LIB) $(QLIB2_LIB) $(SYSLIBS)

#LIBS	= -L$(RTLIBDIR) -ltnstd -ltntime -ltnstd $(QLIB2LIBS) $(SVR4LIBS)
#LIBS	= ../lib/tntime/libtntime.a ../lib/tnstd/libtnstd.a $(QLIB2LIBS) $(SVR4LIBS)
#LIBS	= $(RTLIBDIR)/libtntime.a $(RTLIBDIR)/libtnstd.a $(QLIB2LIBS) $(SVR4LIBS)

OBJS = ConsoleLog.o

# builder info
HOSTNAME=$(shell hostname | cut -d'.' -f1)
BUILDER=$(USER)@$(HOSTNAME)
CFLAGS+= -DBUILDER=\"$(BUILDER)\"

all:${BIN}

conlog:${OBJS}
	${CC} $(CFLAGS) ${OBJS} -o $@ ${LIBS}

.C.o: 
	$(CC) $< -c $(CFLAGS) $(INCL) 

clean:
	-rm -f *.o *~ core ${BIN} 
	-rm  -rf SunWS_cache

veryclean: clean

depend:	# no-op

ids: # no-op

rm-ids: # no-op

docs: #no-op
