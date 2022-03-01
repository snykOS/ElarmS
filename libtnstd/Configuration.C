/***********************************************************

File Name :
        Configuration.C

Original Author:
        Patrick Small

Description:

        This source file defines the methods for the Configuration
class. The Configuration class allows an application to load
key:value pairs of strings from a configuration file residing on 
disk.

        The class recognizes three line formats within a configuration
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


3) "Include config.txt"

        The line consists of the string token "Include" followed by
whitespace and then a text string representing the path and filename
of another configuration file to read.

        Item #3 implemented by Paulf March 28, 2006



Creation Date:
        4 August 1998

Modification History:
	
	28 March 2006 - paulf added Include directive


Usage Notes:


**********************************************************/

// Various include files
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h> // strcasecmp
#include "RetCodes.h"
#include "Configuration.h"

using namespace std;

// Character used as a comment
const char COMMENT_CHAR = '#';



Configuration::Configuration()
{
    valid = TN_FALSE;
    configfile = NULL;
}

//
// this simple func checks to make sure that this config has not already been included
//
bool Configuration::_isIncluded(struct configcb *in) {
    CFlist::iterator c;
#ifdef DEBUG_CONFIGURATION
    std::cout<< "Debug: checking for "<< in->curfile<<std::endl;
#endif

    // check it against itself!
    if (strcmp(configfile->curfile, in->curfile) == 0) {
	std::cout<< "Debug: already found as current file "
		 << configfile->curfile << std::endl;
	return true;
    }
    // debug
#ifdef DEBUG_CONFIGURATION
    if (cflist.size()== 0) {
	std::cout<< "Debug (_isIncluded()): cflist is empty"<<std::endl;
    } else {
	std::cout<< "Debug (_isIncluded()): cflist  has " << cflist.size() 
		 << " elements" << std::endl;
    }
#endif
   
    // check all that are on the cflist
    for(c = cflist.begin(); c != cflist.end(); c++) {
	if (strcmp( in->curfile, (*c)->curfile) == 0) {
	    return true;
	}
    }
    return false;
}

struct configcb * Configuration::_open(const char *cfile) {
    struct stat statbuf;
    struct configcb *config;

    if (strcmp(cfile, "") == 0) {
	std::cout << "Error (Configuration): No config file specified"
		  << std::endl;
	return NULL;
    }
    // Check if the existing file is a directory
    if (lstat(cfile, &statbuf) != 0) {
	std::cout << "Error (Configuration): Unable to stat config file " 
		  << cfile << ": " << strerror(errno) << std::endl;
	return NULL;
    }
    if ((statbuf.st_mode & S_IFDIR) != 0) {
	std::cout << "Error (Configuration): Target file " << cfile 
		  <<" is a directory" << std::endl;
	return TN_FALSE;
    }

    config = new struct configcb;
    if (config == NULL) {
	std::cout << "Error (Configuration): Unable to allocate more memory" << std::endl;
	return NULL;
    }

    config->fd.open(cfile);
    if (!config->fd) {
	std::cout << "Error (Configuration): Error opening config file " 
		  << cfile << ": " << strerror(errno) << std::endl;
	delete config;
	return NULL;
    }
    
    config->refcount = 1;
    strcpy(config->curline, "");
    strcpy(config->curfile, cfile);
    config->lineno = 0;

    return config;
}

Configuration::Configuration(const char *cfile)
{

    valid = TN_FALSE;
    if ( ( configfile = this->_open(cfile) ) == NULL) {
	return;
    }
    valid = TN_TRUE;
}



Configuration::Configuration(const Configuration &c)
{
    this->valid = c.valid;
    this->configfile = c.configfile;
    if (this->configfile != NULL) {
	this->configfile->refcount++;
    }
}



Configuration::~Configuration()
{
    if (configfile != NULL) {
	configfile->refcount--;
	if (configfile->refcount == 0) {
	    configfile->fd.close();
	    delete(configfile);
	}
    }
    valid = TN_FALSE;
}


int Configuration::stripTrailingWhiteSpace(char *str)
{
    int l;
    l = strlen(str);
    if (l==0) 
	return(TN_SUCCESS);
    l--;
    while ( l>0 && (str[l] == ' ' || str[l] == '\t' ) ) {
	str[l]=0;
 	l--;
    }
    l = strlen(str);
    return(TN_SUCCESS);
}

