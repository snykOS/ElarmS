/***********************************************************

File Name :
        Database.C

Original Author:
        Patrick Small

Description:


Creation Date:
        03 May 1999


Modification History:

        Date                 Who               What
        --------            --------------     ----------
        November 11 2003    Kalpesh Solanki    Re-written for OTL Database API


Usage Notes:


**********************************************************/

// Various include files
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include "RetCodes.h"
#include "Database.h"
#include "Duration.h"

using std::string;

// Initialization flag
bool Database::isInitialized = false;


int Database::_Connect()
{
    try{
	
	char connect_string[MAXSTR];
	sprintf(connect_string,"%s/%s@%s",dbuser,dbpass,dbservice);
	// connect to Oracle  using user/passwd@sid
	// The "1" value in the second argument sets the OTL connection to
	// auto_commit mode!
	dbconn.rlogon(connect_string,1);
    }
    catch(otl_exception& p){ // intercept OTL exceptions
	std::cout << 
	    "Error (Database::_Connect): Unable to establish database connectivity, OTL Msg:"<<p.msg 
		  << std::endl;
	return TN_FAILURE;
    }
    return TN_SUCCESS;
}


Database::Database()
{
  if (!isInitialized) {
      otl_connect::otl_initialize(); // initialize OTL environment
      isInitialized = true;
  }
  strcpy(dbservice, "");
  strcpy(dbuser, "");
  strcpy(dbpass, "");
  strcpy(dbschema, "");
  dbinterval = 60;
  dbretries = 3;

  valid = false;
}


Database::Database(const char *dbs, const char *dbu, const char *dbp){
    
    Database();

    if(dbs == NULL || dbu == NULL || dbp == NULL)
	return;

    strcpy(dbservice, dbs);
    strcpy(dbuser, dbu);
    strcpy(dbpass, dbp);
    strcpy(dbschema, "");

    dbinterval = 60;
    dbretries = 3;
    
    if(_Connect()==TN_FAILURE){    
	valid = false;
	return;
    }
    valid = true;
}


Database::~Database()
{
    if(valid)
	dbconn.logoff();
}
bool Database::Connect(const char *dbs, const char *dbu, const char *dbp){
   
    if(dbs == NULL || dbu == NULL || dbp == NULL)
        return false;

    strcpy(dbservice, dbs);
    strcpy(dbuser, dbu);
    strcpy(dbpass, dbp);
    strcpy(dbschema, "");

    dbinterval = 0;
    dbretries = 0;
   
    if(_Connect()==TN_FAILURE){
        valid = false;
        return valid;
    }
    valid = true;
    return valid;
}



int Database::_GetIncrement(const char *seqname, int &incrvalue)
{
    try{
	if(seqname==NULL)
	    return TN_FAILURE;
	
	char sqlstmt[MAXSTR*2];
	sprintf(sqlstmt," begin \
		          :rc<int,out> := sequence.getIncrement(:str1<char[160],in>); \
		          end; ");
	otl_stream seq(1,sqlstmt,dbconn);
	
	seq.set_commit(0);
	seq<<seqname;
	seq>>incrvalue;
    }
    catch(otl_exception& p){ 
	std::cout<<"_error:"<<p.msg<<std::endl;
	std::cout << 
	    "Error (Database::_GetIncrement): Unable to increament sequence "<<seqname 
		  << std::endl;
	return TN_FAILURE;
    }
    return TN_SUCCESS;
}



int Database::_ParseRange(std::string range, unsigned long &lowval, 
				 unsigned long &highval)
{
  int dashpos;
  int n = range.length();

  dashpos = range.find_first_of("-", 0);
  if ((dashpos < 0) || (dashpos > n)) {
    return(TN_FAILURE);
  }
  lowval = atoi(range.substr(0, dashpos).data());
  highval = atoi(range.substr(dashpos + 1, n - dashpos - 1).data());

  return(TN_SUCCESS);
}



int Database::_ParseSequence(int incrvalue, std::string seq, 
				    SequenceList &sl)
{
  int start;
  int stop;
  int n = seq.length();
  unsigned long i;
  int dashpos;
  std::string entry;
  unsigned long lowval;
  unsigned long highval;

  sl.clear();

  start = seq.find_first_not_of(" ");
  while ((start >= 0) && (start < n)) {
    stop = seq.find_first_of(" ", start);
    if ((stop < 0) || (stop > n)) {
      stop = n;
    }
    entry = seq.substr(start, stop - start);
    dashpos = entry.find_first_of("-", 0);
    if ((dashpos < 0) || (dashpos > entry.length())) {
      sl.insert(sl.end(), atoi(entry.data()));
    } else {
      this->_ParseRange(entry, lowval, highval);
      for (i = lowval; i <= highval; i = i + incrvalue) {
	sl.insert(sl.end(), i);
      }
    }

    start = seq.find_first_not_of(" ", stop + 1);
  }

  return(TN_SUCCESS);
}



int Database::SetSchema(const char *dbs)
{
    if(!dbs)
	return TN_FAILURE;
    strcpy(dbschema, dbs);
    return(TN_SUCCESS);
}



int Database::SetRetryInterval(int dbi)
{
  dbinterval = dbi;
  return(TN_SUCCESS);
}



int Database::SetNumRetries(int dbr)
{
  dbretries = dbr;
  return(TN_SUCCESS);
}



