/***********************************************************

File Name :
        Client.h

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef client_H
#define client_H

// Various include files
#include <iostream>
using std::ostream;
#include "GenLimits.h"


class Client
{
 private:

 public:
    char host[MAXSTR];
    unsigned int port;

    Client();
    Client(const char *hostname, unsigned int portnum);
    ~Client();

    Client& operator=(const Client &c);
    friend int operator==(const Client &c1, const Client &c2);
    friend int operator<(const Client &c1, const Client &c2);
    friend ostream& operator<<(ostream &os, const Client &c);
};


#endif

