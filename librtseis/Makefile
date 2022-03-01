# Makefile for librtseis

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)

LIB_NAME = librtseis

QLIB2_INCL	= -I$(EEWDIR)/third_party/qlib2
QLIB2_LIB	= -L$(EEWDIR)/third_party/qlib2/lib64 -lqlib2

AQMS_INCL	= -I$(EEWDIR)/third_party/libtntime/include \
		  -I$(EEWDIR)/third_party/libtnstd/include

INCL = $(AQMS_INCL) $(QLIB2_INCL) -Iinclude

# Commands for making shared and static libraries
GENSTATLIB = ar rv
GENSHARLIB = $(CC) -shared -o

# list object files 
OBJS =   EventEDA.o	\
	 CISN_MlAo.o    \
	 CISN_la100.o   \
	 SeismoFuncs.o	\
	 adjave.o	\
	 dstaz.o	\
	 fcs.o		\
	 m1002m.o	\
	 sdecode.o	\
	 sgcdaf.o	\
	 tt.o		\
	 ynterp.o

ALL_LIBS = $(LIB_NAME).a

all: $(ALL_LIBS)

$(LIB_NAME).a: $(OBJS) 
	$(GENSTATLIB) $@ $(OBJS) $(FLAGS)

$(LIB_NAME).so: $(OBJS) 
	$(GENSHARLIB) $@ $(OBJS) $(FLAGS)

FF  = gfortran
F77LIBS = -lgfortran

CFLAGS += $(CODAS_FROM_PICKEW) $(NC_NO_EVENT_QUALITY)

# where.f needs to be ported to f90 if it is used anywhere,
# but I doubt it is used any more PNL 2011/11/07
#	 where.o	\
#	 wheresub.o	\

.C.o: 
	$(CC) $(USE_KPIC) $< -c $(CFLAGS) $(INCL) 
.c.o: 
	$(cc) $(USE_KPIC) $< -c $(CFLAGS) $(INCL)
.f.o: 
	$(FF) $< -c $(CFLAGS) $(INCL)

clean:
	-rm -f *.o *.a *.so *~ core ${BIN} 
	-rm -fr SunWS_cache

veryclean:  clean
	makedepend

depend:
	makedepend $(INCL) $(CPPINC) *.c *.C
# DO NOT DELETE THIS LINE -- make depend depends on it.
