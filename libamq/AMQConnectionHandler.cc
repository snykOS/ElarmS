/*********************************************************

File Name :
        ConnectionHandler.cc

Original Author:
        Kalpesh Solanki


Creation Date: 25/6/2010


Usage Notes:


*********************************************************/

// Various include files
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "GenLimits.h"
#include "RetCodes.h"
#include "AMQConnectionHandler.h"
#include "MessageTypes.h"
#include "Configuration.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "QWMessageParser.h"
#include "QWMessageSerializer.h"

#include <plog/Log.h>

const string RCSID_AMQConnectionHandler_cc = "$Id: AMQConnectionHandler.cc 0000 2019-05-13 16:08:00Z claude $";

AMQConnectionHandler* AMQConnectionHandler::handle = 0;


// caf 2019-05-10 -- force new AMQ connection
// this is needed because of the static handle
void AMQConnectionHandler::reset() {
    LOGD << __FILE__ << ":" << __FUNCTION__ << ": forcing AMQ reconnect" << std::endl;
    if (handle != NULL) {
        delete handle; handle = NULL;
    }
    initialized = false;
    valid = false;
}

// Denotes if this module has been initialized

int AMQConnectionHandler::initialize(const char* configfile_name, //system configuration file name//
			const char* module_name    //Unique name for each program 
			   ){

  if(initialized)
    return TN_SUCCESS;

  //Initialize XML
  try{
    XMLPlatformUtils::Initialize();
  }
  catch(const XMLException& e){
    char* msg = XMLString::transcode(e.getMessage());
    _print(ERROR,"Error (AMQConnectionHandler::initialize): Can't initialize XML parser:" << msg );
    XMLString::release(&msg);
    return (TN_FAILURE);
  }

  static amqPropertiesST* prop = NULL;
  // Read config file once
  if (prop == NULL) {
      try{
          prop = amqPropertiesST::getInstance();
          prop->init(configfile_name);
      }
      catch(RTException e){
          LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error in Config file: " << e.msg << std::endl
              << "Aborting!";
          exit(9);
      }
  }

  //set params
  module = module_name;
  project = prop->getDomain();
  username = prop->getUsername();
  password = prop->getPassword();


  //Create ActiveMQ connection
  try{
    brokerURI = composeBrokerURI(prop);
    LOGI << __FILE__ << ":" << __FUNCTION__ << ": URI=" << brokerURI;

    // Create a ConnectionFactory
    auto_ptr<cms::ConnectionFactory> connectionFactory(
	  cms::ConnectionFactory::createCMSConnectionFactory( brokerURI ) );
    
    // Create a Connection
    string temp = password.substr(0, 1) + "..." + password.substr(password.length()-1, 1);
    LOGI << __FILE__ << ":" << __FUNCTION__ << ": Connecting to ActiveMQ with user " << username << "/" << temp;
    connection = connectionFactory->createConnection(username,password,project+"."+module);
    connection->start();

    // Create a Session
    LOGD << __FILE__ << ":" << __FUNCTION__ << ": Creating ActiveMQ session";
    session = connection->createSession( cms::Session::AUTO_ACKNOWLEDGE);
  } catch ( cms::CMSException& e ) {
      LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error creating session." << std::endl
          << e.getStackTraceString();
      return (TN_FAILURE);
  }
  
  parser = new QWMessageParser();

    initialized = true;
    valid = true;
    return(TN_SUCCESS);
}

string AMQConnectionHandler::composeBrokerURI(amqPropertiesST* prop){
  if(prop==NULL)
    return "";

  string server1,server2,server3,server4,server5;
  string uri = "";
  bool failover = true;

  server1 = prop->getServer1();
  server2 = prop->getServer2();
  server3 = prop->getServer3();
  server4 = prop->getServer4();
  server5 = prop->getServer5();

  if(server2 == "none" &&
     server3 == "none" &&
     server4 == "none" &&
     server5 == "none"){
    failover = false;
  }

  string serverlist = "";
  serverlist += "tcp://"+server1;
  if(server2 != "none"){
    serverlist += ",tcp://"+server2;
  }
  if(server3 != "none"){
    serverlist += ",tcp://"+server3;
  }
  if(server4 != "none"){
    serverlist += ",tcp://"+server4;
  }
  if(server5 != "none"){
    serverlist += ",tcp://"+server5;
  }
  
  if(failover){
    uri = "failover:("+serverlist+")?";
  }
  else{
    uri = serverlist+"?";
  }

  uri += "initialReconnectDelay="+prop->getinitialReconnectDelay()+"&";
  uri += "maxReconnectDelay="+prop->getmaxReconnectDelay()+"&";
  uri += "useExponentialBackoff="+prop->getuseExponentialBackOff()+"&";
  uri += "maxReconnectAttempts="+prop->getmaxReconnectAttempts()+"&";
  uri += "startupMaxReconnectAttempts="+prop->getstartupMaxReconnectAttempts()+"&";
  uri += "randomize="+prop->getrandomize()+"&";
  uri += "backup="+prop->getbackup()+"&";
  uri += "backupPoolSize="+prop->getbackupPoolSize()+"&";
  uri += "timeout="+prop->gettimeout();

  return uri;

}


