/*****************************************************************************

    Copyright ©2017. 
    The Regents of the University of California (Regents). 
    Authors: Berkeley Seismological Laboratory. All Rights Reserved. 
    Permission to use, copy, modify, and distribute this software
    and its documentation for educational, research, and not-for-profit
    purposes, without fee and without a signed licensing agreement, is hereby
    granted, provided that the above copyright notice, this paragraph and the
    following four paragraphs appear in all copies, modifications, and
    distributions. Contact The Office of Technology Licensing, UC Berkeley,
    2150 Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201,
    otl@berkeley.edu, http://ipira.berkeley.edu/industry-info for commercial
    licensing opportunities.

    Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

    Attribution Rights. You must retain, in the Source Code of any Derivative
    Works that You create, all copyright, patent, or trademark notices from
    the Source Code of the Original Work, as well as any notices of licensing
    and any descriptive text identified therein as an "Attribution Notice."
    You must cause the Source Code for any Derivative Works that You create to
    carry a prominent Attribution Notice reasonably calculated to inform
    recipients that You have modified the Original Work.

    IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
    SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
    ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
    REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
    HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
    MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*****************************************************************************/
#ifndef __CoreEventInfo_h
#define __CoreEventInfo_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <list>
#include <string>
#include <iomanip>
#include <time.h>

using namespace std;

