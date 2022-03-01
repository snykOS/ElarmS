/***********************************************************

File Name :
        TemporaryFile.C

Original Author:
        Patrick Small

Description:

       This header file defines the TemporaryFile class - an
object class which abstracts the use of temporary disk files. 
When objects of this class are created, a file with a unique
name is created in the "/tmp" directory. Bytes may be read to
this temporary file. When the object is destroyed, the
temporary file is deleted from disk.


Creation Date:
        13 November


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>  //tempnam
#include "RetCodes.h"
#include "TemporaryFile.h"

using namespace std;

TemporaryFile::TemporaryFile()
{
  valid = TN_FALSE;
  isopen = TN_FALSE;
  tfile = NULL;
  char *tmpname;

  tmpname = tempnam("/tmp", NULL);
  if (tmpname == NULL) {
    std::cout << "Error (TemporaryFile): Unable to get temporary name" << std::endl;
    return;
  }
  tfile = fopen(tmpname, "w");
  if (tfile == NULL) {
    std::cout << "Error (TemporaryFile): Error opening " << tmpname << std::endl;
    free(tmpname);
    return;
  }
  strcpy(filename, tmpname);
  free(tmpname);
  isopen = TN_TRUE;
  valid = TN_TRUE;  
}




TemporaryFile::~TemporaryFile()
{
  if (valid) {
    if (isopen) {
      fclose(tfile);
    }
    if (unlink(filename) != 0) {
      std::cout << "Error (~TemporaryFile): Unable to unlink temporary file " 
	   << filename << std::endl;
    }
  }
}



int TemporaryFile::getName(char *name)
{
  if (valid) {
    strcpy(name, filename);
    return(TN_SUCCESS);
  } else {
    *name = '\0';
    std::cout << "Error (getName): Invalid temp file object" << std::endl;
    return(TN_FAILURE);
  }
}



int TemporaryFile::write(const char *buf, unsigned int bufsize)
{
  int retval;

  if ((valid) && (isopen)) {
    retval = fwrite((void *)buf, bufsize, 1, tfile);
    if (retval != 1) {
    std::cout << "Error (write): Unable to write buffer to file" << std::endl;
    return(TN_FAILURE);
    }
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (write): No temp file open for writing" << std::endl;
    return(TN_FAILURE);
  }
}


int TemporaryFile::write(const char *buf)
{
  int retval;

  if ((valid) && (isopen)) {
    retval = fwrite((void *)buf, strlen(buf), 1, tfile);
    if (retval != 1) {
    std::cout << "Error (write): Unable to write buffer to file" << std::endl;
    return(TN_FAILURE);
    }
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (write): No temp file open for writing" << std::endl;
    return(TN_FAILURE);
  }
}



int TemporaryFile::close()
{
  if ((valid) && (isopen)) {
    fclose(tfile);
    isopen = TN_FALSE;
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (close): No temp file open for closing" << std::endl;
    return(TN_FAILURE);
  }
}



int TemporaryFile::flush()
{
  if ((valid) && (isopen)) {
    fflush(tfile);
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (flush): No temp file open for flushing" << std::endl;
    return(TN_FAILURE);
  }
}



int operator!(const TemporaryFile& tf)
{
  return (!(tf.valid));
}
