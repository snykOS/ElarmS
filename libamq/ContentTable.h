#ifndef __contenttable_h
#define __contenttable_h

#include <list>
#include <stdio.h>
#include <string.h>
#include <ostream>

using namespace std;

#define PSTEP() {int l = __LINE__;printf("At line:%d\n",l);}

typedef list<char>       ByteList;
typedef list<short>      ShortList;
typedef list<int>        IntList;
typedef list<long long>  LongLongList;
typedef list<float>      FloatList;
typedef list<double>     DoubleList;
typedef list<bool>       BoolList;
typedef list<char*>      StrList;


enum DATATYPE{UNKNOWN=0,BYTE,CHAR,SHORT,INT,LONGLONG,FLOAT,DOUBLE,BOOLEAN,STRING,BYTEARRAY,CHARARRAY,
              SHORTARRAY,INTARRAY,LONGLONGARRAY,FLOATARRAY,DOUBLEARRAY,BOOLEANARRAY,STRINGARRAY};


#define ISARRAY(type) (type == BYTEARRAY|| type == CHARARRAY|| type == SHORTARRAY|| type == INTARRAY|| \
           type == LONGLONGARRAY|| type == FLOATARRAY||type == DOUBLEARRAY|| type == BOOLEANARRAY|| \
            type == STRINGARRAY)

union _value{
    void* _ptr;
    char _byte; //for byte and char
    short _short;
    int   _int;
    long long _longlong;
    float _float;
    double _double;
    bool _bool;
    char* _string;
};

class Row{

 public:
    char* name; //Name of the field
    DATATYPE type;
    _value val;
};

typedef list<Row*> _ContentTable;

class ContentTable: public _ContentTable{

 public:

    ContentTable();
    ContentTable(const ContentTable&);

    ~ContentTable();

    ContentTable& operator=(const ContentTable&);
    friend ostream& operator<<(ostream& os,const ContentTable&);

}; 

#endif//__contenttable_h
