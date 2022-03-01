#include "QWMessageSerializer.h"
#include "MessageTypes.h"
#include <stdlib.h>

//This class is taken from Xerces Samples code. file: CreateDOMDocument.cpp//
class XStr
{
public :
    XStr(const char* const toTranscode)
    {
        // Call the private transcoding method
        fUnicodeForm = XMLString::transcode(toTranscode);
    }

    ~XStr()
    {
        XMLString::release(&fUnicodeForm);
    }
    const XMLCh* unicodeForm() const
    {
        return fUnicodeForm;
    }

private :
    XMLCh*   fUnicodeForm;
};

#define X(str) XStr(str).unicodeForm()

QWMessageSerializer::QWMessageSerializer(){
}
QWMessageSerializer::~QWMessageSerializer(){
}

const char* QWMessageSerializer::serialize(const Message& msg) throw(RTException){

#if XERCES_VERSION_MAJOR > 2
    DOMLSSerializer *serializer;
    DOMLSOutput* theOutput;
#else
    DOMWriter *serializer;
#endif
    XMLFormatTarget  *target;
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(X("LS"));  
    DOMDocument* doc =0;
    const char* xmlbuf=0;
    ContentTable* ctable = msg.getContentTable();
    if(!ctable){
	throw RTException("Error in QWMessageSerializer::serialize : Message::getContentTable() returned NULL ContentTable.");	
    }

    const char* domain = msg.getDomainName();
    char msgtype[128];
    msg.getTypeString(msgtype);
    const char* channelname = msg.getDest();
    const char* source = msg.getSourceName();
    
    try{
      doc = impl->createDocument(0,X("ValuesMessage"),0);
      DOMElement* parent  = doc->getDocumentElement();
      
      if(domain)
	parent->setAttribute(X("Domain"),X(domain));
      if(msgtype)
	parent->setAttribute(X("Name"),X(msgtype));
      if(channelname)
	parent->setAttribute(X("Chan"),X(channelname));
      if(source)
	parent->setAttribute(X("Source"),X(source));
      
      for(ContentTable::iterator cit=ctable->begin();cit!=ctable->end();cit++){
	Row *f = *cit;
	char* value=0;
	char numstr[32];
	if(!ISARRAY(f->type)){
	  if(f->type==STRING)
	    value = f->val._string;
	  else{
	    getValueString(f,numstr);
	    value = numstr;
	  }
	}
	DOMElement* child = createElement(doc,type_name_map[f->type],f->name,value);
	if(ISARRAY(f->type)){
	  insertArrayElements(doc,f,child);
	}
	parent->appendChild(child);
      }
    }
    catch(const DOMException& e){
      doc->release();
      throw RTException("An error occurred during DOM construction");
    }
    catch(const RTException& e){
      doc->release();
      throw RTException("An error occurred during DOM construction");
    }

    //At this point DOM has been created. Next step: write it down in a string buffer*/
    try{
	XMLCh*  gOutputEncoding = 0;   
	target = new MemBufFormatTarget();		
#if XERCES_VERSION_MAJOR > 2
	serializer = impl->createLSSerializer();
    theOutput = impl->createLSOutput();
    theOutput->setByteStream(target);

	serializer->write(doc,theOutput);
#else
	serializer = impl->createDOMWriter();
	
	serializer->setEncoding(gOutputEncoding);
	if (serializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
            serializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
	}
	if (serializer->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, true)) {
            serializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, true);
	}
	serializer->writeNode(target,*doc);
#endif
	xmlbuf = strdup((const char*)((MemBufFormatTarget*)target)->getRawBuffer());
    }
    catch (XMLException& e){
      doc->release();
      delete serializer;
      delete target;
      free((char*)xmlbuf);
      throw RTException("An error occurred during serialization");
    }
    doc->release();
    delete serializer;
    delete target;
    return xmlbuf;
}

DOMElement* QWMessageSerializer::createElement(DOMDocument* doc,const char* name,const char* attr,const char* value)
                                                         throw(RTException){
  DOMElement* node =0;
  DOMText* textnode =0;
  try{

    node = doc->createElement(X(name));
    
    if(attr){
	node->setAttribute(X("Name"),X(attr));
    }
    if(value){
	textnode = doc->createTextNode(X(value));
	node->appendChild(textnode);
    }
  }
  catch(const DOMException& e){
    cout << "DOMException code is:  " << e.code << endl;
    throw RTException("Error creating DOM element");
  }
    return node;
}

void QWMessageSerializer::insertArrayElements(DOMDocument* doc,Row* f,DOMElement* parent) throw(RTException){
    ByteList *blist;
    ShortList *slist;
    IntList *ilist;
    LongLongList *llist;
    FloatList *flist;
    DoubleList *dlist;
    BoolList *boollist;
    StrList *strlist;

    char* v = 0;
    char numstr[32];
    DOMElement* child = 0;
    
    try{
	ContentTable* ct = (ContentTable*)f->val._ptr;
	for(ContentTable::iterator it = ct->begin();it!=ct->end();it++){
	    Row* df = *it;
	    switch(df->type){
	    case BYTE:
	    case CHAR:
		sprintf(numstr,"%c",df->val._byte);
		break;
	    case SHORT: 
		sprintf(numstr,"%d",df->val._short);
		break;
	    case INT: 
		sprintf(numstr,"%d",df->val._int);
		break;
	    case LONGLONG:
		sprintf(numstr,"%lld",df->val._longlong);
		break;
	    case FLOAT: 
		sprintf(numstr,"%f",df->val._float);
		break;
	    case DOUBLE: 
		sprintf(numstr,"%f",df->val._double);
		break;
	    case BOOLEAN: 
		(true==df->val._bool)?strcpy(numstr,"true"):strcpy(numstr,"false");
		break;
	    case STRING:
		if(df->val._string == NULL){
		    sprintf(numstr," ");		    
		}
		else{
		    sprintf(numstr,"%s",df->val._string);
		}
		break;
	    }
	    child = createElement(doc,type_name_map[df->type],NULL,numstr);
	    parent->appendChild(child);	
	}
    }
    catch(const DOMException& e){
	cout << "DOMException code is: " << e.code << endl;
	throw RTException("Error in DOM while inserting array elements");
    }
}

void QWMessageSerializer::getValueString(Row* f,char* v){

    switch(f->type){
    case BYTE:
    case CHAR: 
	sprintf(v,"%c", f->val._byte);
	break;
    case SHORT: 
	sprintf(v,"%d",f->val._short);
	break;
    case INT: 
	sprintf(v,"%d",f->val._int);
	break;
    case LONGLONG: 
	sprintf(v,"%lld",f->val._longlong);
	break;
    case FLOAT:
	sprintf(v,"%f",f->val._float);
	break;
    case DOUBLE: 
	sprintf(v,"%f",f->val._double);
	break;
    case BOOLEAN: 
	(true==f->val._bool)?strcpy(v,"true"):strcpy(v,"false");
	break;
    }
}

