/***********************************************************

File Name :
        MulticastSender.h

Original Author:
        Patrick Small

Description:


Creation Date:
        09 November 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef multicast_sender_H
#define multicast_sender_H


// Various include files
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MulticastAddress.h"

// Socket reference structure
//
// This makes multiple MulticastSender objects possible, that all
// refer to a single socket.
//

struct socketcb {
    int refcount;
    int sd;
};


class MulticastSender {

 private:
    struct socketcb *sock;
    struct sockaddr_in name;
    int valid;
    int destset;

 public:
    MulticastSender();
    MulticastSender(const MulticastAddress &m);
    MulticastSender(char *outinter);
    ~MulticastSender();
    int setDest(char *outaddr, unsigned outport);
    int send(char *buf, unsigned int size);
    
    MulticastSender& operator=(const MulticastSender &ms);
    friend int operator!(const MulticastSender &ms);


};

#endif
