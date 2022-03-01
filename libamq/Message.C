/***********************************************************

File Name :
        Message.C

Original Author:
        Patrick Small
	

Description:

        This source file defines the Message class - an object class 
which abstracts the use of QWServer messages. Methods 
are provided to create new messages, store scalar and array data, and
set most of the properties of the underlying message.

Creation Date:
        06 July 1998

Modification History:
	ReWritten  02 April 2004   by Kalpesh Solanki


Usage Notes:

        The user process can check to make sure that the message
was created successfully by using the "!" overloaded boolean operator.

        Most functions provide an integer return value which is
TN_SUCCESS if the operation was successful, or TN_FAILURE otherwise.


**********************************************************/

#include <stdlib.h>		// free

// Various include files
#include "GenLimits.h"
#include "RetCodes.h"
//#include "Connection.h"
#include "Message.h"
#include "MessageTypes.h"
#include <string.h>

const char* _emptyname = "NoName";

int Message::count = 0;

Message::Message(){
    ctable = new ContentTable();
    cti = ctable->begin();
    iown = true;
    type = TN_TMT_UNKNOWN;
    dest = NULL;
    domainname = NULL;
    sourcename = NULL;
    count++;
    //   cout <<"Total Message instances = "<<count<<endl;
}


Message::Message(const int mtype){
    ctable = new ContentTable();
    cti = ctable->begin();
    iown = true;
    type = (int)mtype;
    dest = NULL;
    domainname = NULL;
    sourcename = NULL;
    count++;
    //   cout <<"Total Message instances = "<<count<<endl;
}

Message::Message(ContentTable* _ctable,const int mtype){
    ctable = _ctable;
    cti = ctable->begin();
    iown = false;
    type = (int)mtype;
    dest = NULL;
    domainname = NULL;
    sourcename = NULL;
    count++;
    //  cout <<"Total Message instances = "<<count<<endl;
}

// Message::Message(const Message& othermsg){

//     type = othermsg.type;

//     if(othermsg.dest)
// 	dest = strdup(othermsg.dest);
//     if(othermsg.ctable){
// 	if(ctable==NULL)
// 	    ctable = new ContentTable();
// 	*ctable = *othermsg.ctable;
//     }

//     if(ctable){
// 	cti = ctable->begin();
//     }
//     cout << "Object copied"<<endl;
// }

Message::~Message(){
    if(iown)
	delete ctable;
    if(dest)
      free(dest);
    if(domainname)
      free(domainname);
    if(sourcename)
      free(sourcename);
    //cleanup temp buffers//
    for(list<void*>::iterator it=gclist.begin();it!=gclist.end();it++){
	free(*it);
    }
    count--;
    //  cout <<"Total Message instances = "<<count<<endl;
}

int Message::append(short int i){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }
    f->type = SHORT;
    f->val._short = i;
    
    ctable->push_back(f);
    return (TN_SUCCESS);
}

