#ifndef __rtexception_h
#define __rtexception_h

#include <string>
#include <iostream>

using namespace std;

class RTException{

 public:
    string msg;
    unsigned int errcode;

    RTException();
    RTException(string);
};

class RTOutOfMemoryException : public RTException{
 public:
    RTOutOfMemoryException(string);
};

class RTIOException: public RTException{
 public:
    RTIOException(string);
};

class RTFileNotFoundException: public RTException{
 public:
    RTFileNotFoundException(string);
};

class RTIllegalArgumentException: public RTException{
 public:
    RTIllegalArgumentException(string);
};

class RTIllegalFileFormatException: public RTException{
 public:
    RTIllegalFileFormatException(string);
};

class RTFileCorruptedError: public RTException{
 public:
    RTFileCorruptedError(string);
};


#endif//__rtexception_h
