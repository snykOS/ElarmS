AMQ_DIR		= /home/elarms/run/lib/amq_cms
AMQ_INC_DIR	= $(AMQ_DIR)/include/activemq-cpp
AMQ_LIB_DIR	= $(AMQ_DIR)/lib

APR_DIR		= /home/elarms/run/lib/apr
APR_INC_DIR	= $(APR_DIR)/include/apr-1
APR_LIB_DIR	= $(APR_DIR)/lib

SSL_DIR		= /usr/local
SSL_INC_DIR	= $(SSL_DIR)/include
SSL_LIB_DIR	= $(SSL_DIR)/lib

DM_DIR		= ../../libs/dmlib
DM_INC_DIR	= $(DM_DIR)
DM_LIB_DIR	= $(DM_DIR)

QLIB2_DIR	= /usr/local
QLIB2_INC_DIR	= $(QLIB2_DIR)/include
QLIB2_LIB_DIR	= $(QLIB2_DIR)

XERCES_DIR      = ../../stlport4
XERCES_INC_DIR  = $(XERCES_DIR)/include
XERCES_LIB_DIR  = $(XERCES_DIR)/lib
XERCES_LIB      = -L$(XERCES_LIB_DIR) -lxerces-c

DM_LIB		= -L$(DM_LIB_DIR) -ldm
AMQ_LIB 	= -L$(AMQ_LIB_DIR) -lactivemq-cpp
APR_LIB 	= -L$(APR_LIB_DIR) -lapr-1 -laprutil-1
SSL_LIB		= -L$(SSL_LIB_DIR) -lssl
QLIB2_LIB	= -L$(QLIB2_LIB_DIR) -lqlib2

#LIBS		= $(DM_LIB) $(AMQ_LIB) $(APR_LIB) $(SSL_LIB) $(QLIB2_DIR) -liconv
LIBS		= $(DM_LIB) $(AMQ_LIB) $(APR_LIB) $(QLIB2_LIB) $(XERCES_LIB) -liconv

NOWARN	= -erroff=wvarhidemem,temwhilespec,temspecend,hidevf

CCC	= CC
CCFLAGS	= -g -O0 -I$(AMQ_INC_DIR) -I$(APR_INC_DIR) -I$(DM_INC_DIR) -I$(XERCES_INC_DIR) -library=stlport4 $(NOWARN)

OBJS	= cmdline_sender.o

cmdline_sender:	$(OBJS)
	$(CCC) $(CCFLAGS) -o $@ $(OBJS) $(LIBS)

%.o:	%.cpp
	$(CCC) -c $(CCFLAGS) $<

clean:
	-rm *.o

veryclean:	clean
	-rm cmdline_sender
