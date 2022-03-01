#ifndef ___crc_h
#define ___crc_h


//CRC Routines//
unsigned long crc ( unsigned char *buf, int len );
void make_crc_table ( void );
void print_crc_table ( void );
unsigned long update_crc_c ( unsigned long crc, unsigned char c );
unsigned long update_crc_s ( unsigned long crc, unsigned char *buf, int len );
void timestamp ( void );

#endif//___crc_h
