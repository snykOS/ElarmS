#include <string.h>
#include "ByteSwap.h"

#if defined (BIG_ENDIAN) || defined (_BIG_ENDIAN) || defined (__BIG_ENDIAN)
# ifndef _BIG_ENDIAN
#  define _BIG_ENDIAN
# endif
#endif

#if defined (LITTLE_ENDIAN) || defined (_LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN)
# ifndef _LITTLE_ENDIAN
#  define _LITTLE_ENDIAN
# endif
#endif

#ifdef _LITTLE_ENDIAN

float htonf( float data )
{
   char temp;

   union {
      char c[4];
   } dat;

   memcpy( &dat, &data, sizeof(long) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[3];
   dat.c[3] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[2];
   dat.c[2] = temp;
   memcpy( &data, &dat, sizeof(long) );
   return(data);
}    

float ntohf(float data)
{
    return(htonf(data));
}


double htond( double data )
{
   char temp;

   union {
       char   c[8];
   } dat;

   memcpy( &dat, &data, sizeof(double) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[7];
   dat.c[7] = temp;

   temp     = dat.c[1];
   dat.c[1] = dat.c[6];
   dat.c[6] = temp;

   temp     = dat.c[2];
   dat.c[2] = dat.c[5];
   dat.c[5] = temp;

   temp     = dat.c[3];
   dat.c[3] = dat.c[4];
   dat.c[4] = temp;
   memcpy( &data, &dat, sizeof(double) );
   return(data);
}

double ntohd(double data) 
{
    return(htond(data));
}

#else /* BIG_ENDIAN */
float htonf(float data)
{
    return(data);
}

float ntohf(float data)
{
    return(data);
}
double htond(double data)
{
    return(data);
}
double ntohd(double data)
{
    return(data);
}
#endif
