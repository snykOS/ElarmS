/***********************************************************

File Name :
        ConnectionStructs.C

Original Author:
        Patrick Small

Description:

        This source file defines class methods needed
by the Connection class.


Creation Date:
        06 July 1998

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include "ConnectionStructs.h"


callbackValue::callbackValue()
{
  func = NULL;
  arg = NULL;
};

    
callbackValue::callbackValue(const callbackValue &cb)
{
  func = cb.func;
  arg = cb.arg;
};


callbackValue::~callbackValue()
{
};

    
