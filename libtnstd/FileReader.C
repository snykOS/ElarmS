/***********************************************************

File Name :
        FileReader.C

Original Author:
        Patrick Small

Description:


Creation Date:
        18 August 2000

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdio>
#ifdef __SUNPRO_CC
#include <stdio.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "RetCodes.h"
#include "FileReader.h"

using namespace std;

FileReader::FileReader()
{
  valid = TN_FALSE;
  file = NULL;
}


FileReader::FileReader(const char *filename)
{
  FILE *fd;
  struct stat statbuf;

  valid = TN_FALSE;
  file = NULL;
  
  if (strcmp(filename, "") == 0) {
    std::cout << "Error (FileReader): No config file specified" << std::endl;
    return;
  }

  // Check if the existing file is a directory
  if (lstat(filename, &statbuf) != 0) {
    std::cout << "Error (FileReader): Unable to stat config file " << filename 
	 << std::endl;
    return;
  }
  if ((statbuf.st_mode & S_IFDIR) != 0) {
    std::cout << "Error (FileReader): Target file is a directory" << std::endl;
    return;
  }

  fd = fopen(filename, "r");
  if (fd == NULL) {
    std::cout << "Error (FileReader): Error opening config file " << filename
	 << std::endl;
    return;
  }

  file = new struct frcb;
  if (file == NULL) {
    std::cout << "Error (FileReader): Unable to allocate more memory" << std::endl;
    fclose(fd);
    return;
  }
  file->refcount = 1;
  file->fd = fd;
  strcpy(file->curline, "");
  file->lineno = 0;

  valid = TN_TRUE;
}



FileReader::FileReader(const FileReader &c)
{
  this->valid = c.valid;
  this->file = c.file;
  if (this->file != NULL) {
    this->file->refcount++;
  }
}



FileReader::~FileReader()
{
  if (file != NULL) {
    file->refcount--;
    if (file->refcount == 0) {
      fclose(file->fd);
      delete(file);
    }
  }
  valid = TN_FALSE;
}



int FileReader::next(char *line)
{
  char *retval;
  char getline[FILE_READER_MAX_STRING_LEN];
  char tmpline[FILE_READER_MAX_STRING_LEN];
  int done = TN_FALSE;
  int len;

  strcpy(line, "");
  strcpy(file->curline, "");

  if (valid) {
    do {
      retval = fgets(getline, FILE_READER_MAX_STRING_LEN, file->fd);
      if (retval == NULL) {
	if (ferror(file->fd) != 0) {
	  perror("Error (next):");
	  clearerr(file->fd);
	  return(TN_FAILURE);
	} else if (feof(file->fd)) {
	  return(TN_EOF);
	}
      } else {
	strcpy(file->curline, getline);
	strcpy(tmpline, getline);
	file->lineno++;
	if ((strlen(tmpline) > 0) && (strcmp(tmpline, "\n") != 0)) {
	  done = TN_TRUE;
	}
      }
    } while (!done);

    strcpy(line, getline);
    return(TN_SUCCESS);
  } else {
    return(TN_FAILURE);
  }

}




int FileReader::getLineNum(int &ln)
{
  if (valid) {
    ln = file->lineno;
    return(TN_SUCCESS);
  } else {
    return(TN_FAILURE);
  }
}







FileReader& FileReader::operator=(const FileReader &c)
{
  // Close any existing configuration file
  if (file != NULL) {
    file->refcount--;
    if (file->refcount == 0) {
      fclose(file->fd);
      delete(file);
    }
  }

  this->valid = c.valid;
  this->file = c.file;
  if (this->file != NULL) {
    this->file->refcount++;
  }
  return(*this);
}



int operator!(const FileReader &cf)
{
  return(!cf.valid);
}
