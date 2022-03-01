#ifndef __readraw_h
#define __readraw_h
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef	MAX_RATE
#define MAX_RATE 200
#endif

struct onesec_pkt{
  char net[4];
  char station[16];
  char channel[16];
  char location[4];
  uint32_t rate;
  uint32_t timestamp_sec;
  uint32_t timestamp_usec;
  int32_t samples[MAX_RATE];
};


void readraw(char*,char*);
int* readraw_data(char* buf,unsigned int& sec, unsigned int& usec,unsigned int& nsamp);
void print_packet(const char* buf);
void read_chancomp(char* buf,char* net,char* sta,char* chan,char* loc);
int get_sample_rate(const char* buf);
void SwapDouble( double *data );
#endif
