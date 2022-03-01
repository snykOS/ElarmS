#ifndef __streambuffer_h
#define __streambuffer_h

#include <iostream>
#include <deque>
#include <vector>
#include "plog/Log.h"       // plog logging library
#include "TimeStamp.h"

using namespace std;

#define RCSID_StreamBuffer_h "$Id: StreamBuffer.h $"

const int DEFAULT_MAX_SECONDS = 60;

template <class T> class StreamBuffer {

 private:
  int samplerate;
  int maxseconds;
  deque<T> dataq;
  TimeStamp first_ts;
  TimeStamp last_ts;

  TimeStamp calcTimeOffset(TimeStamp t,int nsamp);

 public:
  StreamBuffer();
  StreamBuffer(int samprate);
  StreamBuffer(const StreamBuffer& sb);
  ~StreamBuffer();

  StreamBuffer& operator=(const StreamBuffer& sb);

  //store samples
  void setSizeInSeconds(int seconds);

  void putSamples(TimeStamp starttime, vector<T>& samples, unsigned int nsamp);
  void putSamples(TimeStamp starttime, T* samples, unsigned int nsamp);
  void getSamples(TimeStamp starttime, unsigned int nsamp, vector<T>& samples) throw(int); 
  bool checkAvailability(TimeStamp starttime, unsigned int nsamp);
  void clear();
  int size();

  TimeStamp first_timestamp() const;
  TimeStamp last_timestamp() const;

#if 1
  // intended for debug
  friend std::ostream& operator<<(std::ostream& os, const StreamBuffer<T>& sb) {
      os << "samplerate=" << sb.samplerate << ",maxseconds=" << sb.maxseconds << ",size=" << sb.dataq.size();
      os << ",first_ts=" << sb.first_ts << ",last_ts=" << sb.last_ts;
      return os;
  }
#endif

};

template <class T> StreamBuffer<T>::StreamBuffer(){
  first_ts = last_ts = TimeStamp(UNIX_TIME, 0.0);
  samplerate = 0;
  maxseconds = DEFAULT_MAX_SECONDS;
}


template <class T> StreamBuffer<T>::StreamBuffer(int samprate){
  samplerate = samprate;
  maxseconds = DEFAULT_MAX_SECONDS;
}

template <class T> StreamBuffer<T>::~StreamBuffer(){

}

template <class T> void StreamBuffer<T>::setSizeInSeconds(int seconds){
  maxseconds = seconds;
}

template <class T> StreamBuffer<T>::StreamBuffer(const StreamBuffer<T>& sb){
  samplerate = sb.samplerate;
  maxseconds = sb.maxseconds;

  dataq.clear();
  //dataq = sb.dataq;
}

template <class T> StreamBuffer<T>& StreamBuffer<T>::operator=(const StreamBuffer<T>& sb){
  samplerate = sb.samplerate;
  maxseconds = sb.maxseconds;

  dataq.clear();
  //  dataq = sb.dataq;
  return (*this);
}

template <class T> void StreamBuffer<T>::putSamples(TimeStamp starttime, vector<T>& samples, unsigned int nsamp){
    if (samples.size() == 0 || nsamp == 0) {
        return;
    }

    int dt = (float)1.0/(samplerate*1.0)*1000; 
    // int nsamp = samples.size();
    int timediff = 0;

    if(dataq.size()==0){
        // LOGD <<"FIRST: "<<first_ts<<endl;
        first_ts = starttime;
        last_ts = starttime;
    }
  
    timediff = (double)(starttime - last_ts)*1000;

    if( timediff > dt){
        dataq.clear();
        first_ts = starttime;
    }

    last_ts = calcTimeOffset(starttime, nsamp-1);

    int rmcount = 0;
    unsigned int maxq = maxseconds * samplerate;
    for (unsigned int j = 0; j < nsamp; j++) {
        dataq.push_back(samples[j]);
        if (dataq.size() > maxq) {
            dataq.pop_front();
            rmcount++;
        }
    }

    first_ts = calcTimeOffset(first_ts,rmcount);
    // LOGD <<"In putSamples, first_ts:"<<first_ts<<" last_ts"<<last_ts<<" maxq:"<<maxq<<" dataq.size:"<<dataq.size()<<" rmcount:"<<rmcount<<" starttime "<<starttime<<endl;
}

