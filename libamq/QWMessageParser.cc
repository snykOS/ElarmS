#include <iostream>
#include <errno.h>
#include "QWMessageParser.h"
#include <xercesc/sax/InputSource.hpp>

const string RCSID_QWMessageParser_cc = "$Id: QWMessageParser.cc 0000 2019-05-10 17:01:25Z claude $";

using namespace std;
//extern class ContentTable;

/*WARNING: This map is implicitly mapped with the DATATYPE enumeration, if you change the sequence in 
one structure you must change the other one's*/
const char* type_name_map[] = {"Unknown","Byte","Char","Int2","Int4","Int8","Real4","Real8","Boolean","String",
                          "ByteArray","CharArray","Int2Array","Int4Array","Int8Array","Real4Array","Real8Array",
                          "BooleanArray","StringArray"};

QWMessageParser::QWMessageParser(){
    domain = NULL;
    msgtype = NULL;
    channelname = NULL;
    source = NULL;
}

QWMessageParser::~QWMessageParser(){
    // caf 2019-05-10 -- only free if allocated
    if (domain != NULL)      free(domain);      domain = NULL;
    if (msgtype != NULL)     free(msgtype);     msgtype = NULL;
    if (channelname != NULL) free(channelname); channelname = NULL;
    if (source != NULL)      free(source);      source = NULL;
}

ContentTable* QWMessageParser::parse(const char* msgstr) throw(RTException){
    if(!msgstr){
	throw RTException("Illegal argument: xml buffer is null");
    }

    XercesDOMParser* parser = new XercesDOMParser();
    parser->setValidationScheme(XercesDOMParser::Val_Never);
    parser->setDoNamespaces(false);
    
    ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
    parser->setErrorHandler(errHandler);

    XMLCh* sysid = XMLString::transcode("QWMessage");
    MemBufInputSource* xmlbuf = new MemBufInputSource((const   XMLByte* const)msgstr
						      ,(const unsigned int)strlen(msgstr),(const XMLCh* const)sysid);
    XMLString::release(&sysid);
    
    ContentTable *ctable = NULL;    
    try{
	parser->parse(*xmlbuf);
	ctable = retrieveContent(parser->getDocument());
    }
    catch(const XMLException& e){
	char* msg = XMLString::transcode(e.getMessage());
	
	RTException rte;
	rte.msg = string("Error during parsing : ") + string(msg);
	XMLString::release(&msg);

	delete xmlbuf;
	delete parser;
	delete errHandler;
	throw rte;
    }
    catch(const DOMException& dome){
	char* msg = XMLString::transcode(dome.msg);

	RTException e;
	e.msg = string("Error during parsing : ") + string(msg);
	XMLString::release(&msg);

	delete xmlbuf;
	delete parser;
	delete errHandler;
	throw e;
    }
    catch(...){
	delete xmlbuf;
	delete parser;
	delete errHandler;
	RTException e;
	e.msg = "Unexpected exception";
	throw e;
    }
    
    delete xmlbuf;
    delete parser;
    delete errHandler;
    return ctable;
}

char* QWMessageParser::getDomainName(){
    return domain;
}

char* QWMessageParser::getType(){
    return msgtype;
}

char* QWMessageParser::getChannelName(){
    return channelname;
}

char* QWMessageParser::getSourceName(){
    return source;
}

ContentTable* QWMessageParser::retrieveContent(DOMDocument* doc) throw(RTException){
    if(!doc){
	throw RTException("DOMDocument is null");
    }

    DOMNode *n = (DOMNode*)doc->getDocumentElement();
    if(!n)
	throw RTException("DOMDocument returned null node");

    char *_name = NULL;
    char *_value = NULL;
    try{
	if (n->getNodeType() == DOMNode::ELEMENT_NODE){			
	    if(n->hasAttributes()) {
		// get all the attributes of the node
		DOMNamedNodeMap *pAttributes = n->getAttributes();
		int nSize = pAttributes->getLength();
		for(int i=0;i<nSize;++i) {
		    DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
		    // get attribute name
		    _name = XMLString::transcode(pAttributeNode->getName());
		    _value = XMLString::transcode(pAttributeNode->getValue());
		    
		    if(strcmp(_name,"Domain")==0)
			domain = strdup(_value);
		    else if (strcmp(_name,"Name")==0)
			msgtype = strdup(_value);
		    else if (strcmp(_name,"Chan")==0)
			channelname = strdup(_value);
		    else if (strcmp(_name,"Source")==0)
			source = strdup(_value);
		    
		    XMLString::release(&_name);
		    XMLString::release(&_value);
		}
	    }
	}
    }
    catch(...){
	if(_name)
	    XMLString::release(&_name);
	if(_value)
	    XMLString::release(&_value);
	throw RTException("Unable to retrieve XML message content");
    }

    ContentTable* ctable=NULL;
    try{
	ctable = new ContentTable();
	for (DOMNode* child = n->getFirstChild(); child != NULL; child=child->getNextSibling()){
	    Row* f;
	    if((f = parseElement(child))!= NULL)
		ctable->push_back(f);
	}  
    }
    catch(const RTException& e){
	cout << e.msg << endl;
	delete ctable;
	throw RTException("Unable to retrieve XML message content");
    }
    
    return ctable;
}

