/***********************************************************

File Name :
        Logfile.C

Original Author:
        Patrick Small

Description:

        This source file defines the Logfile class - an
object class which abstracts the use of disk files for status
logging. The primary purpose of the class is to allow user
programs to log activity in a consistent, timestamped format.


Creation Date:
        06 July 1998

Modification History:


Usage Notes:

        When a Logfile object is instantiated, a timestamped header 
is written to disk in the filename provided by the user. If the 
logfile already exists, the header is appended to the file.
        The logfile is closed when the Logfile object is destroyed.

        The user process can check to make sure that the logfile
was opened successfully by using the "!" overloaded boolean operator.

**********************************************************/


// Various include files
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include "RetCodes.h"
#include "Logfile.h"

using namespace std;

// Maximum length of logging string
const int MAXLOGSTR = 512;


void Logfile::updateCurrentTime()
{
  char curline[MAXLOGSTR];
  int retval;

  TimeStamp newtime = TimeStamp::current_time();
  if ((newtime.day_of_year() != logfile->curtime.day_of_year()) 
      || (newtime.year() != logfile->curtime.year())) {
    logfile->curtime = newtime;
    logfile->lfile <<string("\n-----------------------------------------\n");
 
    sprintf(curline, "%02d/%02d/%d Change of Calendar Date\n", 
	    logfile->curtime.day_of_month(), logfile->curtime.month_of_year(),
	    logfile->curtime.year());

    logfile->lfile << curline;
    logfile->lfile <<"\n-----------------------------------------\n";
    if (logfile->lfile.fail()) {
      std::cout << "Error (Logfile): Error writing to " << filename;
      return;
    }
  } else {
    logfile->curtime = newtime;
  }
  return;
}



int Logfile::updateCurrentTimeDaily()
{
  char curline[MAXLOGSTR];
  int retval;


  TimeStamp newtime = TimeStamp::current_time();
  if ((newtime.day_of_year() != logfile->curtime.day_of_year()) 
      || (newtime.year() != logfile->curtime.year())) {

    logfile->curtime = newtime;

    // Create new log filename
    filename[strlen(filename) - 13] = 0; // remove "_YYYYMMDD.log"
    sprintf(filename, "%s_%04d%02d%02d.log", filename, newtime.year(),
	    newtime.month_of_year(), newtime.day_of_month());

    //close old file
    logfile->lfile.close();
    // Open new logfile
    logfile->lfile.open(filename,ios::out | ios::app);
    if (logfile->lfile.fail()) {
      std::cout << "Error (Logfile): Error opening " << filename << std::endl; 
      // Open failed, decr refcount and invalidate object
      logfile->refcount--;
      if (logfile->refcount == 0) {
	logfile->lfile.close();
	delete(logfile);
	logfile = NULL;
      }
      valid = TN_FALSE;
      return(TN_FAILURE);
    }

    logfile->lfile <<endl<<"-----------------------------------------"<<endl;

    sprintf(curline, "%02d/%02d/%d Change of Calendar Date\n", 
	    logfile->curtime.day_of_month(), logfile->curtime.month_of_year(),
	    logfile->curtime.year());

    logfile->lfile << curline;
    logfile->lfile <<endl<<"-----------------------------------------"<<endl;
    
    if(logfile->lfile.fail()){
      std::cout << "Error (Logfile): Error writing to " << filename;
      return(TN_FAILURE);
    }
  } else {
    logfile->curtime = newtime;
  }
  return(TN_SUCCESS);
}



Logfile::Logfile()
{
  valid = TN_FALSE;
  logfile = NULL;
  strcpy(filename, "");
}



Logfile::Logfile(const char *file, int openlog)
{
  char curline[MAXLOGSTR];
  int retval = 0;

  valid = TN_FALSE;
  mode = LOG_OBSOLETE;
  logfile = NULL;
  strcpy(filename, "");

  if (openlog == TN_FALSE) {
    valid = TN_TRUE;
    return;
  }
  strcpy(filename, file);
  logfile = new struct logcb;
  if (logfile == NULL) {
    std::cout << "Error (Logfile): Unable to allocate more memory" << std::endl;
    return;
  }
  logfile->lfile.open(filename,ios::out | ios::app);
  if (logfile->lfile.fail()) {
      std::cout << "Error (Logfile): Error opening " << filename << std::endl; 
    delete(logfile);
    logfile = NULL;
    return;
  }
  logfile->refcount = 1;
  logfile->curtime = TimeStamp::current_time();

  logfile->lfile <<endl<<"-----------------------------------------"<<endl;
  
  sprintf(curline, "%02d/%02d/%d %02d:%02d:%02d Logfile initialized\n", 
	  logfile->curtime.day_of_month(), logfile->curtime.month_of_year(), 
	  logfile->curtime.year(), logfile->curtime.hour_of_day(), 
	  logfile->curtime.minute_of_hour(), 
	  logfile->curtime.second_of_minute());

  logfile->lfile << curline;
  logfile->lfile <<endl<<"-----------------------------------------"<<endl;

  if (retval == EOF) {
    std::cout << "Error (Logfile): Error writing to " << filename;
    logfile->lfile.close();
    delete(logfile);
    logfile = NULL;
    return;
  }

  logfile->curtime = TimeStamp::current_time();
  valid = TN_TRUE;
}