int Configuration::stripLeadingWhiteSpace(char *str)
{
    int choplen;

    choplen = strspn(str, " \t");
    if (choplen > 0) {
	char *c = str+choplen;
	while(*c != '\0') *str++ = *c++;
	*str = '\0';
    }
    return(TN_SUCCESS);
}



int Configuration::isComment(char *str)
{
    if (str[0] == COMMENT_CHAR) {
	return(TN_TRUE);
    } else {
	return(TN_FALSE);
    }
}



int Configuration::getTagValue(char *tag, char *value, char *line)
{ 
    int len;
    char *retval, *last;
    char getline[CONFIG_MAX_STRING_LEN*2];
    strcpy(getline, line);
    strcpy(tag, "");
    strcpy(value, "");

    // Strip off newline at the end of the line
    len = strlen(getline);
    if (getline[len -1] == '\n') {
	getline[len - 1] = 0;
    }
    retval = strtok_r(getline, " \t", &last);
    if (retval == NULL) {
	return(TN_FAILURE);
    }
    strcpy(tag, retval);
    retval = strtok_r(NULL, "", &last);
    if (retval == NULL) {
	strcpy(value, "");
    } else {
	strcpy(value, retval);
	this->stripLeadingWhiteSpace(value);
	this->stripTrailingWhiteSpace(value);
    }
    return(TN_TRUE);
}


int Configuration::next(char *tag, char *value)
{ 
    char *retval, *last;
    char getline[CONFIG_MAX_STRING_LEN];
    int done = TN_FALSE;
    int len;

    strcpy(tag, "");
    strcpy(value, "");
    strcpy(configfile->curline, "");

    if (valid) {
	do {
	    if (!configfile->fd.getline(getline, CONFIG_MAX_STRING_LEN)) {
		if (configfile->fd.eof()) {
		    // see if there is another file in the list
		    if (cflist.size() == 0) {
			return(TN_EOF);
		    } else {
			configfile = (struct configcb *) cflist.front();
			cflist.pop_front();
#ifdef DEBUG_CONFIGURATION
			std::cout<< "Debug: in next(), popping next file off cflist" << std::endl;
			std::cout<< "Debug: popped off " << configfile->curfile<< " cflist has " << cflist.size() << " elements left"<<std::endl;
#endif
		    }
		}
		else if (configfile->fd.fail()) {
		    perror("Error (next):");
		    configfile->fd.clear();
		    return(TN_FAILURE);
		}
	    } else {
		strcpy(configfile->curline, getline);
		configfile->lineno++;
		this->stripLeadingWhiteSpace(getline);
		this->stripTrailingWhiteSpace(getline);
		if ((!isComment(getline)) && (strlen(getline) > 0) && 
		    (strcmp(getline, "\n") != 0)) {
		    if ( this->getTagValue(tag, value, getline) == TN_TRUE ) {
			// check for Include directive
			if (strcasecmp(tag, "INCLUDE") == 0) {
			    // get a new config
			    struct configcb *ctmp;
			    if ( (ctmp = this->_open(value)) == NULL ) {
				std::cout << "Error (Configuration): Include config file " << value << " not readable, skipping!" << std::endl;
				continue;
			    } else {
				if ( this->_isIncluded(ctmp) == true) {
				    std::cout << "Error (Configuration): Include config file " << value << " already included, skipping!" << std::endl;
				    continue;
				}
				// push the current config onto the cflist and move the new one into current position
#ifdef DEBUG_CONFIGURATION
				std::cout << "Debug (Configuration): pushing onto cflist " << configfile->curfile << std::endl;
#endif
				cflist.push_front(configfile);
				configfile=ctmp;
			    }
			} else {
			    done = TN_TRUE;
			}
		    } 
		}
	    }
	} while (!done);

	// Strip off newline at the end of the line
	len = strlen(getline);
	if (getline[len -1] == '\n') {
	    getline[len - 1] = 0;
	}
	retval = strtok_r(getline, " \t", &last);
	if (retval == NULL) {
	    return(TN_FAILURE);
	}
	strcpy(tag, retval);
	retval = strtok_r(NULL, "", &last);
	if (retval == NULL) {
	    strcpy(value, "");
	} else {
	    strcpy(value, retval);
	    this->stripLeadingWhiteSpace(value);
	    this->stripTrailingWhiteSpace(value);
	}
	if (strcasecmp(tag, "INCLUDE") == 0) {
	    // get a new config
	    struct configcb *ctmp;
	    if ( (ctmp = this->_open(value)) == NULL ) {
		std::cout << "Error (Configuration): Include config file " << value << " not readable, skipping!" << std::endl;
		return this->next(tag, value);
	    } else {
		if ( this->_isIncluded(ctmp) == true) {
		    std::cout << "Error (Configuration): Include config file " << value << " already included, skipping!" << std::endl;
		    return this->next(tag, value);
		}
		// push the current config onto the cflist and move the new one into current position
#ifdef DEBUG_CONFIGURATION
		std::cout << "Debug (Configuration): pushing onto cflist " << configfile->curfile << std::endl;
#endif
		cflist.push_front(configfile);
		configfile=ctmp;

		return this->next(tag, value);	// now get the next value, since we don't want to return the include line 
	    }
	}
	return(TN_SUCCESS);
    } else {
	return(TN_FAILURE);
    }
}




