/***********************************************************

File Name :
        ConnectionHandler.cc

Original Author:
        Kalpesh Solanki

Description:

        This source file defines the ConnectionHandler class


Creation Date:
        Sometime in January 2005


Usage Notes:


        Most functions provide an integer return value which is
TN_SUCCESS if the operation was successful, or TN_FAILURE otherwise.

**********************************************************/
// Various include files
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "GenLimits.h"
#include "RetCodes.h"
#include "ConnectionHandler.h"
#include "MessageTypes.h"
#include "Configuration.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "QWMessageParser.h"
#include "QWMessageSerializer.h"


const int PROJECT          = 0x0001;
const int HOST             = 0x0002;
const int PORT             = 0x0004;
const int CORBAVER         = 0x0008;
const int MAXQFILESIZE     = 0x0010;
const int MAXCONNRETRY     = 0x0020;
const int QFILEDIR         = 0x0040;
const int SUBSCRIBER_HOST  = 0x0080;
const int SUBSCRIBER_PORT  = 0x0010;
const int SLEEP_DURATION   = 0x0020;

pthread_t ConnectionHandler::rcv_thread;
//com_isti_quakewatch_corbaclient_qw_client_rec_QWRecMsgCallBack_i*  ConnectionHandler::callbackobj = NULL;
serverlist ConnectionHandler::servers;
string ConnectionHandler::project = "";
string ConnectionHandler::module = "";
string ConnectionHandler::corbaversion = "";
string ConnectionHandler::subscriber_host  = "localhost";
string ConnectionHandler::subscriber_port = "";
int    ConnectionHandler::sleepduration = 10;

unsigned short ConnectionHandler::maxconnretry = 0;
unsigned int ConnectionHandler::maxqfilesize = 0;
string ConnectionHandler::qfiledir = "";
string ConnectionHandler::qfilename_send = "";
string ConnectionHandler::qfilename_recv = "";
ConnectionHandler* ConnectionHandler::conn = 0;
CORBA::ORB_var ConnectionHandler::orb = 0;

// Denotes if this module has been initialized
bool ConnectionHandler::initialized = false;

void*  ConnectionHandler::receiver_loop(void* args){
  sigset_t sigset; 
  sigemptyset(&sigset);
  sigaddset(&sigset,SIGINT);
  pthread_sigmask(SIG_BLOCK,&sigset,NULL);

  orb->run();
  _print(ERROR,"FATAL ERROR : ORB returned from its run() function. Client won't be able receive callbacks");
  
  return NULL;
}