/** @defgroup dm_lib USGS ShakeAlert Messaging Library
 *  \mainpage dm_lib_page USGS ShakeAlert Messaging Library
 *  This is the message handling API for the CISN EEW System. It is used to
 *  communicate event detection information between components of the system. It
 *  utilizes the Apache ActiveMQ open source messaging software for the actual data
 *  transmission and the Apache Xerces C++ open source API for encoding and
 *  decoding XML formatted messages. The Xerces DOMDocument class is used to
 *  encode XML messages and XercesDOMParser class is used to decode them.
 *
 *  @section dm_lib_classes Library Classes
 *
 *  This \link dm_lib Messaging Library API\endlink supports string messages and C++ object
 *  messages that consist of a predefined set of event detection attributes. The base
 *  class for messages, CoreEventInfo, is subclassed by the ElarmsMessage, OnSiteMessage,
 *  VSMessage, and DMMessage classes. The DMMessageSender class accepts these four message
 *  classes and encodes them into XML before sending them via the ActiveMQ
 *  interface. The DMMessageReceiver class receives XML messages via ActiveMQ and
 *  parses them with the DMMessageDecoder class before returning them to the
 *  calling routine as one of the four message objects.
 *
 *  The library also includes three classes for sending, receiving, and forwarding
 *  "heartbeat" messages that can be used to determine the live programs in the
 *  system. These classes are called HBProducer, HBConsumer, and HBForward.
 *
 *  The remaining classes in the library, StaNetChanLoc, PG_Info, PGAObservation,
 *  PGVObservation, and PGVPrediction are for setting and accessing message attributes.
 *
 *  @section modifications Library Modifications
 *  The REAME files for library modifications are listed here.
 *
 *  REAME_6.3.11 \ref README_060311
 *
 *
 *  @section messages Message Types
 *  There are five types of XML-formatted messages used by the library.
 *
 *  \ref elarms_page An ELARMS message is sent to the Decision Module
 *
 *  \ref vs_page A Virtual Seismologist message is sent to the Decision Module
 *
 *  \ref onsite_page An OnSite message is sent to the Decision Module
 *
 *  \ref dm_page A Decision Module Message is sent to user programs such as UserDisplay
 *
 *  \ref hb_page A Heartbeat message is periodically sent by all system programs
 *
 *
 *  @section examples Example Code
 *  \ref test_receiver_example
 *
 *  \ref test_alg_receiver_example
 *
 *  \ref elarms_sender_example
 *
 *  \ref vs_sender_example
 *
 *  \ref onsite_sender_example
 *
 *  \ref dm_sender_example
 *
 *  \ref receiver_example
 *
 *  \ref hb_receiver_example
 *
 *  \example send_elarms.cpp
 *  This elarms XML message was produced by the send_elarms.cpp program shown below the XML.
 *  \include elarms.xml
 *  \example send_onsite.cpp
 *  This onsite XML message was produced by the send_onsite.cpp program shown below the XML.
 *  \include onsite.xml
 *  \example send_vs.cpp
 *  This vs XML message was produced by the send_vs.cpp program shown below the XML.
 *  \include vs.xml
 *  \example send_dm.cpp
 *  This dm XML message was produced by the send_dm.cpp program shown below the XML.
 *  \include dm.xml
 *  \example hb_receiver.cpp
 *  This example heartbeat XML message was received by the hb_receiver.cpp program shown below.
 *  \include hb.xml
 *  \example receiver.cpp
 *  This example program receives all messages that are sent to the indicated destination.
 *  If the message type is ElarmsMessage, OnSiteMessage, VSMessage, or DMMessage,
 *  the receive() function returns a CoreEventInfo object. The call to cei->coutPrint() prints
 *  the values of the object's members to standard output.
 *
 *  \include send_elarms.cpp
 *  The program above produces the following ELARMS XML message. The <b>pgv_obs</b> node will not
 *  be present when there are no velocity observations. Similarly, the <b>pga_obs</b> node
 *  will not be present when there are no acceleration observations. The <b>elarms_info</b> node
 *  will not be present when there are no observations of either type.
 *  \include elarms.xml
 *
 *  \page test_receiver_example Decision Module Message Receiver
 *  The following program receives all of the event messages sent by the Decision Module. The event
 *  XML message is printed to standard output. This XML message is parsed into an DMMessage object,
 *  whose member values can also be printed to verify the parsing.
 *  \include test_receiver.cpp
 *
 *  \page test_alg_receiver_example Algorithm Message Receiver (Decision Module Input Messages)
 *  The following program receives all of the algorithm messages sent to the Decision Module. The
 *  algorithm XML message is printed to standard output. This XML message is parsed into a CoreEventInfo
 *  subclass object, which is an ElarmsMessage, OnSiteMessage, VSMessage, or DMMessage object.
 *  Actually, there should not be any messages of type DM sent to the destination "eew.alg.*.data".
 *  The object member values are also printed to standard output to verify the parsing.
 *  \include test_alg_receiver.cpp
 *
 *  \page hb_receiver_example DMMessage and Heartbeat Receiver
 *  The following example program uses the DMMessageReceiver class to receive DMMessage objects
 *  that are sent by the Decision Module. It also receives heartbeat messages that are sent and
 *  forwarded by the Decision Module. This program is compiled in the examples directory from
 *  hb_receiver.cpp.
 *  \include hb_receiver.cpp
 *
 *  \page receiver_example Algorithm Message Receiver
 *  The following example program DMMessageReceiver class to receive algorithm event messages.
 *  These messages are sent by the Elarms, Onsite, and Virtual Seismologist programs to the
 *  Decision Module program. This program is compiled in the examples directory from
 *  receiver.cpp.
 *  \include receiver.cpp
 *
 *  \page elarms_sender_example Elarms Message Sender with Heartbeats
 *  The following example program builds and sends an example Elarms message.
 *  This program is compiled in the examples directory from sender_elarms.cpp.
 *  \include send_elarms.cpp
 *  This is the message produced by send_elarms.cpp program shown above.
 *  \include elarms.xml
 *
 *  \page vs_sender_example Virtual Seismologist Message Sender with Heartbeats
 *  The following example program builds and sends an example VS message.
 *  This program is compiled in the examples directory from sender_vs.cpp.
 *  \include send_vs.cpp
 *  This is the message produced by send_vs.cpp program shown above.
 *  \include vs.xml
 *
 *  \page onsite_sender_example Onsite Message Sender with Heartbeats
 *  The following example program builds and sends an example OnSite message.
 *  This program is compiled in the examples directory from sender_onsite.cpp.
 *  \include send_onsite.cpp
 *  This is the message produced by send_onsite.cpp program shown above.
 *  \include onsite.xml
 *
 *  \page dm_sender_example Decision Module Message Sender
 *  The following example program builds and sends an example Decision Module message.
 *  This program is compiled in the examples directory from sender_dm.cpp.
 *  \include send_dm.cpp
 *  This is the message produced by send_dm.cpp program shown above.
 *  \include dm.xml
 *  
 *  \page README_060311 Xerces XML Parsing, Test Programs, Delete Messages, Hearbeats, doxygen
 *  \include README_6.3.11
 */
 /** @defgroup deprecated Deprecated Messages
  */
