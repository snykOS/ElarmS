/***********************************************************

File Name :
        StatusManager.h

Original Author:
        Patrick Small

Description:


Creation Date:
        06 May 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef status_manager_H
#define status_manager_H

// Various include files
#include "GenLimits.h"
#include "Logfile.h"


class StatusManager
{

 private:

 protected:
    int valid;
    Logfile lf;
    int replevel;
    int conflag;

 public:
    StatusManager();
    StatusManager(Logfile &l, int replev);
    ~StatusManager();

    int LogConsole(int flag);

    int DebugMessage(const char *msg);
    int InfoMessage(const char *msg);
    int ErrorMessage(const char *msg);

    StatusManager& operator<<(const char *);

    StatusManager& operator=(const StatusManager &sm);
    int operator!();
};




#endif