int ConnectionHandler::initialize(const char* configfile_name, //system configuration file name//
			const char* module_name    //Unique name for each program 
			   ){
    if(initialized)
	return TN_SUCCESS;

    if(!configfile_name || !module_name){
	_print(ERROR,"Error (ConnectionHandler::initialize): Illegal Arguments");
	return (TN_FAILURE);
    }

    module = module_name;

    //Initialize XML
    try{
	XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& e){
	char* msg = XMLString::transcode(e.getMessage());
	_print(ERROR,"Error (ConnectionHandler::initialize): Can't initialize XML parser:" << msg );
	XMLString::release(&msg);
	return (TN_FAILURE);
    }

    try{
	int argc=0;
	orb = CORBA::ORB_init(argc,NULL,"");

	_print(DBUG,"Activating POA");
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
	PortableServer::POAManager_var mgr = poa->the_POAManager();

	//	callbackobj = new com_isti_quakewatch_corbaclient_qw_client_rec_QWRecMsgCallBack_i();
	mgr->activate();
       	pthread_create(&rcv_thread,NULL,receiver_loop,NULL);
	//mgr->activate_object(callbackobj);
    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,"Not able to initialize ORB.");
	return(TN_FAILURE);
    }

    Configuration cfg(configfile_name);
    if(!cfg){
	_print(ERROR,"Error in (ConnectionHandler::initialize):Unable to read configuration");
	return (TN_FAILURE);
    }

    int ret = TN_EOF;
    char key[MAXSTR];
    char* value = (char*)malloc(MAXSTR);
    int checklist = 0;
    
    if(!value){
	//_print("Error in ConnectionHandler::ConnectionHandler(): Unable to allocate memory");
	return (TN_FAILURE);
    }

    while((ret=cfg.next(key,value))==TN_SUCCESS){
	if(strcasecmp(key,"Project")==0){
	    project = value;
	    checklist |= PROJECT;
	}
	else if(strcasecmp(key,"Host")==0){
	    char tempbf[1024];
	    char* hostname;
	    char* portstr;

	    strcpy(tempbf,value);
	    char* pstr = strchr(tempbf,':');
	    if(pstr==NULL){
		_print(ERROR,"Error in (ConnectionHandler::initialize):Configuration file is not in a valid format. Server info is wrong");
		return (TN_FAILURE);
	    }
	    *pstr = '\0';
	    pstr++;

	    int ws_port = atoi(pstr);
	    if(ws_port == ERANGE || ws_port == EINVAL){
		_print(ERROR,"Error in (ConnectionHandler::initialize):Configuration file is not in a valid format. Server info is wrong.");
		return (TN_FAILURE);
	    }  
	    
	    hostname = strdup(tempbf);
	    portstr = strdup(pstr);

	    server* s = new server();
	    s->hostname = hostname;
	    s->port = portstr;
	    s->state = DOWN;
	    s->retry = 0;
	    servers.push_back(s);
	    
	    checklist |= HOST;

	}
	else if(strcasecmp(key,"SubscriberHost")==0){
	    subscriber_host = value;
	    checklist |= SUBSCRIBER_HOST;
	}
	else if(strcasecmp(key,"SubscriberPort")==0){
	    int _port = atoi(value);
	    if(_port < 0 && (errno == ERANGE || errno == EINVAL)){
		_print(ERROR,"Error (ConnectionHandler::initialize): Configuration file is not in a valid format. SubscriberPort is invalid");
		return (TN_FAILURE);
	    }
	    subscriber_port = value;
	    checklist |= SUBSCRIBER_PORT;
	}
	else if(strcasecmp(key,"SleepBetweenRetries")==0){
	    int n = atoi(value);
	    if(n < 0 && (errno == ERANGE || errno == EINVAL)){
		_print(ERROR,"Error (ConnectionHandler::initialize): Configuration file is not in a valid format. SleepBetweenRetries is invalid");
		return (TN_FAILURE);
	    }
	    sleepduration = n;
	    checklist |= SLEEP_DURATION;
	}
	else if(strcasecmp(key,"CORBAVersion")==0){
	    corbaversion = value;
	    checklist |= CORBAVER;
	}
	else if(strcasecmp(key,"MaxQFileSize")==0){
	    maxqfilesize = atoi(value);
	    if(maxqfilesize < 0 && (errno == ERANGE || errno == EINVAL)){
		_print(ERROR,"Error (ConnectionHandler::initialize): Configuration file is not in a valid format. MaxQFileSize is invalid");
		return (TN_FAILURE);
	    }
	    checklist |= MAXQFILESIZE;
	}
	else if(strcasecmp(key,"MaxConnRetry")==0){
	    maxconnretry = atoi(value);
	    if(maxconnretry < 0 && (errno == ERANGE || errno == EINVAL)){
		_print(ERROR,"Error (ConnectionHandler::initialize): Configuration file is not in a valid format. MaxConnRetry is invalid");
		return (TN_FAILURE);
	    }
	    checklist |= MAXCONNRETRY;
	}
	else if(strcasecmp(key,"QueueFileHomeDir")==0){
	    qfiledir = value;
	    checklist |= QFILEDIR;
	}
    }
    
    if((checklist & PROJECT) != PROJECT){
	_print(ERROR,"Error (ConnectionHandler::initialize): Project is not specified");
	return (TN_FAILURE);
    }
    if((checklist & HOST) != HOST){
	_print(ERROR,"Error (ConnectionHandler::initialize): Host is not specified");
	return (TN_FAILURE);
    }
    if((checklist & MAXQFILESIZE) != MAXQFILESIZE){
	_print(ERROR,"Error (ConnectionHandler::initialize): MaxQFileSize is not specified");
	return (TN_FAILURE);
    }
    if((checklist & MAXCONNRETRY) != MAXCONNRETRY){
	_print(ERROR,"Error (ConnectionHandler::initialize): MaxConnRetry is not specified");
	return (TN_FAILURE);
    }
    if((checklist & CORBAVER) != CORBAVER){
	_print(ERROR,"Error (ConnectionHandler::initialize): CORBAVersion is not specified");
	return (TN_FAILURE);
    }
    if((checklist & QFILEDIR) != QFILEDIR){
	_print(ERROR,"Error (ConnectionHandler::initialize): QueueFileHomeDir is not specified");
	return (TN_FAILURE);
    }
    if((checklist & SUBSCRIBER_HOST) != SUBSCRIBER_HOST){
	_print(ERROR,"Info: SubscriberHost is not specified.");
	return (TN_FAILURE);
    }
    if((checklist & SUBSCRIBER_PORT) != SUBSCRIBER_PORT){
	_print(ERROR,"Info: SubscriberPort is not specified.");
	return (TN_FAILURE);
    }
    if((checklist & SLEEP_DURATION) != SLEEP_DURATION){
	sleepduration = 10;
    }


    initialized = true;
    return(TN_SUCCESS);
}