template <class T> void StreamBuffer<T>::putSamples(TimeStamp starttime, T* samples, unsigned int nsamp){
  if(nsamp == 0 || samples == NULL){
    return;
  }

  int dt = (float)1.0/(samplerate*1.0)*1000;
  int timediff = 0;

  //  LOGD <<"In putSamples (BEFORE), first_ts:"<<first_ts<<" last_ts"<<last_ts<<" starttime"<<starttime<<" timediff = "<<timediff<<" dt:"<<dt<<" "endl;
 

  if(dataq.size()==0){
    //   LOGD <<"FIRST ";
    first_ts = starttime;
    last_ts = starttime;
  }
  
  timediff = (double)(starttime - last_ts)*1000;

  if( timediff > dt){
    //   LOGD <<"GAP while ";
    dataq.clear();
    first_ts = starttime;
  }

  last_ts = calcTimeOffset(starttime, nsamp-1);

  //  LOGD <<"In putSamples, first_ts:"<<first_ts<<" last_ts:"<<last_ts<<endl;
 
  int rmcount = 0;
  int maxq = maxseconds * samplerate;
  for(int j = 0; j < nsamp; j++) {
    dataq.push_back(samples[j]);
    if(dataq.size() > maxq) {
      dataq.pop_front();
      rmcount++;
    }
  }

  first_ts = calcTimeOffset(first_ts,rmcount);
  //LOGD <<"In putSamples, first_ts:"<<first_ts<<" last_ts"<<last_ts<<" maxq:"<<maxq<<" dataq.size:"<<dataq.size()<<" rmcount:"<<rmcount<<endl;
}

template <class T> bool StreamBuffer<T>::checkAvailability(TimeStamp starttime, unsigned int nsamp){
  if(dataq.size()==0){
     // LOGD <<"In getSamples: Data Not Available"<<endl;
     return false;
  }

  TimeStamp endtime = calcTimeOffset(starttime,nsamp-1);

  // LOGD <<"In getSamples, first_ts:"<<first_ts<<" last_ts"<<last_ts<<" starttime:"<<starttime<<" endtime:"<<endtime<<endl;  
  if(starttime < first_ts || starttime > last_ts || endtime < first_ts || endtime > last_ts){
     // LOGD <<"In getSamples: Data Times "<<first_ts<<" "<<starttime<<" "<<last_ts<<" "<<endtime<<endl;
     return false;
  }
  
  return true;
}

template <class T> void StreamBuffer<T>::getSamples(TimeStamp starttime, unsigned int nsamp, vector<T>& samples) throw(int){
  if(checkAvailability(starttime,nsamp)==false){
    // LOGD <<"In getSamples: Data Not Available"<<endl;
    throw -1000;
  }

  float dt = (float)1.0/(samplerate*1.0);
  int start_index = ((double)(starttime - first_ts))/dt+0.5;
  // LOGD <<"In getSamples: Start Index:"<<start_index<<" Size:"<<nsamp<<endl;
  
  if((start_index+nsamp-1) > (dataq.size()-1)){
      LOGF <<"ERROR: Index out of bound in StreamBuffer::getSamples() at A"<<endl;
      throw -1001;
  }

  for (unsigned int i=start_index,j=0;j<nsamp;i++,j++){
    if(i > (dataq.size()-1)){
      LOGF <<"ERROR: Index out of bound in StreamBuffer::getSamples() at B"<<endl;
      throw -1002;
    }
    samples[j] = dataq[i];
    //  LOGD <<dataq[i]<<",";
  }
//  LOGD <<endl;
  // LOGD <<"============"<<endl;
}


template <class T> void StreamBuffer<T>::clear(){
  dataq.clear();
}


template <class T> int StreamBuffer<T>::size(){
  return dataq.size();
}

template <class T> TimeStamp StreamBuffer<T>::calcTimeOffset(TimeStamp t,int nsamp){
  float dt = (float)1.0/(samplerate*1.0);
  float offset = dt*nsamp;
  struct timeval time_offset;
  time_offset.tv_sec = 0;
  time_offset.tv_usec = offset*1000000;
  return (t + Duration(time_offset));  
}

#endif
