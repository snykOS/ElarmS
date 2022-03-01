/***********************************************************

File Name :
	TemporaryFile.h

Original Author:
        Patrick Small

Description:

        This header file defines the TemporaryFile class - an
object class which abstracts the use of temporary disk files. 
When objects of this class are created, a file with a unique
name is created in the "/tmp" directory. Bytes may be read to
this temporary file. When the object is destroyed, the
temporary file is deleted from disk.


Creation Date:
        13 November 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef tempfile_H
#define tempfile_H

// Various include files
#include <cstdio>


class TemporaryFile
{
 private:
    
    int valid;
    int isopen;
    std::FILE *tfile;
    char filename[FILENAME_MAX];
    
 public:
    
    // Default Constructor
    //
    // Creates the temporary file
    //
    TemporaryFile();
    
    
    // Destructor
    //
    // Closes (if open) and removes the temporary file
    //
    ~TemporaryFile();
    

    // Return the path and filename of the temporary file
    //
    // This method returns TN_SUCCESS upon success, TN_FAILURE otherwise.
    //
    int getName(char *name);
    
    
    // Writes a a buffer of bytes to the temporary file.
    //
    // This method returns TN_SUCCESS upon success, TN_FAILURE otherwise.
    //
    int write(const char *buf, unsigned int bufsize);


    // Writes a a buffer of characters to the temporary file.
    //
    // This method returns TN_SUCCESS upon success, TN_FAILURE otherwise.
    //
    int write(const char *buf);
    

    // Flushes all unwritten data to disk.
    //
    // This method returns TN_SUCCESS upon success, TN_FAILURE otherwise.
    //
    int flush();


    // Closes the temporary file
    //
    // This method returns TN_SUCCESS upon success, TN_FAILURE otherwise.
    //
    int close();


    // Boolean operator which returns TN_TRUE when the TemporaryFile object
    // is not valid, and TN_FALSE when the object references a valid and
    // open temporary file.
    //
    friend int operator!(const TemporaryFile &tf);
};


#endif
