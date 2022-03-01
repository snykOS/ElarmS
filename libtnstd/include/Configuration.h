/***********************************************************

File Name :
        Configuration.h

Original Author:
        Patrick Small

Description:

        This header file defines the interface for the Configuration
class. The Configuration class allows an application to load
key:value pairs of strings from a configuration file residing on 
disk.

        The class recognizes two line formats within a configuration
file:

1) "# Comment text..."

        Beginning the line with the '#' character causes the parser 
to skip over it to the next line in the file.


2) "Key Value"

        The line consists of a text word representing a key or command,
followed by whitespace and then a text string representing a value
or setting.
        Any amount of white space can preceed Key and be contained
between Key and Value. Whitespace within Value and at its end is
preserved and returned to the application.


        Three methods are provided for reading information from
a configuration file:

        The next() method retrieves the next available key:value
pair of text and returns TN_SUCCESS, if there is another key:value
pair in the file that is available. If the class is at the end of the 
file, TN_EOF is returned. If an error occurs during the file read or
parsing the command line, TN_FAILURE is returned.

        The getLineNum() method returns the current line number within
the configuration file.

        The getFullLine() method returns the original line of text
from the configuration file at the current line position.

        The getNumTokensInLine() method returns the number of string
tokens contained in the string at the current line position.

Creation Date:
        4 August 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef configuration_H
#define configuration_H

// Various include files
#include <fstream>
#include <string>
#include <vector>

#include <list>


// Definition of a value list for configuration parameters
typedef std::vector<std::string, std::allocator<std::string> > ValueList;


// Maximum length for a configuration file string
const int CONFIG_MAX_STRING_LEN = 1024;


// Configuration reference structure
//
// This makes multiple Configuration objects possible, that all
// refer to a single disk file.
//
struct configcb {
    int refcount;
    std::ifstream fd;
    char curline[CONFIG_MAX_STRING_LEN];
    char curfile[CONFIG_MAX_STRING_LEN];
    int lineno;
};

typedef std::list<struct configcb *> CFlist;

class Configuration
{

 private:
    int valid;
    struct configcb *configfile;
    CFlist cflist;

    // Private methods
    //
    int stripLeadingWhiteSpace(char *str);
    int stripTrailingWhiteSpace(char *str);
    int isComment(char *str);
    struct configcb * _open(const char *cfile);
    bool _isIncluded(struct configcb *config);
    int getTagValue(char *tag, char *value, char *line);


 public:

    // Default Constructor
    //
    Configuration();


    // Constructor
    //
    Configuration(const char *cfile);


    // Copy constructor
    //
    Configuration(const Configuration &c);


    // Destructor
    //
    ~Configuration();


    // Read next line from file and return the tag/value pair as strings
    //
    int next(char *, char *);


    // Read next line from file and return the entire line with any
    // preceeding white space and terminating newline removed.
    //
    int next(char *);


    // Read next line from file and return the entire line.
    //
    int nextFull(char *);


    // Get the current line number
    //
    int getLineNum(int &ln);


    // Get the original line at the current position
    //
    int getFullLine(char *fl);


    // Get the number of tokens in the line at the current position
    //
    int getNumTokensInLine(const char *sepchars, int &numtok);


    // Break up a string into its individual components
    //
    static int getTokens(const char *val, ValueList &vl);


    // Overloaded assignment operator
    //
    Configuration& operator=(const Configuration &c);


    // Boolean operator which returns TN_TRUE when the Configuration object
    // is not valid, and TN_FALSE when the object references a valid and
    // open Configuration file.
    //
    friend int operator!(const Configuration &cf);
};


#endif

