// Test for TimeStamp, Duration, TimeWindow classes
/*
   04 Sep 2019 CAF - Fixed all compile warnings, updated to use test macro and return number of failed tests.
                     Disabled invalid test which will cause assert test to abort program.
*/

#include <unistd.h>     // usleep -- for ascending time test
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath>
#include <values.h>
#include <sys/time.h>
#include <string.h>  // strdup
#include "Duration.h"
#include "TimeStamp.h"
#include "TimeWindow.h"
#include "Ymdhms.h"
#include "Compat.h"

using namespace std;

bool show_usage     = false;
bool force_fail     = false;
bool test_assert    = false;
bool early_exit     = false;
int  debug_level    = 1;
int  pass_count     = 0;        // incremented by test macro
int  err_count      = 0;        // incremented by test macro
int  total_count    = 0;        // incremented by test macro


// define macros for testing results
#define CHECK_EXPR(EXPR, MESSAGE, EXTRA_TEXT) \
{ \
    if ( !force_fail && (EXPR) ) { \
        if (debug_level > 0) \
            std::cout << MESSAGE << " passed" << EXTRA_TEXT << std::endl; \
        pass_count++; \
    } else { \
        std::cout << MESSAGE << " FAILED at line " << __LINE__ << EXTRA_TEXT << std::endl; \
        err_count++; \
        if (early_exit) {\
            std::cout << std::endl << "exiting after first fail per request" << std::endl; \
            exit(err_count++); \
        } \
    } \
    total_count++; \
}
#define CHECK_EXPR2(EXPR, EXTRA_TEXT) CHECK_EXPR(EXPR, #EXPR, EXTRA_TEXT)
#define CHECK_EXPR1(EXPR) CHECK_EXPR(EXPR, #EXPR, "")


