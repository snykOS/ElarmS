/*
  Author: Kalpesh Solanki
 */

#include "amqProperties.h"
#include "Configuration.h"
#include <stdio.h>
#include <errno.h>
#include "GenLimits.h"



amqProperties::amqProperties(){};


amqProperties::amqProperties(string config_filename) throw(Error){
    //Check if file exists//
    fstream file;
    file.open(config_filename.c_str(),fstream::in);
    if(file.fail()){
	throw Error("amqProperties::amqProperties: Unable to open "+config_filename);
    } 
    file.close();
 
    //Populate KeyValue Map//
    char key[MAXSTR];
    char value[MAXSTR];
    int ret=TN_EOF; 
    Configuration cfg(config_filename.c_str());
   
    while((ret=cfg.next(key,value))==TN_SUCCESS){
	if(kvm[key] != ""){
	    throw Error("amqProperties::amqProperties: Duplicate key found :"+string(key));
	}
	kvm[key] = value;
    }
    //Check for non-defaults//
    
   	if(kvm["Domain"] == ""){
		
			throw Error("amqProperties::amqProperties: Parameter Domain not found.");
				
    	}
	else{
		try{
			
					_domain = kvm["Domain"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. Domain is invalid");
		}
	    }
   	
   	if(kvm["ChannelType"] == ""){
		
			
				_channeltype = "TOPIC";
			
				
    	}
	else{
		try{
			
					_channeltype = kvm["ChannelType"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. ChannelType is invalid");
		}
	    }
   	
   	if(kvm["Server1"] == ""){
		
			throw Error("amqProperties::amqProperties: Parameter Server1 not found.");
				
    	}
	else{
		try{
			
					_server1 = kvm["Server1"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. Server1 is invalid");
		}
	    }
   	
   	if(kvm["Server2"] == ""){
		
			
				_server2 = "none";
			
				
    	}
	else{
		try{
			
					_server2 = kvm["Server2"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. Server2 is invalid");
		}
	    }
   	
   	if(kvm["Server3"] == ""){
		
			
				_server3 = "none";
			
				
    	}
	else{
		try{
			
					_server3 = kvm["Server3"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. Server3 is invalid");
		}
	    }
   	
   	if(kvm["Server4"] == ""){
		
			
				_server4 = "none";
			
				
    	}
	else{
		try{
			
					_server4 = kvm["Server4"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. Server4 is invalid");
		}
	    }
   	
   	if(kvm["Server5"] == ""){
		
			
				_server5 = "none";
			
				
    	}
	else{
		try{
			
					_server5 = kvm["Server5"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. Server5 is invalid");
		}
	    }
   	
   	if(kvm["initialReconnectDelay"] == ""){
		
			
				_initialreconnectdelay = "0";
			
				
    	}
	else{
		try{
			
					_initialreconnectdelay = kvm["initialReconnectDelay"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. initialReconnectDelay is invalid");
		}
	    }
   	
   	if(kvm["maxReconnectDelay"] == ""){
		
			
				_maxreconnectdelay = "3000";
			
				
    	}
	else{
		try{
			
					_maxreconnectdelay = kvm["maxReconnectDelay"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. maxReconnectDelay is invalid");
		}
	    }
   	
   	if(kvm["useExponentialBackOff"] == ""){
		
			
				_useexponentialbackoff = "false";
			
				
    	}
	else{
		try{
			
					_useexponentialbackoff = kvm["useExponentialBackOff"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. useExponentialBackOff is invalid");
		}
	    }
   	
   	if(kvm["maxReconnectAttempts"] == ""){
		
			
				_maxreconnectattempts = "0";
			
				
    	}
	else{
		try{
			
					_maxreconnectattempts = kvm["maxReconnectAttempts"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. maxReconnectAttempts is invalid");
		}
	    }
   	
   	if(kvm["startupMaxReconnectAttempts"] == ""){
		
			
				_startupmaxreconnectattempts = "0";
			
				
    	}
	else{
		try{
			
					_startupmaxreconnectattempts = kvm["startupMaxReconnectAttempts"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. startupMaxReconnectAttempts is invalid");
		}
	    }
   	
   	if(kvm["randomize"] == ""){
		
			
				_randomize = "false";
			
				
    	}
	else{
		try{
			
					_randomize = kvm["randomize"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. randomize is invalid");
		}
	    }
   	
   	if(kvm["backup"] == ""){
		
			
				_backup = "false";
			
				
    	}
	else{
		try{
			
					_backup = kvm["backup"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. backup is invalid");
		}
	    }
   	
   	if(kvm["backupPoolSize"] == ""){
		
			
				_backuppoolsize = "1";
			
				
    	}
	else{
		try{
			
					_backuppoolsize = kvm["backupPoolSize"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. backupPoolSize is invalid");
		}
	    }
   	
   	if(kvm["timeout"] == ""){
		
			
				_timeout = "10";
			
				
    	}
	else{
		try{
			
					_timeout = kvm["timeout"];
					
		}
		catch(Error& e){
			throw Error("Configuration file is not in a valid format. timeout is invalid");
		}
	    }
   	
}

amqProperties::amqProperties(const amqProperties& prop){
     
          _domain = prop.getDomain();
     
          _channeltype = prop.getChannelType();
     
          _server1 = prop.getServer1();
     
          _server2 = prop.getServer2();
     
          _server3 = prop.getServer3();
     
          _server4 = prop.getServer4();
     
          _server5 = prop.getServer5();
     
          _initialreconnectdelay = prop.getinitialReconnectDelay();
     
          _maxreconnectdelay = prop.getmaxReconnectDelay();
     
          _useexponentialbackoff = prop.getuseExponentialBackOff();
     
          _maxreconnectattempts = prop.getmaxReconnectAttempts();
     
          _startupmaxreconnectattempts = prop.getstartupMaxReconnectAttempts();
     
          _randomize = prop.getrandomize();
     
          _backup = prop.getbackup();
     
          _backuppoolsize = prop.getbackupPoolSize();
     
          _timeout = prop.gettimeout();
  
}

amqProperties& amqProperties::operator=(const amqProperties& prop){
     
          _domain = prop.getDomain();
     
          _channeltype = prop.getChannelType();
     
          _server1 = prop.getServer1();
     
          _server2 = prop.getServer2();
     
          _server3 = prop.getServer3();
     
          _server4 = prop.getServer4();
     
          _server5 = prop.getServer5();
     
          _initialreconnectdelay = prop.getinitialReconnectDelay();
     
          _maxreconnectdelay = prop.getmaxReconnectDelay();
     
          _useexponentialbackoff = prop.getuseExponentialBackOff();
     
          _maxreconnectattempts = prop.getmaxReconnectAttempts();
     
          _startupmaxreconnectattempts = prop.getstartupMaxReconnectAttempts();
     
          _randomize = prop.getrandomize();
     
          _backup = prop.getbackup();
     
          _backuppoolsize = prop.getbackupPoolSize();
     
          _timeout = prop.gettimeout();
  
    return (*this);
}


int amqProperties::toInt(string int_val) throw(Error){
        char val[32];
	strcpy(val,int_val.c_str());

	int v = atoi(val);
// 	if(errno == ERANGE || errno == EINVAL){
// 		throw Error("Invalid Integer");
// 	}
	
	return v;
}

float amqProperties::toFloat(string float_val) throw(Error){
        char val[32];
	strcpy(val,float_val.c_str());

	float v = atof(val);
// 	if(errno == ERANGE || errno == EINVAL){
// 		throw Error("Invalid Float");
// 	}
	return v;
}

double amqProperties::toDouble(string double_val) throw(Error){
        char val[32];
	strcpy(val,double_val.c_str());

	double v = (double)atof(val);
// 	if(errno == ERANGE || errno == EINVAL){
// 		throw Error("Invalid Double");
// 	}
	return v;
}


bool amqProperties::toBool(string bool_val) throw(Error){
	if(bool_val == "true"){
		return true; 
	}
	else if(bool_val == "false"){
		return false;
	}
	else{
		throw Error("Invalid Bool");
	}
}



string amqProperties::getDomain() const{
	return _domain;
}

string amqProperties::getChannelType() const{
	return _channeltype;
}

string amqProperties::getServer1() const{
	return _server1;
}

string amqProperties::getServer2() const{
	return _server2;
}

string amqProperties::getServer3() const{
	return _server3;
}

string amqProperties::getServer4() const{
	return _server4;
}

string amqProperties::getServer5() const{
	return _server5;
}

string amqProperties::getinitialReconnectDelay() const{
	return _initialreconnectdelay;
}

string amqProperties::getmaxReconnectDelay() const{
	return _maxreconnectdelay;
}

string amqProperties::getuseExponentialBackOff() const{
	return _useexponentialbackoff;
}

string amqProperties::getmaxReconnectAttempts() const{
	return _maxreconnectattempts;
}

string amqProperties::getstartupMaxReconnectAttempts() const{
	return _startupmaxreconnectattempts;
}

string amqProperties::getrandomize() const{
	return _randomize;
}

string amqProperties::getbackup() const{
	return _backup;
}

string amqProperties::getbackupPoolSize() const{
	return _backuppoolsize;
}

string amqProperties::gettimeout() const{
	return _timeout;
}



