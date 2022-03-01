#ifndef __exceptions_h
#define __exceptions_h

#include <string>       // std::string
#include <list>         // std::list

#define RCSID_Exceptions_h "$Id: Exceptions.h $"
extern const std::string RCSID_Exceptions_cc;

class Error {

    typedef  std::list<std::string> StrList;

    StrList msgstack;

 public:
    Error();
    Error(const char* str);
    Error(const std::string&  str);
    ~Error();

    void addMessage(const char*);
    std::string str(void);
    std::string printStr(void);
    void print(void);

    // unit test
    static int main(int argc, char* argv[]);
};


class LogicalError : public Error{
 public:
    LogicalError();
    LogicalError(const char* str);
};

class CriticalError : public Error{
 public:
    CriticalError();
    CriticalError(const char* str);
};

class OutofMemoryError : public CriticalError{
 public:
    OutofMemoryError();
};

class DatabaseError : public LogicalError{

 public:
    DatabaseError();
    DatabaseError(const char* str);
};

class FileFormatError : public LogicalError{
 public:
    FileFormatError();
    FileFormatError(const char* str);
};

class FileIOError : public LogicalError{
 public:
    FileIOError();
    FileIOError(const char* str);
};


#endif

// end of file: Exceptions.h
