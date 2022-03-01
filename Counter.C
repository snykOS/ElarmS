/***********************************************************

File Name :
        Counter.C

Original Author:
        Patrick Small

Description:


Creation Date:
        26 March 1999


Modification History:


Usage Notes:


**********************************************************/

// Various include files
//#include <stream.h>
#include "Counter.h"



Counter Counter::operator+(const int &i) const
{
  int v;

  v = (val + i) % max;
  if (v < 0) {
    v += max;
  }
  return(Counter(v, max));
}



Counter Counter::operator-(const int &i) const
{
  int v;
  
  v = (val - i) % max;
  if (v < 0) {
    v += max;
  }
  return(Counter(v, max));
}


Counter& Counter::operator=(const Counter &c)
{
  max = c.max;
  val = c.val;
  return(*this);
}


Counter& Counter::operator=(const int &i)
{
  val = i % max;
  if (val < 0) {
    val += max;
  }
  return(*this);
}


Counter& Counter::operator+=(const int &i)
{
  val = (val + i) % max;
  if (val < 0) {
    val += max;
  }
  return(*this);
}


Counter& Counter::operator-=(const int &i)
{
  val = (val - i) % max;
  if (val < 0) {
    val += max;
  }
  return(*this);
}



Counter Counter::operator++(int i)
{
  Counter c = *this;

  val = (val + 1) % max;
  if (val < 0) {
    val += max;
  }
  return(c);
}


Counter Counter::operator--(int i)
{
  Counter c = *this;

  val = (val - 1) % max;
  if (val < 0) {
    val += max;
  }
  return(c);
}

