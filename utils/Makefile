# Makefile for wplib on Linux and SunOS

EEWDIR = ../..
include $(EEWDIR)/Make.include.$(shell uname)

APP = utils
BIN	= lib$(APP).a

SOURCES = Exceptions.cc GetProp.cc \
			geocalc.cc mathfn.cc \
			MessageSender.cc \
            IIRFilter.cc

C_SOURCES = butterworth_c.c

C_OBJS = $(C_SOURCES:.c=.o)
CBINOBJS = $(C_OBJS)


INCL = -I$(EEWDIR)/third_party \
	$(AQMS_INCL) \
	$(OTL_INCL) \
	$(APR_INCL) \
    $(ACTIVEMQ_INCL) 

LIBS+=$(AQMS_LIB) 

# comment this next line out to TURN OFF debugging print statments
DEBUG =-g 

UFLAGS=-D_REENTRANT
ifeq ($(UNAME), SunOS)
UFLAGS+=-mt
NOWARN= -erroff=wvarhidemem,temwhilespec,temspecend,hidevf
CCFLAGS+=$(NOWARN)
endif


CMNOBJS = $(notdir $(SOURCES:.cc=.o) )

BINOBJS = $(CMNOBJS) $(CBINOBJS)


# first target is the default
all: $(BIN)

# app rules
$(BIN): $(BINOBJS)
	$(AR) $@ $^


# generic rules
.cc.o: 
	$(CC) -c $< $(DEBUG) $(UFLAGS) $(CCFLAGS) $(INCL)
.c.o: 
	$(cc) -c $< $(DEBUG) $(UFLAGS) $(CFLAGS) $(INCL) 

# Id expansion
ID_SOURCES = Makefile *.py *.h *.cc *.c
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
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CFLAGS) $(INCL) *.c

cleandepend:
	rm -rf $(DEPENDFILES)


# unit test rules
UNITTESTS=utex utgp
ut: $(UNITTESTS)
cleanut:
	-rm -f $(UNITTESTS)
test: $(UNITTESTS)
	$(foreach test, $^, echo -e "\n**** $(test) ****\n"; ./$(test);)

UT_MAIN=../../libs/utils/ut_main.cc 

utex: $(UT_MAIN) $(BIN)
	$(CC) -o $@ $^ -DCLASS=Error -DHEADER=\"Exceptions.h\" $(DEBUG) $(UFLAGS) $(CCFLAGS) $(INCL) $(BIN) $(LIBS)

utgp: $(UT_MAIN) $(BIN)
	$(CC) -o $@ $^ -DCLASS=GetProp -DHEADER=\"GetProp.h\" $(DEBUG) $(UFLAGS) $(CCFLAGS) $(INCL) $(BIN) $(LIBS)

# documentation
DOCDIR=$(EEWDIR)/docs/$(APP)
docs: doxygen.conf
	mkdir -p $(DOCDIR)
	doxygen doxygen.conf

cleandocs:
	rm -rf $(DOCDIR) *.tag


# cleanup rules
clean:
	-rm -f *.o core $(BIN) $(UNITTESTS)

veryclean: rm-ids cleandepend clean cleandocs


printvars:
	@echo MAKEFILE=$(MAKEFILE)
	@echo MAKEDEPEND=$(MAKEDEPEND)
	@echo DEPENDFILES=$(DEPENDFILES)
	@echo UNAME=$(UNAME)
	@echo BUILDER=$(BUILDER)
	@echo AQMS=$(AQMS)
	@echo BIN=$(BIN)
	@echo SOURCES=$(SOURCES)
	@echo cc=$(cc)
	@echo CC=$(CC)
	@echo CFLAGS=$(CFLAGS)
	@echo CCFLAGS=$(CCFLAGS)
	@echo UFLAGS=$(UFLAGS)
	@echo EWDIR=$(EWDIR)
	@echo APR_INCL=$(APR_INCL)
	@echo ACTIVEMQ=$(ACTIVEMQ)
	@echo OTLDIR=$(OTLDIR)
	@echo PCAP=$(PCAP)

	@echo INCL=$(INCL)
	@echo BINOBJS=$(BINOBJS)
	@echo LIBS=$(LIBS)
