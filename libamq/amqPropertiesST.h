/*
  Author: Kalpesh Solanki
 */

#ifndef __amqPropertiesST_h
#define __amqPropertiesST_h
#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include "RetCodes.h"
#include "RTException.h"

using namespace std;

typedef map<string,string> KeyValueMap;

class amqPropertiesST{
 private:

             string _domain;
             string _username;
             string _password;
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
	static amqPropertiesST *handle;

	int toInt(string) throw(RTException);
	float toFloat(string) throw(RTException);
	double toDouble(string) throw(RTException);
	bool toBool(string) throw(RTException);

	amqPropertiesST();
 public:
	static amqPropertiesST* getInstance();
	void init(string) throw(RTException);

		string getDomain() const;
		string getUsername() const;
		string getPassword() const;
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
