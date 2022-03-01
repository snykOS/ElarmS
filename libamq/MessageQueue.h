#ifndef ___messagequeue_h
#define ___messagequeue_h

class MessageQueue{

    void put(const Message&) throw(RTException) = 0;
    Message* get() throw(RTException) = 0;

};

#endif//___messagequeue_h
