/***********************************************************

File Name :
	GenLimits.h

Programmer:
	Patrick Small

Description:
	These are general limit declarations defined here for use 
	by the TriNet software.

Creation Date:
	02 July 1998

Modification History:
        31 Jan 2017 DSN
                MAXSTR from 256 to 1024.

Usage Notes:


**********************************************************/
#ifndef GenLimits_H
#define GenLimits_H

// Default maximum length of a character string
//
const int MAXSTR = 1024;


// Number of days in each calendar month
//
const int daysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#endif
