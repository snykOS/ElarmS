#ifndef ___filemessagequeue_h
#define ___filemessagequeue_h

#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include "RTException.h"
#include "Message.h"

using namespace std;

enum BlockState{SERVED = 7,UNSERVED};

struct MQHeader{
    char   magicstr[16];
    char queuename[128];
    unsigned long bookmark;
    unsigned long msgcount;
    unsigned long pending;
};

struct MQBlock{
    BlockState state;
    unsigned int msgtype;
    unsigned int msglen;
    unsigned long crc;
};


class FileMessageQueue{
 private:
    bool valid;
    int max_size;
    string filename;
    fstream* qfile;
    MQHeader header;

    unsigned int totalmessage;
    unsigned int backtrack;
    unsigned int length;
    
    bool isBlockCorrupted(const MQBlock& block) throw(RTIOException);
    unsigned long calculateBlockCRC(const MQBlock& block) throw(RTIOException);
    void openFile(void) throw(RTIOException,RTFileCorruptedError);
    void closeFile(unsigned int) throw(RTIOException);
    void init() throw(RTIOException,RTFileCorruptedError);

 public:
    FileMessageQueue(string filename,int max) throw(RTIOException,RTFileCorruptedError);
    ~FileMessageQueue();

    unsigned int size() throw(RTIOException,RTFileCorruptedError);
    unsigned int pending() throw(RTIOException,RTFileCorruptedError);
    void reset();

    void put(const Message&) throw(RTIOException,RTOutOfMemoryException);
    Message* get() throw(RTException,RTIOException,RTFileCorruptedError,RTOutOfMemoryException);
    void remove() throw(RTIOException,RTFileCorruptedError);
    bool isValid();

    bool isFileValid() throw(RTIOException);
    static void printFile(const char*);


    friend bool operator !(const FileMessageQueue& q);
};

#endif//__filemessagequeue_h
