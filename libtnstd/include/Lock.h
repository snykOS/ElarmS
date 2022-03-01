/***********************************************************

File Name :
        Lock.h

Original Author:
        Patrick Small

Description:


Creation Date:
        09 March 2000


Modification History:


Usage Notes:


**********************************************************/

#ifndef lock_H
#define lock_H

// Various include files
#include "GenLimits.h"


class Lock
{
 private:
    int valid;
    char path[MAXSTR];

 public:
    Lock();
    Lock(const char *path);
    ~Lock();

    int Get();
    int Release();

    friend int operator!(const Lock &l);

};

#endif

