/***********************************************************

File Name :
        Compat.C

Original Author:
        Paul Friberg & Patrick Small

Description:


Creation Date:
        27 June 2002

Modification History:


Usage Notes:

	this provides the std:: form() that was deprecated
	for iostreams in the newest forte compiler.


**********************************************************/

// Various include files
#include <cstdarg>
#include <cstdio>
#include <iostream>
#ifdef __SUNPRO_CC
#include <stdio.h>   //vsnprintf
#include <stdarg.h>  //va_start...
#endif
#include "GenLimits.h"
#include "Compat.h"


Compat::Compat()
{
}


Compat::~Compat()
{
}



char* Compat::Form(const char *strf, ...)
{
  std::va_list ap;
  static char tmpstr[MAXSTR];
  int retcode;

  va_start(ap, strf);
  retcode = vsnprintf(tmpstr, MAXSTR, strf, ap);
  if (retcode == -1) {
	std::cerr << "Compat::Form() buffer not large enough, increase MAXSTR" << std::endl;
	tmpstr[0]=0;
  }
  va_end(ap);
	
  return(tmpstr);
}
