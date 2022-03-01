
// Various include files
#include <stdio.h>
#include "GenLimits.h"
#include "RetCodes.h"
#include "Connection.h"
#include "MessageTypes.h"

#include <plog/Log.h>

const string RCSID_Connection_C = "$Id: Connection.C 0000 2019-05-13 16:20:00Z claude $";

Connection::Connection()
{
    connhandler = NULL;
}


Connection::Connection(const Connection &c)
{
    connhandler = c.connhandler;
}



Connection::Connection(const char *configfile, const char *module)
{
    LOGD << __FILE__ << ":" << __FUNCTION__ << ": Initializing ActiveMQ Connection";
    connhandler = AMQConnectionHandler::getInstance();
    connhandler->initialize(configfile,module);
}


Connection::~Connection()
{
    connhandler->reset();
}


int Connection::setDeliveryMode(int mtype, int delmode)
{
    if(!connhandler)
	return (TN_FAILURE);
    return (TN_SUCCESS);
}



int Connection::listen(double timeout)
{
    if(!connhandler)
	return (TN_FAILURE);
    if(timeout <= 0)
	return (TN_SUCCESS);

    return connhandler->listen(timeout);
}



int Connection::listenNext(double timeout)
{
    if(!connhandler)
	return (TN_FAILURE);
    if(timeout <= 0)
	return (TN_SUCCESS);

    return connhandler->listenNext(timeout);
}


int Connection::registerHandler(int mtype, 
				void (*func)(Message&, void *), void *arg)
{
    if(!connhandler)
	return (TN_FAILURE);
    return connhandler->registerHandler(mtype,func,arg);
}




int Connection::send(Message& m)
{
    if(!connhandler)
	return (TN_FAILURE);
    return connhandler->send(m);
}



int Connection::flush()
{
    if(!connhandler)
	return (TN_FAILURE);
    return connhandler->flush();
}


int Connection::subscribe(char *subject)
{
    if(!connhandler)
	return (TN_FAILURE);
    return connhandler->subscribe(subject);
}


int Connection::unsubscribe(char *subject)
{
    if(!connhandler)
	return (TN_FAILURE);
    return connhandler->unsubscribe(subject);
}



int Connection::isAlive()
{
    if(!connhandler)
	return (TN_FAILURE);
    return connhandler->isAlive();
}



int Connection::getNumPending(int &num)
{
  num = 0;
  return (TN_SUCCESS);
}



int operator!(const Connection& conn)
{
  if(!conn.connhandler)
      return (TN_TRUE);
  return (!(*(conn.connhandler)));
}



Connection& Connection::operator=(const Connection &c)
{
    connhandler = c.connhandler;
    return *this;
}
