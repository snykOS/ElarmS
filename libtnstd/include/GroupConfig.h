/***********************************************************

File Name :
        GroupConfig.h

Original Author:
        Patrick Small

Description:


Creation Date:
        31 August 1998

Modification History:
        31 Jan 2017 DSN
                Increased GROUPCONFIG_MAX_STRING_LEN from 256 to 1024.

Usage Notes:


**********************************************************/

#ifndef groupconfig_H
#define groupconfig_H


// Various include files
#include <cstdio>
#include "Configuration.h"
#include "GroupConfig.h"

// Maximum length for a configuration file string
const int GROUPCONFIG_MAX_STRING_LEN = 1024;



class GroupConfig : public Configuration
{

 private:
    int ingroup;
    char curgroup[CONFIG_MAX_STRING_LEN];

    // Private methods
    //
    int isGroupStart(char *line);
    int isGroupStop(char *line);
    int stripWhiteSpace(char *str);
    int stripEndWhiteSpace(char *str);

 public:

    // Default Constructor
    //
    GroupConfig();


    // Default Constructor
    //
    GroupConfig(const char *gcfile);

    // Destructor
    //
    ~GroupConfig();


    // Retrieves the current group identifier
    //
    int getGroup(char *group);


    // Returns TN_TRUE if positioned within a group, TN_FALSE otherwise
    //
    int inGroup();


    // Read next line from file and return the tag/value pair as strings
    //
    int next(char *, char *);


    // Read next line from file and return the entire line as a string
    //
    int next(char *);

};


#endif

