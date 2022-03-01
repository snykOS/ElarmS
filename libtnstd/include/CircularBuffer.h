#ifndef __CircularBuffer_h
#define __CircularBuffer_h

template<class T> class CircularBuffer{
 private:
    int HEAD;
    int TAIL;
    T*  BUF;
    int MAX;

    inline int incr(int pos){
	return (pos + 1)%MAX;
    }

 public:
    CircularBuffer(unsigned int n){
	MAX = n;
	BUF = new T[n];
	HEAD = -1;
	TAIL = -1;
    }

    ~CircularBuffer(){
	delete [] BUF;
    }

    int Write(T& t){
	if(HEAD == -1 && TAIL == -1){
	    HEAD = 0;
	    TAIL = 0;
	    BUF[HEAD] = t;
	}
	else if(incr(HEAD) != TAIL){
	    HEAD = incr(HEAD);
	    BUF[HEAD] = t;
	}
	else{
	    return -1;
	}
	return 0;
    }
    
    int Read(T& t){
	if(HEAD == -1 && TAIL == -1){
	    return -1;
	}
	else if(HEAD == TAIL){
	    t = BUF[TAIL];
	    HEAD = -1;
	    TAIL = -1;
	}
	else{
	    t = BUF[TAIL];
	    TAIL = incr(TAIL);
	}
	return 0;
    }

    int getTotalSize(){
	return MAX;
    }

    int getFullCount(){
	int full = 0;
	//cout <<"HEAD : "<<HEAD<<" TAIL : "<<TAIL<<" MAX : "<<MAX<<endl;
	if(HEAD > TAIL){
	    full = HEAD - TAIL + 1;
	}
	else if(HEAD < TAIL){
	    full = MAX - TAIL + HEAD + 1;
	}
	else if(HEAD == TAIL){
	    full = 0;
	}
	return full;
    }

};

#endif