ConnectionHandler::ConnectionHandler(){
  pthread_mutex_init(&fmq_recv_lock,NULL);
  pthread_cond_init(&fmq_recv_cond,NULL);

    valid = false;

    qwsub_corbaloc =  "corbaloc:iiop:"+corbaversion+"@"+subscriber_host+":"+subscriber_port+"/QWSubscription";
    qfilename_send = qfiledir + "/"+ project + "_" +module + "_SEND.q";
    qfilename_recv =  qfiledir + "/"+ project + "_" +module + "_RECV.q";

    //_print("Feeder location: "<<qwfeeder_corbaloc);

    fmq = createFileMessageQueue(qfilename_send);
    if(fmq==NULL)
	return;
    fmq_recv = createFileMessageQueue(qfilename_recv);
    if(fmq_recv==NULL)
	return;

    parser = new QWMessageParser();

    if(reconnect()==TN_FAILURE){
	return;
    }

    valid = true;
}


ConnectionHandler::~ConnectionHandler(){
    //Disconnect//
    try{
	delete fmq;
	delete fmq_recv;
	delete parser;
	qwpushsub->disconnectSubscriber();
    }
    catch(...){
	//Even if it fails doesn't matter. JAVA client receiver will take care of it//
    }
}


ConnectionHandler* ConnectionHandler::getConnectionHandler(){
    if(conn == NULL){
	conn = new ConnectionHandler();
    }
    return conn;
}

void ConnectionHandler::removeConnectionHandler(){
    if(!conn)
	return;
    delete conn;
    conn = NULL;
}

//Callback function. Will be invoked by Corba server in a separate thread.//
CORBA::Boolean ConnectionHandler::receiveMessage(const char* msgstr)  throw(CORBA::SystemException){
    ContentTable* ctable = NULL;
    Message *msg =  NULL;
    bool fail = false;
    _print(DBUG,"*********  In receiveMessage(): Got message string");
    try{
	_print(DBUG,"In receiveMessage(): Parsing message string");
	ctable = parser->parse(msgstr);
	_print(DBUG,"In receiveMessage(): message parsed");
	if(!ctable){
	    _print(ERROR,"Error in ConnectionHandler::receiveMessage: QWMessageParser returned NULL ContentTable object for message data");
	    return false;
	}
	
	int type = Message::getType(parser->getType());
	msg = new Message(ctable,type);
	msg->setDomainName(parser->getDomainName());
	msg->setSourceName(parser->getSourceName());
	msg->setDest(parser->getChannelName());
	
	_print(DBUG,"In receiveMessage(): Waiting for fmq_recv_lock");
       	pthread_mutex_lock(&fmq_recv_lock);
	_print(DBUG,"In receiveMessage(): Got fmq_recv_lock");
	try{
	    fail = true;
	    _print(DBUG,"In receiveMessage(): Putting message in the queue...");
	    fmq_recv->put(*msg);
	    _print(DBUG,"In receiveMessage(): Message put finished");
	    fail = false;
	}
	catch(RTFileCorruptedError e){
	    _print(ERROR,"Error in ConnectionHandler::send: Message queue file is corrupted! Can't send message");
	    fmq_recv->reset();
	}
	catch(...){
	}
	if(fail == true){
	  _print(DBUG,"In receiveMessage(): Unlocking fmq_recv_lock");
	  pthread_mutex_unlock(&fmq_recv_lock);
	    return false;
	}
	
	_print(DBUG,"In receiveMessage(): Signalling on fmq_recv_cond");
	pthread_cond_signal(&fmq_recv_cond);
	_print(DBUG,"In receiveMessage(): Unlocking fmq_recv_lock");
	pthread_mutex_unlock(&fmq_recv_lock);
	_print(DBUG,"In receiveMessage(): Unlocked fmq_recv_lock");
    }
    catch(...){
	_print(ERROR,"In receiveMessage(): ERROR. Something went wrong");
	if(ctable)
	    delete ctable;
	if(msg)
	    delete msg;
	return false;
    }
    _print(DBUG,"In receiveMessage(): Returning true to the server");
    return true;
}


