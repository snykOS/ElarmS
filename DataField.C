/***********************************************************

File Name :
        DataField.C

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:
	4 April 2006: Added byte-swapping to support little-endian hardware.
	The original protocol does not mention byte order but is implictly
	for big-endian (Sun sparc) hardware. Pete Lombard, UCB


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include "RetCodes.h"
#include "DataField.h"
#include "ByteSwap.h"

using namespace std;

// Structure defining the wire format of a Data Field object
struct wireDataField {
  datatype dtype;
  int len;
  char buf[DATAFIELD_MAX_SIZE];
};

typedef struct wireDataField wireDataField;




DataField::DataField()
{
  dtype = DF_NONE;
  len = 0;
  memset(&value, 0, sizeof(opaque));
}


DataField::DataField(const char *buf)
{
  wireDataField w;
  int temp, buflen;
    
  memset(&value, 0, sizeof(opaque));

  // First have to find out how much we have
  // Copy into temp to avoid data alignment problems
  // WARNING: this is not guaranteed to work: can't be certain that
  // w.len starts immediately after w.dtype
  // Safer design would have len first in wireDataField: PNL 2006/04/04
  memcpy(&temp, buf + sizeof(datatype), sizeof(int));

  len = ntohl((long)temp);
  buflen = len + sizeof(datatype) + sizeof(int);
  if (buflen > sizeof(wireDataField)) {
      std::cout << "Error (DataField::DataField): wire buffer too long"
		<< std::endl;
      len = 0;
      dtype = DF_NONE;
      return;
  }
  
  memcpy(&w, buf, buflen);
  dtype = datatype(ntohl(w.dtype));

  // Extract the data value
  switch (dtype) {
  case DF_NONE:
    std::cout << "Error (DataField::DataField): Cannot have data type of NONE" 
	 << std::endl;
    return;
    break;
  case DF_INTEGER:
    if (len != sizeof(int)) {
      std::cout << "Error (DataField::DataField): Invalid integer size" 
	   << std::endl;
      return;
    }
    memcpy(&temp, w.buf, sizeof(int));
    value.i = ntohl((long)temp);
    break;
  case DF_FLOAT:  // Actually its a double
      {
	  if (len != sizeof(double)) {
	      std::cout << "Error (DataField::DataField): Invalid double size" 
			<< std::endl;
	      return;
	  }
	  double dtemp;
	  memcpy(&dtemp, w.buf, sizeof(double));
	  value.d = ntohd(dtemp);
      }
    break;
  case DF_STRING:
  case DF_BINARY:
    if ((len < 0) || (len > DATAFIELD_MAX_SIZE)) {
      std::cout << "Error (DataField::DataField): Invalid string/binary size" 
	   << std::endl;
      return;
    }
    memcpy(&value, w.buf, len);
    break;
  default:
    std::cout << "Error (DataField::DataField): Invalid data type identifier" 
	 << std::endl;
    return;
  }
}



DataField::DataField(const DataField &df)
{
  memset(&value, 0, sizeof(opaque));
  dtype = df.dtype;
  len = df.len;
  memcpy(&value, &(df.value), df.len);
}



DataField::~DataField()
{
}


int DataField::Set(int i)
{
  memset(&value, 0, sizeof(opaque));

  dtype = DF_INTEGER;
  len = sizeof(int);
  value.i = i;
  return(TN_SUCCESS);
}


int DataField::Set(double d)
{
  memset(&value, 0, sizeof(opaque));

  dtype = DF_FLOAT;
  len = sizeof(double);
  value.d = d;
  return(TN_SUCCESS);
}



int DataField::Set(char *str)
{
  memset(&value, 0, sizeof(opaque));

  if (strlen(str) > DATAFIELD_MAX_SIZE) {
    std::cout << "Error (DataField::Store): String buffer too long" << std::endl;
    return(TN_FAILURE);
  }
  dtype = DF_STRING;
  len = strlen(str);
  strcpy(value.str, str);
  return(TN_SUCCESS);
}


int DataField::Set(char *buf, int buflen)
{
  memset(&value, 0, sizeof(opaque));

  if (buflen > DATAFIELD_MAX_SIZE) {
    std::cout << "Error (DataField::Store): Binary buffer too long" << std::endl;
    return(TN_FAILURE);
  }
  dtype = DF_BINARY;
  len = buflen;
  memcpy(value.bin, buf, buflen);
  return(TN_SUCCESS);
}



int DataField::Get(int &i)
{
  if (dtype == DF_INTEGER) {
    i = value.i;
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (DataField::Get): Field does not contain an integer" 
	 << std::endl;
    return(TN_FAILURE);
  }
}


int DataField::Get(double &d)
{
  if (dtype == DF_FLOAT) {
    d = value.d;
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (DataField::Get): Field does not contain a double" 
	 << std::endl;
    std::cout << "Value is " << dtype << std::endl;
    return(TN_FAILURE);
  }
}



int DataField::Get(char *str)
{
  if (dtype == DF_STRING) {
    strncpy(str, value.str, len);
    str[len] = 0;
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (DataField::Get): Field does not contain a string" 
	 << std::endl;
    return(TN_FAILURE);
  }
}


int DataField::Get(char *buf, int &buflen)
{
  if (buflen <  len) {
    std::cout << "Error (DataField::Get): Buffer not large enough" << std::endl;
    return(TN_FAILURE);
  }
  if (dtype == DF_BINARY) {
    memcpy(buf, value.bin, len);
    buflen = len;
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (DataField::Get): Field does not contain raw binary" 
	 << std::endl;
    return(TN_FAILURE);
  }
}



int DataField::Serialize(char *buf, int &buflen)
{
  wireDataField w;

  if (buflen < sizeof(wireDataField)) {
    std::cout << "Error (DataField::Serialize): Buffer not large enough" << std::endl;
    return(TN_FAILURE);
  }
  w.dtype = datatype(htonl((long)dtype));
  w.len = htonl(len);
  switch(dtype) {
  case DF_INTEGER:
      {
	  int temp = htonl(value.i);
	  memcpy(w.buf, &temp, sizeof(int));
      }
      break;
  case DF_FLOAT:  // Actually its a double
      {
	  double temp = htond(value.d);
	  memcpy(w.buf, &temp, sizeof(double));
      }
      break;
  case DF_STRING:
  case DF_BINARY:
      memcpy(w.buf, &value, len);
      break;
  default:
      std::cout << "Error (DataField::Serialize): illegal type to serialize: "
		<< (int)dtype << std::endl;
      return(TN_FAILURE);
  }

  buflen = sizeof(datatype) + sizeof(int) + len;
  memcpy(buf, &w, buflen);
  return(TN_SUCCESS);
}



int DataField::GetLength()
{
  return(len);
}



datatype DataField::GetType()
{
  return(dtype);
}



int operator!(const DataField &df)
{
  if (df.dtype == DF_NONE) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}



DataField& DataField::operator=(const DataField &df)
{
  memset(&value, 0, sizeof(opaque));
  dtype = df.dtype;
  len = df.len;
  memcpy(&value, &(df.value), df.len);
  return(*this);
}

