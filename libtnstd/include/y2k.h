/***********************************************************

File Name :
	y2k.h

Programmer:
	Phil Maechling

Description:
	This is a y2k routine which applies a rule to determine
	if a 2 digit year is in the 20th or 21st Century.

Creation Date:
	24 August 1998


Usage Notes:



Modification History:



**********************************************************/

#ifndef y2k_H
#define y2k_H

//
// this converts a two digit year into a 4 digit year
//

int find_four_digit_year(int );
int find_two_digit_year(int );


#endif
