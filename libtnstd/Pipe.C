/***********************************************************

File Name :
        Pipe.C

Original Author:
        Patrick Small

Description:


Creation Date:
        18 February 2000


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <unistd.h>
#include "RetCodes.h"
#include "Pipe.h"



Pipe::Pipe()
{
  valid = TN_FALSE;
  active = PIPE_NONE;
  noclose = TN_FALSE;

  // Open two-way pipe
  if (pipe(fds) != 0) {
    std::cout << "Error (Pipe::Pipe): Unable to open 2-way pipe" << std::endl;
    return;
  }

  valid = TN_TRUE;
}



Pipe::Pipe(int fd, int a)
{
  valid = TN_FALSE;
  active = PIPE_NONE;
  noclose = TN_FALSE;

  switch (a) {
  case PIPE_PARENT:
    fds[PIPE_PARENT] = fd;
    break;
  case PIPE_CHILD:
    fds[PIPE_CHILD]= fd;    
    break;
  default:
    std::cout << "Error (Pipe::Pipe): Invalid activity flag " << a << std::endl;
    return;
    break;
  };

  active = a;

  valid = TN_TRUE;
}



Pipe::~Pipe()
{
  if ((valid) && (noclose == TN_FALSE)) {
    switch (active) {
    case PIPE_PARENT:
      close(fds[PIPE_PARENT]);
      break;
    case PIPE_CHILD:
      close(fds[PIPE_CHILD]);
      break;
    default:
      close(fds[PIPE_PARENT]);
      close(fds[PIPE_CHILD]);
      break;
    };
  }
}


int Pipe::SetSide(int a)
{

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  switch (a) {
  case PIPE_PARENT:
    close(fds[PIPE_CHILD]);
    break;
  case PIPE_CHILD:
    close(fds[PIPE_PARENT]);    
    break;
  default:
    std::cout << "Error (Pipe::SetSide): Invalid activity flag " << a << std::endl;
    return(TN_FAILURE);
    break;
  };

  active = a;
  return(TN_SUCCESS);
}



int Pipe::GetDescriptor(int &fd)
{
  if ((valid != TN_TRUE) || (active == PIPE_NONE)) {
    return(TN_FAILURE);
  }

  switch (active) {
  case PIPE_PARENT:
    fd = fds[PIPE_PARENT];
    break;
  case PIPE_CHILD:
    fd = fds[PIPE_CHILD];    
    break;
  default:
    std::cout << "Error (Pipe::GetDescriptor): Invalid activity flag" << std::endl;
    return(TN_FAILURE);
    break;
  };

  return(TN_SUCCESS);
}



int Pipe::DisableClose()
{
  noclose = TN_TRUE;
  return(TN_SUCCESS);
}



int Pipe::Read(char *buf, int &len)
{
  int retval;

  if ((valid != TN_TRUE) || (active == PIPE_NONE)) {
    return(TN_FAILURE);
  }

  retval = read(fds[active], buf, len);
  if (retval == 0) {
    return(TN_EOF);
  } else if (retval != len) {
    std::cout << "Error (Pipe::Read): Unable to read buffer from pipe" << std::endl;
    return(TN_FAILURE);
  }

  return(TN_SUCCESS);
}



int Pipe::Write(char *buf, int len)
{
  int retval;

  if ((valid != TN_TRUE) || (active == PIPE_NONE)) {
    return(TN_FAILURE);
  }

  retval = write(fds[active], buf, len);
  if (retval != len) {
    std::cout << "Error (Pipe::Write): Unable to write buffer to pipe" << std::endl;
    return(TN_FAILURE);
  }
  return(TN_SUCCESS);
}



int operator!(const Pipe &p)
{
  return(!p.valid);
}

