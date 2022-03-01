/***********************************************************

File Name :
        Connection.h

Original Author:
        Patrick Small

Description:

        This header file defines the Connection class - an
object class which abstracts the use of Talarian SmartSockets
publish-subscribe connections.


Creation Date:
        06 July 1998

Modification History:


Usage Notes:

        User wishing to make use of Connections and Messages
should include only this header file in their programs.

        To use the Connection class, it must first be initialized
with a call to the static member function initialize(). This method
creates all of the messages types used by the TriNet Real-time
system.
        Once initialized, a connection can be created by simply
instantiating a connection object. Note that Talarian allows only
one active client connection at a time.
        Two class constructors are provided. One accepts two
arguments: the host containing an RTserver to connect to, and a
unique string identifier for this process connection. The second
constructor takes a single argument, the unique string identifier. It
attempts to connect to a RTserver running on the current host.
        The unique string identifier can be any series of characters,
but is often a combination of the host name, process name, and PID.
The connection class uses this string to create a unique subject for
this process so that RTserver can send status and control messages.
This unique subject is also required for GMD which is not yet 
supported.
        The user process can check to make sure that the connection
was opened successfully by using the "!" overloaded boolean operator.

        To employ callbacks for certain message types, the user
must define a function "void (*func)(Message&)" and call the
method registerHandler() to register the callback function for the
desired message type. When the user program executes the listen()
method and a message arrives whose type has a registered handler,
the callback is executed.

        Most functions provide an integer return value which is
TN_SUCCESS if the operation was successful, or TN_FAILURE otherwise.

**********************************************************/

#ifndef connection_H
#define connection_H

// Various include files
//#include "ConnectionStructs.h"
#include "Message.h"
#include "AMQConnectionHandler.h"


// Message delivery modes
const int CONN_MODE_REL = 0;
const int CONN_MODE_GMD = 1;

// Indefinite listening timeout
const double CONN_FOREVER = 999999999;



class Connection 
{
 private:
    AMQConnectionHandler* connhandler;

 public:

  // Default Constructor
  //
  // This is a dummy constructor which does not yield a valid connection.
  //
  Connection();


  // Copy Constructor
  //
  Connection(const Connection&);


  // Constructor
  //
  // Arguments: Path and filename of SETOPT configuration file
  //            Module/program identifier
  //
  Connection(const char *, const char *);


  // Destructor
  //
  ~Connection();

  // Sets the delivery mode to be Reliable, or GMD for the specified
  // message type. Accepts the message type and and integer argument,
  // which is either CONN_MODE_REL or CON_MODE_GMD
  //
  int setDeliveryMode(int, int);

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

  // Subscribe to a subject. The subject string may contain the
  // wildcard characters "*" and "..." as described in the SmartSockets
  // User's Guide for subject wildcards.
  //
  int subscribe(char *);

  // Unsubscribe to a subject. The subject string may contain the
  // wildcard characters "*" and "..." as described in the SmartSockets
  // User's Guide for subject wildcards.
  int unsubscribe(char *);

  // Send a Message object over an open connection.
  //
  int send(Message&);

  // Flush all outgoing messages over the connection
  //
  int flush();

  // Check if the Connection is alive
  //
  int isAlive();

  // Get the number of pending messages in the message queue
  int getNumPending(int &num);

  // Initialize the Connection class. This static method defines the
  // message types used in the Real-time system.
  //
  static int initialize();

  // Boolean operator which returns TN_TRUE when the Connection object
  // is not valid, and TN_FALSE when the object references a valid and
  // open RTserver connection.
  //
  friend int operator!(const Connection&);

  // Assignment operator for assigning the contents of one Connection
  // object to another
  //
  Connection& operator=(const Connection &);

};

#endif
