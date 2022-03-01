/*
  Author: Kalpesh Solanki
 */

#ifndef __amqProperties_h
#define __amqProperties_h
#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include "RetCodes.h"
#include "Exceptions.h"

using namespace std;

typedef map<string,string> KeyValueMap;

class amqProperties{
 private:

             string _domain;
	     string _channeltype;
	     string _server1;
	     string _server2;
	     string _server3;
	     string _server4;
	     string _server5;
	     string _initialreconnectdelay;
	     string _maxreconnectdelay;
	     string _useexponentialbackoff;
	     string _maxreconnectattempts;
	     string _startupmaxreconnectattempts;
	     string _randomize;
	     string _backup;
	     string _backuppoolsize;
	     string _timeout;
	  

    	KeyValueMap kvm;

	int toInt(string) throw(Error);
	float toFloat(string) throw(Error);
	double toDouble(string) throw(Error);
	bool toBool(string) throw(Error);

 public:
	amqProperties();
	amqProperties(string) throw(Error);
	amqProperties(const amqProperties&);
	amqProperties& operator=(const amqProperties&);

		string getDomain() const;
		string getChannelType() const;
		string getServer1() const;
		string getServer2() const;
		string getServer3() const;
		string getServer4() const;
		string getServer5() const;
		string getinitialReconnectDelay() const;
		string getmaxReconnectDelay() const;
		string getuseExponentialBackOff() const;
		string getmaxReconnectAttempts() const;
		string getstartupMaxReconnectAttempts() const;
		string getrandomize() const;
		string getbackup() const;
		string getbackupPoolSize() const;
		string gettimeout() const;
	
};

#endif