AMQConnectionHandler::AMQConnectionHandler(){
  project = "";
  username = "";
  password = "";
  module = "";
  listenNextFlag = false;

  valid = false;
  initialized = false;
  handle = NULL;

  session = NULL;
  connection = NULL;
}


AMQConnectionHandler::~AMQConnectionHandler(){
    //Disconnect//
    try{
    if (parser != NULL) {
        parser = NULL; parser = NULL;
    }

        // Close open resources.
        try{
            if( session != NULL ) session->close();
            if( connection != NULL ) connection->close();
        } catch ( cms::CMSException& e ) {
            LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error closing connection." << std::endl
                << e.getStackTraceString();
        }

        try{
            if( session != NULL ) delete session;
        } catch ( cms::CMSException& e ) {
            LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error deleting session." << std::endl
                << e.getStackTraceString();
        }
        session = NULL;

        try{
            if( connection != NULL ) delete connection;
        } catch ( cms::CMSException& e ) {
            LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error deleting connection." << std::endl
                << e.getStackTraceString();
        }
        connection = NULL;

	//delete producers and consumers
	for(MessageConsumerMap::iterator cit = mcm.begin();cit!= mcm.end();++cit){
	  delete (*cit).second;
	}	
	mcm.clear();
    
	for(MessageProducerMap::iterator pit = mpm.begin();pit!= mpm.end();++pit){
	  delete (*pit).second;
	}	
	mpm.clear();
    }
    catch(...){
      _print(ERROR,"Error in AMQConnectionHandler::~AMQConnectionHandler()");
    }
}

AMQConnectionHandler* AMQConnectionHandler::getInstance(){
    if(handle == NULL){
	handle = new AMQConnectionHandler();
    }
    return handle;
}

int AMQConnectionHandler::listen(double timeout){
  TimeStamp starttime = TimeStamp::current_time();
  TimeStamp endtime = starttime;

  bool got_msg = false;
  while((double)(endtime - starttime) < timeout){
    for(MessageConsumerMap::iterator cit = mcm.begin();cit!= mcm.end();++cit){
      cms::MessageConsumer *consumer = (*cit).second;
      try{
	cms::Message* cmsmsg = consumer->receiveNoWait();
	if(cmsmsg==NULL){
	  continue;
	}

	got_msg = true;
	const cms::TextMessage* textMessage =
	  dynamic_cast< const cms::TextMessage* >( cmsmsg );
	string text = "";
	const char* msgstr = NULL;
	
	if( textMessage != NULL ) {
	  text = textMessage->getText();
	  msgstr = text.c_str();
	}
	else{
	  _print(ERROR,"Error in AMQConnectionHandler::listen(): Message is not a TextMessage");
	  return (TN_FAILURE);
	}

	ContentTable *ctable = parser->parse(msgstr);

	if(!ctable){
	    _print(ERROR,"Error in AMQConnectionHandler::listen(): QWMessageParser returned NULL ContentTable object for message data");
	    return (TN_FAILURE);
	}
	
	int type = Message::getType(parser->getType());
	Message *msg = new Message(ctable,type);
	msg->setDomainName(parser->getDomainName());
	msg->setSourceName(parser->getSourceName());
	msg->setDest(parser->getChannelName());

	//Check here if we have subscribed for the channel from which we have received this message
	string subject  = (char*)msg->getDest();
	SubjectList::iterator subit = find(subjectlist.begin(),subjectlist.end(),subject);
	if(subit == subjectlist.end()){
	  unsubscribe((char*)subject.c_str());
	  _print(DBUG,"Unsubscribed: "<<subject);
	  delete ctable;
	  delete cmsmsg;
	  delete msg;
	  continue;
	}

	////////
	
	if(cbm[type]!=NULL){
	  (*cbm[type])(*msg,carg[type]); //Calling call back function//
	  
	  delete ctable;
	  delete cmsmsg;
	  delete msg;
	  if(listenNextFlag == true){
	    return(TN_SUCCESS);
	  }
	}
	else{
	  delete ctable;
	  delete cmsmsg;
	  delete msg;
	  _print(ERROR,"Error in AMQConnectionHandler::listen(): Received wrong message, This program is not subscribed to this message type");
	}	
      }
      catch(cms::CMSException e) {
          LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error in listener." << std::endl
              << e.getStackTraceString();
          return (TN_FAILURE);
      }
    }

  if(got_msg == false){
    sleep(1);
  }
  got_msg = false;
  }
  
}


int AMQConnectionHandler::listenNext(double timeout){
  listenNextFlag = true;
  int ret = listen(timeout);
  listenNextFlag = false;
  return ret;
}


int AMQConnectionHandler::registerHandler(int mtype, 
				void (*func)(Message&, void *), void *arg){

    if(!valid)
	return (TN_FAILURE);

    if(!func)
	return (TN_FAILURE);

    cbm[mtype] = func;
    carg[mtype]= arg;
    return TN_SUCCESS;
}


