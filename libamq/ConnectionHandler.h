/***********************************************************

File Name :
        ConnectionHandler.h

Original Author:
        Kalpesh Solanki

**********************************************************/

#ifndef connectionhandler_H
#define connectionhandler_H

// Various include files
//#include "ConnectionStructs.h"
#include "ContentTable.h"
#include "RTException.h"
#include "Message.h"
#include "QWFeederC.h"
#include "QWPullSubServicesC.h"
//#include "QWPushSubServicesC.h"
#include "QWPushSubServicesI.h"
#include "QWSubscriptionC.h"
#include "FileMessageQueue.h"
#include "QWMessageParser.h"
#include "printswitch.h"
#include "TimeStamp.h"

#include "orbsvcs/orbsvcs/CosNamingC.h"
#include "ace/streams.h"
#include <map>
#include <string>
#include <pthread.h>

using namespace std;
using namespace com::isti::quakewatch::server::qw_feeder;
using namespace com::isti::quakewatch::corbaclient::qw_client_rec;

typedef map<int,void (*)(Message&, void*)> CallBackMap;
typedef map<int,void*> CallBackArgs;
typedef list<string> SubjectList;

//const double CONN_FOREVER = 999999999;

enum server_state{UP=0,DOWN};

class server{
 public:
    char* hostname;
    char* port;
    char* subport;
    server_state state;
    int retry;
};

typedef list<server*> serverlist;


class ConnectionHandler : public com_isti_quakewatch_corbaclient_qw_client_rec_QWRecMsgCallBack_i{
 private:
    static bool initialized;

    //Config parameters//
    static string project;
    static serverlist servers;
    static string module;
    static string corbaversion;
    static unsigned short maxconnretry;
    static unsigned int maxqfilesize;
    static string qfiledir;
    static string qfilename_send;
    static string qfilename_recv;
    static int sleepduration;
    static string subscriber_port;
    static string subscriber_host;
    static pthread_t rcv_thread;

    static ConnectionHandler* conn;
    static CORBA::ORB_var orb;
    
    bool                   valid;
    CallBackMap            cbm;
    CallBackArgs           carg;
    SubjectList            subjectlist;
    FileMessageQueue* fmq;
    FileMessageQueue* fmq_recv;
    QWMessageParser *parser;
    
    /*mutexes and condition variables*/
    pthread_mutex_t fmq_recv_lock;
    pthread_cond_t fmq_recv_cond;
    TimeStamp pingtimer;

    string qwfeeder_corbaloc;
    string qwsub_corbaloc;
    
    QWFeeder_var           qwfeeder;
    QWSubscription_var     qwsub;
    QWPullSubServices_var  qwpullsub;  
    QWPushSubServices_var  qwpushsub;  
    char* msgstr_ptr;
    
    // Copy Constructor - This constructor is moved here to make sure that NO ONE use it.  -Kalpesh
    //
    ConnectionHandler(const ConnectionHandler&);

  // Assignment operator for assigning the contents of one ConnectionHandler
  // object to another
  //
  ConnectionHandler& operator=(const ConnectionHandler &);//This function is moved here to make sure that NO ONE use it.  -Kalpesh
  // Constructor
  //
  // Arguments:
  //            None.
  //
  ConnectionHandler();

  // Destructor
  //
  ~ConnectionHandler();

  int setQWFeeder();
  int setQWSubscription();
  int setQWPullSubServices();
  int setQWPushSubServices();
  int reconnect();
  int quick_reconnect();
  FileMessageQueue* createFileMessageQueue(string);
  int sendOldMessages();
  Message* getNextMessage(double);
  
 public:

  // Initialize the ConnectionHandler class. This static method defines the
  // message types used in the Real-time system.
  //
  static int initialize(const char* configfile_name, //system configuration file name//
			const char* module_name    //Unique name for each program 
			);

  static ConnectionHandler* getConnectionHandler();
  static void removeConnectionHandler();
  static void* receiver_loop(void*); 

  //CORBA::Boolean receiveMessage(const char*) throw(CORBA::SystemException);

  
  CORBA::Boolean receiveMessage (const char * xmlMsgStr)
     ACE_THROW_SPEC ((
  	       CORBA::SystemException
  	       ));

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

  // Get the number of pending messages in the message queue
  int getNumPending(int &num);


  // Boolean operator which returns TN_TRUE when the ConnectionHandler object
  // is not valid, and TN_FALSE when the object references a valid and
  // open RTserver connection.
  //
  friend int operator!(const ConnectionHandler&);

};

#endif
