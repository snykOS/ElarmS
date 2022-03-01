/***********************************************************

File Name :
        ChannelExpr.h

Original Author:
        Patrick Small

Description:


Creation Date:
        April 7 2000


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_expr_H
#define channel_expr_H

// Various include files
#include <iostream>
using std::ostream;
#include <sys/types.h>
#include <regex.h>
#include "GenLimits.h"
#include "Channel.h"


// Channel expression reference structure
//
// This makes multiple ChannelExpr objects possible, that all
// refer to a single regular expression.
//
struct exprcb {
    int refcount;
    regex_t net_re;
    regex_t sta_re;
    regex_t loc_re;
    regex_t chan_re;

};


class ChannelExpr
{
 private:
    int valid;
    char network_expr[MAXSTR];
    char station_expr[MAXSTR];
    char channel_expr[MAXSTR];
    char location_expr[MAXSTR];

    struct exprcb *expr;

    int _Cleanup();

 public:
    ChannelExpr();
    ChannelExpr(const ChannelExpr &c);
    ChannelExpr(const char *net, const char *sta, const char *loc,
		const char *chan);
    ~ChannelExpr();

    int IsMatch(const Channel &c) const;

    ChannelExpr& operator=(const ChannelExpr &c);
    friend int operator<(const ChannelExpr &c1, const ChannelExpr &c2);
    friend int operator==(const ChannelExpr &c1, const ChannelExpr &c2);
    friend ostream& operator<<(ostream &os, const ChannelExpr &c);
    friend int operator!(const ChannelExpr &c);
};


#endif
