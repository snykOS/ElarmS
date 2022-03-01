#ifndef __qwmessageserializer_h
#define __qwmessageserializer_h
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include "MessageParser.h"
#include "MessageSerializer.h"
#include "ContentTable.h"
#include "RTException.h"
#include "Message.h"

using namespace std;

extern const char* type_name_map[];

XERCES_CPP_NAMESPACE_USE

class QWMessageSerializer : public MessageSerializer{
    
 private:

    DOMElement* createElement(DOMDocument* doc,const char* name,const char* attr,const char* value) throw(RTException);
    void insertArrayElements(DOMDocument* doc,Row* f,DOMElement* parent) throw(RTException);
    void getValueString(Row* f,char* v);

 public:
    QWMessageSerializer();
    ~QWMessageSerializer();

    const char* serialize(const Message&) throw(RTException);    
};


#endif//__qwmessageserializer_h
