/***********************************************************

File Name :
        StatusManager.C

Original Author:
        Patrick Small

Description:


Creation Date:
        06 May 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <cstring>
#include <cstdlib>
#include <cstdio>
#ifdef __SUNPRO_CC
#include <stdio.h>
#endif
#include "RetCodes.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "StatusManager.h"


StatusManager::StatusManager()
{
  valid = TN_FALSE;
  replevel = 0;
  conflag = TN_FALSE;
}



StatusManager::StatusManager(Logfile &l, int replev)
{
  lf = l;
  replevel = replev;
  valid = TN_TRUE;
  conflag = TN_FALSE;
}



StatusManager::~StatusManager()
{
}


int StatusManager::LogConsole(int flag)
{
  conflag = flag;
  return(TN_SUCCESS);
}


int StatusManager::DebugMessage(const char *msg)
{
  char logstr[MAXSTR];

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  if (replevel == LOG_ALL) {
      snprintf(logstr, MAXSTR, "DEBUG: %s", msg);
    
    if (conflag) {
      // Send the message to the console
      std::cout << logstr << std::endl;
    }

    // Send the message to the logfile
    lf << logstr;
    lf.flush();
  }

  return(TN_SUCCESS);
}



int StatusManager::InfoMessage(const char *msg)
{
  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  char* logstr = (char*)malloc(strlen(msg)+128);
  if(logstr == NULL)
      return(TN_FAILURE);

  if (replevel >= LOG_MAJOR) {
      snprintf(logstr, MAXSTR, "INFO: %s", msg);

    if (conflag) {
      // Send the message to the console
      std::cout << logstr << std::endl;
    }

    // Send the message to the logfile
    lf << logstr;
    lf.flush();
  }

  free(logstr);
  return(TN_SUCCESS);
}



int StatusManager::ErrorMessage(const char *msg)
{
  char logstr[MAXSTR];

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  if (replevel >= LOG_DEFAULT) {
    snprintf(logstr, MAXSTR, "ERROR: %s", msg);

    if (conflag) {
      // Send the message to the console
      std::cout << logstr << std::endl;
    }

    // Send the message to the logfile
    lf << logstr;
    lf.flush();
  }

  return(TN_SUCCESS);
}


StatusManager& StatusManager::operator<<(const char *str)
{
  this->InfoMessage(str);
  return(*this);
}




StatusManager& StatusManager::operator=(const StatusManager& sm)
{
  valid = sm.valid;
  lf = sm.lf;
  replevel = sm.replevel;
  conflag = sm.conflag;
  return(*this);
}


int StatusManager::operator!()
{
  return(!valid);
}