Row* QWMessageParser::parseElement(DOMNode* node) throw(RTException){

  if(node->getNodeType() != DOMNode::ELEMENT_NODE){
    return NULL;
  }

  char* att_name=NULL;
  Row* df =NULL;
  DATATYPE type;

  try{
      df = new Row();
      
      char *elename = XMLString::transcode(node->getNodeName());
      type = getDataType(elename);
      XMLString::release(&elename);  
      

      if(node->hasAttributes()) {
	  // get all the attributes of the node
	  DOMNamedNodeMap *pAttributes = node->getAttributes();
	  int nSize = pAttributes->getLength();
	  if(nSize){
	      DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(0);
	      // get attribute name
	      char *attname = XMLString::transcode(pAttributeNode->getName());
	      if(strcmp(attname,"Name")==0){
		  // get attribute value
		  char* _value = XMLString::transcode(pAttributeNode->getValue());
		  att_name = strdup(_value);
		  XMLString::release(&_value);
	      }
	      XMLString::release(&attname);	
	  }
      }
  }
  catch(...){
      delete df;
      free(att_name);
      throw RTException("Error during attribute parsing");
  }
  
  df->name = att_name;
  df->type = type;

  DOMNode *child=NULL;
  char* value=NULL;
  ContentTable* ct=NULL;

  try{
      if(ISARRAY(type)){
	  df->val._ptr = new ContentTable();
	  ct = (ContentTable*)df->val._ptr;
	  for (child = node->getFirstChild(); child != NULL; child=child->getNextSibling()){
	      Row* f;
	      if((f = parseElement(child))!= NULL)
		  ct->push_back(f);
	  }  
      }
      else{
	  for (child = node->getFirstChild(); child != NULL; child=child->getNextSibling()){
	      if(node->getNodeType() != DOMNode::TEXT_NODE){
		  DOMText* textNode = (DOMText*)child;
		  XMLCh* string = (XMLCh*)textNode->getData();
		  char* data = XMLString::transcode(string);
		  fillField(df,data);
		  XMLString::release(&data);
		  break;
	      }	  
	  }  
      }
  }
  catch(...){
      free(df->name);
      if(df->val._ptr && ISARRAY(df->type))
	  delete ((ContentTable*)df->val._ptr);
      if(df->type==STRING)
	  free(df->val._string);
      delete df;
      throw RTException("Error during node parsing");
  }
  return df;
}

void QWMessageParser::fillField(Row* field,char* value) throw(RTException){

	void* array =NULL;
	try{
	    switch(field->type){
	    case BYTE:
	    case CHAR: 
		field->val._byte = getByte(value);
		break;
	    case SHORT: 
		field->val._short = getShort(value);
		break;
	    case INT: 
		field->val._int = getInt(value);
		break;
	    case LONGLONG: 
		field->val._longlong = getLongLong(value);
		break;
	    case FLOAT:
		field->val._float = getFloat(value);
		break;
	    case DOUBLE: 
		field->val._double = getDouble(value);
		break;
	    case BOOLEAN: 
		field->val._bool = getBoolean(value);
		break;
	    case STRING: 
		if(value==NULL){
		    field->val._string = strdup("");
		}
		else{
		    field->val._string = strdup(value);
		}
		break;
	    }
	}
	catch(RTIllegalArgumentException e){
	    cout << e.msg << endl;
	    throw RTException("Illegal Data field");
	}
}

