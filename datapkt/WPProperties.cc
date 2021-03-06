/*
 * THIS IS AN AUTOGENERATED FILE.  DO NOT MODIFY BY HAND!
 * Created by propertiesGen_new.py 0000 2019-04-30 01:33:00 claude
 * Generated 2019-10-01 19:05:43
 * Edit WPProperties.meta and rerun as follows:
 *     ../../libs/utils/propertiesGen_new.py WP
 */

/** @file WPProperties.cc This auto-generated file handles configuration file reading and checking, variable initialisation (parameter setting) and variable get functions for the WPProperties class.*/
/** @class WPProperties 
 * @brief class is auto-generated and handles configuration file parameters for the WP class and is derived from the WPProperties.meta file.*/

#include <iostream>             // std::cout
#include <sstream>              // std::ostringstream
#include <algorithm>            // std::find

// logging stuff
#include <plog/Log.h>
#include "ExternalMutexConsoleAppender.h"
#include "SeverityFormatter.h"

// autogenerated header file
#include "WPProperties.h"


const std::string RCSID_WPProperties_cc = "$Id: WPProperties.cc auto-generated 2019-10-01 19:05:43 from WPProperties.meta using propertiesGen_new.py 0000 2019-04-30 01:33:00 claude $";

WPProperties * WPProperties::handle = NULL;

/** function auto-generated and returns the handle to existing properties class, or if no class instance exists it creates a new instance and returns the handle. Expected usage is when only a single instance of this properties class is needed and therefore this function allows global access.*/
WPProperties* WPProperties::getInstance(){
	if (handle==NULL) {
		handle = new WPProperties();
	}
	return handle;
} // WPProperties::getInstance

WPProperties* WPProperties::destroyInstance(){
	if (handle!=NULL) {
		delete handle;
		handle = NULL;
	}
	return handle;
} // WPProperties::destroyInstance

/** function auto-generated and returns complete list of class variables defined in meta file. @return contents of metalist std::vector<std::string>*/
std::vector<std::string> WPProperties::getPropListNames() {
	std::vector<std::string> retList;
	for (std::set<std::string>::iterator it = metalist.begin(); it != metalist.end(); it++) {
		retList.push_back(*it);
	}
	return retList;
} // WPProperties::getPropListNames

/** function auto-generated and returns number of configurable parameters missing in configuration file (defaults used). @return _missing int*/
int WPProperties::getMissing() const {
	return missing;
}

/** function auto-generated and returns prefix/qualifier for properties class. @return m_sPrefix std::string*/
std::string WPProperties::getPrefix() const {
	return m_sPrefix;
}

void WPProperties::init(std::string filename, int argc, char* argv[]) throw(Error) {
	GetProp::init(filename, argc, argv);
	init();
	checkVars();
} // WPProperties::init

