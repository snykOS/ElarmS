# Makefile for libtntime
# updated for ShakeAlert

EEWDIR=../..
include $(EEWDIR)/Make.include.$(shell uname)


LIB_NAME = libtntime

AQMS_INCL       = -I$(EEWDIR)/third_party/libtnstd/include

QLIB2_INCL	= -I$(EEWDIR)/third_party/qlib2

INCL = -I./include $(AQMS_INCL) $(QLIB2_INCL)

# If you are building shared libraries, uncomment the following line:
USE_KPIC = -fPIC

# Commands for making shared and static libraries
GENSTATLIB = ar rv
GENSHARLIB = $(CXX) -shared -o

# list object files 
OBJS =   TimeStamp.o     \
	 TimeWindow.o    \
	 Duration.o	 \
	 Ymdhms.o

.C.o: 
	$(CXX) $(USE_KPIC) $< -c $(CFLAGS) $(INCL) 

.cc.o: 
	$(CXX) $(USE_KPIC) $< -c $(CFLAGS) $(INCL) 

.c.o: 
	$(CC)  $(USE_KPIC) $< -c $(CFLAGS) $(INCL)

.f.o:
	$(FC) $< -c $(CFLAGS) $(INCL)

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

TESTS = testtime
TESTLIBS = $(AQMS_LIB) $(QLIB2_LIB)

test: ${TESTS}
	$(foreach test, $^, echo -e "\n**** $(test) ****\n"; ./$(test);)

testtime: testtime.o $(LIB_NAME).a
	$(CXX) -o $@ -g $@.o $(TESTLIBS)

testltv: testltv.o $(LIB_NAME)
	$(CXX) -o $@ $@.o $(TESTLIBS)

veryclean:  clean
	-rm -f testtime testltv
	makedepend
	-rm -f Makefile.bak

depend:
	makedepend $(INCL) $(CPPINC) *.c *.C

printvars:
	@echo CC=$(CC)
	@echo FLAGS=$(FLAGS)
	@echo OBJS=$(OBJS)
	@echo LIB_NAME=$(LIB_NAME)
	@echo RTLIBPATH=$(RTLIBPATH)
	@echo QLIB2LIBS=$(QLIB2LIBS)
	@echo SVR4LIBS=$(SVR4LIBS)
	@echo TESTLIBS=$(TESTLIBS)
# DO NOT DELETE THIS LINE -- make depend depends on it.