enum eewSystemName {ELARMS, ONSITE, VS, FINDER, GPSLIP, FINITE_FAULT, DM, DMREVIEW, EQINFO2GM, EPIC, BEFORES, SA};
const string eewSystemNameString[] =  { "elarms", "onsite", "vs", "finder", "gpslip", "finite_fault", "dm", "dmreview", "eqinfo2gm", "epic", "befores", "sa"};
enum nudMessageType {NEW, UPDATE, DELETE};
const string MessageTypeString[] = { "new", "update", "delete" };

enum MessageCategory {LIVE, TEST};
const string MessageCategoryString[] = { "live", "test" };

//typedef list<StaNetChanLoc>::iterator SNCL_Iter;


/** The base class for all message container classes.
 *  @ingroup dm_lib
 */
class CoreEventInfo 
{
private:
    string event_id;
    string reference_id;
    string reference_src;
    string originating_system;
    enum eewSystemName system_name;
    enum nudMessageType message_type;
    enum MessageCategory message_category;
    int version;
    double magnitude;
    double magnitude_uncertainty;
    double latitude;
    double latitude_uncertainty;
    double longitude;
    double longitude_uncertainty;
    double depth;
    double depth_uncertainty;
    double origin_time;
    double origin_time_uncertainty;
    string origin_time_string;
    double likelyhood;
    int number_stations;
    bool sent_flag;

    string timestamp;
    string algorithm_version;
    string program_instance;
    string magnitude_units;
    string magnitude_uncertainty_units;
    string latitude_units;
    string latitude_uncertainty_units;
    string longitude_units;
    string longitude_uncertainty_units;
    string depth_units;
    string depth_uncertainty_units;
    string origin_time_units;
    string origin_time_uncertainty_units;

    string log_time;

public:
    CoreEventInfo(enum eewSystemName sys_name = ELARMS,
		  string id="-1",
		  double mag=-9.9,
		  double mag_uncer=-9.9,
		  double lat=-999.9,
		  double lat_uncer=-999.9,
		  double lon=-999.9,
		  double lon_uncer=-999.9,
		  double dep=-9.9,
		  double dep_uncer=-9.9,
		  double o_time = -99999.99,
		  double o_time_uncer=-9.9,
		  double likelihood=-9.9,
		  enum nudMessageType type = NEW,
		  int ver = 0,
		  enum MessageCategory category = LIVE,
		  string time_stamp = "",
		  string alg_ver = "-",
		  string instance = "",
		  int num_stations = 0,
		  string ref_id = "-1",
		  string ref_src = "dm",
		  string orig_sys = "",
		  string mag_units = "Mw",
		  string mag_uncer_units = "Mw",
		  string lat_units = "deg",
		  string lat_uncer_units = "deg",
		  string lon_units = "deg",
		  string lon_uncer_units = "deg",
		  string dep_units = "km",
		  string dep_uncer_units = "km",
		  string o_time_units = "UTC",
		  string o_time_uncer_units = "sec"
		  );
    CoreEventInfo(enum eewSystemName sys_name = ELARMS,
		  int id=-1,
		  double mag=-9.9,
		  double mag_uncer=-9.9,
		  double lat=-999.9,
		  double lat_uncer=-999.9,
		  double lon=-999.9,
		  double lon_uncer=-999.9,
		  double dep=-9.9,
		  double dep_uncer=-9.9,
		  double o_time = -99999.99,
		  double o_time_uncer=-9.9,
		  double likelihood=-9.9,
		  enum nudMessageType type = NEW,
		  int ver = 0,
		  enum MessageCategory category = LIVE,
		  string time_stamp = "",
		  string alg_ver = "-",
		  string instance = "",
		  int num_stations = 0,
		  int ref_id = -1,
		  string ref_src = "dm",
		  string orig_sys = "",
		  string mag_units = "Mw",
		  string mag_uncer_units = "Mw",
		  string lat_units = "deg",
		  string lat_uncer_units = "deg",
		  string lon_units = "deg",
		  string lon_uncer_units = "deg",
		  string dep_units = "km",
		  string dep_uncer_units = "km",
		  string o_time_units = "UTC",
		  string o_time_uncer_units = "sec"
		  );

    virtual ~CoreEventInfo();

