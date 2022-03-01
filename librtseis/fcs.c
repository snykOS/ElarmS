/***********************************************************

File Name :
	fcs.c

Programmer:
	Phil Maechling

Description:
	This is a checksum routine used by the qpager software.

Creation Date:
	9 September 1995

Modification History:


Usage Notes:


**********************************************************/

#include <sys/types.h>
#include "fcs.h"

/*
 * Compute checksum to characterize a character string
 * This is Bruce Julian's cksum program:
 */


unsigned int cksum(char* s)
{
	unsigned int sum;
	int     c;

	sum = 0;
	while ((c = *s++) != 0) {
		if (sum&0x0001)
			sum = (sum>>1) + 0x8000;
		else
			sum >>= 1;
		sum += c;
		sum &= 0xFFFF;
	}
	return sum;
}

/*
	Checksum A1	80	V  A checksum with 91 possible characters as
                   per Bruce Julian's code.  Checks columns 2 to 79.
                   The characters produced are:
                   $%&'()*+,-./0123456789:;<=>?@
                   ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`
                   abcdefghijklmnopqrstuvwxyz{|}~

     70 columns of 80 available are used by this format

The # in column 81 is an end of message that doesn't make it to the pagers.

The cksum on this format is produced by the following formula:
msg[79] = 36 + cksum(&msg[0])%91;

This, in the column numbers given in the format, does the sum on
columns 2 to 79 and puts the cksum character in column 80.

EXAMPLE:
E 019t3   NC0199410171814387+385893-1222685  4218 32 32   3 044  10  3410D    R6

*/