Message* ConnectionHandler::getNextMessage(double seconds){
    Message* msg = NULL;
    int qsize = 0;
    bool fail = false;

    _print(DBUG,"In getNextMessage: Waiting on fmq_recv_lock");
    pthread_mutex_lock(&fmq_recv_lock);
    _print(DBUG,"In getNextMessage: Got fmq_recv_lock");
    
    try{
	qsize = fmq_recv->pending();
    }
    catch(RTIOException e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: I/O Error occured while checking queue size");
	fmq_recv->reset();
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: Removed queue, All pending messages are lost");
	fail = true;
    }
    catch(RTOutOfMemoryException e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: System is out of Memory!");
	fail = true;
    }
    catch(RTFileCorruptedError e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: Message queue file is corrupted! Can't send message");
	fmq_recv->reset();
	fail = true;
    }
    catch(RTException e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: RTException is raised, Can't send message");
	fail = true;
    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: CORBA exception occured, Can't send message");
	fail = true;
    }
    catch(...){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: Unknown exception. Can't send message");
	fail = true;
    }
    if(fail==true){
	_print(DBUG,"In getNextMessage: Failed. Can't find receive queue size");
	pthread_mutex_unlock(&fmq_recv_lock);
	_print(DBUG,"In getNextMessage: Unlocked fmq_recv_lock");
	sleep(2);
	return NULL;
    }

    if(qsize==0){
      _print(DBUG,"In getNextMessage: Receiver Queue is empty.");
      //     return NULL;

      /*Remove this portion if you want to make it single threaded [ */
	struct timespec timeout;
	struct timeval now;
	gettimeofday(&now,NULL);

	timeout.tv_sec = now.tv_sec + (long)seconds;
	// tv_usec is microseconds; tv_nsec is nanoseconds; be careful with units!
	timeout.tv_nsec = now.tv_usec * 1.0e+3;
	timeout.tv_nsec += (long)((seconds - (long)seconds) * 1.0e+9);
	if (timeout.tv_nsec >= 1000000000) {
	    timeout.tv_nsec -= 1000000000;
	    timeout.tv_sec++;
	}
	
	_print(DBUG,"In getNextMessage: Waiting on fmq_recv_cond for "<<seconds<<" seconds");
	
	int erno=0;
	if((erno = pthread_cond_timedwait(&fmq_recv_cond,&fmq_recv_lock,&timeout))!=0){
	    if(erno==ETIMEDOUT){
		_print(DBUG,"In getNextMessage: ERROR: call timedout");
	    }
	    else if(erno==EINVAL){
		_print(DBUG,"In getNextMessage: ERROR: Invalid arguments");
	    }
	    else{
		_print(DBUG,"In getNextMessage: UNKNOWN ERROR! errno:"<<erno);
	    }
	}
	_print(DBUG,"In getNextMessage: Returned from fmq_recv_cond");
      /* ] */
    }

    try{
	_print(DBUG,"In getNextMessage: Trying to get the message from the receive queue");
	msg = fmq_recv->get();
	_print(DBUG,"In getNextMessage: Got the message from the receive queue");
	fmq_recv->remove();
	_print(DBUG,"In getNextMessage: Message removed from the receive queue");
	
    }
    catch(RTIOException e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: I/O Error occured while getting or removing the message from the queue");
	fmq_recv->reset();
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: Removed queue, All pending messages are lost");
	fail = true;
    }
    catch(RTOutOfMemoryException e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: System is out of Memory!");
	fail = true;
    }
    catch(RTFileCorruptedError e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: Message queue file is corrupted! Can't send message");
	fmq->reset();
	fail = true;
    }
    catch(RTException e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: RTException is raised, Can't send message");
	fail = true;
    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: CORBA exception occured, Can't send message");
	fail = true;
    }
    catch(...){
	_print(ERROR,"Error in ConnectionHandler::getNextMessage: Unknown exception. Can't send message");
	fail = true;
    }    
    pthread_mutex_unlock(&fmq_recv_lock);
    _print(DBUG,"In getNextMessage: Unlocked fmq_recv_lock");
    
    if(fail==true && msg!=NULL){
	_print(DBUG,"In getNextMessage: Failed: deleting message and returning null");
	delete msg;
	sleep(2);
	return NULL;
    }
    return msg;
}

