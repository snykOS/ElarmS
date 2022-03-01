/***********************************************************

File Name :
        TimeWindow.C

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <cstring>
#include "RetCodes.h"
#include "Client.h"

using namespace std;

Client::Client()
{
    memset(host, 0, MAXSTR);
    strcpy(host, "");
    port = 0;
}


Client::Client(const char *hostname, unsigned int portnum)
{
    memset(host, 0, MAXSTR);
    strcpy(host, hostname);
    port = portnum;
}


Client::~Client()
{
}


Client& Client::operator=(const Client &c)
{
    memset(host, 0, MAXSTR);
    strcpy(host, c.host);
    port = c.port;
    return(*this);
}



int operator==(const Client &c1, const Client &c2)
{
  if ((strcmp(c1.host, c2.host) == 0) && 
      (c1.port == c2.port)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}



int operator<(const Client &c1, const Client &c2)
{
  if (strcmp(c1.host, c2.host) < 0) {
    return(TN_TRUE);
  } else if ((strcmp(c1.host, c2.host) == 0) && (c1.port < c2.port)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}



ostream& operator<<(ostream &os, const Client &c)
{
  os << c.host << ":" << c.port;
  return(os);
}