int main(int argc, char **argv)
{
    char opt;
    while ((opt = getopt(argc, argv, "hqfea")) != -1) {
        switch (opt) {
            case 'q':
                debug_level--;
                break;
            case 'f':
                force_fail = true;
                break;
            case 'e':
                early_exit = true;
                break;
            case 'a':
                test_assert = true;
                break;
            default:
                show_usage = true;
        } // switch opt
    } // while getopt

    if (show_usage) {
        std::cout 
            << "Usage: [-h][-q][-f][-e][-a]" << std::endl
            << "where -h -- this help" << std::endl
            << "      -q -- decrease verbosity" << std::endl
            << "      -f -- force failure of all test cases to confirm reporting" << std::endl
            << "      -e -- exit after first failure (used when testing code changes)" << std::endl
            << "      -a -- enable assert test of invalid TimeStamp, will cause coredump!" << std::endl
            << std::endl;
        exit(0);
    }

    std::cout << "Use -h to see command line options" << std::endl;

    std::cout << std::endl << "Starting unit test for TimeStamp classes..." << std::endl;

    int testno = 0;
    char *str = 0;
    char *exp = 0;

    // TimeStamp 7-component constructor and accessors
    testno = 1;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp 7-component constructor and accessors" << std::endl;
    TimeStamp t1 = TimeStamp(2007,10,22,17,27,27,123456);
    CHECK_EXPR1( t1.year() == 2007 );
    CHECK_EXPR1( t1.month_of_year() == 10 );
    CHECK_EXPR1( t1.day_of_month() == 22 );
    CHECK_EXPR1( t1.month_of_year() == 10 );
    CHECK_EXPR1( t1.day_of_month() == 22 );
    CHECK_EXPR1( t1.hour_of_day() == 17 );
    CHECK_EXPR1( t1.minute_of_hour() == 27 );
    CHECK_EXPR1( t1.second_of_minute() == 27 );
    CHECK_EXPR1( t1.u_seconds() == 123456 );

    // TimeStamp day_of_year and fractional second accessors
    testno = 2;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp day_of_year and fractional second accessors" << std::endl;
    CHECK_EXPR1( t1.day_of_year() == 295 );
    CHECK_EXPR1( t1.milliseconds() == 123 );
    CHECK_EXPR1( t1.tenth_milliseconds() == 1234 );

    // TimeStamp seconds accessor for True Epoch Time
    testno = 3;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp seconds accessor for True Epoch Time" << std::endl;
    CHECK_EXPR1( t1.seconds(TRUE_EPOCH_TIME) == 1193074070 );

    // TimeStamp seconds accessor for Nominal Epoch Time
    testno = 4;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp seconds accessor for Nominal Epoch Time" << std::endl;
    CHECK_EXPR1( t1.seconds(NOMINAL_EPOCH_TIME) == 1193074047 );

    // TimeStamp ts_as_double accessor for True Epoch Time
    testno = 5;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp ts_as_double accessor for True Epoch Time" << std::endl;
    // Need "close-to" test for flointing-point values?
    CHECK_EXPR1( t1.ts_as_double(TRUE_EPOCH_TIME) == 1193074070.123456 );

    // TimeStamp ts_as_double accessor for Nominal Epoch Time
    testno = 6;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp ts_as_double accessor for Nominal Epoch Time" << std::endl;
    // Need "close-to" test for flointing-point values?
    CHECK_EXPR1( t1.ts_as_double(NOMINAL_EPOCH_TIME) == 1193074047.123456 );

    // TimeStamp 6-component constructor and equals operator
    testno = 7;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp 6-component constructor and equals operator" << std::endl;
    TimeStamp t2 = TimeStamp(2007,295,17,27,27,123456);
    CHECK_EXPR1( t1 == t2 );

    // TimeStamp less-than operator
    testno = 8;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp less-than operator" << std::endl;
    t1 = TimeStamp(1996,1,1,12,30,29,123000);
    CHECK_EXPR1( t1 < t2 );
    CHECK_EXPR1( ! (t2 < t2) );

    // TimeStamp less-than-or-equal operator
    testno = 9;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp less-than-or-equal operator" << std::endl;
    CHECK_EXPR1( t1 <= t2 );
    CHECK_EXPR1( t1 <= t1 );

    // TimeStamp greater-than operator
    testno = 10;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp greater-than operator" << std::endl;
    CHECK_EXPR1( t2 > t1 );
    CHECK_EXPR1( ! (t1 > t2) );

    // TimeStamp greater-than-or-equal operator
    testno = 11;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp greater-than-or-equal operator" << std::endl;
    CHECK_EXPR1( t2 >= t1 );
    CHECK_EXPR1( t1 >= t1 );

    // TimeStamp output stream operator
    testno = 12;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp output stream operator" << std::endl;
    std::ostringstream ost;
    ost << t1;
    str = strdup(ost.str().c_str());
    exp = (char*)"1996/01/01,12:30:29.1230";
    CHECK_EXPR( strcmp(str, exp) == 0, "ost << t1", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // TimeStamp input stream operator
    testno = 13;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp input stream operator" << std::endl;
    std::string input;
    std::stringstream ss;
    TimeStamp tin;

    input = "2019/09/04,15:16:17.1234"; exp = strdup(input.c_str());
    ss.str(input); ss >> tin;
    ost.str(""); if (!!tin) ost << tin; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, input + " >> timestamp", " -- got '" << str << "' expected '" << exp << "'");

    input = "2019-01-24,15:16:17.1819"; exp = (char*)"2019/01/24,15:16:17.1819";
    ss.str(input); ss.seekg(0); ss >> tin;
    ost.str(""); if (!!tin) ost << tin; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, input + " >> timestamp", " -- got '" << str << "' expected '" << exp << "'");

    input = "2009-08-07,06:05:04"; exp = (char*)"2009/08/07,06:05:04.0000";
    ss.str(input); ss.seekg(0); ss >> tin;
    ost.str(""); if (!!tin) ost << tin; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, input + " >> timestamp", " -- got '" << str << "' expected '" << exp << "'");

    input = "2001/02/03,04:05:06"; exp = (char*)"2001/02/03,04:05:06.0000";
    ss.str(input); ss.seekg(0); ss >> tin;
    ost.str(""); if (!!tin) ost << tin; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, input + " >> timestamp", " -- got '" << str << "' expected '" << exp << "'");

    input = "1999/06/30"; exp = (char*)"1999/06/30,00:00:00.0000";
    ss.str(input); ss.seekg(0); ss >> tin;
    ost.str(""); if (!!tin) ost << tin; str = strdup(ost.str().c_str());