void WPProperties::init() throw(Error) {
	missing = 0;
	dups = 0;
	std::ostringstream ostrm;

	ostrm << "CONFIG: WPProperties Config file: " << getConfigFile() << " (prefix: " << m_sPrefix.substr(0,m_sPrefix.size()-1) << ")" << std::endl;

	if ( !contains(m_sPrefix + "CMSConfigFile") ) {
		throw Error(getConfigFile() + ": Parameter CMSConfigFile not found.");
	}
	else {
		try {
			_cmsconfigfile = getString(m_sPrefix + "CMSConfigFile");
			ostrm << "CONFIG:   CMSConfigFile: " << _cmsconfigfile << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter CMSConfigFile has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ChannelBlackList") ) {
		throw Error(getConfigFile() + ": Parameter ChannelBlackList not found.");
	}
	else {
		try {
			_channelblacklist = getString(m_sPrefix + "ChannelBlackList");
			ostrm << "CONFIG:   ChannelBlackList: " << _channelblacklist << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ChannelBlackList has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ChannelFile") ) {
		throw Error(getConfigFile() + ": Parameter ChannelFile not found.");
	}
	else {
		try {
			_channelfile = getString(m_sPrefix + "ChannelFile");
			ostrm << "CONFIG:   ChannelFile: " << _channelfile << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ChannelFile has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ChannelFilter") ) {
		throw Error(getConfigFile() + ": Parameter ChannelFilter not found.");
	}
	else {
		try {
			_channelfilter = getString(m_sPrefix + "ChannelFilter");
			ostrm << "CONFIG:   ChannelFilter: " << _channelfilter << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ChannelFilter has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ChannelGreyList") ) {
		throw Error(getConfigFile() + ": Parameter ChannelGreyList not found.");
	}
	else {
		try {
			_channelgreylist = getString(m_sPrefix + "ChannelGreyList");
			ostrm << "CONFIG:   ChannelGreyList: " << _channelgreylist << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ChannelGreyList has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "DoReplay") ) {
		_doreplay = false;
		ostrm << "CONFIG:   DoReplay: false using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_doreplay = getBool(m_sPrefix + "DoReplay");
			ostrm << "CONFIG:   DoReplay: " << _doreplay << " (default is false)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter DoReplay has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "EEWChannel") ) {
		throw Error(getConfigFile() + ": Parameter EEWChannel not found.");
	}
	else {
		try {
			_eewchannel = getString(m_sPrefix + "EEWChannel");
			ostrm << "CONFIG:   EEWChannel: " << _eewchannel << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter EEWChannel has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "EWConfig") ) {
		throw Error(getConfigFile() + ": Parameter EWConfig not found.");
	}
	else {
		try {
			_ewconfig = getString(m_sPrefix + "EWConfig");
			ostrm << "CONFIG:   EWConfig: " << _ewconfig << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter EWConfig has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "FLRTfeed") ) {
		_flrtfeed = false;
		ostrm << "CONFIG:   FLRTfeed: false using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_flrtfeed = getBool(m_sPrefix + "FLRTfeed");
			ostrm << "CONFIG:   FLRTfeed: " << _flrtfeed << " (default is false)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter FLRTfeed has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "FLdebug") ) {
		_fldebug = false;
		ostrm << "CONFIG:   FLdebug: false using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_fldebug = getBool(m_sPrefix + "FLdebug");
			ostrm << "CONFIG:   FLdebug: " << _fldebug << " (default is false)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter FLdebug has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "FLfile") ) {
		_flfile = "";
		ostrm << "CONFIG:   FLfile: '' using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_flfile = getString(m_sPrefix + "FLfile");
			ostrm << "CONFIG:   FLfile: " << _flfile << " (default is '')" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter FLfile has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "FLpktlen") ) {
		_flpktlen = 0;
		ostrm << "CONFIG:   FLpktlen: 0 using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_flpktlen = getInt(m_sPrefix + "FLpktlen");
			ostrm << "CONFIG:   FLpktlen: " << _flpktlen << " (default is 0)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter FLpktlen has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "IFID") ) {
		_ifid = "not_specified";
		ostrm << "CONFIG:   IFID: not_specified using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_ifid = getString(m_sPrefix + "IFID");
			ostrm << "CONFIG:   IFID: " << _ifid << " (default is not_specified)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter IFID has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "IgnoreFirstPackets") ) {
		throw Error(getConfigFile() + ": Parameter IgnoreFirstPackets not found.");
	}
	else {
		try {
			_ignorefirstpackets = getInt(m_sPrefix + "IgnoreFirstPackets");
			ostrm << "CONFIG:   IgnoreFirstPackets: " << _ignorefirstpackets << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter IgnoreFirstPackets has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "MulticastAddress") ) {
		_multicastaddress = "not_specified";
		ostrm << "CONFIG:   MulticastAddress: not_specified using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_multicastaddress = getString(m_sPrefix + "MulticastAddress");
			ostrm << "CONFIG:   MulticastAddress: " << _multicastaddress << " (default is not_specified)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter MulticastAddress has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "PacketLatencyThreshold") ) {
		_packetlatencythreshold = 0;
		ostrm << "CONFIG:   PacketLatencyThreshold: 0 using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_packetlatencythreshold = getInt(m_sPrefix + "PacketLatencyThreshold");
			ostrm << "CONFIG:   PacketLatencyThreshold: " << _packetlatencythreshold << " (default is 0)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter PacketLatencyThreshold has wrong format!");
		}
		if (_packetlatencythreshold < -3600 || _packetlatencythreshold > 3600)
			throw Error(getConfigFile() + ": Parameter PacketLatencyThreshold (" + getString(m_sPrefix + "PacketLatencyThreshold") + ") is not between -3600 and 3600");
	}

	if ( !contains(m_sPrefix + "QMAMulticastAddress") ) {
		_qmamulticastaddress = "not_specified";
		ostrm << "CONFIG:   QMAMulticastAddress: not_specified using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_qmamulticastaddress = getString(m_sPrefix + "QMAMulticastAddress");
			ostrm << "CONFIG:   QMAMulticastAddress: " << _qmamulticastaddress << " (default is not_specified)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter QMAMulticastAddress has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ReadIntervalBlackGreyList") ) {
		_readintervalblackgreylist = 60;
		ostrm << "CONFIG:   ReadIntervalBlackGreyList: 60 using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_readintervalblackgreylist = getInt(m_sPrefix + "ReadIntervalBlackGreyList");
			ostrm << "CONFIG:   ReadIntervalBlackGreyList: " << _readintervalblackgreylist << " (default is 60)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ReadIntervalBlackGreyList has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ReadIntervalWhiteList") ) {
		_readintervalwhitelist = 60;
		ostrm << "CONFIG:   ReadIntervalWhiteList: 60 using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_readintervalwhitelist = getInt(m_sPrefix + "ReadIntervalWhiteList");
			ostrm << "CONFIG:   ReadIntervalWhiteList: " << _readintervalwhitelist << " (default is 60)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ReadIntervalWhiteList has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ReplayFile") ) {
		_replayfile = "";
		ostrm << "CONFIG:   ReplayFile: '' using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_replayfile = getString(m_sPrefix + "ReplayFile");
			ostrm << "CONFIG:   ReplayFile: " << _replayfile << " (default is '')" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ReplayFile has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ReplayRealTime") ) {
		_replayrealtime = false;
		ostrm << "CONFIG:   ReplayRealTime: false using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_replayrealtime = getBool(m_sPrefix + "ReplayRealTime");
			ostrm << "CONFIG:   ReplayRealTime: " << _replayrealtime << " (default is false)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ReplayRealTime has wrong format!");
		}
	}

	if ( !contains(m_sPrefix + "ThreadCount") ) {
		throw Error(getConfigFile() + ": Parameter ThreadCount not found.");
	}
	else {
		try {
			_threadcount = getInt(m_sPrefix + "ThreadCount");
			ostrm << "CONFIG:   ThreadCount: " << _threadcount << " (field is mandatory)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter ThreadCount has wrong format!");
		}
		if (_threadcount < 1 || _threadcount > 1000)
			throw Error(getConfigFile() + ": Parameter ThreadCount (" + getString(m_sPrefix + "ThreadCount") + ") is not between 1 and 1000");
	}

	if ( !contains(m_sPrefix + "VelocityThreshold") ) {
		_velocitythreshold = 0.8;
		ostrm << "CONFIG:   VelocityThreshold: 0.8 using default value" << std::endl;
		missing++;
	}
	else {
		try {
			_velocitythreshold = (float)getDouble(m_sPrefix + "VelocityThreshold");
			ostrm << "CONFIG:   VelocityThreshold: " << _velocitythreshold << " (default is 0.8)" << std::endl;
		} catch(Error& e) {
			throw Error(getConfigFile() + ": Parameter VelocityThreshold has wrong format!");
		}
		if (_velocitythreshold < -1 || _velocitythreshold > 1000)
			throw Error(getConfigFile() + ": Parameter VelocityThreshold (" + getString(m_sPrefix + "VelocityThreshold") + ") is not between -1 and 1000");
	}

	metalist.insert(m_sPrefix + "CMSConfigFile");
	metalist.insert(m_sPrefix + "ThreadCount");
	metalist.insert(m_sPrefix + "IgnoreFirstPackets");
	metalist.insert(m_sPrefix + "FLfile");
	metalist.insert(m_sPrefix + "IFID");
	metalist.insert(m_sPrefix + "DoReplay");
	metalist.insert(m_sPrefix + "PacketLatencyThreshold");
	metalist.insert(m_sPrefix + "MulticastAddress");
	metalist.insert(m_sPrefix + "EWConfig");
	metalist.insert(m_sPrefix + "VelocityThreshold");
	metalist.insert(m_sPrefix + "ReadIntervalBlackGreyList");
	metalist.insert(m_sPrefix + "ReplayRealTime");
	metalist.insert(m_sPrefix + "ChannelFilter");
	metalist.insert(m_sPrefix + "FLRTfeed");
	metalist.insert(m_sPrefix + "ReadIntervalWhiteList");
	metalist.insert(m_sPrefix + "ChannelBlackList");
	metalist.insert(m_sPrefix + "FLpktlen");
	metalist.insert(m_sPrefix + "EEWChannel");
	metalist.insert(m_sPrefix + "ReplayFile");
	metalist.insert(m_sPrefix + "FLdebug");
	metalist.insert(m_sPrefix + "QMAMulticastAddress");
	metalist.insert(m_sPrefix + "ChannelGreyList");
	metalist.insert(m_sPrefix + "ChannelFile");

	LOGI << ostrm.str();

} // WPProperties::init

