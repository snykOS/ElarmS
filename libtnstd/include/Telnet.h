/***********************************************************

File Name :
        Telnet.h

Original Author:
        Patrick Small

Description:


Creation Date:
        09 November 2000

Modification History:


Usage Notes:


**********************************************************/

#ifndef telnet_H
#define telnet_H

// Various include files
#include "Client.h"


// Socket reference structure
//
// This makes multiple Telnet objects possible, that all
// refer to a single socket.
//

struct socketcb {
    int refcount;
    int sd;
};


class Telnet {

 private:
    struct socketcb *sock;
    int valid;

    int _SafePrint(const char *buf);
    int _Send(const char *buf, int buflen);
    int _Receive(char *buf, int &buflen);

 public:
    Telnet();
    Telnet(const Client &haddr);
    ~Telnet();

    int Send(const char *buf, int buflen);
    int Receive(char *buf, int &buflen);

    Telnet& operator=(const Telnet &t);
    friend int operator!(const Telnet &t);
};

#endif
