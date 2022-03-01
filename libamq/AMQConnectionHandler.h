/***********************************************************

File Name :
        AMQConnectionHandler.h

Original Author:
        Kalpesh Solanki

**********************************************************/

#ifndef amqconnectionhandler_H
#define amqconnectionhandler_H

// Various include files
//#include "ConnectionStructs.h"

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Integer.h>
#include <activemq/util/Config.h>
#include <decaf/util/Date.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <pthread.h>

#include "ContentTable.h"
#include "RTException.h"
#include "Message.h"
#include "QWMessageParser.h"
#include "printswitch.h"
#include "TimeStamp.h"
#include "amqPropertiesST.h"

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace std;


typedef map<int,void (*)(Message&, void*)> CallBackMap;
typedef map<int,void*> CallBackArgs;
typedef map<string,cms::MessageProducer*> MessageProducerMap;
typedef map<string,cms::MessageConsumer*> MessageConsumerMap;
typedef list<string> SubjectList;

class AMQConnectionHandler{

 private:
    bool initialized;

    //Config parameters//
    string brokerURI;
    string project;
    string username;
    string password;
    string module;
    unsigned short maxconnretry;
    unsigned int maxqfilesize;
    int sleepduration;
    static AMQConnectionHandler* handle;
    cms::Connection* connection;
    cms::Session* session;

    //Data Structures
    MessageProducerMap mpm;
    MessageConsumerMap mcm;

    ///
    bool                   valid;
    bool  listenNextFlag;
    CallBackMap            cbm;
    CallBackArgs           carg;
    SubjectList            subjectlist;
    QWMessageParser *parser;
    
    // Copy Constructor - This constructor is moved here to make sure that NO ONE use it.  -Kalpesh
    //
    AMQConnectionHandler(const AMQConnectionHandler&);

  // Assignment operator for assigning the contents of one ConnectionHandler
  // object to another
  //
  AMQConnectionHandler& operator=(const AMQConnectionHandler &);//This function is moved here to make sure that NO ONE use it.  -Kalpesh
  // Constructor
  //
  // Arguments:
  //            None.
  //
  AMQConnectionHandler();

  // Destructor
  //
  ~AMQConnectionHandler();

  string composeBrokerURI(amqPropertiesST*);
 
 public:

  // Initialize the ConnectionHandler class. This static method defines the
  // message types used in the Real-time system.
  //
  int initialize(const char* configfile_name, //system configuration file name//
			const char* module_name    //Unique name for each program 
			);

  // caf 2019-05-10 -- force new AMQ connection
  // this is needed because of the static handle
  void reset();

  static AMQConnectionHandler* getInstance();

  // Listens on the RTserver connection for incoming messages on the
  // subscribed subjects. Listens for up to the timeout value provided
  // in the argument.
  //
  // This method may return before the timeout if a SmartSockets
  // error occurs while listening.
  //
  int listen(double);


  // Listens on the RTserver connection for an incoming message
  // on the subscribed subjects. Listens for up to the timeout value
  // provided in the first argument. If a message is received before
  // the timeout expires, the message is processed through the
  // normal callback mechanism.
  //
  int listenNext(double);

  // Registers a callback function for a specific message type. The
  // user-defined function must take a Message object (the received
  // message) as an argument.
  //
  int registerHandler(int, void (*func)(Message&, void*), void *);

  void onMessage( const cms::Message* message );

  // Subscribe to a subject. //It must be an opaque string//
  int subscribe(char *);

  // Unsubscribe to a subject.
  int unsubscribe(char *);

  // Send a Message object over an open connection.
  //
  int send(Message&);

  // Flush all outgoing messages over the connection
  //
  int flush();

  // Check if the ConnectionHandler is alive
  //
  int isAlive();

  // Boolean operator which returns TN_TRUE when the ConnectionHandler object
  // is not valid, and TN_FALSE when the object references a valid and
  // open RTserver connection.
  //
  friend int operator!(const AMQConnectionHandler&);

};

#endif
