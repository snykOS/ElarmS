# Makefile for libamq

# change this to your library name
LIB = libamq.a

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

# comment this next line out to TURN OFF debugging print statments
DEBUG =-g 

# remove -Wall so we don't see warnings about unused vars
# this should be fixed in the source code -- CAF 09/06/2016
CFLAGS:=$(filter-out -Wall,$(CFLAGS))

INCL = $(AQMS_INCL) -I./ -I./include \
	   $(APR_INCL) \
	   $(ACTIVEMQ_INCL) \
	   $(XERCES_INCL) \
	   -I..

# list object files 
OBJS =   ContentTable.o 	\
	 Message.o		\
	 QWMessageParser.o	\
	 QWMessageSerializer.o	\
	 AMQConnectionHandler.o	\
	 Connection.o		\
	 amqPropertiesST.o


all:$(LIB)

$(LIB): $(OBJS) 
	$(AR) $@ $^

.C.o: 
	$(CC) $(USE_KPIC) $< -c $(DEBUG) $(CFLAGS) $(INCL) 

.cc.o: 
	$(CC) $(USE_KPIC) $< -c $(DEBUG) $(CFLAGS)  $(INCL) 

clean:
	-rm -f *.o *.so *.a *~ core


veryclean:  clean
	makedepend
	-rm -f Makefile.bak

depend:
	makedepend
	makedepend -a -Y -- $(CCFLAGS) $(INCL) *.cc *.C

printvars:
	@echo CC=$(CC)
	@echo CFLAGS=$(CFLAGS)
	@echo DEBUG=$(DEBUG)
	@echo INCL=$(INCL)

# DO NOT DELETE