int ConnectionHandler::listen(double timeout){
    char* msgstr = NULL;
    Message *msg = NULL;
    ContentTable *ctable = NULL;
    bool isin = false;
    const int period = 5;//seconds

    // DEBUG
    TimeStamp enter = TimeStamp::current_time();
    _print(DBUG, "In listen: Entering");

    if(!valid){
      _print(DBUG,"<< Returning from listen: ConnectionHandler not valid");
	return (TN_FAILURE);
    }

    TimeStamp starttime = TimeStamp::current_time();
    TimeStamp before = TimeStamp::current_time();
    TimeStamp after = TimeStamp::current_time(); 
    
    double timeleft = timeout;
    
    while(1){
	int sleep_dur = 0;
	TimeStamp now = TimeStamp::current_time();
	if(((double)(now - starttime)) >= timeout){
	    _print(DBUG,"<< Returning from listen: took too much time to start the loop:"<<(double)(now - starttime));
	    return (TN_SUCCESS);
	}
	
	try{
	    timeleft = timeleft - (double)(after - before);
	    
	    before = TimeStamp::current_time();	    
	    Message *msg = getNextMessage(timeleft);
	    after = TimeStamp::current_time();
	    
	    if(!msg){
		_print(DBUG,"In listen(): Message pointer is NULL. Keep polling..");

		if(period < (double)(TimeStamp::current_time() - pingtimer)){
		    _print(DBUG,"Not heared from QWCC for long time. Checking ConnectionHandler...");
		    pingtimer = TimeStamp::current_time();
		    //Check connection//
		    if(isAlive()==TN_FALSE){
			//Attemp to reconnect//
			_print(INFO,"ConnectionHandler LOST with QWCC. Attempting to reconnect...");
			if(reconnect()==TN_FAILURE){
			    _print(INFO,"All reconnection attempts failed. Returning from listen().");
			    return (TN_FAILURE);
			}
			else{
			    _print(INFO,"Reconnected with QWCC");
			}
		    }
		    else{
			_print(DBUG,"ConnectionHandler to QWCC is OK");
		    }
		}
		continue;
	    }
	    
	    pingtimer = TimeStamp::current_time();
	    //Check here if we have subscribed for the channel from which we have received this message
	    char* chan  = (char*)msg->getDest();
	    isin = false;
	    for(SubjectList::iterator it = subjectlist.begin();it!=subjectlist.end();it++){
	      if(strcmp(chan,(*it).c_str())==0){
		isin = true;
		break;
	      }
	    }	    
	    if(isin==false){
	      unsubscribe(chan);
	      _print(DBUG,"Unsubscribed: "<<chan);
	      return(TN_FAILURE);
	    }

	    int type = 0;
	    msg->getType(type);

	    if(cbm[type]!=NULL){
	      (*cbm[type])(*msg,carg[type]); //Calling call back function//
	    }
	    else{
	      _print(ERROR,"Error in ConnectionHandler::listen(): Received wrong message, This program is not subscribed to this message type");
	    }
	}
	catch(const CORBA::SystemException& e){
	    _print(ERROR,"Error in ConnectionHandler::listen(): CORBA exception occured");
	    //Print message here//
	    if(reconnect()==TN_FAILURE)
		return (TN_FAILURE);
	}
	catch(const RTException e){
	    _print(ERROR,e.msg);
	    _print(ERROR,"Error in ConnectionHandler::listen(): Received exception while dispatching message");
	    return (TN_FAILURE);
	}

	if(msg)
	    delete msg;

	msg = NULL;
    }
}


