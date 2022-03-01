#include <errno.h>              // errno
#include <unistd.h>             // usleep
//#include "wp.h"                 // print lock/unlock
#include "plog/Log.h"           // plog logging library
#include "globals.h"
#include "MessageSender.h"

const string RCSID_MessageSender_cc = "$Id: MessageSender.cc $";

#include "MessageDeque.h"       // Timed Message Queue

// THIS DEFINITION FOR LOGGING IS TEMPORARILY HERE AS IT HAS NOWHERE ELSE TO GO
// AND WILL BE REMOVED SOON..... HOPEFULLY.
pthread_mutex_t* logging::print_lock = NULL;

bool MessageSender::isInit = false;
cms::Connection* MessageSender::conn = NULL;
Session* MessageSender::session = NULL;
Destination* MessageSender::destination = NULL;
MessageProducer* MessageSender::producer = NULL;
pthread_mutex_t MessageSender::lock = PTHREAD_MUTEX_INITIALIZER;
std::string MessageSender::topic_name = "undefined_topic_name";

bool keep_running;
MessageDeque<std::string> queue;

// worker thread

extern "C" void* dequeue_worker_thread(void* arg __attribute__((unused)) ) {

    while (keep_running) {

        while (!queue.empty()) {
            std::string msg;
            pthread_mutex_lock(&MessageSender::lock);
            msg = queue.pop();
            pthread_mutex_unlock(&MessageSender::lock);
            MessageSender::send(msg);
        }
        usleep(10000);  // 0.01 second
    }
    return NULL;
} // dequeue_worker_thread


// public
void MessageSender::init(cms::Connection* connection, string topic){
    if(isInit)
        return;

    topic_name = topic;
    conn = connection;
    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
    destination = session->createTopic(topic);
    producer = session->createProducer( destination );
    producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );  
    isInit = true;

//    queue.setMaxQueueSize(20);//50);
//    queue.setMaxQueueTime(30);//10);

    run(); // start the thread

} // MessageSender.init

void MessageSender::run() {
    // Start sender thread
    keep_running = true;
    pthread_t tid = 0;
    int status = pthread_create(&tid, NULL, dequeue_worker_thread, NULL);
    if (status != 0) {
        LOGE << "MessageSender thread for topic " << topic_name << " could not be started, pthread_create returned status=" << status << ", errno=" << errno;
    }
} // MessageSender.run

void MessageSender::sendMessage(const string msgstr){
    pthread_mutex_lock(&lock);
    int status = queue.push(msgstr);
    pthread_mutex_unlock(&lock);
    if (status > 0) {
        LOGW << "MessageSender(): discarded " << status << " old message(s) for topic " << topic_name;
    } else if (status < 0) {
        LOGW << "MessageSender(): queue full, message lost for topic " << topic_name;
    }

} // MessageSender.sendMessage

void MessageSender::send(const string msgstr){
    TextMessage* message = NULL;

    message = session->createTextMessage(msgstr.c_str()); 
    if(message) {
	    producer->send(message);
        delete message;
    }	
} // MessageSender.send

// end of file MessageSender.cc