int Configuration::next(char *line)
{
    char *retval;
    char getline[CONFIG_MAX_STRING_LEN];
    int done = TN_FALSE;
    int len;

    strcpy(line, "");
    strcpy(configfile->curline, "");

    if (valid) {
	do {
	    if (!configfile->fd.getline(getline, CONFIG_MAX_STRING_LEN)) {
		if (configfile->fd.eof()) {
		    // see if there is another file in the list
		    if (cflist.size() == 0) {
			return(TN_EOF);
		    } else {
			configfile = (struct configcb *) cflist.front();
			cflist.pop_front();
#ifdef DEBUG_CONFIGURATION
			std::cout<< "Debug: in next(), poping next file off cflist" << std::endl;
			std::cout<< "Debug: popped off " << configfile->curfile<< " cflist has " << cflist.size() << " elements left"<<std::endl;
#endif
		    }
		}
		else if (configfile->fd.fail()) {
		    perror("Error (next):");
		    configfile->fd.clear();
		    return(TN_FAILURE);
		}
	    } else {
		strcpy(configfile->curline, getline);
		configfile->lineno++;
		this->stripLeadingWhiteSpace(getline);
		if ((!isComment(getline)) && (strlen(getline) > 0) && 
		    (strcmp(getline, "\n") != 0)) {
		    // check for Include directive
		    char tag[CONFIG_MAX_STRING_LEN], value[CONFIG_MAX_STRING_LEN];
		    if ( this->getTagValue(tag, value, getline) == TN_TRUE ) {
			if (strcasecmp(tag, "INCLUDE") == 0) {
			    // get a new config
			    struct configcb *ctmp;
			    if ( (ctmp = this->_open(value)) == NULL ) {
				std::cout << "Error (Configuration): Include config file " << value << " not readable, skipping!" << std::endl;
				// return just this line
				done = TN_TRUE;
			    } else {
				if ( this->_isIncluded(ctmp) == true) {
				    std::cout << "Error (Configuration): Include config file " << value << " already included, skipping!" << std::endl;
				    done=TN_TRUE;	// return the full line with the include
				} else {
				    // push the current config onto the cflist and move the new one into current position
#ifdef DEBUG_CONFIGURATION
				    std::cout << "Debug (Configuration): pushing onto cflist " << configfile->curfile << std::endl;
#endif
				    cflist.push_front(configfile);
				    configfile=ctmp;
				}
			    }
			} else {
			    done = TN_TRUE;
			}
		    } else {
			done = TN_TRUE;
		    }
		} 
	    }
	} while (!done);

	// Strip off newline at the end of the line
	len = strlen(getline);
	if (getline[len -1] == '\n') {
	    getline[len - 1] = 0;
	}
	strcpy(line, getline);
	return(TN_SUCCESS);
    } else {
	return(TN_FAILURE);
    }
}



