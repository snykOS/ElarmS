/***********************************************************

File Name :
        Lock.C

Original Author:
        Patrick Small

Description:


Creation Date:
        09 March 2000


Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <csignal>
#ifdef __SUNPRO_CC
#include <signal.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _SOLARIS
#include <sys/mkdev.h>  // for major() and minor()
#endif
#include "RetCodes.h"
#include "Lock.h"

using namespace std;

// Default lock directory
const char DEFAULT_LOCK_DIR[] = "/var/spool/locks";


Lock::Lock()
{
  valid = TN_FALSE;
  strcpy(path, "");
}



Lock::Lock(const char *p)
{
  valid = TN_TRUE;
  strcpy(path, p);
}


Lock::~Lock()
{
}



int Lock::Get()
{
  struct stat statbuf;
  char lockfile[MAXSTR];
  char tmpfile[MAXSTR];
  mode_t mode;
  int fd;
  char buf[MAXSTR];
  FILE *fp;
  pid_t	pid;

  if (valid != TN_TRUE) {
    return(TN_FALSE);
  }

  // Construct lock file name
  if (stat(path, &statbuf) < 0) {
    std::cout << "Error (Lock::Get): Unable to stat device "  << path << std::endl;
    return(TN_FAILURE);
  }
  sprintf(lockfile, "%s/LK.%3.3lu.%3.3lu.%3.3lu", DEFAULT_LOCK_DIR,
	  (unsigned long)major(statbuf.st_dev),
	  (unsigned long)major(statbuf.st_rdev),
	  (unsigned long)minor(statbuf.st_rdev));

  sprintf(tmpfile, "%s/LTMP.%lu", DEFAULT_LOCK_DIR, getpid());

  // Override the default umask because everyone should be
  // able to read this lockfile.
  mode = umask(0);
  fd = creat(tmpfile, 0444);
  umask(mode);

  if (fd < 0) {
    std::cout << "Error (Lock::Get): Unable to create lock file " << tmpfile 
	 << std::endl;
    return(TN_FAILURE);
  }

  sprintf(buf, "%10lu\n", getpid());
  write(fd, buf, strlen(buf));
  close(fd);

  errno = 0;
  if (link(tmpfile, lockfile) < 0) {
    if (errno != EEXIST) {
      std::cout << "Error (Lock::Get): Unable to link lock file" << std::endl;
      unlink(tmpfile);
      return(TN_FAILURE);
    }
    
    if ((fp = fopen(lockfile, "r")) == NULL) {
      std::cout << "Error (Lock::Get): Lock file already exists" << std::endl;
      unlink(tmpfile);
      return(TN_FAILURE);
    }

    buf[0] = '\0';
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    pid = atoi(buf);

    // Check if the lock owner exists
    errno = 0;
    if ((pid > 0) && ((kill(pid, 0) == 0) || (errno != ESRCH))) {
      std::cout << "Error (Lock::Get): Device " << path << " already in use" 
	   << std::endl;
      unlink(tmpfile);
      return(TN_FAILURE);
    }

    // Lock owner is dead
    // Delete the existing lock and create a new one
    errno = 0;
    if (unlink(lockfile) < 0) {   
      std::cout << "Error (Lock::Get): Cannot delete lock file" << std::endl;
      unlink(tmpfile);
      return(TN_FAILURE);
    }
    
    if (link(tmpfile, lockfile) < 0) {
      std::cout << "Error (Lock::Get): Unable to link lock file" << std::endl;
      unlink(tmpfile);
      return(TN_FAILURE);
    }
  }
  
  unlink(tmpfile);

  return(TN_SUCCESS);
}



int Lock::Release()
{
  struct stat statbuf;
  char lockfile[MAXSTR];

  if (valid != TN_TRUE) {
    return(TN_FALSE);
  }

  // Construct lock file name
  if (stat(path, &statbuf) < 0) {
    std::cout << "Error (Lock::Release): Unable to stat device " << path << std::endl;
    return(TN_FAILURE);
  }
  sprintf(lockfile, "%s/LK.%3.3lu.%3.3lu.%3.3lu", DEFAULT_LOCK_DIR,
	  (unsigned long)major(statbuf.st_dev),
	  (unsigned long)major(statbuf.st_rdev),
	  (unsigned long)minor(statbuf.st_rdev));

  // Delete the lock
  if (unlink(lockfile) < 0) {
    std::cout << "Error (Lock::Release): Unable to delete lock file " 
	 << lockfile << std::endl;
    return(TN_FAILURE);
  }

  return(TN_SUCCESS);
}



int operator!(const Lock &l)
{
  return(!l.valid);
}

