#include <stdlib.h>		// free
#include "ContentTable.h"

ContentTable::ContentTable(){}

ContentTable::ContentTable(const ContentTable& _ctable){
    ContentTable* ctable = (ContentTable*)(&_ctable);
    for(ContentTable::iterator cit = ctable->begin();cit != ctable->end();cit++){
	Row *sf = *cit;
	Row *df = new Row();
	df->name = strdup(sf->name);
	df->type = sf->type;

	if(sf->type == STRING){
	    df->val._string = strdup(sf->val._string);
	}
	else if(ISARRAY(sf->type)){
	    ContentTable *ct = new ContentTable();
	    *ct = *((ContentTable*)sf->val._ptr);
	    df->val._ptr = ct;
	}
	else{
	    df->val = sf->val;
	}
	push_back(df);
    } 
}

ContentTable::~ContentTable(){
    //  cout << "Deleting.. Size of ContentTable is "<<size()<<endl;
    for(ContentTable::iterator cit  = begin();cit!=end();cit++){
	Row *field = *cit;
	if(field == NULL)
	  continue;
	free(field->name);

	if(field->type == STRING){
	    free(field->val._string);
	}
	else if(ISARRAY(field->type)){
	    //   cout << "~ Type : "<<field->type<<endl;
	    //   cout << "~ Table size: "<< ((ContentTable*)field->val._ptr)->size();
	    ContentTable* ct = (ContentTable*)field->val._ptr;
	    delete ct;
	}
	delete field;
    }
    clear();
}


ContentTable& ContentTable::operator=(const ContentTable& _ctable){
    //   cout << "calling = !!!!"<<endl;

    ContentTable* ctable = (ContentTable*)(&_ctable);
    for(ContentTable::iterator cit = ctable->begin();cit != ctable->end();cit++){
	Row *sf = *cit;
	Row *df = new Row();
	df->name = strdup(sf->name);
	df->type = sf->type;

	if(sf->type == STRING){
	    df->val._string = strdup(sf->val._string);
	}
	else if(ISARRAY(sf->type)){
	    ContentTable *ct = new ContentTable();
	    *ct = *((ContentTable*)sf->val._ptr);
	    df->val._ptr = ct;
	}
	else{
	    df->val = sf->val;
	}
	push_back(df);
    } 

    return *this;
}

ostream& operator<<(ostream& os,const ContentTable& _ctable){
    static int tabcount =0;
    ContentTable* ct;
    ContentTable* ctable = (ContentTable*)(&_ctable);
    os <<"Table size : "<<ctable->size()<<endl;
    for(ContentTable::iterator cit = ctable->begin();cit != ctable->end();cit++){
	Row *f = *cit;

	//	os <<f->type <<"   "<<endl;
	os << "Name: "<<f->name;
	os << " type :";
	switch(f->type){
	case BYTE:
	    os << "byte, "; 
	    os << "value:" << f->val._byte <<endl;
	case CHAR: 
	    os << "char, ";
	    os << "value:" << f->val._byte <<endl;
	    break;
	case SHORT: 
	    os << "short, ";
	    os << "value:" << f->val._short <<endl;
	    break;
	case INT: 
	    os << "int, ";
	    os << "value:" << f->val._int <<endl;
	    break;
	case LONGLONG: 
	    os << "longlong, ";
	    os << "value:" << f->val._longlong <<endl;
	    break;
	case FLOAT:
	    os << "float, ";
	    os << "value:" << f->val._float <<endl;
	    break;
	case DOUBLE: 
	    os << "double, ";
	    os << "value:" << f->val._double <<endl;
	    break;
	case BOOLEAN: 
	    os << "boolean, ";
	    os << "value:" << f->val._bool <<endl;
	    break;
	case STRING: 
	    os << "string, ";
	    os << "value:" << f->val._string <<endl;
	    break;
	case BYTEARRAY:
	case CHARARRAY:
	case SHORTARRAY: 
	case INTARRAY: 
	case LONGLONGARRAY:
	case FLOATARRAY: 
	case DOUBLEARRAY: 
	case BOOLEANARRAY: 
	case STRINGARRAY:
	    for(int i=0;i<tabcount;i++)
		os <<"\t";

	    tabcount++;
	    ct = (ContentTable*)f->val._ptr;
	    os << *ct;
	    break;
	default:
	    os << "Unknown type" <<endl;
   	}
    }

    tabcount--;
    return os;
}