int ConnectionHandler::listenNext(double timeout){
    char* msgstr = NULL;
    Message *msg = NULL;
    ContentTable *ctable = NULL;
    bool isin = false;
    const int period = 5;//seconds

    // DEBUG
    TimeStamp enter = TimeStamp::current_time();
    _print(DBUG, "In listenNext: Entering");

    if(!valid){
      _print(DBUG,"<< Returning from listenNext: ConnectionHandler not valid");
	return (TN_FAILURE);
    }

    TimeStamp starttime = TimeStamp::current_time();
    TimeStamp before = TimeStamp::current_time();
    TimeStamp after = TimeStamp::current_time(); 
    
    double timeleft = timeout;
    
    while(1){
	int sleep_dur = 0;

	try{
	    timeleft = timeleft - (double)(after - before);
	    
	    before = TimeStamp::current_time();	    
	    Message *msg = getNextMessage(timeleft);
	    after = TimeStamp::current_time();
	    
	    if(!msg){
		_print(DBUG,"In listenNext(): Message pointer is NULL. Keep polling..");

		if(period < (double)(TimeStamp::current_time() - pingtimer)){
		    _print(DBUG,"Not heared from QWCC for long time. Checking ConnectionHandler...");
		    pingtimer = TimeStamp::current_time();
		    //Check connection//
		    if(isAlive()==TN_FALSE){
			//Attemp to reconnect//
			_print(INFO,"ConnectionHandler LOST with QWCC. Attempting to reconnect...");
			if(reconnect()==TN_FAILURE){
			    _print(INFO,"All reconnection attempts failed. Returning from listenNext().");
			    return (TN_FAILURE);
			}
			else{
			    _print(INFO,"Reconnected with QWCC");
			}
		    }
		    else{
			_print(DBUG,"ConnectionHandler to QWCC is OK");
		    }
		}
		TimeStamp now = TimeStamp::current_time();
		if(((double)(now - starttime)) >= timeout){
		    _print(DBUG,"<< Returning from listenNext: timed out");
		    return (TN_SUCCESS);
		}
		continue;
	    }
	    
	    pingtimer = TimeStamp::current_time();
	    //Check here if we have subscribed for the channel from which we have received this message
	    char* chan  = (char*)msg->getDest();
	    isin = false;
	    for(SubjectList::iterator it = subjectlist.begin();it!=subjectlist.end();it++){
	      if(strcmp(chan,(*it).c_str())==0){
		isin = true;
		break;
	      }
	    }	    
	    if(isin==false){
	      unsubscribe(chan);
	      _print(DBUG,"Unsubscribed: "<<chan);
	      return(TN_FAILURE);
	    }

	    int type = 0;
	    msg->getType(type);

	    if(cbm[type]!=NULL){
	      (*cbm[type])(*msg,carg[type]); //Calling call back function//
	      delete msg;
	      _print(DBUG, "<< listenNext handled one message");
	      return(TN_SUCCESS);
	    }
	    else{
	      _print(ERROR,"Error in ConnectionHandler::listenNext(): Recevied wrong message, This program is not subscribed to this message type");
	    }
	}
	catch(const CORBA::SystemException& e){
	    _print(ERROR,"Error in ConnectionHandler::listenNext(): CORBA exception occured");
	    //Print message here//
	    if(reconnect()==TN_FAILURE)
		return (TN_FAILURE);
	}
	catch(const RTException e){
	    _print(ERROR,e.msg);
	    _print(ERROR,"Error in ConnectionHandler::listenNext(): Received exception while dispatching message");
	    return (TN_FAILURE);
	}

	if(msg)
	    delete msg;

	msg = NULL;

	TimeStamp now = TimeStamp::current_time();
	if(((double)(now - starttime)) >= timeout){
	    _print(DBUG,"<< Returning from listenNext: timed out");
	    return (TN_SUCCESS);
	}
	
    }
}


int ConnectionHandler::registerHandler(int mtype, 
				void (*func)(Message&, void *), void *arg){

    if(!valid)
	return (TN_FAILURE);

    if(!func)
	return (TN_FAILURE);

    cbm[mtype] = func;
    carg[mtype]= arg;
    return TN_SUCCESS;
}

int ConnectionHandler::sendOldMessages(){
  Message *msg = NULL;
  try{
    msg = fmq->get();
  }
  catch(...){
    _print(ERROR,"Error in ConnectionHandler::sendOldMessage(): Unable to get old message from the queue file");
    return TN_FAILURE;
  }
  
  if(msg==NULL)
    return TN_SUCCESS;

  try{
    fmq->remove();
  }
  catch(...){
    _print(ERROR,"Error in ConnectionHandler::sendOldMessage(): Unable to remove old message from the queue file");
    return TN_FAILURE;    
  }

  if(send(*msg)==TN_FAILURE){
    _print(ERROR,"Error in ConnectionHandler::sendOldMessage(): Unable to send old message from the queue file");      
  }

  return TN_SUCCESS;

}

