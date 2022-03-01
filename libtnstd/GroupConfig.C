/***********************************************************

File Name :
        GroupConfiguration.C

Original Author:
        Patrick Small

Description:


Creation Date:
        31 August 1998

Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include "RetCodes.h"
#include "GroupConfig.h"

using namespace std;

// Group start delimiter
// const char GROUP_START_CHAR = '{';


// Group stop delimiter
const char GROUP_END_CHAR = '}';


// Code for the ASCII space character
const char GROUP_CONFIG_SPACE = 32;


// Code for the ASCII tab character
const char GROUP_CONFIG_TAB = 9;



GroupConfig::GroupConfig() : Configuration() 
{
  ingroup = TN_FALSE;
  strcpy(curgroup, "");
};



GroupConfig::GroupConfig(const char *gcfile) : Configuration(gcfile) 
{
  ingroup = TN_FALSE;
  strcpy(curgroup, "");
};



GroupConfig::~GroupConfig()
{
};



int GroupConfig::isGroupStart(char *line)
{
  char *tok;
  char str[CONFIG_MAX_STRING_LEN];
  int toknum = -1;
  int grouppos = -1;
  int found = TN_FALSE;

  strcpy(str, line);
  tok = strtok(str, " \t");
  while (tok != NULL) {
    toknum++;
    if (strcmp(tok, "{") == 0) {
      grouppos = toknum;
      found = TN_TRUE;
    }
    tok = strtok(NULL, " \t");
  }

  if ((found == TN_TRUE) && (grouppos == toknum)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}



int GroupConfig::isGroupStop(char *line)
{
  int retval;
  char c;

  retval = sscanf(line, "%c", &c);
  if ((retval != 1) || (c != GROUP_END_CHAR)) {
    return(TN_FALSE);
  } else {
    return(TN_TRUE);
  }
}



int GroupConfig::stripWhiteSpace(char *str)
{
  int choplen;

  choplen = strspn(str, " \t");
  if (choplen > 0) {
    strcpy(str, (str + choplen));
  }
  return(TN_SUCCESS);
}



int GroupConfig::stripEndWhiteSpace(char *str)
{
  int curpos;

  curpos = strlen(str) - 1;
  for (curpos = strlen(str) - 1; curpos >= 0; curpos--) {
    if ((str[curpos] == GROUP_CONFIG_SPACE) || 
	(str[curpos] == GROUP_CONFIG_TAB)) {
      str[curpos] = '\0';
    } else {
      return(TN_SUCCESS);
    }
  }
  return(TN_SUCCESS);
}



int GroupConfig::next(char *tag, char *value)
{
  int retval;
  char *tokretval;
  char getline[CONFIG_MAX_STRING_LEN];
  int len;

  if (!(*this)) {
    return(TN_FAILURE);
  } else {
    retval = ((Configuration *)(this))->next(getline);
    if (retval == TN_SUCCESS) {
      
      // Check for unexpected start/stop of groups
      if (((ingroup) && (isGroupStart(getline))) ||
	  ((!ingroup) && (isGroupStop(getline)))) {
	ingroup = TN_FALSE;
	strcpy(curgroup, "");
	std::cout << "Error (next): Unexpected group delimiter encountered" << std::endl;
	return(TN_FAILURE);
      }
      
      // Check for end of group
      if ((ingroup) && (isGroupStop(getline))) {
	ingroup = TN_FALSE;
	strcpy(curgroup, "");
	return(TN_ENDGROUP);
      }
      
      // Check for start of group
      if (isGroupStart(getline)) {

	// Grab the group identifier
	tokretval = strtok(getline, "{");
	if (tokretval == NULL) {
	  return(TN_FAILURE);
	}
	strcpy(tag, tokretval);
	this->stripEndWhiteSpace(tag);
	ingroup = TN_TRUE;
	strcpy(curgroup, getline);
	this->stripEndWhiteSpace(curgroup);
	return(TN_STARTGROUP);
// 	tokretval = strtok(getline, " \t");
// 	if (tokretval == NULL) {
// 	  return(TN_FAILURE);
// 	}
// 	strcpy(tag, tokretval);
// 	ingroup = TN_TRUE;
// 	strcpy(curgroup, getline);
// 	return(TN_STARTGROUP);
      } else {

	// Just a regular configuration line
	tokretval = strtok(getline, " \t");
	if (tokretval == NULL) {
	  return(TN_FAILURE);
	}
	strcpy(tag, tokretval);
	tokretval = strtok(NULL, "");
	if (tokretval == NULL) {
	  strcpy(value, "");
	} else {
	  strcpy(value, tokretval);
	  this->stripWhiteSpace(value);
	}
	return(TN_SUCCESS);
      }
    } else {
      ingroup = TN_FALSE;
      strcpy(curgroup, "");
      return(retval);
    }
  }
}



int GroupConfig::next(char *line)
{
  int retval;
  char *tokretval;
  char getline[CONFIG_MAX_STRING_LEN];
  int len;

  strcpy(line, "");

  if (!(*this)) {
    return(TN_FAILURE);
  } else {
    retval = ((Configuration *)(this))->next(getline);
    if (retval == TN_SUCCESS) {
      
      // Check for unexpected start/stop of groups
      if (((ingroup) && (isGroupStart(getline))) ||
	  ((!ingroup) && (isGroupStop(getline)))) {
	ingroup = TN_FALSE;
	strcpy(curgroup, "");
	std::cout << "Error (next): Unexpected group delimiter encountered" << std::endl;
	return(TN_FAILURE);
      }
      
      // Check for end of group
      if ((ingroup) && (isGroupStop(getline))) {
	ingroup = TN_FALSE;
	strcpy(curgroup, "");
	return(TN_ENDGROUP);
      }
      
      // Check for start of group
      if (isGroupStart(getline)) {

	// Grab the group identifier
	tokretval = strtok(getline, "{");
	if (tokretval == NULL) {
	  return(TN_FAILURE);
	}
	strcpy(line, tokretval);
	this->stripEndWhiteSpace(line);
	ingroup = TN_TRUE;
	strcpy(curgroup, tokretval);
	this->stripEndWhiteSpace(curgroup);
	return(TN_STARTGROUP);
// 	tokretval = strtok(getline, " \t");
// 	if (tokretval == NULL) {
// 	  return(TN_FAILURE);
// 	}
// 	strcpy(line, tokretval);
// 	ingroup = TN_TRUE;
// 	strcpy(curgroup, tokretval);
// 	return(TN_STARTGROUP);
      } else {

	// Just a regular configuration line
	strcpy(line, getline);
	return(TN_SUCCESS);
      }
    } else {
      ingroup = TN_FALSE;
      strcpy(curgroup, "");
      return(retval);
    }
  }
}


int GroupConfig::getGroup(char *group) 
{
  if (!(*this)) {
    strcpy(group, "");
    return(TN_FAILURE);
  } else {
    strcpy(group, curgroup);
    return(TN_SUCCESS);
  }
}




int GroupConfig::inGroup()
{
  if (!(*this)) {
    return(TN_FALSE);
  } else {
    return(ingroup);
  }
}
