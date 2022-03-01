/***********************************************************

File Name :
        MulticastAddress.C

Original Author:
        Patrick Small

Description:


Creation Date:
        28 July 1999

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <cstring>
#include "MulticastAddress.h"

using namespace std;


MulticastAddress::MulticastAddress()
{
  strcpy(inter, "");
  strcpy(addr, "");
  port = 0;
}


MulticastAddress::MulticastAddress(const MulticastAddress &m)
{
  strcpy(inter, m.inter);
  strcpy(addr, m.addr);
  port = m.port;
}


MulticastAddress::~MulticastAddress() 
{
}


MulticastAddress& MulticastAddress::operator=(const MulticastAddress &m)
{
  strcpy(inter, m.inter);
  strcpy(addr, m.addr);
  port = m.port;

  return(*this);
};
