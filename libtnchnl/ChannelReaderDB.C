/***********************************************************

File Name :
        ChannelReaderDB.C

Original Author:
        Patrick Small

Description:


Creation Date:
        01 July 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include "RetCodes.h"
#include "ChannelReaderDB.h"
#include "DatabaseLimits.h"


using namespace std;

// Definition of the maximum number of channels to read at a time
const int MAX_CHANNELS = 2048;



ChannelReaderDB::ChannelReaderDB() : Database()
{
}


ChannelReaderDB::ChannelReaderDB(const char *dbs, const char *dbu, 
				 const char *dbp) : Database(dbs, dbu, dbp)
{
}


ChannelReaderDB::~ChannelReaderDB()
{
}



// int ChannelReaderDB::GetChannels(const char *progname, ChannelConfigList &cl)
// {
//   ChannelConfig config;
//   Channel chan;
//   int numread;
//   int i;
//   char net[MAXSTR],sta[MAXSTR],channel[MAXSTR],configstr[MAXSTR];
//   double lat,lon,elev,samprate;

//   //OTL
//   try{
//       otl_datetime lddate;
//       GetCurrentTime(lddate);

//       // Find all configured channels for this program
//       // This assumes that the selected net-sta-channel identifiers are 
//       // foreign keys into the channel_data table, and are therefore unique.
//       otl_stream chanstream (500,"select cc.net,cc.sta,cc.seedchan,cc.config,cd.lat,cd.lon,cd.elev,cd.samprate \
//                              from config_channel cc, channel_data cd,program p      \
//                              where p.progid = cc.progid and p.name = :progname<char[16]> \
//                              and cd.net = cc.net and cd.sta = cc.sta and cd.seedchan = cc.seedchan and \
//                              cd.ondate <= :time<timestamp> and cd.offdate > :time",dbconn);

//       chanstream<<progname;
//       chanstream<<lddate;

//       while(!chanstream.eof()){
// 	  chanstream>>net;
// 	  chanstream>>sta;
// 	  chanstream>>channel;
// 	  chanstream>>configstr;
// 	  chanstream>>lat;
// 	  chanstream>>lon;
// 	  chanstream>>elev;
// 	  chanstream>>samprate;

// 	  chan = Channel(net,sta,channel);
// 	  chan.latitude = lat;
// 	  chan.longitude = lon;
// 	  chan.elevation = elev;
// 	  chan.samprate = samprate;
// 	  strcpy(config.config, configstr);
// 	  cl.insert(ChannelConfigList::value_type(chan, config));
//       }
//   }
//   catch(otl_exception& p){
//       cout<<"Error (ChannelReaderDB::GetChannels): Can't retrieve channels" << endl;
//       cout<<"  OTL Message: "<<p.msg<<endl;
//       cout<<"  sql statement: "<<p.stm_text<<endl;
//       return(TN_FAILURE);
//   }

//   return(TN_SUCCESS);
// }


// int ChannelReaderDB::GetActiveChannels(ChannelSet &cl)
// {
//   Channel chan;
//   char net[MAXSTR],sta[MAXSTR],channel[MAXSTR];

//   //OTL
//   try{
//       otl_datetime lddate;
//       GetCurrentTime(lddate);

//       otl_stream chanstream (500,"select net,sta,seedchan from channel_data where ondate <= :curtime<timestamp> \
//                                   and offdate > :curtime",dbconn);

//       chanstream<<lddate;

//       while(!chanstream.eof()){
// 	  chanstream>>net;
// 	  chanstream>>sta;
// 	  chanstream>>channel;

// 	  chan = Channel(net,sta,channel);
// 	  cl.insert(chan);
//       }
//   }
//   catch(otl_exception& p){
//       cout<<"Error (ChannelReaderDB::GetActiveChannels): Can't retrieve channels" << endl;
//       cout<<"  OTL Message: "<<p.msg<<endl;
//       cout<<"  sql statement: "<<p.stm_text<<endl;
//       return(TN_FAILURE);
//   }

//   return(TN_SUCCESS);
// }




int ChannelReaderDB::GetChannels(const char *progname, 
				      ChannelConfigList &cl)
{

  ChannelConfig config;
  Channel chan;
  int numread;
  int i;
  char net[MAXSTR],sta[MAXSTR],location[MAXSTR],channel[MAXSTR],configstr[MAXSTR];
  double lat,lon,elev,samprate;

  //OTL
  try{
      otl_stream chanstream (500,"select cc.net,cc.sta,cc.location,cc.seedchan,cc.config,cd.lat,cd.lon,cd.elev,cd.samprate \
                             from config_channel cc, channel_data cd,program p      \
                             where p.progid = cc.progid and p.name = :progname<char[16]> \
                             and cd.net = cc.net and cd.sta = cc.sta and cd.seedchan = cc.seedchan and \
                             cd.location = cc.location and cd.ondate <= cast(sys_extract_utc(systimestamp) as date) and cd.offdate > cast(sys_extract_utc(systimestamp) as date)",dbconn);

      chanstream<<progname;

      while(!chanstream.eof()){
	  chanstream>>net;
	  chanstream>>sta;
	  chanstream>>location;
	  chanstream>>channel;
	  chanstream>>configstr;
	  chanstream>>lat;
	  chanstream>>lon;
	  chanstream>>elev;
	  chanstream>>samprate;

	  chan = Channel(net,sta,mapLC(net,location,MEMORY),channel);
	  chan.latitude = lat;
	  chan.longitude = lon;
	  chan.elevation = elev;
	  chan.samprate = samprate;
	  strcpy(config.config, configstr);
	  cl.insert(ChannelConfigList::value_type(chan, config));
      }
  }
  catch(otl_exception& p){
      cout<<"Error (ChannelReaderDB::GetChannels): Can't retrieve channels" << endl;
      cout<<"  OTL Message: "<<p.msg<<endl;
      cout<<"  sql statement: "<<p.stm_text<<endl;
      return(TN_FAILURE);
  }
  return(TN_SUCCESS);
}



int ChannelReaderDB::GetActiveChannels(ChannelSet &cl)
{
  Channel chan;
  char net[MAXSTR],sta[MAXSTR],location[MAXSTR],channel[MAXSTR];

  //OTL
  try{

      otl_stream chanstream (500,"select net,sta,location,seedchan from channel_data \
                                  where ondate <= cast(sys_extract_utc(systimestamp) as date) \
                                  and offdate > cast(sys_extract_utc(systimestamp) as date)",dbconn);

      while(!chanstream.eof()){
	  chanstream>>net;
	  chanstream>>sta;
	  chanstream>>location;
	  chanstream>>channel;

	  chan = Channel(net,sta,mapLC(net,location,MEMORY),channel);
	  cl.insert(chan);
      }
  }
  catch(otl_exception& p){
      cout<<"Error (ChannelReaderDB::GetActiveChannels): Can't retrieve channels" << endl;
      cout<<"  OTL Message: "<<p.msg<<endl;
      cout<<"  sql statement: "<<p.stm_text<<endl;
      return(TN_FAILURE);
  }

  return(TN_SUCCESS);
}

