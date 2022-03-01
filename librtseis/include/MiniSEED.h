/***********************************************************

File Name :
        MiniSEED.h

Original Author:
        Patrick Small

Description:

        This header file defines general constants that
are required by the MiniSEEDReader and MiniSEEDWriter classes.


Creation Date:
        12 March 1999

Modification History:


Usage Notes:


**********************************************************/

#ifndef miniseed_H
#define miniseed_H

// OBSOLETE
const int MINISEEDFILE_STEIM1 = 0;
const int MINISEEDFILE_STEIM2 = 1;
const int MINISEED_STEIM1 = 1;
const int MINISEED_STEIM2 = 2;


// Supported Data formats
const int MINISEED_FORMAT_ORIGINAL = 0;
const int MINISEED_FORMAT_STEIM1 = 1;
const int MINISEED_FORMAT_STEIM2 = 2;


// Supported record sizes
const int MINISEED_REC_ORIGINAL = 0;
const int MINISEED_REC_512 = 512;
const int MINISEED_REC_4096 = 4096;


// Supported word orders
const int MINISEED_WORDORDER_LITTLE_ENDIAN = 0;
const int MINISEED_WORDORDER_BIG_ENDIAN = 1;

#endif