int Message::append(short int* i, int num){
    if(!i)
	return (TN_FAILURE);

    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = SHORTARRAY;
    ContentTable* ct = new ContentTable();
    f->val._ptr = (void*)ct;
    for(int x=0;x<num;x++){
	Row* df = new Row();
	df->name = strdup(_emptyname);
	if(!df->name){
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	df->type = SHORT;
	df->val._short = i[x];
	ct->push_back(df);
    }

    ctable->push_back(f);
    return (TN_SUCCESS);
}

int Message::append(int i){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = INT;
    f->val._int = i;
    
    ctable->push_back(f);
    return (TN_SUCCESS);
}

int Message::append(int* i, int num){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = INTARRAY;

    ContentTable* ct = new ContentTable();
    f->val._ptr = (void*)ct;
    for(int x=0;x<num;x++){
	Row* df = new Row();
	df->name = strdup(_emptyname);
	if(!df->name){
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	df->type = INT;
	df->val._int = i[x];
	ct->push_back(df);
    }

    ctable->push_back(f);
    return (TN_SUCCESS);
}

int Message::append(long i){
    return append((int)i);
}

int Message::append(long* i, int num){
    return append((int*)i,num);
}

int Message::append(long long i){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = LONGLONG;
    f->val._longlong = i;
 
    ctable->push_back(f);   
    return (TN_SUCCESS);
}

int Message::append(long long* i, int num){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = LONGLONGARRAY;

    ContentTable* ct = new ContentTable();
    f->val._ptr = (void*)ct;
    for(int x=0;x<num;x++){
	Row* df = new Row();
	df->name = strdup(_emptyname);
	if(!df->name){
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	df->type = LONGLONG;
	df->val._longlong = i[x];
	ct->push_back(df);
    }


    ctable->push_back(f);   
    return (TN_SUCCESS);
}

int Message::append(double d){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }


    f->type = DOUBLE;
    f->val._double = d;
    
    ctable->push_back(f);   
    return (TN_SUCCESS);
}

int Message::append(double* d, int num){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = DOUBLEARRAY;

    ContentTable* ct = new ContentTable();
    f->val._ptr = (void*)ct;
    for(int x=0;x<num;x++){
	Row* df = new Row();
	df->name = strdup(_emptyname);
	if(!df->name){
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	df->type = DOUBLE;
	df->val._double = d[x];
	ct->push_back(df);
    }

    ctable->push_back(f);   
    return (TN_SUCCESS);
}

int Message::append(float i){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }


    f->type = FLOAT;
    f->val._float = i;
    
    ctable->push_back(f); 
    return (TN_SUCCESS);
}

int Message::append(float* i, int num){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = FLOATARRAY;

    ContentTable* ct = new ContentTable();
    f->val._ptr = (void*)ct;
    for(int x=0;x<num;x++){
	Row* df = new Row();
	df->name = strdup(_emptyname);
	if(!df->name){
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	df->type = FLOAT;
	df->val._float = i[x];
	ct->push_back(df);
    }

    ctable->push_back(f); 
    return (TN_SUCCESS);
}

int Message::append(char *s){
    Row *f = new Row();
    f->name = strdup(_emptyname);

    f->type = STRING;
    f->val._string = strdup(s);
    
    if(!f->name || !f->val._string){
	delete f;
	return (TN_FAILURE);
    }
       
    ctable->push_back(f);  
    return (TN_SUCCESS);
}

int Message::append(const char *s){
    return append((char*)s);
}

int Message::append(char **s, int num){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = STRINGARRAY;

    ContentTable* ct = new ContentTable();
    f->val._ptr = (void*)ct;
    for(int x=0;x<num;x++){
	Row* df = new Row();
	df->name = strdup(_emptyname);
	if(!df->name){
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	char* str = strdup(s[x]);
	if(!str){
	  free(df->name);
	  delete df;
	  free(f->name);
	  delete f;
	  delete ct;
	  return (TN_FAILURE);
	}
	df->type = STRING;
	df->val._string = str;
	ct->push_back(df);
    }

    ctable->push_back(f);  
    return (TN_SUCCESS);
}

int Message::append(char c){
    Row *f = new Row();
    f->name = strdup(_emptyname);
    if(!f->name){
	delete f;
	return (TN_FAILURE);
    }

    f->type = CHAR;
    f->val._byte = c;
    
    ctable->push_back(f); 
    return (TN_SUCCESS);
}

//nexts


int Message::next(short int& i){
    if(cti==ctable->end())
	return (TN_FAILURE);

    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);

    if(f->type!=SHORT)
	return (TN_FAILURE);

    i = f->val._short;
    cti++;
    return (TN_SUCCESS);
}

int Message::next(short int** i, int& num){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=SHORTARRAY)
	return (TN_FAILURE);


    ContentTable* ct = (ContentTable*)f->val._ptr;
    num = ct->size();
    *i = (short*)malloc(num*sizeof(short));
    short* p = *i;
    if(!p)
	return (TN_FAILURE);
    
    gclist.push_back((void*)p);
    int c=0;
    for(ContentTable::iterator _it = ct->begin();_it!=ct->end();_it++){
      p[c] = (*_it)->val._short;
      c++;
    }
    cti++;
    return (TN_SUCCESS);
}

int Message::next(int& i){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=INT)
	return (TN_FAILURE);

    i = f->val._int;
    cti++;
    return (TN_SUCCESS);
}

int Message::next(int** i, int& num){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=INTARRAY)
	return (TN_FAILURE);

    ContentTable* ct = (ContentTable*)f->val._ptr;
    num = ct->size();
    *i = (int*)malloc(num*sizeof(int));
    int* p = *i;
    if(!p)
	return (TN_FAILURE);
    
    gclist.push_back((void*)p);
    int c=0;
    for(ContentTable::iterator _it = ct->begin();_it!=ct->end();_it++){
      p[c] = (*_it)->val._int;
      c++;
    }
    cti++;
    return (TN_SUCCESS);
}

