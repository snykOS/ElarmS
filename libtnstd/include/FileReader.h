/***********************************************************

File Name :
        FileReader.h

Original Author:
        Patrick Small

Description:


Creation Date:
        18 August 2000

Modification History:
        31 Jan 2017 DSN
                Increased FILE_READER_MAX_STRING_LEN from 256 to 1024.

Usage Notes:


**********************************************************/

#ifndef file_reader_H
#define file_reader_H

// Various include files
#include <cstdio>



// Maximum length for a configuration file string
const int FILE_READER_MAX_STRING_LEN = 1024;


// FileReader reference structure
//
// This makes multiple FileReader objects possible, that all
// refer to a single disk file.
//
struct frcb {
    int refcount;
    FILE *fd;
    char curline[FILE_READER_MAX_STRING_LEN];
    int lineno;
};


class FileReader
{

 private:

 protected:

    int valid;
    struct frcb *file;

 public:

    // Default Constructor
    //
    FileReader();


    // Constructor
    //
    FileReader(const char *file);


    // Copy constructor
    //
    FileReader(const FileReader &c);


    // Destructor
    //
    ~FileReader();


    // Read next line from file
    //
    int next(char *);


    // Get the current line number
    //
    int getLineNum(int &ln);


    // Overloaded assignment operator
    //
    FileReader& operator=(const FileReader &c);


    // Boolean operator which returns TN_TRUE when the FileReader object
    // is not valid, and TN_FALSE when the object references a valid and
    // open FileReader file.
    //
    friend int operator!(const FileReader &cf);
};


#endif