void WPProperties::sethandle(WPProperties* pHandle) {
	handle = pHandle;
} // WPProperties::sethandle

/** function compares expected and provided (by configuration file) variables. Provides runtime warnings for missing (default used) and unused variables.*/
void WPProperties::checkVars() {
	std::ostringstream ostrm;
	int unused = 0;

	std::vector<std::string> configList = getConfigListNames(); // Get all properties from config
	std::vector<std::string> propList = getPropListNames(); // Get all properties from config
	for (std::vector<std::string>::iterator it = configList.begin(); it != configList.end(); it++) {
		if (std::find(propList.begin(), propList.end(), *it) == propList.end()) {
			ostrm << "WARNING: config variable " << *it << " (" << getString(*it) << ") is unused!" << std::endl;
			unused++;
		}
	}
	if (unused > 0) {
		LOGW << ostrm.str();
	}
	LOGI << "CONFIG: Parsed " << getConfigFile() << ". Expected:" << propList.size() << ", read:" << configList.size() << ", default:" << missing << ", unused:" << unused << ", duplicates:" << dups << std::endl;
} // WPProperties::checkVars

/** function auto-generated and returns CMSConfigFile variable from configuration file parameters. Parameter: CMS config for recenter notification, use false to disable
*@return _cmsconfigfile */
std::string WPProperties::getCMSConfigFile() const {
	return _cmsconfigfile;
}