DATATYPE QWMessageParser::getDataType(char* name){
    if(!name){
	return UNKNOWN;
    }
    if(strcmp(name,"Byte")==0){
	return BYTE;
    }
    else if(strcmp(name,"Char")==0){
	return CHAR;
    }
    else if(strcmp(name,"Int2")==0){
	return SHORT;
    }
    else if(strcmp(name,"Int4")==0){
	return INT;
    }
    else if(strcmp(name,"Int8")==0){
	return LONGLONG;
    }
    else if(strcmp(name,"Real4")==0){
	return FLOAT;
    }
    else if(strcmp(name,"Real8")==0){
	return DOUBLE;
    }
    else if(strcmp(name,"Boolean")==0){
	return BOOLEAN;
    }
    else if(strcmp(name,"String")==0){
	return STRING;
    }
    else if(strcmp(name,"ByteArray")==0){
	return BYTEARRAY;
    }
    else if(strcmp(name,"CharArray")==0){
	return CHARARRAY;
    }
    else if(strcmp(name,"Int2Array")==0){
	return SHORTARRAY;
    }
    else if(strcmp(name,"Int4Array")==0){
	return INTARRAY;
    }
    else if(strcmp(name,"Int8Array")==0){
	return LONGLONGARRAY;
    }
    else if(strcmp(name,"Real4Array")==0){
	return FLOATARRAY;
    }
    else if(strcmp(name,"Real8Array")==0){
	return DOUBLEARRAY;
    }
    else if(strcmp(name,"BooleanArray")==0){
	return BOOLEANARRAY;
    }
    else if(strcmp(name,"StringArray")==0){
	return STRINGARRAY;
    }
    else{
	return UNKNOWN;
    }
}

/*String-to-Data Conversion Functions*/
char QWMessageParser::getByte(char* str) throw(RTIllegalArgumentException){
    if(!str){
	 throw RTIllegalArgumentException("String is null. Can not convert string to byte value");
    }

    return str[0];
}

short QWMessageParser::getShort(char* str) throw(RTIllegalArgumentException){

    if(!str){
	throw RTIllegalArgumentException("String is null. Can not convert string to short value");
    }  
    short v = (short)atoi(str);
    if(v==0 & (errno == ERANGE || errno == EINVAL)){
      throw RTIllegalArgumentException("Illegal value");
    }  	
    return v;    
}

int QWMessageParser::getInt(char* str) throw(RTIllegalArgumentException){
    if(!str){
	throw RTIllegalArgumentException("String is null. Can not convert string to integer value");
    }

    int v = (int)atoi(str);
    if(v==0 & (errno == ERANGE || errno == EINVAL)){
	throw RTIllegalArgumentException("Illegal value");
    }  	

    return v;
}

long long QWMessageParser::getLongLong(char* str) throw(RTIllegalArgumentException){
    if(!str){
	throw RTIllegalArgumentException("String is null. Can not convert string to long long value");
    }

    long long v = atoll(str);
    if(v==0 & (errno == ERANGE || errno == EINVAL)){
	throw RTIllegalArgumentException("Illegal value");
    }  
    return v;
}

float QWMessageParser::getFloat(char* str) throw(RTIllegalArgumentException){
    if(!str){
	throw RTIllegalArgumentException("String is null. Can not convert string to float value");
    }

    float v  = (float)atof(str);
    if(v==0 & (errno == ERANGE || errno == EINVAL)){
	throw RTIllegalArgumentException("Illegal value");
    }  
    return v;
}

double QWMessageParser::getDouble(char* str) throw(RTIllegalArgumentException){
    if(!str){
	throw RTIllegalArgumentException("String is null. Can not convert string to double value");
    }

    double v =  atof(str);
    if(v==0 & (errno == ERANGE || errno == EINVAL)){
	throw RTIllegalArgumentException("Illegal value");
    }  
    return v;
}

bool QWMessageParser::getBoolean(char* str) throw(RTIllegalArgumentException){
    if(!str){
	throw RTIllegalArgumentException("String is null. Can not convert string to boolean value");
    }
    if(strcmp(str,"true")==0)
	return true;
    else if(strcmp(str,"false")==0)
	return false;
    else 
	throw RTIllegalArgumentException("Illegal value");
}

