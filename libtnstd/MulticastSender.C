/***********************************************************

File Name :
        MulticastSender.C

Original Author:
        Patrick Small

Description:


Creation Date:
        9 November 1998

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include "RetCodes.h"
#include "MulticastSender.h"

using namespace std;

MulticastSender::MulticastSender()
{
  valid = TN_FALSE;
}


MulticastSender::MulticastSender(const MulticastAddress &m)
{
  struct in_addr out_addr;

  valid = TN_FALSE;
  destset = TN_FALSE;

  sock = new struct socketcb;
  if (sock == NULL) {
    std::cout << "Error (MulticastSender): Unable to allocate more memory" 
	 << std::endl;
    return;
  }

  // Open the socket
  sock->sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock->sd == -1) {
    std::cout << "Error (MulticastSender): Unable to open new socket" << std::endl;
    delete(sock);
    return;
  }

  // Specify the Local system's interface to transmit on 
  out_addr.s_addr = inet_addr(m.inter);   
  if (setsockopt(sock->sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&out_addr,
		 sizeof(out_addr)) == -1) {
    std::cout << "Error (MulticastSender): Unable to set transmit interface" 
	 << std::endl;
    perror("Reason");
    close(sock->sd);
    delete(sock);
    return;
  }

  // Specify the destination address and port number
  memset(&name, 0, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_port = htons((unsigned short)m.port);
  name.sin_addr.s_addr = inet_addr(m.addr);
  destset = TN_TRUE;

  sock->refcount = 1;
  valid = TN_TRUE;
}




MulticastSender::MulticastSender(char *outinter)
{
  struct in_addr out_addr;

  valid = TN_FALSE;
  destset = TN_FALSE;

  sock = new struct socketcb;
  if (sock == NULL) {
    std::cout << "Error (MulticastSender): Unable to allocate more memory" 
	 << std::endl;
    return;
  }

  // Open the socket
  sock->sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock->sd == -1) {
    std::cout << "Error (MulticastSender): Unable to open new socket" << std::endl;
    delete(sock);
    return;
  }

  // Specify the Local system's interface to transmit on 
  out_addr.s_addr = inet_addr(outinter);   
  if (setsockopt(sock->sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&out_addr,
		 sizeof(out_addr)) == -1) {
    std::cout << "Error (MulticastSender): Unable to set transmit interface" 
	 << std::endl;
    perror("Reason");
    close(sock->sd);
    delete(sock);
    return;
  }

  sock->refcount = 1;
  valid = TN_TRUE;
}


MulticastSender::~MulticastSender() 
{
  if (valid) {
    sock->refcount--;
    if (sock->refcount == 0) {
      if (close(sock->sd) != 0) {
	std::cout << "Error (~MulticastSender): Unable to close socket" 
	     << std::endl;
      }
      delete(sock);
    }
  }
}


int MulticastSender::setDest(char *outaddr, unsigned outport)
{
  if (valid) {
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons((unsigned short)outport);
    name.sin_addr.s_addr = inet_addr(outaddr);
    destset = TN_TRUE;
    return(TN_SUCCESS);
  } else {
    return(TN_FAILURE);
  }
}


int MulticastSender::send(char *buf, unsigned int bufsize)
{
  if (valid) {
    if (destset == TN_FALSE) {
      std::cout << "Error (send): Destination address and port not set" << std::endl;
      return(TN_FAILURE);
    }
    if (sendto(sock->sd, buf, bufsize, 0, (struct sockaddr *)&name, 
	       sizeof(name)) == -1) {
      std::cout << "Error (send): Unable to send user packet" << std::endl;
      perror("Reason");
      return(TN_FAILURE);
    }
    return(TN_SUCCESS);
  } else {
    return(TN_FAILURE);
  }
}


int operator!(const MulticastSender &ms)
{
  return(!ms.valid);
}


MulticastSender& MulticastSender::operator=(const MulticastSender &ms)
{
  this->sock = ms.sock;
  memcpy((char *)&name, (char *)&(ms.name), sizeof(struct sockaddr_in));
  this->valid = ms.valid;
  this->destset = ms.destset;
  if (this->valid) {
    this->sock->refcount++;
  }
  return(*this);
};
