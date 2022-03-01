#include <math.h>
#include "readraw.h"
#include <stdio.h>
#include <stdlib.h>
#include "nscl.h"
//#include "TimeStamp.h"

//using namespace std;

#define pascal_h

#include <libclient.h>
#include <libtypes.h>
#include <libmsgs.h>
#include <libsupport.h>

void read_chancomp(char* buf,char* net,char* sta,char* chan,char* loc){
      onesec_pkt *pkt = (onesec_pkt*)(buf);

      strcpy(net,pkt->net);
      strcpy(sta,pkt->station);
      strcpy(loc,pkt->location);
      strcpy(chan,pkt->channel);
      mapLC(net,loc,MEMORY);
}

void readraw(char* buf,char* sncl){
      onesec_pkt *pkt = (onesec_pkt*)(buf);

      char net[16];
      char sta[16];
      char loc[4];

      strcpy(net,pkt->net);
      strcpy(sta,pkt->station);
      strcpy(loc,pkt->location);

      mapLC(net,loc,MEMORY);

      sprintf(sncl,"%s.%s.%s.%s",net,sta,loc,pkt->channel);
}

int* readraw_data(char* buf,unsigned int& sec, unsigned int& usec,unsigned int& nsamp){
  double janFirst2000 =  946684800.000000;
  onesec_pkt *pkt = (onesec_pkt*)(buf);
  nsamp = pkt->rate;

    for(unsigned int i=0;i<nsamp;i++){
      pkt->samples[i] = ntohl(pkt->samples[i]);
    }

  sec = ntohl(pkt->timestamp_sec);
  sec += janFirst2000;
  usec = ntohl(pkt->timestamp_usec);//(int)((timestamp - (double)(((int)timestamp)))*1000000);

  return (int*)pkt->samples;
}

void print_packet(const char* buf){
  double janFirst2000 =  946684800.000000;
  onesec_pkt *pkt = (onesec_pkt*)(buf);


  unsigned int sec = ntohl(pkt->timestamp_sec);
  sec += janFirst2000;
  //unsigned int usec = ntohl(pkt->timestamp_usec);
  //int rate = ntohl(pkt->rate);

  //printf("Number of samples in buffer : %d\n",rate);
  //printf("Got Packet for %s\n",pkt->station);
  //printf("Channel %s\n",pkt->channel);


  struct timeval time;
  gettimeofday(&time,NULL);
  //int timediff = time.tv_sec - sec;
  //printf("Delay: %d\n",timediff);

  // LOGD <<" Station: "<<pkt->station<<"-"<<rp->channel<<" TimeStamp: "<<TimeStamp(janFirst2000 + rp->timestamp)<<" Delay:"<<timediff<<" seconds"<<endl;
  //LOGD <<"           total_size: "<<rp->total_size<<" rate: "<<rp->rate<<" channel: "<<rp->channel<<endl;
  // printf("[");
  // for(int i=0;i < rate;i++){
  //  printf("%d,",ntohl(pkt->samples[i]));
  //}
  //printf("]\n");
}


void SwapDouble( double *data )
{
  char temp;

  union {
      char   c[8];
  } dat;

  memcpy( &dat, data, sizeof(double) );
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
  memcpy( data, &dat, sizeof(double) );
  return;
}

int get_sample_rate(const char* buf){
 return ntohl(((onesec_pkt*)buf)->rate);  
}
#undef pascal_h
