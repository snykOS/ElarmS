/*
  Author: Kalpesh Solanki
 */

#include <stdlib.h>		// atoi, atof
#include <string.h>		// strcpy

#include "amqPropertiesST.h"
#include "Configuration.h"
#include <stdio.h>
#include <errno.h>
#include "GenLimits.h"

amqPropertiesST * amqPropertiesST::handle = NULL;




amqPropertiesST::amqPropertiesST(){};


amqPropertiesST* amqPropertiesST::getInstance(){
  if(handle==NULL){
    handle = new amqPropertiesST();
  }
  return handle;
}

void amqPropertiesST::init(string config_filename) throw(RTException){
    //Check if file exists//
    fstream file;
    file.open(config_filename.c_str(),fstream::in);
    if(file.fail()){
	throw RTException("amqPropertiesST::amqPropertiesST: Unable to open "+config_filename);
    } 
    file.close();
 
    //Populate KeyValue Map//
    char key[MAXSTR];
    char value[MAXSTR];
    int ret=TN_EOF; 
    Configuration cfg(config_filename.c_str());
   
    while((ret=cfg.next(key,value))==TN_SUCCESS){
	if(kvm[key] != ""){
	    throw RTException("amqPropertiesST::amqPropertiesST: Duplicate key found :"+string(key));
	}
	kvm[key] = value;
    }
    //Check for non-defaults//
    
   	if(kvm["Domain"] == ""){
		
			throw RTException("amqPropertiesST::amqPropertiesST: Parameter Domain not found.");
				
    	}
	else{
		try{
			
					_domain = kvm["Domain"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. Domain is invalid");
		}
	    }
   	
   	if(kvm["username"] == ""){
		
			throw RTException("amqPropertiesST::amqPropertiesST: Parameter username not found.");
				
    	}
	else{
		try{
			
					_username = kvm["username"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. username is invalid");
		}
	    }
   	
   	if(kvm["password"] == ""){
		
			throw RTException("amqPropertiesST::amqPropertiesST: Parameter password not found.");
				
    	}
	else{
		try{
			
					_password = kvm["password"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. password is invalid");
		}
	    }
   	
   	if(kvm["ChannelType"] == ""){
		
			
				_channeltype = "TOPIC";
			
				
    	}
	else{
		try{
			
					_channeltype = kvm["ChannelType"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. ChannelType is invalid");
		}
	    }
   	
   	if(kvm["Server1"] == ""){
		
			throw RTException("amqPropertiesST::amqPropertiesST: Parameter Server1 not found.");
				
    	}
	else{
		try{
			
					_server1 = kvm["Server1"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. Server1 is invalid");
		}
	    }
   	
   	if(kvm["Server2"] == ""){
		
			
				_server2 = "none";
			
				
    	}
	else{
		try{
			
					_server2 = kvm["Server2"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. Server2 is invalid");
		}
	    }
   	
   	if(kvm["Server3"] == ""){
		
			
				_server3 = "none";
			
				
    	}
	else{
		try{
			
					_server3 = kvm["Server3"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. Server3 is invalid");
		}
	    }
   	
   	if(kvm["Server4"] == ""){
		
			
				_server4 = "none";
			
				
    	}
	else{
		try{
			
					_server4 = kvm["Server4"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. Server4 is invalid");
		}
	    }
   	
   	if(kvm["Server5"] == ""){
		
			
				_server5 = "none";
			
				
    	}
	else{
		try{
			
					_server5 = kvm["Server5"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. Server5 is invalid");
		}
	    }
   	
   	if(kvm["initialReconnectDelay"] == ""){
		
			
				_initialreconnectdelay = "0";
			
				
    	}
	else{
		try{
			
					_initialreconnectdelay = kvm["initialReconnectDelay"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. initialReconnectDelay is invalid");
		}
	    }
   	
   	if(kvm["maxReconnectDelay"] == ""){
		
			
				_maxreconnectdelay = "3000";
			
				
    	}
	else{
		try{
			
					_maxreconnectdelay = kvm["maxReconnectDelay"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. maxReconnectDelay is invalid");
		}
	    }
   	
   	if(kvm["useExponentialBackOff"] == ""){
		
			
				_useexponentialbackoff = "false";
			
				
    	}
	else{
		try{
			
					_useexponentialbackoff = kvm["useExponentialBackOff"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. useExponentialBackOff is invalid");
		}
	    }
   	
   	if(kvm["maxReconnectAttempts"] == ""){
		
			
				_maxreconnectattempts = "0";
			
				
    	}
	else{
		try{
			
					_maxreconnectattempts = kvm["maxReconnectAttempts"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. maxReconnectAttempts is invalid");
		}
	    }
   	
   	if(kvm["startupMaxReconnectAttempts"] == ""){
		
			
				_startupmaxreconnectattempts = "0";
			
				
    	}
	else{
		try{
			
					_startupmaxreconnectattempts = kvm["startupMaxReconnectAttempts"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. startupMaxReconnectAttempts is invalid");
		}
	    }
   	
   	if(kvm["randomize"] == ""){
		
			
				_randomize = "false";
			
				
    	}
	else{
		try{
			
					_randomize = kvm["randomize"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. randomize is invalid");
		}
	    }
   	
   	if(kvm["backup"] == ""){
		
			
				_backup = "false";
			
				
    	}
	else{
		try{
			
					_backup = kvm["backup"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. backup is invalid");
		}
	    }
   	
   	if(kvm["backupPoolSize"] == ""){
		
			
				_backuppoolsize = "1";
			
				
    	}
	else{
		try{
			
					_backuppoolsize = kvm["backupPoolSize"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. backupPoolSize is invalid");
		}
	    }
   	
   	if(kvm["timeout"] == ""){
		
			
				_timeout = "10";
			
				
    	}
	else{
		try{
			
					_timeout = kvm["timeout"];
					
		}
		catch(RTException& e){
			throw RTException("Configuration file is not in a valid format. timeout is invalid");
		}
	    }
   	
}

int amqPropertiesST::toInt(string int_val) throw(RTException){
        char val[32];
	strcpy(val,int_val.c_str());

	int v = atoi(val);
// 	if(errno == ERANGE || errno == EINVAL){
// 		throw RTException("Invalid Integer");
// 	}
	
	return v;
}

float amqPropertiesST::toFloat(string float_val) throw(RTException){
        char val[32];
	strcpy(val,float_val.c_str());

	float v = atof(val);
// 	if(errno == ERANGE || errno == EINVAL){
// 		throw RTException("Invalid Float");
// 	}
	return v;
}

double amqPropertiesST::toDouble(string double_val) throw(RTException){
        char val[32];
	strcpy(val,double_val.c_str());

	double v = (double)atof(val);
// 	if(errno == ERANGE || errno == EINVAL){
// 		throw RTException("Invalid Double");
// 	}
	return v;
}


bool amqPropertiesST::toBool(string bool_val) throw(RTException){
	if(bool_val == "true"){
		return true; 
	}
	else if(bool_val == "false"){
		return false;
	}
	else{
		throw RTException("Invalid Bool");
	}
}



string amqPropertiesST::getDomain() const{
	return _domain;
}

string amqPropertiesST::getUsername() const{
	return _username;
}

string amqPropertiesST::getPassword() const{
	return _password;
}

string amqPropertiesST::getChannelType() const{
	return _channeltype;
}

string amqPropertiesST::getServer1() const{
	return _server1;
}

string amqPropertiesST::getServer2() const{
	return _server2;
}

string amqPropertiesST::getServer3() const{
	return _server3;
}

string amqPropertiesST::getServer4() const{
	return _server4;
}

string amqPropertiesST::getServer5() const{
	return _server5;
}

string amqPropertiesST::getinitialReconnectDelay() const{
	return _initialreconnectdelay;
}

string amqPropertiesST::getmaxReconnectDelay() const{
	return _maxreconnectdelay;
}

string amqPropertiesST::getuseExponentialBackOff() const{
	return _useexponentialbackoff;
}

string amqPropertiesST::getmaxReconnectAttempts() const{
	return _maxreconnectattempts;
}

string amqPropertiesST::getstartupMaxReconnectAttempts() const{
	return _startupmaxreconnectattempts;
}

string amqPropertiesST::getrandomize() const{
	return _randomize;
}

string amqPropertiesST::getbackup() const{
	return _backup;
}

string amqPropertiesST::getbackupPoolSize() const{
	return _backuppoolsize;
}

string amqPropertiesST::gettimeout() const{
	return _timeout;
}



