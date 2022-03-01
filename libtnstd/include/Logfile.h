/***********************************************************

File Name :
        Logfile.h

Original Author:
        Patrick Small

Description:

        This header file defines the Logfile class - an
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

#ifndef logfile_H
#define logfile_H

// Various include files
#include <cstdio>
#include <iostream>
#include <fstream>
#include "TimeStamp.h"

using namespace std;

// Enumerated type describing valid logging modes
enum logType {LOG_OBSOLETE, LOG_DAILY};


// Constants for the defined logging levels. Note that these
// levels are not enforced by the Logfile class. These constants
// are provided for consistency between user programs only.
//
// LOG_DEFAULT: minimal error logging
// LOG_MAJOR  : LOG_DEFAULT + logging of major actions
// LOG_ALL    : LOG_MAJOR + log all actions and all results
//
const int LOG_DEFAULT = 0;
const int LOG_MAJOR = 1;
const int LOG_ALL = 2;


// Logfile reference structure
//
// This makes multiple Logfile objects possible, that all
// refer to a single logfile.
//
struct logcb {
    int refcount;
  /// FILE *lfile;
    ofstream lfile;
    TimeStamp curtime;
};



class Logfile
{
 private:

  int valid;
  logType mode;
  //  int objnum; //TEST
  struct logcb *logfile;
  char filename[FILENAME_MAX];

  
  // Private method which checks that if a day boundary has been crossed.
  // If a day change has occurred, it makes an entry in the logfile
  // and updates the internal time within the class.
  //
  void updateCurrentTime();
  int updateCurrentTimeDaily();

 public:

  // Default Constructor
  //
  // Does not create a valid Logfile object
  Logfile();

  // Constructor
  //
  // If the second argument is TN_TRUE, the constructor opens the file 
  // specified in the first argument and prepares it for logging. If it 
  // is TN_FALSE, the object is created without opening a logfile.
  //
  // OBSOLETE
  //
  Logfile(const char *, int);


  // Constructor
  // 
  // The second argument specifies the logging mode: Single File, or Daily.
  //
  // If the third argument is TN_TRUE, the constructor opens the file 
  // specified in the first argument and prepares it for logging. If it 
  // is TN_FALSE, the object is created without opening a logfile.
  //
  Logfile(const char *, logType, int);


  // Copy Constructor
  //
  Logfile(const Logfile&);


  // Destructor
  //
  // Closes the logfile
  //
  ~Logfile();


  // Writes a character string to the logfile. A timestamp is prepended to
  // the string, and a new-line character is appended.
  //
  // This method returns TN_SUCCESS upon success, TN_FAILURE otherwise.
  //
  int write(const char *);



  // Overloaded stream-out operator
  //
  Logfile& operator<<(const char *);



  // Flushes the write buffer to disk
  //
  void flush();


  // Overloaded assignment operator
  //
  Logfile& operator=(const Logfile&);


  // Boolean operator which returns TN_TRUE when the Logfile object
  // is not valid, and TN_FALSE when the object references a valid and
  // open logfile.
  //
  friend int operator!(const Logfile&);

  // new function to return current filename
  std::string getFilename() const
  {
        return std::string(filename);
  }

};


#endif