int Configuration::nextFull(char *line)
{
    char *retval;
    char getline[CONFIG_MAX_STRING_LEN];
    char tmpline[CONFIG_MAX_STRING_LEN];
    int done = TN_FALSE;
    int len;

    strcpy(line, "");
    strcpy(configfile->curline, "");

    if (valid) {
	do {
	    if (!configfile->fd.getline(getline, CONFIG_MAX_STRING_LEN)) {
		if (configfile->fd.eof()) {
		    // see if there is another file in the list
		    if (cflist.size() == 0) {
			return(TN_EOF);
		    } else {
			configfile = (struct configcb *) cflist.front();
			cflist.pop_front();
#ifdef DEBUG_CONFIGURATION
			std::cout<< "Debug: in nextFull(), poping next file off cflist" << std::endl;
			std::cout<< "Debug: popped off " << configfile->curfile<< " cflist has " << cflist.size() << " elements left"<<std::endl;
#endif
		    }
		}
		else if (configfile->fd.fail()) {
		    perror("Error (nextFull):");
		    configfile->fd.clear();
		    return(TN_FAILURE);
		}
	    } else {
		strcpy(configfile->curline, getline);
		strcpy(tmpline, getline);
		configfile->lineno++;
		this->stripLeadingWhiteSpace(tmpline);
		if ((!isComment(tmpline)) && (strlen(tmpline) > 0) && 
		    (strcmp(tmpline, "\n") != 0)) {
		    // check for Include directive
		    char tag[CONFIG_MAX_STRING_LEN], value[CONFIG_MAX_STRING_LEN];
		    if ( this->getTagValue(tag, value, getline) == TN_TRUE ) {
			if (strcasecmp(tag, "INCLUDE") == 0) {
			    // get a new config
			    struct configcb *ctmp;
			    if ( (ctmp = this->_open(value)) == NULL ) {
				std::cout << "Error (Configuration): Include config file " << value << " not readable, skipping!" << std::endl;
				// return just this line
				done = TN_TRUE;
			    } else {
				if ( this->_isIncluded(ctmp) == true) {
				    std::cout << "Error (Configuration): Include config file " << value << " already included, skipping!" << std::endl;
				    done=TN_TRUE;	// return the full line with the include
				} else {
				    // push the current config onto the cflist and move the new one into current position
#ifdef DEBUG_CONFIGURATION
				    std::cout << "Debug (Configuration): pushing onto cflist " << configfile->curfile << std::endl;
#endif
				    cflist.push_front(configfile);
				    configfile=ctmp;
				}
			    }
			} else {
			    done = TN_TRUE;
			}
		    } else {
			done = TN_TRUE;
		    }
		}
	    }
	} while (!done);

	strcpy(line, getline);
	return(TN_SUCCESS);
    } else {
	return(TN_FAILURE);
    }
}




int Configuration::getLineNum(int &ln)
{
    if (valid) {
	ln = configfile->lineno;
	return(TN_SUCCESS);
    } else {
	return(TN_FAILURE);
    }
}




int Configuration::getFullLine(char *fl)
{
    if (valid) {
	strcpy(fl, configfile->curline);
	return(TN_SUCCESS);
    } else {
	return(TN_FAILURE);
    }
}



int Configuration::getNumTokensInLine(const char *sepchars, int &numtok)
{
    char *pos, *last;
    char tmpstr[CONFIG_MAX_STRING_LEN];

    numtok = 0;
    if (valid) {
	strcpy(tmpstr, configfile->curline);
	pos = strtok_r(tmpstr, sepchars, &last);
	while (pos != NULL) {
	    numtok++;
	    pos = strtok_r(NULL, sepchars, &last);
	}
	return(TN_SUCCESS);
    } else {
	return(TN_FAILURE);
    }
}


int Configuration::getTokens(const char *val, ValueList &vl)
{
    std::string value(val);
    int start;
    int stop;
    int n = value.length();

    vl.clear();
    start = value.find_first_not_of(" \t");
    while ((start >= 0) && (start < n)) {
	stop = value.find_first_of(" \t", start);
	if ((stop < 0) || (stop > n)) {
	    stop = n;
	}
	vl.insert(vl.end(), value.substr(start, stop - start));
	start = value.find_first_not_of(" \t", stop + 1);
    }

    return(TN_SUCCESS);
}



Configuration& Configuration::operator=(const Configuration &c)
{
    // Close any existing configuration file
    if (configfile != NULL) {
	configfile->refcount--;
	if (configfile->refcount == 0) {
	    configfile->fd.close();
	    delete(configfile);
	}
    }

    this->valid = c.valid;
    this->configfile = c.configfile;
    if (this->configfile != NULL) {
	this->configfile->refcount++;
    }
    return(*this);
}



int operator!(const Configuration &cf)
{
    return(!cf.valid);
}