int ConnectionHandler::send(Message& m){
    QWMessageSerializer s; 
    Message *msg = NULL;
    bool msgsafe = false;
    const char* buf = NULL;

    if(!valid)
	return (TN_FAILURE);

    if(fmq==NULL){
	fmq = createFileMessageQueue(qfilename_send);
	if(fmq==NULL)
	    return (TN_FAILURE);
    }

    m.setDomainName(project.c_str());
    m.setSourceName(module.c_str());

    bool fail = false;
    try{
	//Put Message in the queue//
	fmq->put(m);
	msgsafe = true;
	//Get Messages from the queue//
	while((msg = fmq->get())!=NULL){
	    buf = s.serialize(*msg);
	    if(!buf){
		//Log message//
		_print(ERROR,"Error in  ConnectionHandler::send: Unable to serialize the message");
		//Return error//
		delete msg->getContentTable();
		delete msg;
		return (TN_FAILURE);
	    }

	    int skip = strstr(buf,"?>") - buf;
	    if(skip<=0){
	      //error
	    }

	    char* buf_ptr = (char*)buf;
	    skip += 2;
	    buf_ptr += skip;
	    
	    qwfeeder->sendDomainTypeMessage(project.c_str(),msg->getDest(),buf_ptr);
	    fmq->remove();
	    delete msg->getContentTable();
	    delete msg;
	    msg = NULL;

	    free((void*)buf);
	    buf = NULL;
	}
    }
    catch(RTIOException e){
	_print(ERROR,"Error in ConnectionHandler::send: I/O Error occured while sending message");
	fmq->reset();
	_print(ERROR,"Removed queue: All pending messages are lost");
	fail = true;
	msgsafe = false;
    }
    catch(RTOutOfMemoryException e){
	_print(ERROR,"Error in ConnectionHandler::send: System is out of Memory!");
	fail = true;
    }
    catch(RTFileCorruptedError e){
	_print(ERROR,"Error in ConnectionHandler::send: Message queue file is corrupted! Can't send message");
	fmq->reset();
	fail = true;
    }
    catch(RTException e){
	_print(ERROR,"Error in ConnectionHandler::send: RTException is raised, Can't send message");
	fail = true;
    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,"Error in ConnectionHandler::send: CORBA exception occured, Can't send message");
	fail = true;
       	quick_reconnect();
    }
    catch(...){
	_print(ERROR,"Error in ConnectionHandler::send: Unknown exception. Can't send message");
	fail = true;
    }
    if(msg){
	delete msg->getContentTable();
	delete msg;
	if(buf){
	    free((void*)buf);
	}
    }
    if(msgsafe == false)
	return TN_FAILURE;
    return TN_SUCCESS;
}


int ConnectionHandler::flush(){
    if(!valid)
	return (TN_FAILURE);

    return(TN_SUCCESS);
}

int ConnectionHandler::subscribe(char *subject){
    if(!valid)
	return (TN_FAILURE);

    if(!subject)
	return TN_FAILURE;
    try{
	qwpushsub->subscribe(project.c_str(),subject);
	_print(DBUG,"Subscribed to Domain:"<<project.c_str()<<" Channel:"<<subject);
    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,"Error in ConnectionHandler::subscribe: CORBA exception occured, Can't subscribe to the subject "<<subject);
	return (TN_FAILURE);
    }

    string str = subject;
    subjectlist.push_back(str);
   
    return (TN_SUCCESS);
}

int ConnectionHandler::unsubscribe(char *subject){
    if(!valid)
	return (TN_FAILURE);

    if(!subject)
	return TN_FAILURE;

    try{
	qwpushsub->unsubscribe(project.c_str(),subject);
	_print(DBUG,"Unsubscribed "<<project.c_str()<<":"<<subject);
    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,"Error in ConnectionHandler::unsubscribe: CORBA exception occured, Can't unsubscribe the subject"<<subject);
	return (TN_FAILURE);
    }

    string str = subject;
    //Remove subject from the subject list//
    for(SubjectList::iterator it = subjectlist.begin();it!=subjectlist.end();){
	if(str.compare(*it)==0){
	    it = subjectlist.erase(it);
	    continue;
	}
	it++;
    }

    return (TN_SUCCESS);
}

int ConnectionHandler::isAlive(){
    if(!valid)
	return (TN_FALSE);
    try{
	qwpushsub->checkConnection();
	return (TN_TRUE);
    }
    catch(...){
	return (TN_FALSE);
    }
}

int ConnectionHandler::getNumPending(int &num){
    if(!valid)
	return (TN_FAILURE);

    if(!fmq){
	num = 0;
	return (TN_SUCCESS);
    }
    
    try{
	num = fmq->pending();
    }
    catch(...){
	num = 0;
	return (TN_FAILURE);
    }
    return (TN_SUCCESS);
}

int operator!(const ConnectionHandler& conn){
    return !conn.valid;
}


//Private//

