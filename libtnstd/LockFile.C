#include <cstdio>
#include "LockFile.h"

struct flock* file_lock(short type,short whence);


LockFile::LockFile(char* filename){
    lockfd = open(filename,O_RDWR | O_CREAT,0664);
    locked = false;
    if(lockfd < 0){
	valid = false;
	return;
    }
    
    valid = true;
}

bool LockFile::isValid(){
    return valid;
}


int LockFile::lock(){
    if(fcntl(lockfd,F_SETLK,file_lock(F_WRLCK,SEEK_SET))<0){
	locked = false;
	return -1;
    }
    locked = true;
    return lockfd;
}

void LockFile::unlock(){
    fcntl(lockfd,F_SETLK,file_lock(F_UNLCK,SEEK_SET));
    locked = false;
    close(lockfd);
}

bool LockFile::isLocked(){
    return locked;
}

struct flock* file_lock(short type,short whence){
    static struct flock ret;
    ret.l_type = type;
    ret.l_start = 0;
    ret.l_whence = whence;
    ret.l_len = 0;
    ret.l_pid = getpid();
    return &ret;
}
