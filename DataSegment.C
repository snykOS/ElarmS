/***********************************************************

File Name :
        DataSegment.C

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:
	4 April 2006: Added byte-swapping to support little-endian hardware.
	The original protocol does not mention byte order but is implictly
	for big-endian (Sun sparc) hardware. Pete Lombard, UCB
	NOTE: the actual data in a DataSegement is not byte-swapped.
	We assume this data to be miniSEED format without its own byte
	order declarations.

Usage Notes:


**********************************************************/


// Various include files
#include <cstring>
#include "RetCodes.h"
#include "DataSegment.h"
#include "ByteSwap.h"

using namespace std;

// Structure defining the wire format of a Data Segment
struct wireDataSegment
{
  double start;
  int numsamp;
  int len;
  char data[DS_MAX_SIZE];
};

typedef wireDataSegment wireDataSegment;



DataSegment::DataSegment()
{
  start = TimeStamp(TRUE_EPOCH_TIME, 0.0);
  numsamp = 0;
  len = 0;
  memset(data, 0, DS_MAX_SIZE);
}



DataSegment::DataSegment(const char *buf)
{
  wireDataSegment w;
  int tmp_int;
  double tmp_start;
  
   memset(data, 0, DS_MAX_SIZE);

    // First have to find out how much we have
    // Copy into temp to avoid data alignment problems
    memcpy(&tmp_int, buf + sizeof(double) + sizeof(int), sizeof(int));
    len = ntohl(tmp_int);
    if (len > DS_MAX_SIZE) {
	std::cout << "Error (DataSegment::DataSegment): wire buffer too long"
		  << std::endl;
	len = 0;
	numsamp = 0;
	start = TimeStamp(TRUE_EPOCH_TIME, 0.0);

	return;
    }

   // Copy into wireDataSegment to avoid data alignment problems
    memcpy(&w, buf, len + sizeof(double) + 2 * sizeof(int));

    tmp_start = ntohd(w.start);
    start = TimeStamp(TRUE_EPOCH_TIME,tmp_start);

    numsamp = ntohl(w.numsamp);
    memcpy(data, w.data, len);
}



DataSegment::DataSegment(const DataSegment &ds)
{
  start = ds.start;
  numsamp = ds.numsamp;
  len = ds.len;
  memcpy(data, ds.data, ds.len);
}



DataSegment::~DataSegment()
{
}



int DataSegment::Serialize(char *buf, int &buflen)
{
  wireDataSegment w;

  if (buflen < sizeof(wireDataSegment)) {
    std::cout << "Error (DataSegment::Serialize): Buffer is too small" << std::endl;
    return(TN_FAILURE);
  }
  w.start = htond(start.ts_as_double(TRUE_EPOCH_TIME));
  w.numsamp = htonl(numsamp);
  w.len = htonl(len);
  buflen = sizeof(double) + (2 * sizeof(int));
  memcpy(buf, &w, buflen);
  memcpy(buf + buflen, data, len);
  buflen += len;
  return(TN_SUCCESS);
}



DataSegment& DataSegment::operator=(const DataSegment &ds)
{
  start = ds.start;
  numsamp = ds.numsamp;
  len = ds.len;
  memcpy(data, ds.data, ds.len);
  return(*this);
}


int operator<(const DataSegment &ds1, const DataSegment &ds2)
{
  if (ds1.start < ds2.start) {
    return(TN_TRUE);
  } else if ((ds1.start == ds2.start) && (ds1.numsamp < ds2.numsamp)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}


int operator==(const DataSegment &ds1, const DataSegment &ds2)
{
  if ((ds1.start == ds2.start) && (ds1.numsamp == ds2.numsamp)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}

