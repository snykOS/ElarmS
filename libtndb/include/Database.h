/***********************************************************

File Name :
        Database.h

Original Author:
        Patrick Small

Description:


Creation Date:
        05 May 1999

Modification History:


Usage Notes:


**********************************************************/

#ifndef database_H
#define database_H
#define OTL_ORA9I 
// Various include files
#include <vector>
#include "otlv4.h"
#include "GenLimits.h"
#include "TimeStamp.h"


// Definition of a sequence number list
typedef std::vector<unsigned long> SequenceList;


class Database
{
 private:
    char dbservice[MAXSTR];
    char dbuser[MAXSTR];
    char dbpass[MAXSTR];
    int dbinterval;
    int dbretries;
    int _Connect();

    static bool isInitialized;

    //    int GetSysDate(TimeStamp& curtime);
    //    int GetSysDate(otl_datetime& lddate);
    //    void GetCurrentTime(otl_datetime& lddate);

 protected:
    bool valid;
    char dbschema[MAXSTR];
//    otl_connect dbconn;
    
    int _GetIncrement(const char *seqname, int &incrvalue);
    int _ParseRange(std::string range, unsigned long &lowval, 
		    unsigned long &highval);
    int _ParseSequence(int incrvalue, std::string seq, SequenceList &sl);


 public:

    otl_connect dbconn;
    Database();
    Database(const char *dbs, const char *dbu, const char *dbp);
    ~Database();


    bool Connect(const char *dbs, const char *dbu, const char *dbp);
    int SetSchema(const char *dbs);
    int SetRetryInterval(int dbi);
    int SetNumRetries(int dbr);
    int CheckAndReconnect();
    
    //    otl_connect* getConnection();

    bool IsConnected();
    int GetNextSequence(const char *seqtype, unsigned long &seqno);
    int GetNextNSequence(const char *seqname, int num, SequenceList &sl);
    bool operator!();
};


#endif
