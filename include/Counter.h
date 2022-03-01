/***********************************************************

File Name :
        Counter.h

Original Author:
        Patrick Small

Description:


Creation Date:
        March 26 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef counter_H
#define counter_H

// Various include files
#include <values.h>


class Counter
{
 private:
    int val;
    int max;

 protected:

 public:
    inline Counter();
    inline Counter(int v, int mx = MAXINT);

    inline operator int() const;

    Counter operator+(const int &i) const;
    Counter operator-(const int &i) const;
    Counter& operator=(const Counter &c);
    Counter& operator=(const int &i);
    Counter& operator+=(const int &i);
    Counter& operator-=(const int &i);
    Counter operator++(int);
    Counter operator--(int);
};


// In-line methods


Counter::Counter()
{
  max = MAXINT;
  val = 0;
}



Counter::Counter(int v, int mx)
{
  if (mx < 0) {
    max = MAXINT;
  } else {
    max = mx;
  }
  val = v % max;
  if (val < 0) {
    val += max;
  }
}


Counter::operator int() const
{
  return (val);
}


#endif

