/***********************************************************

File Name :
        DataCounts.C

Original Author:
        Patrick Small

Description:


Creation Date:
        12 March 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include "RetCodes.h"
#include "DataCounts.h"


DataCounts::DataCounts()
{
  start = TimeStamp(TRUE_EPOCH_TIME, 0.0);
  numsamp = 0;
}



DataCounts::DataCounts(const DataCounts &dc)
{
  start = dc.start;
  numsamp = dc.numsamp;
  data = dc.data;
}



DataCounts::~DataCounts()
{
  // Clear the contents of the count list
  data.clear();
}



DataCounts& DataCounts::operator=(const DataCounts &dc)
{
  start = dc.start;
  numsamp = dc.numsamp;
  data = dc.data;
  return(*this);
}



int operator<(const DataCounts &dc1, const DataCounts &dc2)
{
  if (dc1.start < dc2.start) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}

