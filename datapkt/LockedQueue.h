#ifndef __lockedqueue_h
#define __lockedqueue_h

using namespace std;

#include <pthread.h>
#include <queue>
#include <map>

enum QUEUE_ACCESS_MODE {BLOCKED,UNBLOCKED};

template <class T> 
class LockedQueue {

 private:
  queue<T> _queue;
  pthread_mutex_t _qlock;   
  pthread_cond_t  _qcond; 
  bool mywaitflag;
  
 public:
  LockedQueue(){
    pthread_mutex_init(&_qlock,NULL);
    pthread_cond_init(&_qcond,NULL);   
    mywaitflag = 0;
  }

  ~LockedQueue(){
  }

  int push(T e,QUEUE_ACCESS_MODE mode){
    switch (mode){
    case BLOCKED:
      pthread_mutex_lock(&_qlock);
      _queue.push(e);
      if(mywaitflag==1){
	  mywaitflag = 0;
	  pthread_cond_signal(&_qcond);
      }
      pthread_mutex_unlock(&_qlock); 
      return 0;

    case UNBLOCKED:
      int errcode = pthread_mutex_trylock(&_qlock);
      if(errcode==0){
	_queue.push(e);	
	if(mywaitflag==1){
	  mywaitflag = 0;
	  pthread_cond_signal(&_qcond);
	}
	pthread_mutex_unlock(&_qlock); 
      }
      else{
	return -1;
      }
      return 0;
    };
    return 0;
  }
  
  int pop(T& e,QUEUE_ACCESS_MODE mode){
    switch (mode){
    case BLOCKED:
      pthread_mutex_lock(&_qlock);
      if(_queue.empty()){
	mywaitflag = 1; 
	pthread_cond_wait(&_qcond,&_qlock);
      }
      e = _queue.front();
      _queue.pop();
      pthread_mutex_unlock(&_qlock);
      return 0;
      
    case UNBLOCKED:
      int errcode = pthread_mutex_trylock(&_qlock);
      if(errcode == 0){
	if(_queue.empty()){
	  mywaitflag = 1; 
	  pthread_cond_wait(&_qcond,&_qlock);
	}
	e = _queue.front();
	_queue.pop();
	pthread_mutex_unlock(&_qlock);
	return 0;
      }
      else{
	return -1;
      }
    };
    return 0;
  }
 
  int size() {
    int num;
    pthread_mutex_lock(&_qlock);
    num = (int)_queue.size();
    pthread_mutex_unlock(&_qlock);
    return num;
  }
};


#endif
