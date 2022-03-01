#include "Crc.h"
#include <iostream>

//**** CRC Routines *****//
//
//  Global, unsigned long crc_table[256], the table of cyclic 
//  redundancy checksums for all 8-bit messages.
//
//  Global, int crc_table_computed, a flag recording whether 
//  crc_table has been computed.
//

using namespace std;

unsigned long crc_table[256];
bool crc_table_computed = false;

//**********************************************************************

unsigned long crc ( unsigned char* buf, int len )

//**********************************************************************
//
//  Purpose:
//
//    CRC returns the CRC of the bytes in BUF[0...LEN-1].
//
//  Discussion:
//
//    Recall that ^  is the bitwise XOR operator and that
//   0xNNN introduces a hexadecimal constant..
//
//  Reference:
//
//    G Randers-Pehrson, et al,
//    PNG (Portable Network Graphics) Specification,
//    Version 1.2, July 1999.
//
//  Modified:
//
//    09 March 2002
//
//  Parameters:
//
//    Input, unsigned char* buf, a string whose CRC is to be computed.
//
//    Input, int len, the number of characters in the string.
//
//    Output, unsigned long crc, the CRC of the string.
//
{
  return update_crc_s ( 0xffffffffL, buf, len ) ^ 0xffffffffL;
}
//**********************************************************************

void make_crc_table ( void )

//**********************************************************************
//
//  Purpose:
//
//    MAKE_CRC_TABLE makes the table for a fast CRC computation.
//
//  Discussion:
//
//    Recall that & is the bitwise AND operator, ^ is the bitwise XOR operator,
//    >> is the bitwise right shift operator, and 0xNNN introduces a 
//    hexadecimal constant.
//
//  Reference:
//
//    G Randers-Pehrson, et al,
//    PNG (Portable Network Graphics) Specification,
//    Version 1.2, July 1999.
//
//  Modified:
//
//    09 March 2002
//
{
  unsigned long c;
  int k;
  int n;

  for ( n = 0; n < 256; n++ )
  {
    c = ( unsigned long ) n;
    for ( k = 0; k < 8; k++ )
    {
      if ( c & 1 )
      {
        c = 0xedb88320L ^ ( c >> 1 );
      } 
      else
      {
        c = c >> 1;
      }
    }
    crc_table[n] = c;
  }
  crc_table_computed = true;
}
//**********************************************************************

void print_crc_table ( void )

//**********************************************************************
//
//  Purpose:
//
//    PRINT_CRC_TABLE prints the CRC table.
//
//  Modified:
//
//    25 February 2001
//
//  Author:
//
//    John Burkardt
//
{
  int n;

  if ( !crc_table_computed )
  {
    make_crc_table ( );
  }

  cout << "\n";
  cout << "CRC_TABLE\n";
  cout << "  N  dec(CRC(N))  hex(CRC(N))\n";
  cout << "\n";

  for ( n = 0; n < 256; n++ )
  {
    cout << n << " " 
      << crc_table[n] << hex << " " 
      << crc_table[n] << dec << "\n";
  }
  return;
}
//**********************************************************************

void timestamp ( void )

//**********************************************************************
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    May 31 2001 09:45:54 AM
//
//  Modified:
//
//    02 October 2003
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
{
#define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  size_t len;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  len = strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  cout << time_buffer << "\n";

  return;
#undef TIME_SIZE
}
//**********************************************************************

unsigned long update_crc_c ( unsigned long crc, unsigned char c )

//**********************************************************************
//
//  Purpose:
//
//    UPDATE_CRC_C updates a running CRC with one more character.
//
//  Discussion:
//
//    The value of CRC should have been initialized to all 1's, and the
//    transmitted value is the 1's complement of the final running CRC.
//
//    Recall that & is the bitwise AND operator, ^ is the bitwise XOR operator,
//    and >> is the bitwise right shift operator.
//
//  Reference:
//
//    G Randers-Pehrson, et al,
//    PNG (Portable Network Graphics) Specification,
//    Version 1.2, July 1999.
//
//  Parameters:
//
//    Input, unsigned long crc, the current value of the cyclic redundancy
//    checksum.
//
//    Input, unsigned char c, the next character to be processed.
//
//    Output, unsigned long update_crc_c, the updated crc.
//
{
  unsigned long crc2 = crc;

  if ( !crc_table_computed )
  {
    make_crc_table ( );
  }

  crc2 = crc_table[ ( crc2 ^ c ) & 0xff ] ^ ( crc2 >> 8 );
  
  return crc2;
}

//**********************************************************************

unsigned long update_crc_s ( unsigned long crc, unsigned char* buf, 
  int len )

//**********************************************************************
//
//  Purpose:
//
//    UPDATE_CRC_S updates a running CRC with a new string of characters.
//
//  Discussion:
//
//    The value of CRC should have been initialized to all 1's, and the
//    transmitted value is the 1's complement of the final running CRC.
//
//    Recall that & is the bitwise AND operator, ^ is the bitwise XOR operator,
//    and >> is the bitwise right shift operator.
//
//  Reference:
//
//    G Randers-Pehrson, et al,
//    PNG (Portable Network Graphics) Specification,
//    Version 1.2, July 1999.
//
//  Parameters:
//
//    Input, unsigned long crc, the current value of the cyclic redundancy
//    checksum.
//
//    Input, unsigned char* buf, the "next" string of characters to be
//    processed.
//
//    Input, int len, the number of characters in buf.
//
//    Output, unsigned long update_crc_s, the updated crc.
//
{
  unsigned long c = crc;
  int n;

  if ( !crc_table_computed )
  {
    make_crc_table ( );
  }

  for ( n = 0; n < len; n++ )
  {
    c = crc_table[ ( c ^ buf[n] ) & 0xff ] ^ ( c >> 8 );
  }
  
  return c;
}
