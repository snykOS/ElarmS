/***********************************************************

File Name :
        MulticastRecv.h

Original Author:
        Patrick Small

Description:


Creation Date:
        09 November 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef multicast_recv_H
#define multicast_recv_H


// Various include files
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


// Socket reference structure
//
// This makes multiple MulticastRecv objects possible, that all
// refer to a single socket.
//
struct socketcb {
    int refcount;
    int sd;
};


class MulticastRecv {

 private:
    struct socketcb *sock;
    int valid;

 public:
    MulticastRecv();
    MulticastRecv(char *outinter, char *inaddr, unsigned inport);
    ~MulticastRecv();
    int receive(char *buf, unsigned int &bufsize);
    
    MulticastRecv& operator=(const MulticastRecv &ms);
    friend int operator!(const MulticastRecv &ms);
};

#endif
