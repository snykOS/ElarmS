/***********************************************************

File Name :
        String.h

Programmer:
	Patrick Small

Description:

        This unit implements an abstract string class. The 
	following operations are allowed:

	=     Assignment (char array, String)
	+=    Concatenation, assignment (char, char array, String)
	[]    Subscripts
	<<    Stream output
	>>    Stream input
	==    Equality
	!=    Inequality
	+     Concatenation


       Methods are also provided to cast strings to the following
       data-types:

       char*
       int
       double


       Finally, methods are provided to retrieve the string length,
       set it to upper or lower case, and calculate a hash value
       based on the string.


Creation Date:
        20 March 1998


Usage Notes:



**********************************************************/


#ifndef string_H
#define string_H

#include<iostream>
using std::ostream;
using std::istream;

class String
{
  struct srep {
    char* s;
    int n;
    srep() { n = 1;}
  };
  srep *p;


 public:
  String();
  String(const char*);
  String(const String&);
  String(int);
  ~String();

  int getLength();
  void toUpper();
  void toLower();
  long hashValue();

  String& operator=(const char*);
  String& operator=(const String&);
  String& operator+=(const String&);
  String& operator+=(const char*);
  String& operator+=(const char);

  char& operator[](int);
  const char& operator[](int) const;

  friend ostream& operator<<(ostream&, const String&);
  friend istream& operator>>(istream&, String&);

  friend int operator==(const String&, const char*);
  friend int operator==(const String&, const String&);
  friend int operator!=(const String&, const char*);
  friend int operator!=(const String&, const String&);

  friend String operator+(const String&, const String&);
  friend String operator+(const String&, const char*);

  operator char*();
  operator int();
  operator double();

};


#endif
