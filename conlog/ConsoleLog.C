/***********************************************************

File Name :
        ConsoleLog.C

Original Author:
        Patrick Small

Description:


Creation Date:
        13 June 2003


Modification History:
    03 CAF 2016-06-03 -- Added -L: option to override default name of symlink file.
    02 CAF 2015-07-09 -- Added command line options: -h and -l for creating of sym link.
    01 CAF 2013-10-23 -- use std::getline(string) instead of cin::getline(char*, int size)
                         which would fail quietly if size exceeded.


Usage Notes:


**********************************************************/

// Various include files
#include <unistd.h>     // getopt, unlink, symlink
#include <iostream>
#include <stdlib.h>
#include <libgen.h>     // basename
#include <string.h>
#include "RetCodes.h"
#include "GenLimits.h"
#include "Logfile.h"

#include "TimeStamp.h"
#include "Duration.h"

const string PROGRAM = "conlog";
const string VERSION = "2016-06-03";
#ifndef BUILDER
#define BUILDER "unknown"
#endif
const string DESCRIPTION = PROGRAM + " " + VERSION + " (Built " + __DATE__ + " " + __TIME__ + " by " + BUILDER + ")";


// Definition of the maximum length of an e-mail message line
const int MAX_LINE_LENGTH = 1024;

// format of linkname, initialized with default format.
char link_format[200] = "%s_current.log";

// Displays program usage information to the user
void usage(char *progname)
{
  std::cout << std::endl << "usage: " << progname << " [flags] <log file>" << std::endl << std::endl;
  std::cout << "where flags can be:" << std::endl;
  std::cout << "  -h            -- this help" << std::endl;
  std::cout << "  -l            -- enable creation and updates to symlink file " << link_format << std::endl;
  std::cout << "  -L <symlink>  -- enable creation and updates to specified symlink file" << std::endl;
  exit(1);
}

void updateLink(Logfile& lf, const char* link_filename)
{
    // erase old link assuming it exists
    unlink(link_filename);

    // create the new link
    symlink(lf.getFilename().c_str(), link_filename);

    // write message to new log
    
    char msg[MAX_LINE_LENGTH];
    snprintf(msg, sizeof(msg)-1, "Updated symlink %s -> %s\n", link_filename, lf.getFilename().c_str() );
    lf.write(msg);
} // updateLink

int listenConsole(const char* lfile, int autolink, const char* link_format)
{
  char linkname[200];
  std::string old_logname("");
  if (autolink) {
      memset(linkname, '\0', sizeof(linkname) );
      snprintf(linkname, sizeof(linkname)-1, link_format, lfile);
      sprintf(linkname, link_format, lfile);
      std::cout <<"Link file: "<< linkname << std::endl;
  }

  Logfile lf(lfile, LOG_DAILY, TN_TRUE);
  char msg[MAX_LINE_LENGTH];
  int len;

  if (!lf) {
    std::cout << "Error (listenConsole): Unable to open logfile" 
	      << lfile << std::endl;
    return(TN_FAILURE);
  }

  // create initial link to log file
  if (autolink) {
      updateLink(lf, linkname);
      old_logname = lf.getFilename();
  }

  while ( std::cin.good() ) {

      std::string line;

      // read a line text
      std::getline(std::cin, line);

      // copy string into working array without overflowing 
      (void)memset((char*)msg, '\0', sizeof(msg));
      strncpy(msg, line.c_str(), sizeof(msg)-1);

      // Remove any terminating newlines
      len = strlen(msg);
      while (msg[len - 1] == '\n') {
          len--;
          msg[len] = 0;
      }

      // Write to disk log
      lf.write(msg);
      lf.flush();

      // if autolink enabled, check if actual log file has changed
      // Note, this should be done AFTER file is updated to insure it exists.
      static TimeStamp last_check = TimeStamp::current_time();
      static Duration check_interval(60);
      TimeStamp now = TimeStamp::current_time();
      if (autolink && now - last_check > check_interval) {
          std::string current_filename = lf.getFilename();
          if (old_logname != current_filename) {
              updateLink(lf, linkname);
              old_logname = current_filename;
          }
          last_check = now;
      }
  }

  lf.write("\n\n*** conlog exiting! ***\n\n");
  lf.flush();

  return(TN_SUCCESS);
} // listenConsole()



int main(int argc, char **argv)
{
  std::cout << "Program: " << DESCRIPTION << std::endl;
  std::cout << "Arguments:";
  for (int idx = 1; idx < argc; idx++)
      std::cout << " " << argv[idx];
  std::cout << std::endl;

  // enable automatic link to allow tailing the same file past day rollover.
  int autolink = 0;


  short show_usage = 0;
  char ch;
  while ((ch = getopt(argc, argv, "hlL:")) != -1) {
    switch (ch) {
        case 'h':
            show_usage = 1;
            break;
        case 'L':
            memset(link_format, '\0', sizeof(link_format));
            strncpy(link_format, optarg, sizeof(link_format)-1 );
            autolink = 1;
            break;
        case 'l':
            autolink = 1;
            break;
        case '?': 
        default:
            std::cout << "Unexpected option: " << ch << std::endl;
            show_usage = 1;
            break;
    } // switch
  } // while

  // adjust args 

  // Check for correct number of required arguments
  if (argc - optind <= 0) {
      std::cout << "Required argument not found." << std::endl;
      show_usage = 1;
  }

  if (show_usage) {
    usage(basename(argv[0]));
    exit(1);
  }

  // basename for log file
  char logname[200];
  memset(logname, '\0', sizeof(logname));
  strncpy(logname, argv[optind], sizeof(logname)-1);
  std::cout <<"Log file: "<< logname << std::endl;

  TimeStamp curtime = TimeStamp::current_time();
  std::cout <<"Time: "<<curtime<<std::endl;

  if (listenConsole(logname, autolink, link_format) != TN_SUCCESS) {
    std::cout << "Error (main) Unable to receive console lines" << std::endl;
    return(TN_FAILURE);
  }

  std::cout << std::endl << std::endl;
  std::cout << "*************************" << std::endl;
  std::cout << "*** " << argv[0] << " exiting!" << " *** " << std::endl;
  std::cout << "*************************" << std::endl;

  return(0);
} // main()

