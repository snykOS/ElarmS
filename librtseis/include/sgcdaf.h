/***********************************************************

File Name :
	sgcdaf.h

Programmer:
	Phil Maechling

Description:
	Scan for gcda ids.
	The cda and eda have a config file which identifies all the gcdas.
	However a writer, that is a version of ritcher only writer 
	to one cda (or eda).
 	This file is opened by a richter (or event detect) and contains
	the ids used to identify the cda (or eda) to which is should write.

Creation Date:
	17 September 1996

Usage Notes:


Modification History:
	27 August 1997
	Was cda. Now used by both cda and eda.

**********************************************************/
#ifndef scgdaf_H
#define scgdaf_H

int find_gcda_ids(char* cfg_file,
		  char* station,
		  char* channel);

#endif
