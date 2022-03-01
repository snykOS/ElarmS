/***********************************************************

File Name :
        MulticastAddress.h

Original Author:
        Patrick Small

Description:


Creation Date:
        28 July 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef multicast_addr_H
#define multicast_addr_H


// Various include files
#include "GenLimits.h"

class MulticastAddress {

 private:

 protected:

 public:
    char inter[MAXSTR];
    char addr[MAXSTR];
    unsigned int port;

    MulticastAddress();
    MulticastAddress(const MulticastAddress &m);
    ~MulticastAddress();
    
    MulticastAddress& operator=(const MulticastAddress &m);

};

#endif