int Message::next(long& i){  
    return next((int&)i);
}


int Message::next(long** i, int& num){
    return next((int**)i,num);
}


int Message::next(long long& i){  
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);

    if(f->type!=LONGLONG)
	return (TN_FAILURE);

    i = f->val._longlong;
    cti++;
    return (TN_SUCCESS);
}


int Message::next(long long** i, int& num){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=LONGLONGARRAY)
	return (TN_FAILURE);

    ContentTable* ct = (ContentTable*)f->val._ptr;
    num = ct->size();
    *i = (long long*)malloc(num*sizeof(long long));
    long long* p = *i;
    if(!p)
	return (TN_FAILURE);
    
    gclist.push_back((void*)p);
    int c=0;
    for(ContentTable::iterator _it = ct->begin();_it!=ct->end();_it++){
      p[c] = (*_it)->val._longlong;
      c++;
    }
    cti++;
    return (TN_SUCCESS);
}

int Message::next(double& d){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=DOUBLE)
	return (TN_FAILURE);

    d = f->val._double;
    cti++;
    return (TN_SUCCESS);
}

int Message::next(double** d, int& num){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=DOUBLEARRAY)
	return (TN_FAILURE);

    ContentTable* ct = (ContentTable*)f->val._ptr;
    num = ct->size();
    *d = (double*)malloc(num*sizeof(double));
    double* p = *d;
    if(!p)
	return (TN_FAILURE);
    
    gclist.push_back((void*)p);
    int c=0;
    for(ContentTable::iterator _it = ct->begin();_it!=ct->end();_it++){
      p[c] = (*_it)->val._double;
      c++;
    }
    cti++;
    return (TN_SUCCESS);
}

int Message::next(float& i){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=FLOAT)
	return (TN_FAILURE);

    i  = f->val._float;
    cti++;
    return (TN_SUCCESS);
}

int Message::next(float** i, int& num){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=FLOATARRAY)
	return (TN_FAILURE);

    ContentTable* ct = (ContentTable*)f->val._ptr;
    num = ct->size();
    *i = (float*)malloc(num*sizeof(float));
    float* p = *i;
    if(!p)
	return (TN_FAILURE);
    
    gclist.push_back((void*)p);
    int c=0;
    for(ContentTable::iterator _it = ct->begin();_it!=ct->end();_it++){
      p[c] = (*_it)->val._float;
      c++;
    }
    cti++;
    return (TN_SUCCESS);
}

int Message::next(char** s){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);
    if(f->type!=STRING)
	return (TN_FAILURE);

    *s = f->val._string;
    cti++;
    return (TN_SUCCESS);
}

int Message::next(char ***s, int &num){
    if(cti==ctable->end())
	return (TN_FAILURE);
    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);

    if(f->type!=STRINGARRAY)
	return (TN_FAILURE);

    ContentTable* ct = (ContentTable*)f->val._ptr;
    num = ct->size();
    *s = (char**)malloc(num*sizeof(char*));
    char** p = *s;
    if(!p)
	return (TN_FAILURE);
    
    gclist.push_back((void*)p);
    int c=0;
    for(ContentTable::iterator _it = ct->begin();_it!=ct->end();_it++){
      p[c] = (*_it)->val._string;
      c++;
    }
    cti++;
    return (TN_SUCCESS);
}

int Message::next(char& c){
    if(cti==ctable->end())
	return (TN_FAILURE);

    Row *f = *cti;
    if(!f)
	return (TN_FAILURE);

    if(f->type!=CHAR)
	return (TN_FAILURE);

    c = f->val._byte;
    cti++;
    return (TN_SUCCESS);
}

int Message::setDest(const char *_dest){
    if(!_dest)
	return (TN_FAILURE);
    free(dest);
    dest = strdup(_dest);
    if(!dest)
	return (TN_FAILURE);
    return (TN_SUCCESS);
}

int Message::clearData(){
    if(ctable){
	if(iown){
	    delete ctable;
	    ctable = NULL;
	}
    }

    ctable = new ContentTable();
    cti = ctable->begin();
    iown = true;
    free(dest);
    free(domainname);
    free(sourcename);
    dest = NULL;

    //cleanup temp buffers//
    for(list<void*>::iterator it=gclist.begin();it!=gclist.end();it++){
	free(*it);
    }
    return (TN_SUCCESS);
}