/** function auto-generated and returns ChannelBlackList variable from configuration file parameters. Parameter: channels to be dropped by WPlib, use none to disable
*@return _channelblacklist */
std::string WPProperties::getChannelBlackList() const {
	return _channelblacklist;
}

/** function auto-generated and returns ChannelFile variable from configuration file parameters. Parameter: text file with channel information
*@return _channelfile */
std::string WPProperties::getChannelFile() const {
	return _channelfile;
}

/** function auto-generated and returns ChannelFilter variable from configuration file parameters. Parameter: limit to only using channels e.g. HH|HN|HL
*@return _channelfilter */
std::string WPProperties::getChannelFilter() const {
	return _channelfilter;
}

/** function auto-generated and returns ChannelGreyList variable from configuration file parameters. Parameter: channels to be dropped by application, use none to disable
*@return _channelgreylist */
std::string WPProperties::getChannelGreyList() const {
	return _channelgreylist;
}

/** function auto-generated and returns DoReplay variable from configuration file parameters. Parameter: replay from tankfile using ewfeeder module
*@return _doreplay */
bool WPProperties::getDoReplay() const {
	return _doreplay;
}

/** function auto-generated and returns EEWChannel variable from configuration file parameters. Parameter: CMS notification topic, use false to disable
*@return _eewchannel */
std::string WPProperties::getEEWChannel() const {
	return _eewchannel;
}

/** function auto-generated and returns EWConfig variable from configuration file parameters. Parameter: config file for earthworm params
*@return _ewconfig */
std::string WPProperties::getEWConfig() const {
	return _ewconfig;
}

