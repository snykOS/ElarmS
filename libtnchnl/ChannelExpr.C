/***********************************************************

File Name :
        ChannelExpr.C

Original Author:
        Patrick Small

Description:


Creation Date:
        April 7 2000


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include "RetCodes.h"
#include "ChannelExpr.h"

using namespace std;

ChannelExpr::ChannelExpr()
{
  valid = TN_FALSE;
  strcpy(network_expr, "");
  strcpy(station_expr, "");
  strcpy(location_expr, "");
  strcpy(channel_expr, "");
  expr = NULL;
}



ChannelExpr::ChannelExpr(const ChannelExpr &c)
{
  valid = c.valid;
  strcpy(network_expr, c.network_expr);
  strcpy(station_expr, c.station_expr);
  strcpy(location_expr, c.location_expr);
  strcpy(channel_expr, c.channel_expr);
  expr = c.expr;

  if (expr != NULL) {
    expr->refcount++;
  }
}



ChannelExpr::ChannelExpr(const char *net, const char *sta, const char *loc,
			 const char *chan)
{
  valid = TN_FALSE;
  strcpy(network_expr, net);
  strcpy(station_expr, sta);
  strcpy(location_expr, loc);
  strcpy(channel_expr, chan);
  int retval;

  expr = new struct exprcb;
  if (expr == NULL) {
    std::cout << "Error (ChannelExpr::ChannelExpr): Unable to allocate more memory"
	 << std::endl;
    return;
  }

  // Create the regular expressions
  retval = regcomp(&(expr->net_re), net, REG_EXTENDED|REG_NOSUB);
  if (retval != 0) {
    std::cout << "Error (ChannelExpr::ChannelExpr): Unable to create net expr"
	 << std::endl;
    delete(expr);
    return;
  }
  retval = regcomp(&(expr->sta_re), sta, REG_EXTENDED|REG_NOSUB);
  if (retval != 0) {
    std::cout << "Error (ChannelExpr::ChannelExpr): Unable to create sta expr"
	 << std::endl;
    regfree(&(expr->net_re));
    delete(expr);
    return;
  }

  retval = regcomp(&(expr->loc_re), loc, REG_EXTENDED|REG_NOSUB);
  if (retval != 0) {
    std::cout << "Error (ChannelExpr::ChannelExpr): Unable to create loc expr"
	 << std::endl;
    regfree(&(expr->net_re));
    regfree(&(expr->sta_re));
    delete(expr);
    return;
  }

  retval = regcomp(&(expr->chan_re), chan, REG_EXTENDED|REG_NOSUB);
  if (retval != 0) {
    std::cout << "Error (ChannelExpr::ChannelExpr): Unable to create channel expr"
	 << std::endl;
    regfree(&(expr->net_re));
    regfree(&(expr->sta_re));
    regfree(&(expr->loc_re));
    delete(expr);
    return;
  }

  expr->refcount = 1;

  valid = TN_TRUE;
}



ChannelExpr::~ChannelExpr()
{
  this->_Cleanup();
}


int ChannelExpr::_Cleanup()
{
  if (expr != NULL) {
    expr->refcount--;
    if (expr->refcount == 0) {
      regfree(&(expr->net_re));
      regfree(&(expr->sta_re));
      regfree(&(expr->loc_re));
      regfree(&(expr->chan_re));
      delete(expr);
    }
  }
  valid = TN_FALSE;

  return(TN_SUCCESS);
}



int ChannelExpr::IsMatch(const Channel &c) const
{
  if (!valid) {
    return(TN_FALSE);
  }

  if ((regexec(&(expr->net_re), c.network, (size_t) 0, NULL, 0) == 0) &&
      (regexec(&(expr->sta_re), c.station, (size_t) 0, NULL, 0) == 0) &&
      (regexec(&(expr->loc_re), c.location, (size_t) 0, NULL, 0) == 0) &&
      (regexec(&(expr->chan_re), c.channel, (size_t) 0, NULL, 0) == 0)) {
    return(TN_TRUE);
  }

  return(TN_FALSE);
}



ChannelExpr& ChannelExpr::operator=(const ChannelExpr &c)
{
  if (valid) {
    this->_Cleanup();
  }

  valid = c.valid;
  strcpy(network_expr, c.network_expr);
  strcpy(station_expr, c.station_expr);
  strcpy(location_expr, c.location_expr);
  strcpy(channel_expr, c.channel_expr);
  expr = c.expr;

  if (expr != NULL) {
    this->expr->refcount++;
  }

  return(*this);
}


int operator<(const ChannelExpr &c1, const ChannelExpr &c2)
{
  if (strcmp(c1.network_expr, c2.network_expr) < 0) {
    return(TN_TRUE);
  }
  if ((strcmp(c1.network_expr, c2.network_expr) == 0) 
    && (strcmp(c1.station_expr, c2.station_expr) < 0)) {
    return(TN_TRUE);
  }

  if ((strcmp(c1.network_expr, c2.network_expr) == 0) 
    && (strcmp(c1.station_expr, c2.station_expr) == 0)
    && (strcmp(c1.location_expr, c2.location_expr) < 0)) {
    return(TN_TRUE);
  }

  if ((strcmp(c1.network_expr, c2.network_expr) == 0) 
    && (strcmp(c1.station_expr, c2.station_expr) == 0)
    && (strcmp(c1.location_expr, c2.location_expr) == 0)
    && (strcmp(c1.channel_expr, c2.channel_expr) < 0)) {
    return(TN_TRUE);
  }

  return(TN_FALSE);
}


int operator==(const ChannelExpr &c1, const ChannelExpr &c2)
{
  if ((strcmp(c1.network_expr, c2.network_expr) == 0) && 
      (strcmp(c1.station_expr, c2.station_expr) == 0) && 
      (strcmp(c1.location_expr, c2.location_expr) == 0) && 
      (strcmp(c1.channel_expr, c2.channel_expr) == 0)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}


ostream& operator<<(ostream &os, const ChannelExpr &c)
{
  os << c.network_expr << " " << c.station_expr << " " 
     << c.location_expr << " " << c.channel_expr;
  return(os);
}


int operator!(const ChannelExpr &c)
{
  return(!c.valid);
}
