/***********************************************************

File Name :
        Pipe.h

Original Author:
        Patrick Small

Description:


Creation Date:
        18 February 2000


Modification History:


Usage Notes:


**********************************************************/

#ifndef pipe_H
#define pipe_H


// Flags that determine which side of pipe is used by a process
const int PIPE_NONE = -1;
const int PIPE_CHILD = 0;
const int PIPE_PARENT = 1;


class Pipe
{
 private:
    int valid;
    int fds[2];
    int active;
    int noclose;

 public:
    Pipe();
    Pipe(int fd, int a);
    ~Pipe();

    int SetSide(int a);
    int GetDescriptor(int &fd);
    int DisableClose();

    int Read(char *buf, int &len);
    int Write(char *buf, int len);

    friend int operator!(const Pipe &p);

};

#endif

