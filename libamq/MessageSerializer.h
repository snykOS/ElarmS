#ifndef __messageserializer_h
#define __messageserializer_h

#include "ContentTable.h"
#include "RTException.h"
#include "Message.h"

class MessageSerializer{

 public:
    virtual const char* serialize(const Message&) throw(RTException) = 0;

};


#endif//__messageserializer_h
