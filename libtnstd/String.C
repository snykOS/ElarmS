/***********************************************************

File Name :
        String.C

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

Modification History:


Usage Notes:



**********************************************************/


// Various include files
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <values.h>
#include <cctype>
#include <stdio.h> // snprintf
#include "GenLimits.h"
#include "String.h"

using namespace std;

String::String()
{
  p = new srep;
  p->s = NULL;
}



String::String(const char* s)
{
  p = new srep;
  p->s = new char[strlen(s)+1];
  strcpy(p->s, s);
}



String::String(const String& x)
{
  x.p -> n++;
  p = x.p;
}


String::String(int i)
{
  char buf[MAXSTR];

  snprintf(buf, MAXSTR, "%d", i);
  p = new srep;
  p->s = new char[strlen(buf)+1];
  strcpy(p->s, buf);
}


String::~String()
{
  if (--p->n == 0) {
    delete[] p->s;
    delete p;
  }
}


int String::getLength()
{
  if (p->s != NULL)
    return(strlen(p->s));
  else
    return(0);
}



void String::toUpper()
{
  char* tmp;
  int i;

  if (p->s != NULL) {
    tmp = p->s;
    if (p->n > 1) {
      p->n--;
      p = new srep;
      p->s = new char[strlen(tmp) + 1];
    }
    
    for(i = 0; *tmp != 0; tmp++) {
      *((p->s) + i) = toupper(*tmp);
      i++;
    }
  }
}


void String::toLower()
{
  char* tmp;
  int i;

  if (p->s != NULL) {
    tmp = p->s;
    if (p->n > 1) {
      p->n--;
      p = new srep;
      p->s = new char[strlen(tmp) + 1];
    }
    
    for(i = 0; *tmp != 0; tmp++) {
      *((p->s) + i) = tolower(*tmp);
      i++;
    }
  }
}


long String::hashValue()
{
  int i = 3;
  int pos;
  int len;
  long hashval = 0;

  if (p->s == NULL)
    return(0);
  else {
    len = strlen(p->s);
    for (pos = 0; pos < len; pos++) {
      hashval += ((*(p->s + pos) ^ 0xFF) + (*(p->s) ^ 0xAA)) * pos * i;
      i += 2;
    }
    return(hashval % MAXLONG);
  }
}


String& String::operator=(const char* s)
{
  if (p->n > 1) {
    p->n--;
    p = new srep;
  } else
    delete[] p->s;

  p->s = new char[strlen(s) + 1];
  if (*s == '\0')
    *(p->s) = 0;
  else
    strcpy(p->s, s);
  return(*this);
}



String& String::operator=(const String& x)
{
  if (--p->n == 0) {
    delete[] p->s;
    delete p;
  }
  x.p->n++;
  p = x.p;
  return(*this);
}


String& String::operator+=(const String&x)
{
  char *tmp;

  if (p->s != NULL) {
    tmp = new char[strlen(p->s) + strlen(x.p->s) + 1];
    strcpy(tmp, p->s);
    strcat(tmp, x.p->s);
    if (p->n > 1) {
      p->n--;
      p = new srep;
    } else
      delete[] p->s; 
    p->s = tmp;
  } else {
    p = new srep;
    p->s = new char[strlen(x.p->s) + 1];
    strcpy(p->s, x.p->s);
  }
  return(*this);
}


String& String::operator+=(const char* x)
{
  char *tmp;
 
  if (p->s != NULL) {
    tmp = new char[strlen(p->s) + strlen(x) + 1];
    strcpy(tmp, p->s);
    strcat(tmp, x);
    if (p->n > 1) {
      p->n--;
      p = new srep;
    } else
      delete[] p->s;
    p->s = tmp;
  } else {
    p = new srep;
    p->s = new char[strlen(x) + 1];
    strcpy(p->s, x);
  }
  return(*this);
}


String& String::operator+=(const char x)
{
  char *tmp;
  int len;

  if (p->s != NULL) {
    len = strlen(p->s);
    tmp = new char[len + 1];
    strcpy(tmp, p->s);
    tmp[len++] = x;
    tmp[len] = 0;
    if (p->n > 1) {
      p->n--;
      p = new srep;
    } else
      delete[] p->s;
    p->s = tmp;
  } else {
    p = new srep;
    p->s = new char[2];
    p->s[0] = x;
    p->s[1] = 0;
  }
  return(*this);
}


char& String::operator[](int i)
{
  if (i < 0 || strlen(p->s) < i)
    std::cout << "Error (String): index out of range" << std::endl;
  if (p->n > 1) {
    srep* np = new srep;
    np->s = new char[strlen(p->s)+1];
    strcpy(np->s, p->s);
    p->n--;
    p = np;
  }
  return(p->s[i]);
}


const char& String::operator[](int i) const
{
  if (i < 0 || strlen(p->s) < i)
    std::cout << "Error (String): index out of range" << std::endl;
  return(p->s[i]);
}


ostream& operator<<(ostream& s, const String& x)
{
  return s << x.p->s;
}


istream& operator>>(istream& s, String& x)
{
  char buf[MAXSTR];

  s >> buf;
  x = buf;
  return(s);
}


int operator==(const String& x, const char* s)
{
  if ((x.p->s != NULL) && (s != NULL)) {
    return(strcmp(x.p->s, s) == 0);
  } else {
    std::cout << "Error (==): String and/or char* are NULL" << std::endl; 
    return(1);
  }
}



int operator==(const String& x, const String& y)
{
  if ((x.p->s != NULL) && (y.p->s != NULL)) {
    return(strcmp(x.p->s, y.p->s) == 0);
  } else {
    std::cout << "Error (==): One or both strings are NULL" << std::endl; 
    return(1);
  }
}



int operator!=(const String& x, const char* s)
{
  if ((x.p->s != NULL) && (s != NULL)) {
    return(strcmp(x.p->s, s) == 0);
  } else {
    std::cout << "Error (!=): String and/or char* are NULL" << std::endl; 
    return(1);
  }
}



int operator!=(const String& x, const String& y)
{
  if ((x.p->s != NULL) && (y.p->s != NULL)) {
    return(strcmp(x.p->s, y.p->s) == 0);
  } else {
    std::cout << "Error (!=): One or both strings are NULL" << std::endl; 
    return(1);
  }
}



String operator+(const String& x, const String& y)
{
  String z;

  if ((x.p->s != NULL) && (y.p->s != NULL)) {
    z.p->s = new char[strlen(x.p->s) + strlen(y.p->s) + 1];
    strcpy(z.p->s, x.p->s);
    strcat(z.p->s, y.p->s);
  } else {
    std::cout << "Error (+): One or both strings are NULL" << std::endl; 
  }
  return(z);
}


String operator+(const String& x, const char* y)
{
  String z;

  if ((x.p->s != NULL) && (y != NULL)) {
    z.p->s = new char[strlen(x.p->s) + strlen(y) + 1];
    strcpy(z.p->s, x.p->s);
    strcat(z.p->s, y);
  } else {
    std::cout << "Error (+): String and/or char* are NULL" << std::endl; 
  }
  return(z);
}


String::operator char*()
{
  return(p->s);
}


String::operator int()
{
  if (p->s == NULL) {
    return(0);
  } else {
    return(atoi(p->s));
  }
}


String::operator double()
{
  if (p->s == NULL) {
    return(0.0);
  } else {
    return(atof(p->s));
  }
}
