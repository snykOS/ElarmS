/***********************************************************

File Name :
        MulticastRecv.C

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
#include "MulticastRecv.h"

using namespace std;

MulticastRecv::MulticastRecv()
{
  valid = TN_FALSE;
}


MulticastRecv::MulticastRecv(char *outinter,
			     char *inaddr, unsigned inport)
{
  struct ip_mreq mreq;
  int optionval = 1;
  struct sockaddr_in name;
  struct in_addr out_addr;

  valid = TN_FALSE;

  sock = new struct socketcb;
  if (sock == NULL) {
    std::cout << "Error (MulticastRecv): Unable to allocate more memory" 
	 << std::endl;
    return;
  }

  // Open the socket
  sock->sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock->sd == -1) {
    std::cout << "Error (MulticastRecv): Unable to open new socket" << std::endl;
    delete(sock);
    return;
  }

  // Specify the Local system's interface to receive on 
  out_addr.s_addr = inet_addr(outinter);   
  if (setsockopt(sock->sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&out_addr,
		 sizeof(out_addr)) == -1) {
    std::cout << "Error (MulticastRecv): Unable to set rcv interface" << std::endl;
    perror("Reason");
    close(sock->sd);
    delete(sock);
    return;
  }

  // Allow reuse of sockets
  if (setsockopt(sock->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optionval, 
		 sizeof(int)) == -1) {
    std::cout << "Error (MulticastRecv): Unable to set socket reuse option" 
	 << std::endl;
    perror("Reason");
    close(sock->sd);
    delete(sock);
    return;
  }

  // Fill in the server's socket address structure
  memset((char *) &name, '\0', sizeof(name));
  name.sin_family = AF_INET;
  name.sin_port = htons((unsigned short)inport);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  // name.sin_addr.s_addr = inet_addr(outinter);
  // name.sin_addr.s_addr = inet_addr(inaddr);

  // Bind socket to the port number
  if (bind(sock->sd, (struct sockaddr *)&name, sizeof(name)) != 0) {
    std::cout << "Error (MulticastRecv): Unable to bind socket" << std::endl;
    perror("Reason");
    close(sock->sd);
    delete(sock);
    return;
  }
  // 131.215.66.186 192.1.2.2

  // Subscribe to the multicast group
  mreq.imr_multiaddr.s_addr = inet_addr(inaddr);
  // mreq.imr_interface.s_addr = inet_addr(outinter);
  mreq.imr_interface.s_addr = name.sin_addr.s_addr;
  if (setsockopt(sock->sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		 (char *)&mreq, sizeof(mreq)) == -1) {
    std::cout << 
      "Error (MulticastRecv): Unable to subscribe to multicast group" 
	 << std::endl;
    perror("Reason");
    close(sock->sd);
    delete(sock);
    return;
  }
  
  sock->refcount = 1;
  valid = TN_TRUE;
}


MulticastRecv::~MulticastRecv() 
{
  if (valid) {
    sock->refcount--;
    if (sock->refcount == 0) {
      if (close(sock->sd) != 0) {
	std::cout << "Error (~MulticastRecv): Unable to close socket" << std::endl;
      }
      delete(sock);
    }
  }
}


int MulticastRecv::receive(char *buf, unsigned int &bufsize)
{
  static struct sockaddr_in sender;
  int len;
  socklen_t senderlen;
  int dummy = 0;
  
  if (valid) {
    senderlen = sizeof(sender);
    len = recvfrom(sock->sd, buf, bufsize, 0, (struct sockaddr *)&sender, 
		   &senderlen);
    if (len == -1) {
      if (errno == EINTR) {
	return(TN_SIGNAL);
      } else {
	std::cout << "Error (receive): Unable to receive multicast message" << std::endl;
	perror("Reason");
	return(TN_FAILURE);
      }
    }
    bufsize = (unsigned int)len;
    return(TN_SUCCESS);
  } else {
    return(TN_FAILURE);
  }
}


int operator!(const MulticastRecv &ms)
{
  return(!ms.valid);
}


MulticastRecv& MulticastRecv::operator=(const MulticastRecv &ms)
{
  this->sock = ms.sock;
  this->valid = ms.valid;
  if (this->valid) {
    this->sock->refcount++;
  }
  return(*this);
};
