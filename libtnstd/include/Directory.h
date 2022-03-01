/***********************************************************

File Name :
        Directory.h

Original Author:
        Patrick Small

Description:

        This header file defines the interface for the Directory
class. The Directory class allows an application to read the files
and sub-directories contained within a UNIX filesystem directory.

Creation Date:
        25 January 2000

Modification History:


Usage Notes:


**********************************************************/

#ifndef directory_H
#define directory_H

// Various include files
#include <sys/types.h>
#include <dirent.h>


// Directory reference structure
//
// This makes multiple Directory objects possible, that all
// refer to a single directory.
//
struct dircb {
    int refcount;
    DIR* dirp;
};


class Directory
{

 private:
    int valid;
    struct dircb *dirfile;

 public:

    // Default Constructor
    //
    Directory();


    // Constructor
    //
    Directory(const char *pathdir);


    // Copy constructor
    //
    Directory(const Directory &d);


    // Destructor
    //
    ~Directory();


    // Read next directory entry from the directory
    //
    int next(char *);


    // Overloaded assignment operator
    //
    Directory& operator=(const Directory &d);


    // Boolean operator which returns TN_TRUE when the Directory object
    // is not valid, and TN_FALSE when the object references a valid and
    // open Directory file.
    //
    friend int operator!(const Directory &df);
};


#endif