bool Database::IsConnected()
{
  if (valid != TN_TRUE) {
    return false;
  }
  return true;
}



int Database::GetNextSequence(const char *seqtype, unsigned long &seqno)
{
  SequenceList sl;

  if (this->GetNextNSequence(seqtype, 1, sl) != TN_SUCCESS) {
    return(TN_FAILURE);
  }
  
  seqno = sl[0];
  return(TN_SUCCESS);
}



int Database::GetNextNSequence(const char *seqname, int num, 
				      SequenceList &sl)
{
  int numleft = num;
  int numrecv = 0;
  int incrvalue;
  SequenceList parsed;
  char seqstr[1024*6];

  sl.clear();

  // Get the increment value
  if (_GetIncrement(seqname, incrvalue) != TN_SUCCESS) {
    std::cout << 
      "Error (Database::GetNextNSequence): Unable to get increment value" 
	 << std::endl;
    return(TN_FAILURE);
  }

  while (numleft > 0) {

    numrecv = numleft;
    try{
	if(seqname==NULL){
	    //print message
	    return TN_FAILURE;
	}
	
	char sqlstmt[MAXSTR*2];
	sprintf(sqlstmt," begin \
		  :rc<char[6000],out> := sequence.getNext(:str1<char[15],in>,:int1<int,inout>); \
		  end ;");
	otl_stream seq(1,sqlstmt, dbconn);
	
	seq.set_commit(0);
	seq<<seqname;
	seq<<numrecv;

	seq>>seqstr;
	seq>>numrecv;

    }
    catch(otl_exception& p){ 
	std::cout<<"_error:"<<p.msg<<std::endl;
	std::cout << 
	    "Error (Database::GetNextNSequence): Stored procedure call failed" 
		  << std::endl;
	return(TN_FAILURE);
    }

    // Verify that we received at least one sequence number
    if (numrecv == 0) {
      std::cout << "Error (Database::GetNextNSequence): Received zero sequence numbers" << std::endl;
      return(TN_FAILURE);
    }
    numleft -= numrecv;

    // Parse the sequence number list
    string seqstring = seqstr;
    this->_ParseSequence(incrvalue,seqstring,parsed);
    if (parsed.size() != numrecv) {
      std::cout << "Error (Database::GetNextNSequence): Parsed sequence list does not match number of sequence numbers" << parsed.size() << " " << numrecv << std::endl;
      return(TN_FAILURE);
    } else {
      sl.insert(sl.end(), parsed.begin(), parsed.end());
    }
  }

  return(TN_SUCCESS);
}

int Database::CheckAndReconnect(){
  
  int attempts = 0; 

  while(attempts < dbretries){
    try{
      otl_datetime lddate;
      otl_stream o(50, "select SysDate from dual", dbconn);
      o>>lddate;
      return (TN_SUCCESS);
    } catch(otl_exception& p){ 
      std::cout<<"OTL Error:"<<p.msg<<std::endl;
      std::cout <<"Error (Database::CheckAndReconnect): Database Connection Failed"<<std::endl;
      std::cout <<"Info (Database::CheckAndReconnect): Sleeping for "<<dbinterval<<" seconds"<<std::endl;
      sleep(dbinterval);
      std::cout <<"Info (Database::CheckAndReconnect): Attempting to Reconnect to the Database"<<std::endl;
      if(_Connect()==(TN_SUCCESS)){
	std::cout<<"Info (Database::CheckAndReconnect): Reconnect Successful."<<std::endl;
	return (TN_SUCCESS);
      }
      attempts++;
    }
  }
  std::cout<<"Error (Database::CheckAndReconnect): Reconnect Attempt Failed."<<std::endl;

  return (TN_FAILURE);
}

/*
int Database::GetSysDate(TimeStamp& curtime)
{
    try{
	otl_datetime lddate;
	otl_stream o(50, "select SysDate from dual", dbconn);
	o>>lddate;

	TimeStamp time(lddate.year,lddate.month,lddate.day,lddate.hour,lddate.minute,lddate.second,0);
	curtime = time;
	
    } catch(otl_exception& p){ 
	std::cout << 
	    "Error (Database::GetSysDate): Unable to get system date"
		  << std::endl;
	std::cout<<"OTL Error:"<<p.msg<<std::endl;
	return TN_FAILURE;
    }
    return TN_SUCCESS;
}

int Database::GetSysDate(otl_datetime& lddate){
    TimeStamp time;
    if(GetSysDate(time)==TN_FAILURE)
	return TN_FAILURE;
    
    lddate.year = time.year();
    lddate.month = time.month_of_year();
    lddate.day   = time.day_of_month();
    lddate.hour  = time.hour_of_day();
    lddate.minute = time.minute_of_hour();
    lddate.second = time.second_of_minute();
    
    return TN_SUCCESS;
}

void Database::GetCurrentTime(otl_datetime& lddate){
      struct timeval curtime;
      gettimeofday(&curtime,NULL);
      TimeStamp time(curtime.tv_sec,curtime.tv_usec);
      time.set_time_mode(PACIFIC_TIME_ZONE); //Converting from UTC(default) to Local time//

      lddate.year = time.year();
      lddate.month = time.month_of_year();
      lddate.day   = time.day_of_month();
      lddate.hour  = time.hour_of_day();
      lddate.minute = time.minute_of_hour();
      lddate.second = time.second_of_minute();
}
*/

bool Database::operator!()
{
    return !valid;
}