/** function auto-generated and returns FLRTfeed variable from configuration file parameters. Parameter: flag for file feed to determine if replay should be close to real time
*@return _flrtfeed */
bool WPProperties::getFLRTfeed() const {
	return _flrtfeed;
}

/** function auto-generated and returns FLdebug variable from configuration file parameters. Parameter: when debug is true sac files will be written, one per channel per hour
*@return _fldebug */
bool WPProperties::getFLdebug() const {
	return _fldebug;
}

/** function auto-generated and returns FLfile variable from configuration file parameters. Parameter: file containing the list of files to be processed by file feeder
*@return _flfile */
std::string WPProperties::getFLfile() const {
	return _flfile;
}

/** function auto-generated and returns FLpktlen variable from configuration file parameters. Parameter: desired packet length during file feed
*@return _flpktlen */
int WPProperties::getFLpktlen() const {
	return _flpktlen;
}

/** function auto-generated and returns IFID variable from configuration file parameters. Parameter: network device for multicast
*@return _ifid */
std::string WPProperties::getIFID() const {
	return _ifid;
}

/** function auto-generated and returns IgnoreFirstPackets variable from configuration file parameters. Parameter: number of packets to ignore after a gap, use -1 to disable gap checking
*@return _ignorefirstpackets */
int WPProperties::getIgnoreFirstPackets() const {
	return _ignorefirstpackets;
}

/** function auto-generated and returns MulticastAddress variable from configuration file parameters. Parameter: for multicast
*@return _multicastaddress */
std::string WPProperties::getMulticastAddress() const {
	return _multicastaddress;
}

/** function auto-generated and returns PacketLatencyThreshold variable from configuration file parameters. Parameter: Drop packets older than this threshold (seconds), use 0 to disable.  Negative for debug print.  Default is 0.
*@return _packetlatencythreshold */
int WPProperties::getPacketLatencyThreshold() const {
	return _packetlatencythreshold;
}

/** function auto-generated and returns QMAMulticastAddress variable from configuration file parameters. Parameter: for multicast
*@return _qmamulticastaddress */
std::string WPProperties::getQMAMulticastAddress() const {
	return _qmamulticastaddress;
}

/** function auto-generated and returns ReadIntervalBlackGreyList variable from configuration file parameters. Parameter: Interval in seconds to wait before checking black and grey lists for any changes. Use value 0 to disable subsequent refresh
*@return _readintervalblackgreylist */
int WPProperties::getReadIntervalBlackGreyList() const {
	return _readintervalblackgreylist;
}

/** function auto-generated and returns ReadIntervalWhiteList variable from configuration file parameters. Parameter: Interval in seconds to wait before checking white list for any change. Use value 0 to disable subsequent refresh
*@return _readintervalwhitelist */
int WPProperties::getReadIntervalWhiteList() const {
	return _readintervalwhitelist;
}

/** function auto-generated and returns ReplayFile variable from configuration file parameters. Parameter: tankfile replay file name, must be zipped (.gz extension)
*@return _replayfile */
std::string WPProperties::getReplayFile() const {
	return _replayfile;
}

/** function auto-generated and returns ReplayRealTime variable from configuration file parameters. Parameter: mimic real time
*@return _replayrealtime */
bool WPProperties::getReplayRealTime() const {
	return _replayrealtime;
}

/** function auto-generated and returns ThreadCount variable from configuration file parameters. Parameter: max number of threads allowed when processing packets
*@return _threadcount */
int WPProperties::getThreadCount() const {
	return _threadcount;
}

/** function auto-generated and returns VelocityThreshold variable from configuration file parameters. Parameter: default clipping threshold for velocity if unknown
*@return _velocitythreshold */
double WPProperties::getVelocityThreshold() const {
	return _velocitythreshold;
}



