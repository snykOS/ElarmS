/***********************************************************

File Name :
        DataField.h

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:
	15 October 2004: increased DATAFIELD_MAX_SIZE to 4200 from 1000. 
			PNL, UCB


Usage Notes:


**********************************************************/

#ifndef data_field_H
#define data_field_H

// Maximum size of a data field (in bytes)
const int DATAFIELD_MAX_SIZE = 4200;


// Maximum size of a data field in wire format (in bytes)
//
// Minimum:
// 4 byte datatype + 4 byte length + DATAFIELD_MAX_SIZE bytes of data
//
// When DATAFIELD_MAX_SIZE was 1000, DATAFIELD_MAX_WIRE was 1024 = 1000 + 24
// No explanation was given for this number: PNL 10/15/2004
const int DATAFIELD_MAX_WIRE = DATAFIELD_MAX_SIZE + 24;


// Enumerated type definition for the possible data types
enum datatype {DF_NONE, DF_INTEGER, DF_FLOAT, DF_STRING, DF_BINARY};

typedef enum datatype datatype;


// Storage for a field value
union opaqueStore {
    int i;
    double d;
    char str[DATAFIELD_MAX_SIZE];
    char bin[DATAFIELD_MAX_SIZE];
};

typedef union opaqueStore opaque;


class DataField
{
 private:
    datatype dtype;
    int len;
    opaque value;

 public:
    DataField();
    DataField(const char *buf);
    DataField(const DataField &df);
    ~DataField();
   
    int Set(int i);
    int Set(double d);
    int Set(char *str);
    int Set(char *buf, int buflen);
    int Get(int &i);
    int Get(double &d);
    int Get(char *str);
    int Get(char *buf, int &buflen);
    int Serialize(char *buf, int &buflen);
    int GetLength();
    datatype GetType();

    friend int operator!(const DataField &df);
    DataField& operator=(const DataField &df);
};


#endif

