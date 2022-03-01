#ifndef __messagesender_h
#define __messagesender_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>

using namespace activemq::transport;
using namespace decaf::lang;
using namespace cms;
using namespace std;

// rcsid version strings
#define RCSID_MessageSender_h "$Id: MessageSender.h $"
extern const std::string RCSID_MessageSender_cc;

class MessageSender {
 private:
  //CMS vars
  static bool isInit;
  static cms::Connection* conn;
  static Session* session;
  static Destination* destination;
  static MessageProducer* producer;
  static std::string topic_name;

  static void run();
  
 public:
  static void init(cms::Connection* connection, string topic);
  static void sendMessage(const string msgstr);

  static void send( const string message );
  static pthread_mutex_t lock;
}; // class MessageSender

#endif
