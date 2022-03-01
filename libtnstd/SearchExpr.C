/***********************************************************

File Name :
        SearchExpr.C

Original Author:
        Patrick Small

Description:


Creation Date:
        August 17 2000


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include "RetCodes.h"
#include "SearchExpr.h"

using namespace std;

// Maximum number of regexp matches to make
const int MAX_MATCHES = 1;


SearchExpr::SearchExpr()
{
  valid = TN_FALSE;
  strcpy(search_expr, "");
  expr = NULL;
}



SearchExpr::SearchExpr(const SearchExpr &s)
{
  valid = s.valid;
  strcpy(search_expr, s.search_expr);
  expr = s.expr;

  if (expr != NULL) {
    expr->refcount++;
  }
}



SearchExpr::SearchExpr(const char *str)
{
  valid = TN_FALSE;
  strcpy(search_expr, str);
  int retval;

  expr = new struct exprcb;
  if (expr == NULL) {
    std::cout << "Error (SearchExpr::SearchExpr): Unable to allocate more memory"
	 << std::endl;
    return;
  }

  // Create the regular expressions
  retval = regcomp(&(expr->search_re), str, REG_EXTENDED);
  if (retval != 0) {
    std::cout << "Error (SearchExpr::SearchExpr): Unable to create search expr"
	 << std::endl;
    delete(expr);
    return;
  }

  expr->refcount = 1;

  valid = TN_TRUE;
}



SearchExpr::~SearchExpr()
{
  this->_Cleanup();
}


int SearchExpr::_Cleanup()
{
  if (expr != NULL) {
    expr->refcount--;
    if (expr->refcount == 0) {
      regfree(&(expr->search_re));
      delete(expr);
    }
  }
  valid = TN_FALSE;

  return(TN_SUCCESS);
}



int SearchExpr::IsMatch(const char *str) const
{
  if (!valid) {
    return(TN_FALSE);
  }

  if (regexec(&(expr->search_re), str, (size_t) 0, NULL, 0) == 0) {
    return(TN_TRUE);
  }

  return(TN_FALSE);
}



int SearchExpr::MatchAndReplace(char *str, const char *repstr) const
{
  int retval;
  regmatch_t pmatch[MAX_MATCHES];
  char tmpstr[MAXSTR];

  if (!valid) {
    return(TN_FALSE);
  }

  retval = regexec(&(expr->search_re), str, (size_t) MAX_MATCHES, pmatch, 0);
  if (retval != 0) {
    return(TN_FAILURE);
  } else {
    if ((pmatch[0].rm_so + strlen(repstr) + strlen(str + pmatch[0].rm_eo))
	>= MAXSTR) {
      std::cout << "Error (SearchExpr::MatchAndReplace): Replacement string is too long" << std::endl;
      return(TN_FALSE);
    }
    // Replace that substring with the new string
    strncpy(tmpstr, str, pmatch[0].rm_so);
    tmpstr[pmatch[0].rm_so] = 0;
    strcat(tmpstr, repstr);
    strcat(tmpstr, str + pmatch[0].rm_eo);
    strcpy(str, tmpstr);
  }
  
  return(TN_SUCCESS);
}



SearchExpr& SearchExpr::operator=(const SearchExpr &s)
{
  if (valid) {
    this->_Cleanup();
  }

  valid = s.valid;
  strcpy(search_expr, s.search_expr);
  expr = s.expr;

  if (expr != NULL) {
    expr->refcount++;
  }

  return(*this);
}


int operator<(const SearchExpr &s1, const SearchExpr &s2)
{
  if (strcmp(s1.search_expr, s2.search_expr) < 0) {
    return(TN_TRUE);
  }

  return(TN_FALSE);
}


int operator==(const SearchExpr &s1, const SearchExpr &s2)
{
  if (strcmp(s1.search_expr, s2.search_expr) == 0) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}


ostream& operator<<(ostream &os, const SearchExpr &s)
{
  os << s.search_expr;
  return(os);
}


int operator!(const SearchExpr &s)
{
  return(!s.valid);
}