    /** @name Set Value Methods */
    //@{
    string setID(string id);
    int setVersion(int v);
    double setMagnitude(double mag);
    double setMagnitudeUncertainty(double mag_uncer);
    double setLatitude(double lat);
    double setLatitudeUncertainty(double lat_uncer);
    double setLongitude(double lon);
    double setLongitudeUncertainty(double lon_uncer);
    double setDepth(double dep);
    double setDepthUncertainty(double dep_uncer);
    double setOriginTime(double o_time);
    bool setOriginTime(string o_time_str);
    double setOriginTimeUncertainty(double o_time_uncer);
    double setLikelyhood(double lklyhd);
    enum eewSystemName setSystemName(enum eewSystemName sys_name);
    enum nudMessageType setMessageType(enum nudMessageType type);
    enum MessageCategory setMessageCategory(enum MessageCategory category);
    void setLogTime(string logtime) { log_time = logtime; }
    string setOrigSys(string orig_sys);
    string setTimestamp(string time_stamp);
    string setTimestamp(double epoch_time);
    string setAlgorithmVersion(string alg_ver);
    string setProgramInstance(string instance);
    int setNumberStations(int num_stations);
    string setReferenceId(string id);
    string setReferenceSrc(string src);

    //@}
    
    /** @name Set Units Methods */
    //@{
    string setMagnitudeUnits(string mag_units);
    string setMagnitudeUncertaintyUnits(string mag_uncer_units);
    string setLatitudeUnits(string lat_units);
    string setLatitudeUncertaintyUnits(string lat_uncer_units);
    string setLongitudeUnits(string lon_units);
    string setLongitudeUncertaintyUnits(string lon_uncer_units);
    string setDepthUnits(string dep_units);
    string setDepthUncertaintyUnits(string dep_uncer_units);
    string setOriginTimeUnits(string o_time_units);
    string setOriginTimeUncertaintyUnits(string o_time_uncer_units);
    //@}

    /** @name Get Value Methods */
    //@{
    string getID() const;
    int getVersion() const;
    double getMagnitude() const;
    double getMagnitudeUncertainty() const;
    double getLatitude() const;
    double getLatitudeUncertainty() const;
    double getLongitude() const;
    double getLongitudeUncertainty() const;
    double getDepth() const;
    double getDepthUncertainty() const;
    string getOriginTimeString() const;
    double getOriginTime() const;
    double getOriginTimeUncertainty() const;
    double getLikelyhood() const;
    enum eewSystemName getSystemName() const;
    enum nudMessageType getMessageType() const;
    enum MessageCategory getMessageCategory() const;
    string getSystemNameString() const;
    string getMessageTypeString() const;
    string getMessageCategoryString() const;
    string getLogTime() const { return log_time; }
    string getOrigSys() const;
    string getTimestamp() const;
    string getAlgorithmVersion() const;
    string getProgramInstance();
    int getNumberStations() const;
    string getReferenceId() const;
    string getReferenceSrc() const;

    //@}
    
    /** @name Get Units Methods */
    //@{
    string getMagnitudeUnits() const;
    string getMagnitudeUncertaintyUnits() const;
    string getLatitudeUnits() const;
    string getLatitudeUncertaintyUnits() const;
    string getLongitudeUnits() const;
    string getLongitudeUncertaintyUnits() const;
    string getDepthUnits() const;
    string getDepthUncertaintyUnits() const;
    string getOriginTimeUnits() const;
    string getOriginTimeUncertaintyUnits() const;
    //@}

    /** @name Version Control Methods
     *  Methods updateSent and updateVersion can provide for automatic incrementing
     *  of the version number when changes are made via the Set Value Methods or the
     *  Set Units Methods.
     */
    //@{
    void updateSent();
    void updateVersion();
    //@}
    virtual string updateFrom(const CoreEventInfo &);
    virtual string updateString(const CoreEventInfo &);

    static string trueEpochToString(double tepoch);
    static string nominalEpochToString(double nepoch);
    static bool stringToDouble(string &s, double *value);
    static bool stringToInt(string &s, int *value);
    static bool stringToTepoch(string otime_str, double *otime);

    void coutCore(bool long_form=true, bool new_line=true);
    virtual void coutPrint(bool long_form=false) { coutCore(long_form); }
    string toString(bool long_form=false);
    string coreToString();
    static string labelString();

    static enum eewSystemName eewSystemNameArray[];
    static enum nudMessageType nudMessageTypeArray[];
    static enum MessageCategory MessageCategoryArray[];
    static int compareTimeString(CoreEventInfo *c1, CoreEventInfo *c2);
};
#endif

