static char sccsid[] = "@(#) UCB CVS: $Id: DataArray.C,v 1.1 2005/08/05 21:02:53 redi Exp $";

/***********************************************************

File Name :
        DataArray.C

Original Author:
        Pete Lombard

Description:


Creation Date:
        15 October 2004


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include "RetCodes.h"
#include "DataArray.h"


DataArray::DataArray()
{
  start = TimeStamp(TRUE_EPOCH_TIME, 0.0);
}



DataArray::DataArray(const DataArray &dc)
{
  start = dc.start;
  data = dc.data;
}



DataArray::~DataArray()
{
  // Clear the contents of the count array
  data.clear();
}



DataArray& DataArray::operator=(const DataArray &dc)
{
  start = dc.start;
  data = dc.data;
  return(*this);
}



int operator<(const DataArray &dc1, const DataArray &dc2)
{
  if (dc1.start < dc2.start) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}

