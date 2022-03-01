# Makefile for libtnstd

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)


LIB_NAME = libtnstd

AQMS_INCL       = -I$(EEWDIR)/third_party/libtnchnl/include \
                  -I$(EEWDIR)/third_party/libtntime/include \
                  -I$(EEWDIR)/third_party/librtseis/include \
                  -I$(EEWDIR)/third_party/libtnwave/include \
                  -I$(EEWDIR)/third_party/libtndb/include

QLIB2_INCL      = -I$(EEWDIR)/third_party/qlib2
ORACLE_INCL     = -I$(EEWDIR)/third_party/oracle/product/12.1.0.2/rdbms/public
OTL_INCL        = -I$(EEWDIR)/third_party/OTL

INCL = -I./include $(AQMS_INCL) $(QLIB2_INCL) $(ORACLE_INCL) $(OTL_INCL)

# If you are building shared libraries, uncomment the following line:
USE_KPIC = -fPIC

# Commands for making shared and static libraries
GENSTATLIB = ar rv
GENSHARLIB = $(CXX) -shared -o

# list object files 
OBJS =  Compat.o		\
	Configuration.o		\
	Directory.o		\
	FileReader.o		\
	GroupConfig.o		\
	Lock.o			\
	LockFile.o		\
	Logfile.o		\
	MulticastAddress.o	\
	MulticastRecv.o		\
	MulticastSender.o	\
	Pipe.o			\
	SearchExpr.o		\
	String.o		\
	Telnet.o		\
	TemporaryFile.o		\
	great.o			\
	y2k.o			\
	system_timeout.o	\
	StatusManager.o		\
	RTException.o		\
	Dd2Dm.o			\
	nscl.o			\
	nscl_cpp.o		\
	ByteSwap.o		\
	utils.o

FF  = gfortran
F77LIBS = -lgfortran
CC = gcc

.C.o: 
	$(CXX) $(USE_KPIC) $< -c $(CFLAGS) $(INCL) 

.cc.o: 
	$(CXX) $(USE_KPIC) $< -c $(CFLAGS) $(INCL) 

.c.o: 
	$(CC)  $(USE_KPIC) $< -c $(CFLAGS) $(INCL)

.f.o:
	$(FF) $< -c $(CFLAGS) $(INCL)

ALL_LIBS = $(LIB_NAME).a	# $(LIB_NAME).so

all:	$(ALL_LIBS)

$(LIB_NAME).a: $(OBJS) 
	$(GENSTATLIB) $@ $(OBJS) $(FLAGS)

$(LIB_NAME).so: $(OBJS) 
	$(GENSHARLIB) $@ $(OBJS) $(FLAGS)

install: $(LIB_NAME).a | $(INSTALL_EEW_LIB_DIR) $(INSTALL_EEW_INCL_DIR)
	cp $(LIB_NAME).a  $(INSTALL_EEW_LIB_DIR)
#	cp $(LIB_NAME).so $(INSTALL_EEW_LIB_DIR)
	cp include/*.h    $(INSTALL_EEW_INCL_DIR)/

clean:
	-rm -f *.o *.a *.so *~ core

veryclean:  clean
	makedepend

depend:
	makedepend $(INCL) $(CPPINC) *.c *.C *.cc
# DO NOT DELETE THIS LINE -- make depend depends on it.
