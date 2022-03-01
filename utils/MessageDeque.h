#ifndef __messagedeque_h
#define __messagedeque_h

#include <deque>                        // STL deque template
#include "plog/Log.h"                   // plog logging library
#include "TimeStamp.h"                  // AQMS timestamp class
#include "Duration.h"                   // AQMS duration class

// rcsid version strings
#define RCSID_MessageDeque_h "$Id: MessageDeque.h $"
extern const std::string RCSID_MessageDeque_cc;
 

template <class MESSAGE_TYPE> class MessageDeque {

    private:

        template <class T> class Entry {
            public:
                T value;
                TimeStamp timestamp;
                Entry(T val, const TimeStamp time = TimeStamp::current_time() ) :
                    value(val), timestamp(time) {}
        };  // class Entry
        typedef Entry<MESSAGE_TYPE> EntryType;

        std::deque<EntryType> queue;
        Duration max_queue_time;
        unsigned int max_queue_size;

    public:
        const static int DEFAULT_MAX_TIME = 180;    // seconds
        const static int DEFAULT_MAX_SIZE = 100;

        // constructor with optional overrides of default size and age
        MessageDeque(const double max_time = DEFAULT_MAX_TIME, const int max_size = DEFAULT_MAX_SIZE) :
            max_queue_time(max_time), max_queue_size(max_size) {
#ifdef DEBUG_MESSAGEDEQUE
LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":MessageDeque.ctor() max_queue_time=" << max_queue_time << ", max_queue_size=" << max_queue_size;
#endif
            }

        // change max age
        void setMaxQueueTime(const double max_time) {
            this->max_queue_time = Duration(max_time);
#ifdef DEBUG_MESSAGEDEQUE
LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":setMaxQueueTime() queue.max_queue_time=" << this->max_queue_time;
#endif
        }

        // change max number of messages
        void setMaxQueueSize(const int max_size) {
            this->max_queue_size = max_size;
#ifdef DEBUG_MESSAGEDEQUE
LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":setMaxQueueSize() queue.max_queue_size=" << this->max_queue_size;
#endif
        }

        // check if queue is empty
        bool empty() const {
            return this->queue.empty();
        }

        // push message onto queue, return 0 if okay, count of old messages discarded else -1 if queue is full
        int push(const MESSAGE_TYPE message) {
            TimeStamp now = TimeStamp::current_time();

            int status = 0;

            // if timeout is non zero, discard any messages that are too old, assumes ordered
            while (!this->queue.empty() && (double)this->max_queue_time > 0.0 &&
                    now - this->queue.front().timestamp > this->max_queue_time) {
// LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":push(): popping old message with timestamp=" << this->queue.front().timestamp;
                this->queue.pop_front();
                status++;
            }

            // discard new entries if queue is full
            if (this->max_queue_size > 0 && this->queue.size() > this->max_queue_size) {
// LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":push(): queue is full! size=" << this->queue.size() << ", discarding entry";
                return -1;
            }
            this->queue.push_back(EntryType(message, now));
// LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":push(): inserted, size=" << this->queue.size();

            return status;
        } // push

        MESSAGE_TYPE pop() {
// LOGD << "%%%%%%" << __FILE__ << ":" << __LINE__ << ":pop(): queue.size was " << this->queue.size();
            if (this->queue.empty()) {
                throw "Pop when queue empty";
            } 

            MESSAGE_TYPE message = this->queue.front().value;
            this->queue.pop_front();

            return message;
        } // pop

}; // class MessageDeque

#endif // __messagedeque_h
