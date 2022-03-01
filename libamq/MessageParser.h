#ifndef __messageparser_h
#define __messageparser_h

#include "ContentTable.h"
#include "RTException.h"

class MessageParser{
    
 public:
    virtual ContentTable* parse(const char*) throw(RTException) = 0;
};


#endif//__messageparser_h
