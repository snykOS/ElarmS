/***********************************************************

File Name :
        SearchExpr.h

Original Author:
        Patrick Small

Description:


Creation Date:
        August 17 2000


Modification History:


Usage Notes:


**********************************************************/

#ifndef search_expr_H
#define search_expr_H

// Various include files
#include <iostream>
using std::ostream;
#include <sys/types.h>
#include <regex.h>
#include "GenLimits.h"


// Search expression reference structure
//
// This makes multiple SearchExpr objects possible, that all
// refer to a single regular expression.
//
struct exprcb {
    int refcount;
    regex_t search_re;
};


class SearchExpr
{
 private:
    int valid;
    char search_expr[MAXSTR];
    struct exprcb *expr;

    int _Cleanup();

 public:
    SearchExpr();
    SearchExpr(const SearchExpr &s);
    SearchExpr(const char *str);
    ~SearchExpr();

    int IsMatch(const char *str) const;
    int MatchAndReplace(char *str, const char *repstr) const;

    SearchExpr& operator=(const SearchExpr &s);
    friend int operator<(const SearchExpr &s1, const SearchExpr &s2);
    friend int operator==(const SearchExpr &s1, const SearchExpr &s2);
    friend ostream& operator<<(ostream &os, const SearchExpr &s);
    friend int operator!(const SearchExpr &s);
};


#endif
