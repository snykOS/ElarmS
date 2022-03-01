#ifndef __LOCKFILE_H
#define __LOCKFILE_H

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

class LockFile{

 private:
    int lockfd;
    bool valid;
    bool locked;

 public:

    LockFile(char*);

    bool isValid();
    bool isLocked();
    int lock();
    void unlock();
};




#endif//__LOCKFILE_H