int Message::setDomainName(const char* _domain){
    if(!_domain)
	return (TN_FAILURE);
    free(domainname);
    domainname = strdup(_domain);
    if(!domainname)
	return (TN_FAILURE);
    return (TN_SUCCESS);
}

int Message::setSourceName(const char* _source){
    if(!_source)
	return (TN_FAILURE);
    free(sourcename);
    sourcename = strdup(_source);
    if(!sourcename)
	return (TN_FAILURE);
    return (TN_SUCCESS);
}

const char* Message::getDomainName() const{
    return domainname;
}

const char* Message::getSourceName() const{
    return sourcename;
}


const char* Message::getDest() const{
    return dest;
}

int Message::getDest(char *_dest){
    if(!_dest)
	return (TN_FAILURE);
    strcpy(_dest,dest);
    return (TN_SUCCESS);
}

int Message::getType(int& msgtype) const{
    if(type == TN_TMT_UNKNOWN){
	cerr <<"Error in Message::getType(int&): Message type is unknown"<<endl;
	return (TN_FAILURE);
    }
    msgtype = type;
    return (TN_SUCCESS);
}

int Message::getTypeString(char* str) const{
    if(type == TN_TMT_UNKNOWN){
	cerr <<"Error in Message::getTypeString(char*): Message type is unknown"<<endl;
	return (TN_FAILURE);
    }
    for(int i=0;i<=MTYPE_MAX_MTYPES;i++){
	if(msgDefs[i].id == type){
	    strcpy(str,(char*)msgDefs[i].label);
	    return (TN_SUCCESS);
	}
    }
    cerr <<"Error in Message::getTypeString(char*): Couldn't find type name"<<endl;
    return (TN_FAILURE);
}

int Message::getType(const char* typestr){
    if(!typestr)
	return TN_TMT_UNKNOWN;

    for(int i=0;i<=MTYPE_MAX_MTYPES;i++){
	if(strcmp(msgDefs[i].label,typestr)==0){
	    return msgDefs[i].id;
	}
    }  
    return TN_TMT_UNKNOWN;
}

ContentTable* Message::getContentTable() const{
    return ctable;
}

int operator!(const Message& msg)
{
    return false;//(!(msg.valid));
}
  
ostream& operator<<(ostream& os,Message& m){
//     if(!m.valid){
// 	os << "Message is invalid" <<endl;
// 	return os;
//     }
    char* typestr = (char*)malloc(128);
    m.getTypeString(typestr);
    
    os <<"- Message Type : "<<typestr<<endl;
    if(m.dest)
	os <<"- Destination: "<<m.dest<<endl;
    else
	os <<"- Destination is unknown"<<endl;
    os << "- GC list contains "<<m.gclist.size()<<" elements."<<endl;
    if(m.ctable){
	int c=0;
	bool found=false;
	for(ContentTable::iterator it=m.ctable->begin();it!=m.ctable->end();it++,c++){
	    if(it==m.cti){
		os <<"- Content Table iterator is pointing to element at position "<<c<<endl;
		found = true;
	    }
	}
	if(!found)
	    os <<"- Content Table iterator is pointing to nowhere!" <<endl;
	os << *(m.ctable);
    }
    else
	os <<"- Content Table is null"<<endl;

    free(typestr);
    return os;
}

/* It copies the message content but not the temporary state, e.g temp buffers and iterators*/
// Message& Message::operator=(const Message& othermsg){
//     clearData();

//     // valid = othermsg.valid;
//     type = othermsg.type;

//     if(othermsg.dest)
// 	dest = strdup(othermsg.dest);
        
//     if(othermsg.ctable){
// 	*ctable = *othermsg.ctable;
// 	cout << *ctable;
//     }

//     if(ctable){
// 	cti = ctable->begin();
//     }
//     return *this;
// }

/* Message index starts with 0 (ZERO) */
int Message::curField(int fnum){
    int c=0;
    for(ContentTable::iterator it=ctable->begin();it!=ctable->end();it++,c++){
	if(c==fnum){
	    if(it == ctable->end())
		return (TN_FAILURE);
	    cti = it;
	    return (TN_SUCCESS);
	}	
    }
    return (TN_FAILURE);
}


int Message::print(){
    cout << *this;
    return (TN_SUCCESS);
}
