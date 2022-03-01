/***********************************************************

File Name :
        Directory.C

Original Author:
        Patrick Small

Description:

        This source file defines the methods for the Directory
class. The Directory class allows an application to read the files
and sub-directories contained within a UNIX filesystem directory.

Creation Date:
        25 January 2000

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include "RetCodes.h"
#include "Directory.h"

using namespace std;

Directory::Directory()
{
  valid = TN_FALSE;
  dirfile = NULL;
}


Directory::Directory(const char *pathdir)
{
  DIR *dirp;

  valid = TN_FALSE;
  dirfile = NULL;
  
  if (strcmp(pathdir, "") == 0) {
    std::cout << "Error (Directory::Directory): No configuration file specified" 
	 << std::endl;
    return;
  }

  dirp = opendir(pathdir);
  if (dirp == NULL) {
    std::cout << "Error (Directory::Directory): Error opening directory " 
	 << pathdir << std::endl;
    return;
  }

  dirfile = new struct dircb;
  if (dirfile == NULL) {
    std::cout << "Error (Directory::Directory): Unable to allocate more memory" 
	 << std::endl;
    closedir(dirp);
    return;
  }
  dirfile->refcount = 1;
  dirfile->dirp = dirp;

  valid = TN_TRUE;
}



Directory::Directory(const Directory &d)
{
  valid = d.valid;
  dirfile = d.dirfile;
  if (dirfile != NULL) {
    this->dirfile->refcount++;
  }
}



Directory::~Directory()
{
  if (dirfile != NULL) {
    dirfile->refcount--;
    if (dirfile->refcount == 0) {
      closedir(dirfile->dirp);
      delete(dirfile);
    }
  }
  valid = TN_FALSE;
}



int Directory::next(char *line)
{
  dirent* dp;

  if (valid) {
    errno = 0;
    dp = readdir(dirfile->dirp);
    if (dp == NULL) {
      if (errno != 0) {
	perror("Error (Directory::next):");
	std::cout << "Error (Directory::next): Unable to read next entry" << std::endl;
	return(TN_FAILURE);
      } else {
	return(TN_EOF);
      }
    }
    strcpy(line, dp->d_name);
    return(TN_SUCCESS);
  } else {
    return(TN_FAILURE);
  }
}



Directory& Directory::operator=(const Directory &d)
{
  // Close any existing directory
  if (dirfile != NULL) {
    dirfile->refcount--;
    if (dirfile->refcount == 0) {
      closedir(dirfile->dirp);
      delete(dirfile);
    }
  }

  this->valid = d.valid;
  this->dirfile = d.dirfile;
  if (this->dirfile != NULL) {
    this->dirfile->refcount++;
  }
  return(*this);
}



int operator!(const Directory &df)
{
  return(!df.valid);
}