Logfile::Logfile(const char *name, logType m, int openlog)
{

  char curline[MAXLOGSTR];
  int retval = 0;

  valid = TN_FALSE;
  mode = m;
  logfile = NULL;
  strcpy(filename, "");

  if (openlog == TN_FALSE) {
    valid = TN_TRUE;
    return;
  }

  TimeStamp curtime = TimeStamp::current_time();
  switch (mode) {
  case LOG_OBSOLETE:
    strcpy(filename, name);
    break;
  case LOG_DAILY:
    sprintf(filename, "%s_%04d%02d%02d.log", name, curtime.year(),
	    curtime.month_of_year(), curtime.day_of_month());
    break;
  default:
    return;
    break;
  };

  logfile = new struct logcb;
  if (logfile == NULL) {
    std::cout << "Error (Logfile): Unable to allocate more memory" 
	      << std::endl;
    return;
  }
  logfile->lfile.open(filename,ios::out | ios::app);
  if (logfile->lfile.fail()) {
      std::cout << "Error (Logfile): Error opening " << filename << std::endl; 
      perror("Error");
    delete(logfile);
    logfile = NULL;
    return;
  }

  logfile->curtime = TimeStamp::current_time();
    logfile->lfile <<endl<<"-----------------------------------------"<<endl;

  sprintf(curline, "%02d/%02d/%d %02d:%02d:%02d Logfile initialized\n", 
	  logfile->curtime.day_of_month(), logfile->curtime.month_of_year(), 
	  logfile->curtime.year(), logfile->curtime.hour_of_day(), 
	  logfile->curtime.minute_of_hour(), 
	  logfile->curtime.second_of_minute());


  logfile->lfile << curline;
  logfile->lfile <<endl<<"-----------------------------------------"<<endl;

  if (logfile->lfile.fail()) {
    std::cout << "Error (Logfile): Error writing to " << filename;
    logfile->lfile.close();
    delete(logfile);
    logfile = NULL;
    return;
  }

  logfile->refcount = 1;
  logfile->curtime = TimeStamp::current_time();
  valid = TN_TRUE;
}




Logfile::Logfile(const Logfile &l)
{
  valid = l.valid;
  mode = l.mode;
  logfile = l.logfile;
  strcpy(filename, l.filename);
  if (logfile != NULL) {
    logfile->refcount++;
  }
}



Logfile::~Logfile()
{
  if (logfile != NULL) {
    logfile->refcount--;
    if (logfile->refcount == 0) {
      logfile->lfile.close();
      delete(logfile);
    }
  }
}



int Logfile::write(const char *str)
{
  char curline[MAXLOGSTR];
  char writeline[MAXLOGSTR];

  if (valid) {

    switch (mode) {
    case LOG_OBSOLETE:

      if (logfile != NULL) {
	this->updateCurrentTime();
	if (strlen(str) > MAXLOGSTR - 14) {
	  // Source of magic number 14:
	  // 12: length of timestamp
	  // 1: "|" character
	  // 1: NULL terminator
	  strncpy(curline, str, MAXLOGSTR - 14);
	  curline[MAXLOGSTR - 14] = '\0';
	} else {
	  strcpy(curline, str);
	}
	sprintf(writeline, "%02d:%02d:%02d:%03d|%s\n", 
		logfile->curtime.hour_of_day(), 
		logfile->curtime.minute_of_hour(), 
		logfile->curtime.second_of_minute(),logfile->curtime.milliseconds(), curline);
	logfile->lfile << writeline;

	  if(logfile->lfile.fail()) {
	  std::cout << "Error (write): Error writing to " << filename << std::endl;
	  return(TN_FAILURE);
	}
      }
      return(TN_SUCCESS);

      break;
    case LOG_DAILY:

      if (logfile != NULL) {
	if (this->updateCurrentTimeDaily() != TN_SUCCESS) {
	  std::cout << "Error (write): Unable to roll daily log" 
		    << std::endl;
	  return(TN_FAILURE);
	}
	if (strlen(str) > MAXLOGSTR - 14) {
	  // Source of magic number 14:
	  // 12: length of timestamp
	  // 1: "|" character
	  // 1: NULL terminator
	  strncpy(curline, str, MAXLOGSTR - 14);
	  curline[MAXLOGSTR - 14] = '\0';
	} else {
	  strcpy(curline, str);
}
	sprintf(writeline, "%02d:%02d:%02d:%03d|%s\n", 
		logfile->curtime.hour_of_day(), 
		logfile->curtime.minute_of_hour(), 
		logfile->curtime.second_of_minute(),logfile->curtime.milliseconds(),curline);
	logfile->lfile << writeline;
	if (logfile->lfile.fail()) {
	  std::cout << "Error (write): Error writing to " << filename << std::endl;
	  return(TN_FAILURE);
	}
      }
      return(TN_SUCCESS);

      break;
    default:
	std::cout << "Error (write): Invalid logging mode" << std::endl;
	return(TN_FAILURE);
      break;
    };


  } else {
    std::cout << "Error (write): No logfile open for writing" << std::endl;
    return(TN_FAILURE);
  }
}



void Logfile::flush()
{
  if (valid) {
    if (logfile != NULL) {
      logfile->lfile.flush();
    }
  } else {
    std::cout << "Error (flush): No logfile open for flushing" << std::endl;
  }
}






Logfile& Logfile::operator<<(const char *str)
{
  this->write(str);
  return(*this);
}



Logfile& Logfile::operator=(const Logfile &l)
{
  // Close any existing logfile
  if (logfile != NULL) {
    logfile->refcount--;
    if (logfile->refcount == 0) {
      logfile->lfile.close();
      delete(logfile);
    }
  }

  this->valid = l.valid;
  this->mode = l.mode;
  this->logfile = l.logfile;
  strcpy(this->filename, l.filename);
  if (this->logfile != NULL) {
    this->logfile->refcount++;
  }
  return(*this);
}



int operator!(const Logfile& lf)
{
  return (!(lf.valid));
}