int AMQConnectionHandler::send(Message& m){
    QWMessageSerializer s; 
    bool msgsafe = false;
    const char* buf = NULL;
    string destStr = "";

    if(!valid)
	return (TN_FAILURE);


    m.setDomainName(project.c_str());
    m.setSourceName(module.c_str());

    bool fail = false;
    try{
	    buf = s.serialize(m);
	    if(!buf){
		//Log message//
		_print(ERROR,"Error in  AMQConnectionHandler::send: Unable to serialize the message");
		//Return error//
		delete m.getContentTable();
		return (TN_FAILURE);
	    }

	    int skip = strstr(buf,"?>") - buf;
	    if(skip<=0){
	      //error
	    }

	    char* buf_ptr = (char*)buf;
	    skip += 2;
	    buf_ptr += skip;
	    
	    
	    //Send message to the broker
	    destStr = project + "." + string(m.getDest());
	    cms::MessageProducer *producer = NULL;
	    MessageProducerMap::iterator mpmit = mpm.find(destStr);
	    if(mpmit == mpm.end()){
            // Create a MessageProducer from the Session to the Topic or Queue
	      cms::Destination *dest = NULL;
	      if(amqPropertiesST::getInstance()->getChannelType() == "TOPIC"){
		dest = session->createTopic(destStr);
	      }
	      else{
	        dest = session->createQueue(destStr);
	      }

	      producer = session->createProducer(dest);
	      producer->setDeliveryMode( cms::DeliveryMode::PERSISTENT );
	      mpm[destStr] = producer;
	    }
	    else{
	      producer = (*mpmit).second;
	    }
 
	    cms::TextMessage* message = session->createTextMessage(buf_ptr);
	    LOGD << __FILE__ << ":" << __FUNCTION__ << ": sending [" << buf_ptr << "]";
	    producer->send( message );
	    delete message;
	    ///////////////////////////
	    free((void*)buf);
	    buf = NULL;
    }
    catch(cms::CMSException& e ){
        LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error sending." << std::endl
            << e.getStackTraceString();
        fail = true;
    }
    catch(RTIOException e){
        LOGE << __FILE__ << ":" << __FUNCTION__ << ": I/O Error occured while sending message" << std::endl
            << "Removed queue: All pending messages are lost";
        fail = true;
    }
    catch(RTOutOfMemoryException e){
        LOGE << __FILE__ << ":" << __FUNCTION__ << ": System is out of Memory!";
        fail = true;
    }
    catch(RTException e){
        LOGE << __FILE__ << ":" << __FUNCTION__ << ": RTException is raised, Can't send message";
        fail = true;
    }
    catch(...){
        LOGE << __FILE__ << ":" << __FUNCTION__ << ": Unknown exception. Can't send message";
        fail = true;
    }
    return TN_SUCCESS;
}


int AMQConnectionHandler::flush(){
    if(!valid)
	return (TN_FAILURE);

    return(TN_SUCCESS);
}

int AMQConnectionHandler::subscribe(char *subject){
    if(!valid)
	return (TN_FAILURE);

    if(!subject)
	return TN_FAILURE;

    try{
    ///
     // Create the destination (Topic or Queue)
      string subjectStr = project + "." + subject;
      string consumerID = project + "." + module + "." + subject;
      cms::MessageConsumer *consumer = NULL;
 
      if(amqPropertiesST::getInstance()->getChannelType() == "TOPIC"){
	cms::Topic* topic = session->createTopic(subjectStr);	
	consumer = session->createDurableConsumer(topic,consumerID,"",false);
      }
      else{
	cms::Destination *dest = session->createQueue(subjectStr);
	consumer = session->createConsumer(dest,"",false);
      }
          
      // Create a MessageConsumer from the Session to the Topic or Queue
      mcm[subjectStr] = consumer;
    }
    catch(cms::CMSException& e ) {
        LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error subscribing." << std::endl
            << e.getStackTraceString();
        return (TN_FAILURE);
    }

    string str = subject;
    subjectlist.push_back(str);
   
    return (TN_SUCCESS);
}

int AMQConnectionHandler::unsubscribe(char *subject){

  string subjectStr = project + "." + string(subject);
  MessageConsumerMap::iterator it = mcm.find(subjectStr);
  if(it != mcm.end()){
    //remove message consumer for this subject/channel
    delete (*it).second;
  }

  try{
    session->unsubscribe(subjectStr);
  }
  catch(cms::CMSException& e){
      LOGE << __FILE__ << ":" << __FUNCTION__ << ": Error unsubscribing." << std::endl
          << e.getStackTraceString();
      return (TN_FAILURE);
  }
  return (TN_SUCCESS);
}

int AMQConnectionHandler::isAlive(){
 return (TN_FALSE);
}

int operator!(const AMQConnectionHandler& conn){
    return !conn.valid || !conn.initialized;
}