int ConnectionHandler::setQWFeeder(){
    int flag = 0;

    for(serverlist::iterator sit = servers.begin();sit!=servers.end();sit++){
	server* s  = (*sit);
	try{
	    //printf("char* for qwfeeder location is %s\n", qwfeeder_corbaloc.c_str());
	    string corbaloc = "corbaloc:iiop:"+corbaversion+"@"+s->hostname+":"+s->port+"/";
	    qwfeeder_corbaloc = corbaloc+"QWFeeder";
	    CORBA::Object_var qwfeeder_obj = orb->string_to_object(qwfeeder_corbaloc.c_str());
	    qwfeeder = QWFeeder::_narrow(qwfeeder_obj.in());	
	}
	catch(const CORBA::SystemException& e){
	    //_print("");
	    continue;
	}
	catch(...){
	    //_print( "");
	    continue;
	}
	_print(INFO,"Connected to QWFeeder at "<<s->hostname<<":"<<s->port);
	flag = 1;
	break;
    }
    if(flag){
	return (TN_SUCCESS);
    }

    return (TN_FAILURE);
}

int ConnectionHandler::setQWSubscription(){
    try{
	CORBA::Object_var qwsub_obj = orb->string_to_object(qwsub_corbaloc.c_str());
       	qwsub = QWSubscription::_narrow(qwsub_obj.in());	
    }
    catch(const CORBA::SystemException& e){
	//_print("");
	return (TN_FAILURE);
    }
    catch(...){
	//_print( "");
	return (TN_FAILURE);
    }

    return TN_SUCCESS;
}

int ConnectionHandler::setQWPullSubServices(){
    return 0;
}

int ConnectionHandler::setQWPushSubServices(){
    try{
	if(setQWSubscription()==TN_FAILURE){
	    //Log Message//
	    return(TN_FAILURE);
	}

	_print(INFO,"Getting pushsubscriber for "<<module.c_str());
	char* res = qwsub->connectPushSubscriber(module.c_str(),qwpushsub);
	if(!qwpushsub){
	  _print(ERROR,"Error: "<<res);
	  return (TN_FAILURE);
	}

	_print(DBUG,"Got pushsubscriber");
	//Subscribe to all subjects again//
	for(SubjectList::iterator it = subjectlist.begin();it!=subjectlist.end();it++){
	    qwpushsub->subscribe(project.c_str(),(*it).c_str());
	}

	_print(INFO,"Trying connectRecMsgCallBack");	
	qwpushsub->connectRecMsgCallBack(_this(),true);

    }
    catch(const CORBA::SystemException& e){
	_print(ERROR,e._name());
	return (TN_FAILURE);
    }
    catch(...){
	//_print( "");
	return (TN_FAILURE);
    }

    return TN_SUCCESS;
}

int ConnectionHandler::reconnect(){
    int retrycount = 0;

    while(retrycount <= maxconnretry){
	_print(INFO,"Trying to connect...");
	if(setQWFeeder()==TN_SUCCESS && setQWPushSubServices()==TN_SUCCESS){
	  _print(INFO,"Connected!");
	  //sendOldMessages();
	  pingtimer = TimeStamp::current_time();	
	  return TN_SUCCESS;
	}
	_print(INFO,"Failed.");
	_print(INFO,"Sleeping for "<<sleepduration<<" seconds");
	sleep(sleepduration);
	retrycount++;
    }
    return (TN_FAILURE);
}

int ConnectionHandler::quick_reconnect(){
    if(setQWFeeder()==TN_SUCCESS && setQWPushSubServices()==TN_SUCCESS){
	return TN_SUCCESS;
    }    
    return (TN_FAILURE);
}

FileMessageQueue* ConnectionHandler::createFileMessageQueue(string filename){
    FileMessageQueue* qptr = NULL;
    try{
	qptr = new FileMessageQueue(filename,maxqfilesize);//TODO: Create file at proper location//
	if(qptr==NULL)
	    return NULL;
	if(!*qptr){
	    delete qptr;
	    return NULL;
	}
    }
    catch(RTIOException e){
	//Print Message//
	_print(ERROR, e.msg );
	//And invalidate the object//
	return NULL;
    }
    catch(RTFileCorruptedError e){
	//TODO: Move corrupted file to other folder and create new file//
	_print(ERROR, e.msg );
	return NULL;
    }
    catch(...){
	_print(ERROR,"Unknown exception raised while creating FileMessageQueue object");
	return NULL;
    }
    _print(DBUG,"Returning File pointer");
    return qptr;
}
