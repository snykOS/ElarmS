#ifndef __qwmessageparser_h
#define __qwmessageparser_h

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include "MessageParser.h"
#include "ContentTable.h"


XERCES_CPP_NAMESPACE_USE

class QWMessageParser : public MessageParser{
 private:
    char* domain;
    char* channelname;
    char* msgtype;
    char* source;

    ContentTable* retrieveContent(DOMDocument* doc) throw(RTException);
    ContentTable* retrieveContent2(DOMDocument* doc) throw(RTException);
    Row* parseElement(DOMNode* node) throw(RTException);

    void getNodeInfo(DOMNode* node,char** name,char** value,DATATYPE& type) throw(RTException);
    void fillField(Row*,char*) throw(RTException);
    void fillField2(Row*,char*) throw(RTException);

    DATATYPE getDataType(char* name);
    char getByte(char* str) throw(RTIllegalArgumentException);
    short getShort(char* str) throw(RTIllegalArgumentException);
    int getInt(char* str) throw(RTIllegalArgumentException);
    long long getLongLong(char* str) throw(RTIllegalArgumentException);
    float getFloat(char* str) throw(RTIllegalArgumentException);
    double getDouble(char* str) throw(RTIllegalArgumentException);
    bool getBoolean(char* str) throw(RTIllegalArgumentException);
    
 public:
    QWMessageParser();
    ~QWMessageParser();

    ContentTable* parse(const char*) throw(RTException);
    char* getDomainName();
    char* getType();
    char* getChannelName();
    char* getSourceName();
};

#endif//__qwmessageparser_h