#if 1
    CHECK_EXPR( !tin , "!(" + input + " >> timestamp)", " -- Parsing of date without time not supported at this time" );
#else
    CHECK_EXPR( strcmp(str, exp) == 0, input + " >> timestamp", " -- got '" << str << "' expected '" << exp << "'");
#endif
    input = "not-a-timestamp"; exp = (char*)"";
    ss.str(input); ss.seekg(0); ss >> tin;
    ost.str(""); if (!!tin) ost << tin; str = strdup(ost.str().c_str());
    CHECK_EXPR( !tin , "!('" + input + "' >> timestamp)", " -- check parsing of invalid timestamp" );

    // ascending time (unix, TimeStamp, unix)
    testno = 14;
    std::cout << std::endl << "Test #" << testno << ": ascending check" << std::endl;
    struct timeval before_tv, after_tv;
    // unused -- int res;
    struct tm *tm_p;

    /*res =*/ gettimeofday(&before_tv, NULL);
    usleep(100);
    TimeStamp today = TimeStamp::current_time();
    usleep(100);
    /*res =*/ gettimeofday(&after_tv, NULL);

    std::cout << "The following three times should be in ascending order" 
        << std::endl;
    std::cout << "and within milliseconds of each other (or identical):"
        << std::endl;
    tm_p = gmtime(&before_tv.tv_sec);
    std::cout << Compat::Form("before:    %04d/%02d/%02d,%02d:%02d:%02d.%04d",
            tm_p->tm_year + 1900, tm_p->tm_mon + 1,
            tm_p->tm_mday, tm_p->tm_hour,
            tm_p->tm_min, tm_p->tm_sec, 
            before_tv.tv_usec / 100)
        << std::endl;
    std::cout << "TimeStamp: " << today << std::endl;    
    tm_p = gmtime(&after_tv.tv_sec);
    std::cout << Compat::Form("after:     %04d/%02d/%02d,%02d:%02d:%02d.%04d",
            tm_p->tm_year + 1900, tm_p->tm_mon + 1,
            tm_p->tm_mday, tm_p->tm_hour,
            tm_p->tm_min, tm_p->tm_sec, 
            after_tv.tv_usec / 100)
        << std::endl;
    CHECK_EXPR1( before_tv.tv_usec < after_tv.tv_usec );

    // TimeStamp difference -> Duration
    testno = 15;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp difference -> Duration" << std::endl;
    Duration d1 = t2 - t1;
    ost << d1;
    str = strdup(ost.str().c_str());
    exp = (char*)"372574621.000456 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "t2 - t1", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // TimeStamp, Duration addition
    testno = 16;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp, Duration addition" << std::endl;
    Duration std_day = Duration((double)24 * 60 * 60);
    ost << (t1 + std_day);
    str = strdup(ost.str().c_str());
    exp = (char*)"1996/01/02,12:30:29.1230";
    CHECK_EXPR( strcmp(str, exp) == 0, "t1 + std_day", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // TimeStamp, Duration subtraction across leapsecond
    testno = 17;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp, Duration subtraction across leapsecond" << std::endl;
    TimeStamp t3 = t1 - std_day;
    ost << t3;
    str = strdup(ost.str().c_str());
    exp = (char*)"1995/12/31,12:30:30.1230";
    CHECK_EXPR( strcmp(str, exp) == 0, "t1 - std_day", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // TimeStamp increment with Duration
    testno = 18;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp increment with Duration" << std::endl;
    t3 += std_day;
    CHECK_EXPR1( t3 == t1 );

    // TimeStamp decrement with Duration
    testno = 19;
    std::cout << std::endl << "Test #" << testno << ": TimeStamp decrement with Duration" << std::endl;
    t3 = t2;
    t3 -= d1;
    CHECK_EXPR1( t3 == t1 );

    // Duration, TimeStamp addition
    testno = 20;
    std::cout << std::endl << "Test #" << testno << ": Duration, TimeStamp addition" << std::endl;
    ost << (std_day + t1);
    str = strdup(ost.str().c_str());
    exp = (char*)"1996/01/02,12:30:29.1230";
    CHECK_EXPR( strcmp(str, exp) == 0, "std_day + t1", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration constructor with negative length
    testno = 21;
    std::cout << std::endl << "Test #" << testno << ": Duration constructor with negative length" << std::endl;
    Duration d2(-2.5);
    ost << (d2);
    str = strdup(ost.str().c_str());
    exp = (char*)"-2.500000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d2(-2.5)", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration addition
    testno = 22;
    std::cout << std::endl << "Test #" << testno << ": Duration addition" << std::endl;
    ost << (d2 + d2);
    str = strdup(ost.str().c_str());
    exp = (char*)"-5.000000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d2 + d2", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration subtraction
    testno = 23;
    std::cout << std::endl << "Test #" << testno << ": Duration subtraction" << std::endl;
    ost << (std_day - d2);
    str = strdup(ost.str().c_str());
    exp = (char*)"86402.500000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "std_day - d2", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration multiplication
    testno = 24;
    std::cout << std::endl << "Test #" << testno << ": Duration multiplication" << std::endl;
    ost << (d2 * 4);
    str = strdup(ost.str().c_str());
    exp = (char*)"-10.000000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d2 * 4", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration increment
    testno = 25;
    std::cout << std::endl << "Test #" << testno << ": Duration increment" << std::endl;
    d2 += (d2 * -2);
    ost << d2;
    str = strdup(ost.str().c_str());
    exp = (char*)"2.500000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d2 += (d2 * -2)", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration decrement
    testno = 26;
    std::cout << std::endl << "Test #" << testno << ": Duration decrement" << std::endl;
    d2 -= d2;
    ost << d2;
    str = strdup(ost.str().c_str());
    exp = (char*)"0.000000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d2 -= d2", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration equals operator
    testno = 27;
    std::cout << std::endl << "Test #" << testno << ": Duration equals operator" << std::endl;
    CHECK_EXPR1( d1 == d1 );
    CHECK_EXPR1( ! (d1 == std_day) );

    // Duration less-than operator
    testno = 28;
    std::cout << std::endl << "Test #" << testno << ": Duration less-than operator" << std::endl;
    CHECK_EXPR1( d2 < d1 );
    CHECK_EXPR1( ! (d1 < std_day) );

    // Duration less-than-or-equal operator
    testno = 29;
    std::cout << std::endl << "Test #" << testno << ": Duration less-than-or-equal operator" << std::endl;
    CHECK_EXPR1( d2 <= d1 );
    CHECK_EXPR1( d1 <= d1 );
    CHECK_EXPR1( ! (d1 <= d2) );

    // Duration greater-than operator
    testno = 30;
    std::cout << std::endl << "Test #" << testno << ": Duration greater-than operator" << std::endl;
    CHECK_EXPR1( !(d2 > d1) );
    CHECK_EXPR1( d1 > std_day );

    // Duration greater-than-or-equal operator
    testno = 31;
    std::cout << std::endl << "Test #" << testno << ": Duration greater-than-or-equal operator" << std::endl;
    CHECK_EXPR1( d1 >= d2 );
    CHECK_EXPR1( d1 >= d1 );
    CHECK_EXPR1( ! (d2 >= d1) );

    // Duration double operator
    testno = 32;
    std::cout << std::endl << "Test #" << testno << ": Duration double operator" << std::endl;
    CHECK_EXPR1( (double)d1 == 372574621.000456 );
    CHECK_EXPR1( (double)d2 == 0.0 );
    CHECK_EXPR1( (double)std_day == 86400.0 );

    // TimeWindow constructor from two TimeStamps
    testno = 33;
    std::cout << std::endl << "Test #" << testno << ": TimeWindow constructor from two TimeStamps" << std::endl;
    TimeWindow twin1(TimeStamp(2005,12,31,12,30,0,0),
            TimeStamp(2006,1,1,12,30,0,0));
    ost << twin1;
    str = strdup(ost.str().c_str());
    exp = (char*)"2005/12/31,12:30:00.0000->2006/01/01,12:30:00.0000";
    CHECK_EXPR( strcmp(str, exp) == 0, "TimeWindow(TimeStamp,TimeStamp)", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // TimeWindow constructor from TimeStamp and Duration across leap second
    testno = 34;
    std::cout << std::endl << "Test #" << testno << ": TimeWindow constructor from TimeStamp and Duration across leap second" << std::endl;
    TimeWindow twin2(TimeStamp(2005,12,31,12,30,0,0), Duration(86401.0));
    ost << twin2;
    str = strdup(ost.str().c_str());
    exp = (char*)"2005/12/31,12:30:00.0000->2006/01/01,12:30:00.0000";
    CHECK_EXPR( strcmp(str, exp) == 0, "TimeWindow(TimeStamp,Duration)", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // TimeWindow equals operator
    testno = 35;
    std::cout << std::endl << "Test #" << testno << ": TimeWindow equals operator" << std::endl;
    TimeWindow twin3 = TimeWindow(TimeStamp(2005,12,31,12,30,0,0), std_day);
    CHECK_EXPR1( twin1 == twin2 );
    CHECK_EXPR1( ! (twin1 == twin3) );

    // TimeWindow Serialize/Unserialize
    testno = 36;
    std::cout << std::endl << "Test #" << testno << ": TimeWindow Serialize/Unserialize" << std::endl;
    char buffer[129];
    int buflen = 129;
    twin1.Serialize(buffer, buflen);
    TimeWindow twin4(buffer);
    CHECK_EXPR1( twin1 == twin4 );

    // TimeWindow Overlap - several
    testno = 37;
    std::cout << std::endl << "Test #" << testno << ": TimeWindow Overlap - several" << std::endl;
    twin2 = TimeWindow(twin1.end, Duration(5.0));
    twin3 = TimeWindow(twin1.start, Duration(10.0));
    CHECK_EXPR1( twin1.Overlap(twin3) );
    CHECK_EXPR1( ! twin1.Overlap(twin2) );

    // Ymdhms: PST output
    testno = 38;
    std::cout << std::endl << "Test #" << testno << ": Ymdhms: PST output" << std::endl;
    struct ymdhms utc_time;
    struct ymdhms pst8pdt_time;
    /*res =*/ year_mdhms(TimeStamp(2007,11,4,12,13,14,0), utc_time, pst8pdt_time);
    ost << Compat::Form("%3s %d, %4d   %02d:%02d:%02d %s %s",
            pst8pdt_time.month_str, pst8pdt_time.day, 
            pst8pdt_time.year, pst8pdt_time.hour, 
            pst8pdt_time.minute, (int) pst8pdt_time.seconds,
            pst8pdt_time.am_pm, pst8pdt_time.timezone);
    str = strdup(ost.str().c_str());
    exp = (char*)"Nov 4, 2007   04:13:14 AM PST";
    CHECK_EXPR( strcmp(str, exp) == 0, "PST output", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // Ymdhms: UTC output
    testno = 39;
    std::cout << std::endl << "Test #" << testno << ": Ymdhms: UTC output" << std::endl;
    ost << Compat::Form("%3s %d, %4d   %02d:%02d:%02d    %s",
            utc_time.month_str, utc_time.day, utc_time.year, 
            utc_time.hour, utc_time.minute, 
            (int) utc_time.seconds, utc_time.timezone);
    str = strdup(ost.str().c_str());
    exp = (char*)"Nov 4, 2007   12:13:14    UTC";
    CHECK_EXPR( strcmp(str, exp) == 0, "UTC output", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // Ymdhms: PDT output
    testno = 40;
    std::cout << std::endl << "Test #" << testno << ": Ymdhms: PDT output" << std::endl;
    /*res =*/ year_mdhms(TimeStamp(2007,11,3,2,13,14,0), utc_time, pst8pdt_time);
    ost << Compat::Form("%3s %d, %4d   %02d:%02d:%02d %s %s",
            pst8pdt_time.month_str, pst8pdt_time.day, 
            pst8pdt_time.year, pst8pdt_time.hour, 
            pst8pdt_time.minute, (int) pst8pdt_time.seconds,
            pst8pdt_time.am_pm, pst8pdt_time.timezone);
    str = strdup(ost.str().c_str());
    exp = (char*)"Nov 2, 2007   07:13:14 PM PDT";
    CHECK_EXPR( strcmp(str, exp) == 0, "PDT output", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");

    // Ymdhms: UTC output
    testno = 41;
    std::cout << std::endl << "Test #" << testno << ": Ymdhms: UTC output" << std::endl;
    ost << Compat::Form("%3s %d, %4d   %02d:%02d:%02d    %s",
            utc_time.month_str, utc_time.day, utc_time.year, 
            utc_time.hour, utc_time.minute, 
            (int) utc_time.seconds, utc_time.timezone);
    str = strdup(ost.str().c_str());
    exp = (char*)"Nov 3, 2007   02:13:14    UTC";
    CHECK_EXPR( strcmp(str, exp) == 0, "UTC output", " -- got '" << str << "' expected '" << exp << "'");
    free(str);    
    ost.str("");


    // Duration multiplication
    testno = 42;
    std::cout << std::endl << "Test #" << testno << ": Duration multiplication" << std::endl;
    d1 = Duration(0.0125);  // 1/80 seconds
    d2 = d1 * -24000;  // 1/80 * -5 * 60 * 80 = -300 seconds
    ost << d2;
    str = strdup(ost.str().c_str());
    exp = (char*)"-300.000000 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d1 * -24000", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // Duration multiplication
    testno = 43;
    std::cout << std::endl << "Test #" << testno << ": Duration multiplication" << std::endl;
    d1 = Duration(0.999999);
    d2 = d1 * -1111111;
    ost << d2;
    str = strdup(ost.str().c_str());
    exp = (char*)"-1111109.888889 seconds";
    CHECK_EXPR( strcmp(str, exp) == 0, "d1 * -1111111", " -- got '" << str << "' expected '" << exp << "'");
    free(str);
    ost.str("");

    // tests for usec close to 1,000,000
    testno = 44;
    std::cout << std::endl << "Test #" << testno << ": tests for usec close to 1,000,000" << std::endl;
    t1 = TimeStamp(2007,10,22,17,27,37,999999);
    ost.str(""); ost << t1;
    std::cout << "t1 = " << ost.str() << std::endl;
    CHECK_EXPR1( t1.year() == 2007 );
    CHECK_EXPR1( t1.month_of_year() == 10 );
    CHECK_EXPR1( t1.day_of_month() == 22 );
    CHECK_EXPR1( t1.hour_of_day() == 17 );
    CHECK_EXPR1( t1.minute_of_hour() == 27 );
    CHECK_EXPR1( t1.second_of_minute() == 37 );
    CHECK_EXPR1( t1.u_seconds() == 999999 );

    testno = 45;
    std::cout << std::endl << "Test #" << testno << ": tests for usec close to 1,000,000" << std::endl;
    t1 = TimeStamp(NOMINAL_EPOCH_TIME, 1193074047.9999997);
    ost.str(""); ost << t1;
    std::cout << "t1 = " << ost.str() << std::endl;
    CHECK_EXPR1( t1.year() == 2007 );
    CHECK_EXPR1( t1.month_of_year() == 10 );
    CHECK_EXPR1( t1.day_of_month() == 22 );
    CHECK_EXPR1( t1.hour_of_day() == 17 );
    CHECK_EXPR1( t1.minute_of_hour() == 27 );
    CHECK_EXPR1( t1.second_of_minute() == 28 );
    CHECK_EXPR1( t1.u_seconds() == 0 );
    CHECK_EXPR1( t1.seconds(NOMINAL_EPOCH_TIME) == 1193074048 );

    testno = 46;
    std::cout << std::endl << "Test #" << testno << ": tests for usec close to 1,000,000" << std::endl;
    t1 = TimeStamp(TRUE_EPOCH_TIME, 1193074070.9999997);
    ost.str(""); ost << t1;
    std::cout << "t1 = " << ost.str() << std::endl;
    CHECK_EXPR1( t1.year() == 2007 );
    CHECK_EXPR1( t1.month_of_year() == 10 );
    CHECK_EXPR1( t1.day_of_month() == 22 );
    CHECK_EXPR1( t1.hour_of_day() == 17 );
    CHECK_EXPR1( t1.minute_of_hour() == 27 );
    CHECK_EXPR1( t1.second_of_minute() == 28 );
    CHECK_EXPR1( t1.u_seconds() == 0 );

    // tests for negative usecs, c/o Ivan Henson
    testno = 47; 
    std::cout << std::endl << "Test #" << testno << ": tests for negative usecs, c/o Ivan Henson" << std::endl;
    TimeStamp a, b;   
    TimeStamp current = TimeStamp::current_time();

    // make current usecs = 0
    current = TimeStamp(current.year(), current.day_of_year(), 
            current.hour_of_day(), current.minute_of_hour(), 
            current.second_of_minute(), 0);

    a = current - Duration(1.0) + Duration(0.4); // current - 0.6
    b = current - Duration(0.8);		 // current - 0.8
    CHECK_EXPR1 ( !(a < b) );

    testno = 48;
    std::cout << std::endl << "Test #" << testno << ": tests for negative usecs, c/o Ivan Henson" << std::endl;
    a = current - Duration(1.0) + Duration(0.5); // current - 0.5
    b = current - Duration(0.5);		 // current - 0.5
    CHECK_EXPR1( a == b );

    testno = 49;
    std::cout << std::endl << "Test #" << testno << ": nominal time, c/o Alexei" << std::endl;

    // caf 2019-09094 -- is {0,MAX_INT} correct?  Should it not be {0,0}?
    timeval t = {0, INT_MAX };
    exp = (char*)"1970/01/01,00:35:47.4836";
    TimeStamp zero (UNIX_TIME, t);
    std::cout << "max microseconds = 2147483647" << std::endl;
    std::cout << "zero = " << zero << std::endl;
    ost.str(""); ost << zero; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, "nominal epoch", " -- got '" << str << "' expected '" << exp << "'");

    testno = 50;
    std::cout << std::endl << "Test #" << testno << ": true epoch, c/o Alexei" << std::endl;
    TimeStamp lzero (LEAPSECOND_TIME, t);
    exp = (char*)"1970/01/01,00:35:47.4836";
    std::cout << "lzero = " << lzero << std::endl;
    ost.str(""); ost << zero; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, "true epoch", " -- got '" << str << "' expected '" << exp << "'");

    testno = 51;
    std::cout << std::endl << "Test #" << testno << ": big double, c/o Alexei" << std::endl;
    Duration dbig (t);
    exp = (char*)"2147.483647 seconds";
    ost.str(""); ost << dbig; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, "Duration dbig(t)", " -- got '" << str << "' expected '" << exp << "'");

    // caf 2019-09-04 -- changing these to expect not initialized because the constructor will not accept NAN
    testno = 52;
    std::cout << std::endl << "Test #" << testno << ": nan nominal with NAN, c/o Alexei" << std::endl;
    double d = NAN;//nan(0) ;
    TimeStamp td(UNIX_TIME,d);
    CHECK_EXPR2( !td , " -- nan nominal is not allowed");

    // caf 2019-09-04 -- changing these to expect not initialized because the constructor will not accept NAN
    testno = 53;
    std::cout << std::endl << "Test #" << testno << ": nan leap with NAN, c/o Alexei" << std::endl;
    TimeStamp tdl(LEAPSECOND_TIME,d);
    CHECK_EXPR2( !tdl , " -- nan leap is not allowed");

    testno = 54;
    std::cout << std::endl << "Test #" << testno << ": 1.0e+10 nominal seconds, c/o Alexei" << std::endl;
    double d22 = 1.0e+10;
    exp = (char*)"2286/11/20,17:46:40.0000";
    TimeStamp td2(UNIX_TIME,d22);
    ost.str(""); ost << td2; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, "1.0e+10 nominal seconds", " -- got '" << str << "' expected '" << exp << "'");

    testno = 55;
    std::cout << std::endl << "Test #" << testno << ": 1.0e+10 leap seconds (varies as more leapseconds happen)" << std::endl;
    TimeStamp tdl2(LEAPSECOND_TIME,d22);
    exp = (char*)"2286/11/20,17:46:13.0000";
    ost.str(""); ost << tdl2; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, "1.0e+10 leap seconds", " -- got '" << str << "' expected '" << exp << "'");

    testno = 56;
    std::cout << std::endl << "Test #" << testno << ": 1.0e+20 nominal seconds" << std::endl;
    double d2a = 1.0e+20;
    TimeStamp td2a(UNIX_TIME,d2a);
    CHECK_EXPR2( !td2a , " -- 1.0e+20 nominal seconds is not allowed");

    // caf 2019-09-04 -- no idea what this is suppose to do
    testno = 57;
    std::cout << std::endl << "Test #" << testno << ": setting max bounds to 1.0e+12" << std::endl;
    double min, max;
    TimeStamp::getBounds(min, max);
    max = 1.0e+12;
    TimeStamp::setBounds(min, max);

    // caf 2019-09-04 -- not sure what the answer should be
    testno = 58;
    std::cout << std::endl << "Test #" << testno << ": 1.0e+11 nom" << std::endl;
    double d2b = 1.0e+11;
    TimeStamp td2b(UNIX_TIME,d2b);
    CHECK_EXPR2( ! !td2b , " -- 1.0e+11 nominal seconds");
    exp = (char*)"5138/11/16,09:46:40.0000";
    ost.str(""); ost << td2b; str = strdup(ost.str().c_str());
    CHECK_EXPR( strcmp(str, exp) == 0, "1.0e+11 nom seconds", " -- got '" << str << "' expected '" << exp << "'");



    std::cout << std::endl << "Completing unit test for TimeStamp classes." << std::endl
        << "pass_count=" << pass_count << ", "
        << "err_count=" << err_count << ", "
        << "total_count=" << total_count 
        << std::endl;

    if ( !test_assert ) {

        std::cout << std::endl
            << "Final TimeStamp unitialized test skipped as assertion will cause program to abort." << std::endl
            << "Use -h to see command line options." << std::endl;

    } else {

        std::cout << std::endl
            << "invalid assert test is enabled..." << std::endl
            << std::endl;

        // caf 2019-09-04 -- how do you test for assert when it crashes program?
        // Uninitialized TimeStamp use; should FAIL
        std::cout << std::endl
            << "The following test should cause an assertion failure:"
            << std::endl;
        TimeStamp dead;
        std::cout << "Unitialized TimeStamp: " << dead << std::endl;

    }

    std::cout << std::endl
        << "Returning " << err_count << std::endl
        << std::endl;

    return(err_count);

} // file testtime.C
