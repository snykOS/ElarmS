/***********************************************************

File Name :
        Telnet.C

Original Author:
        Patrick Small

Description:


Creation Date:
        9 November 2000

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <iostream>
#include <cstring>
#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/telnet.h>
#include "RetCodes.h"
#include "Compat.h"
#include "Telnet.h"

using namespace std;

// The definition of a telnet command terminator
const char crlf[] = "\r\n\377\371";


Telnet::Telnet()
{
  valid = TN_FALSE;
}



Telnet::Telnet(const Client &haddr)
{
  struct hostent *hp;
  struct servent *sp;
  int s;
  struct sockaddr_in sin;
  char ackit[MAXSTR];
  char buf[MAXSTR];
  int buflen;

  valid = TN_FALSE;

//   std::cout << "Getting host info" << std::endl;
//   std::cout.flush();

  // Get the host information
  hp = gethostbyname (haddr.host);
  if (hp == NULL) {
    std::cout << "Error (Telnet::Telnet): Unable to get host information" << std::endl;
    return;
  }

//   std::cout << "Opening socket" << std::endl;
//   std::cout.flush();

  // Open socket
  s = socket (hp->h_addrtype, SOCK_STREAM, 0);
  if ( s < 0 ) {
    std::cout << "Error (Telnet::Telnet): Unable to open socket" << std::endl;
    return;
  }

//   std::cout << "Get the port number" << std::endl;
//   std::cout.flush();

  // Get the port number of the telnet service
  sp = getservbyname ("telnet", "tcp");
  if (sp == NULL) {
    std::cout << "Error (Telnet::Telnet): Unable to get telnet service information" 
	 << std::endl;
    return;
  }

//   std::cout << "Create sockaddr_in" << std::endl;
//   std::cout.flush();

  //  Create a "sockaddr_in" structure which describes the remote
  //  IP address we want to connect to (from gethostbyname()) and
  //  the remote TCP port number (from getservbyname()).
  sin.sin_family = hp->h_addrtype;
  memcpy (&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = sp->s_port;
  if (haddr.port > 0) {
    sin.sin_port = htons(haddr.port);
  }

//   std::cout << "Open connection" << std::endl;
//   std::cout.flush();

  // Open connection
  if (connect (s, (struct sockaddr *)&sin, sizeof (sin)) < 0 ) {
    std::cout << "Error (Telnet::Telnet): Unable to open connection" << std::endl;
    return;
  }

//   std::cout << "Allocating memory" << std::endl;
//   std::cout.flush();

  sock = new struct socketcb;
  if (sock == NULL) {
    std::cout << "Error (Telnet::Telnet): Unable to allocate more memory" << std::endl;
    close(s);
    return;
  }
  sock->refcount = 1;
  sock->sd = s;

//   std::cout << "Establishing telnet session" << std::endl;
//   std::cout.flush();

  // Establish the telnet session
  buflen = MAXSTR - 1;
  if (this->_Receive(buf, buflen) != TN_SUCCESS) {
    std::cout << "Error (Telnet::Telnet): Unable to receive data" << std::endl;
    close(sock->sd);
    delete(sock);
    return;
  }

//   std::cout << "Received: " << buflen << " chars" << std::endl;
//   std::cout.flush();

  // Set the acknowledgment flags
  ackit[0] = IAC;
  ackit[1] = DONT;
  ackit[2] = TELOPT_SGA;
  // don't suppress go-ahead
  ackit[3] = IAC;
  ackit[4] = WONT;
  ackit[5] = TELOPT_SGA;
  // tell it "don't echo"
  ackit[6] = '\0';

  if (this->_Send(ackit, strlen(ackit)) != TN_SUCCESS) {
    std::cout << "Error (Telnet::Telnet): Unable to send acknowledgment" << std::endl;
    close(sock->sd);
    delete(sock);
    return;
  }

//   std::cout << "all done" << std::endl;
//   std::cout.flush();

  valid = TN_TRUE;
}



Telnet::~Telnet() 
{
  if (valid) {
    sock->refcount--;
    if (sock->refcount == 0) {
      if (close(sock->sd) != 0) {
	std::cout << "Error (Telnet::~Telnet): Unable to close socket" << std::endl;
      }
      delete(sock);
    }
  }
}


int Telnet::_SafePrint(const char *buf)
{
  for (; *buf; buf++) {
    if ((*buf < 32) || (*buf > 125)) {
      std::cout << Compat::Form("<%03d>", (unsigned int)*buf);
    } else {
      std::cout << *buf;
    }
  }
  return(TN_SUCCESS);
}


int Telnet::_Send(const char *buf, int buflen)
{
  int retval;

  std::cout << "SENDING: ";
  this->_SafePrint(buf);
  std::cout << std::endl;
  retval = write(sock->sd, buf, buflen);
  if (retval != buflen) {
    std::cout << "Error (Telnet::_Send): Unable to write data to socket" 
	 << std::endl;
    return(TN_FAILURE);
  } else {
    return(TN_SUCCESS);
  }
}



int Telnet::_Receive(char *buf, int &buflen)
{
  int retval;

  memset(buf, 0, buflen);
  errno = 0;
  retval = read(sock->sd, buf, buflen);
  if (retval < 0) {
    std::cout << "Error (Telnet::_Receive): Unable to read data from socket" 
	 << std::endl;
    buf[0] = 0;
    buflen = 0;
    return(TN_FAILURE);
  }
  buflen = retval;
  buf[buflen] = 0;

  std::cout << "RECVING: ";
  this->_SafePrint(buf);
  std::cout << std::endl;

  return(TN_SUCCESS);
}



int Telnet::Send(const char *buf, int buflen)
{
  char sendbuf[MAXSTR];

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  // Make a copy of the send string and append telnet terminator
  strcpy(sendbuf, buf);
  strcat(sendbuf, crlf);

  return(this->_Send(sendbuf, strlen(sendbuf)));
}



int Telnet::Receive(char *buf, int &buflen)
{
  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  return(this->_Receive(buf, buflen));
}



int operator!(const Telnet &t)
{
  return(!t.valid);
}



Telnet& Telnet::operator=(const Telnet &t)
{
  this->sock = t.sock;
  this->valid = t.valid;
  if (this->valid) {
    this->sock->refcount++;
  }
  return(*this);
};