std::string WPProperties::unittest() {
	std::ostringstream ostrm;
	ostrm << std::endl << "Exercise generated get methods for class: WPProperties (prefix: " << m_sPrefix << ")" << std::endl;
	ostrm << "  getCMSConfigFile() => " << getCMSConfigFile() << std::endl;
	ostrm << "  getThreadCount() => " << getThreadCount() << std::endl;
	ostrm << "  getIgnoreFirstPackets() => " << getIgnoreFirstPackets() << std::endl;
	ostrm << "  getFLfile() => " << getFLfile() << std::endl;
	ostrm << "  getIFID() => " << getIFID() << std::endl;
	ostrm << "  getDoReplay() => " << getDoReplay() << std::endl;
	ostrm << "  getPacketLatencyThreshold() => " << getPacketLatencyThreshold() << std::endl;
	ostrm << "  getMulticastAddress() => " << getMulticastAddress() << std::endl;
	ostrm << "  getEWConfig() => " << getEWConfig() << std::endl;
	ostrm << "  getVelocityThreshold() => " << getVelocityThreshold() << std::endl;
	ostrm << "  getReadIntervalBlackGreyList() => " << getReadIntervalBlackGreyList() << std::endl;
	ostrm << "  getReplayRealTime() => " << getReplayRealTime() << std::endl;
	ostrm << "  getChannelFilter() => " << getChannelFilter() << std::endl;
	ostrm << "  getFLRTfeed() => " << getFLRTfeed() << std::endl;
	ostrm << "  getReadIntervalWhiteList() => " << getReadIntervalWhiteList() << std::endl;
	ostrm << "  getChannelBlackList() => " << getChannelBlackList() << std::endl;
	ostrm << "  getFLpktlen() => " << getFLpktlen() << std::endl;
	ostrm << "  getEEWChannel() => " << getEEWChannel() << std::endl;
	ostrm << "  getReplayFile() => " << getReplayFile() << std::endl;
	ostrm << "  getFLdebug() => " << getFLdebug() << std::endl;
	ostrm << "  getQMAMulticastAddress() => " << getQMAMulticastAddress() << std::endl;
	ostrm << "  getChannelGreyList() => " << getChannelGreyList() << std::endl;
	ostrm << "  getChannelFile() => " << getChannelFile() << std::endl;
	return ostrm.str();
}


int WPProperties::main(int argc, char* argv[]) {

	int rc = 0;       // return code

	// logging stuff
	static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
	static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&print_lock);

	plog::init(plog::verbose, &Severity_appender);

	LOGN << "Program: " << argv[0] << " -- autogenerated unit test for WPProperties" << std::endl;
	LOGN << "Modules:";
	LOGN << "  " << RCSID_Exceptions_h;
	LOGN << "  " << RCSID_Exceptions_cc;
	LOGN << "  " << RCSID_GetProp_h;
	LOGN << "  " << RCSID_GetProp_cc;
	LOGN << "  " << RCSID_WPProperties_h;
	LOGN << "  " << RCSID_WPProperties_cc;
	LOGN;

	LOGI << "Begin unit test for WPProperties";

	if (argc < 2) {
		LOGI << "Usage: " << argv[0] << " <config file>" << std::endl;

		LOGI << "No file specified";
		try {
			LOGI << "Trying GetProp::Internal_unit_tests...";
			GetProp::Internal_unit_tests();
		} catch (Error& e) {
			rc++;
			LOGE << "Error using GetProp file unit tests." << std::endl << e.str();
		}
		LOGI << std::endl << "End unit test for WPProperties. returning " << rc;
		exit(rc);
	}

	WPProperties* prop = WPProperties::getInstance();
	try {
		std::string filename = argv[1];
		// LOGD << "Attempting to open file " << filename;
		prop->init(filename, argc, argv);

		LOGI << "Trying GetProp::File_unit_tests...";
		GetProp::File_unit_tests(prop);

	} catch (Error& e) {
		rc++;
		LOGE << "Error using GetProp file unit tests." << std::endl << e.str();
		exit(rc);
	}

	LOGI << std::endl << "Trying WPProperties specific unit tests...";
	try {
		LOGI << prop->unittest();
	} catch (Error& e) {
		rc++;
		LOGE << "Unexpected error using generated methods." << std::endl << e.str() << std::endl;
	}

	LOGI << std::endl << "End unit test of WPProperties. returning " << rc;

	exit(rc);
} // static WPProperties::main

// end of file: WPProperties.cc
