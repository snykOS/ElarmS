/***********************************************************

File Name :
        DataCounts.h

Original Author:
        Patrick Small

Description:


Creation Date:
        12 March 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef data_counts_H
#define data_counts_H

// Various include files
#include <list>
#include "TimeStamp.h"
#include "Duration.h"


// Definition of a Count list
typedef std::list<int, std::allocator<int> > CountList;

class DataCounts
{
 private:

 public:
    TimeStamp start;
    int numsamp;
    CountList data;

    DataCounts();
    DataCounts(const DataCounts &dc);
    ~DataCounts();

    DataCounts& operator=(const DataCounts &dc);
    friend int operator<(const DataCounts &dc1, const DataCounts &dc2);
};


#endif

