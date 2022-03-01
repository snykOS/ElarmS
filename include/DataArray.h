/***********************************************************

File Name :
        DataArray.h

Original Author:
        Pete Lombard

Description:


Creation Date:
        15 October 2004


Modification History:


Usage Notes:


**********************************************************/

#ifndef data_array_H
#define data_array_H

// Various include files
#include <vector>
#include "TimeStamp.h"
#include "Duration.h"


// Definition of a Count list
typedef std::vector<int, std::allocator<int> > CountArray;

class DataArray
{
 private:

 public:
    TimeStamp start;
    CountArray data;

    DataArray();
    DataArray(const DataArray &dc);
    ~DataArray();

    DataArray& operator=(const DataArray &dc);
    friend int operator<(const DataArray &dc1, const DataArray &dc2);
};


#endif

