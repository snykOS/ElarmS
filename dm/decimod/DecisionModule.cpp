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
#include "DecisionModule.h"
#include "HBProducer.h"
#include "HBForward.h"
#include "GetProp.h"
#include "TimeStamp.h"
#include <errno.h>

extern string program_version;
extern GetProp *ewprop;

stringstream DecisionModule::os;

#ifdef TEST_MODS

// carrier for heartbeat objects - allows cleanup on exit
// (basically a simple smart pointer)
class HeartBeat {
private:
  HBProducer *producer;
  HBForward *forward;
public:
  HeartBeat(bool use_heartbeat, DMMessageSender *dmevent_sender, 
	    DMMessageReceiver *algorithm_receiver, 
	    string heartbeat_sender, string heartbeat_originator,
	    string heartbeat_out_topic, string heartbeat_in_topic) {
    if (use_heartbeat) {
      producer = new HBProducer(dmevent_sender->getConnection(), 
				heartbeat_sender, heartbeat_originator, 
				heartbeat_out_topic);
      forward = new HBForward(algorithm_receiver->getConnection(), 
			      dmevent_sender->getConnection(), 
			      heartbeat_sender, producer->getInterval(), 
			      heartbeat_in_topic, heartbeat_out_topic);
    } else {
      producer = NULL;
      forward = NULL;
    }
  }
  ~HeartBeat() {
    delete producer;
    delete forward;
  }
};

void DecisionModule::construct(int message_loop_limit, bool heartbeat, 
			       DMMessageReceiver *algorithm,
			       DMMessageSender *event,
			       DMMessageSender *log) {
    loop_countdown = message_loop_limit;
    use_heartbeat = heartbeat;
    evid_filename = "";
    alg_username = "";
    alg_password = "";
    alg_uri = "";
    alg_topic = "";
    alert_username = "";
    alert_password = "";
    alert_uri = ""; 
    alert_topic = "";
    verbose = 1;
    publish_changes_only = false; 
    compare_num_contrib = true;
    next_event_id = 0; 
    use_republish_threshold = false;
    republish_interval = 0;
    algorithm_receiver = algorithm;
    dmevent_sender = event;
    log_sender = log;
    os << setiosflags(ios::fixed);
}

DecisionModule::DecisionModule() {
  // NULL receivers / senders are set in run()
  construct(-1, true, NULL, NULL, NULL);
}

/* 
  Constructor for testing - supports mock AMQ 
  Will only run message_loop_limit iterations in run()
*/
DecisionModule::DecisionModule(int message_loop_limit, 
			       DMMessageReceiver *algorithm,
			       DMMessageSender *event,
			       DMMessageSender *log,
			       map<string, DMMessageSender*> &forward) :
  forward_senders(forward) {
  construct(message_loop_limit, false, algorithm, event, log);
}

/** Provide access for testing */
DMEventMap DecisionModule::getDmEvents(){
    return dm_events;
}

#else

DecisionModule::DecisionModule() : evid_filename(""),
	alg_username(""), alg_password(""), alg_uri(""), alg_topic(""),
	alert_username(""), alert_password(""), alert_uri(""), alert_topic(""),
	verbose(1), publish_changes_only(false), compare_num_contrib(true),
        next_event_id(0), use_republish_threshold(false),
	republish_interval(0), algorithm_receiver(NULL), dmevent_sender(NULL)
{
    os << setiosflags(ios::fixed);
}

#endif

DecisionModule::~DecisionModule() 
{
    dm_events.clear();
    algo_messages.clear();
    if(algorithm_receiver != NULL) delete algorithm_receiver;
    if(dmevent_sender != NULL) delete dmevent_sender;
}

/** Run the main message processing loop. There is no return from the method.
 *  @param[in] useRepublishThreshold if true, DMevents are published only if there
 *    have been any changes (greated than or equal to the threshold) to its CoreEventInfo values since
 *    the last publishment.
 *  @param[in] republishInterval if greater than zero, a DMEvent will be republished at this
 *    time interval (seconds). (Not implemented yet)
 */
void DecisionModule::run() throw(string)
{
    CoreEventInfo *cei;

    getProperties();
    getEventId();

#ifdef TEST_MODS
    if (algorithm_receiver == NULL) {
#endif

    algorithm_receiver = new DMMessageReceiver(alg_uri, alg_username, alg_password, alg_topic);

#ifdef TEST_MODS
    }
#endif
    algorithm_receiver->run(); // create Connection. call before getConnection()

#ifdef TEST_MODS
    if (dmevent_sender == NULL) {
#endif

    if(alert_uri == alg_uri) {
      dmevent_sender = new DMMessageSender(algorithm_receiver->getConnection(), alert_topic);
      os << "Using receiver connection for sender." << endl;
    }
    else {
      dmevent_sender = new DMMessageSender(alert_uri, alert_username, alert_password, alert_topic);
    }
	
#ifdef TEST_MODS
    }
#endif

    dmevent_sender->run();

#ifdef TEST_MODS
    if (log_sender == NULL) {
#endif

    if(log_uri == alg_uri) {
      log_sender = new DMMessageSender(algorithm_receiver->getConnection(), log_topic);
    }
    else {
      log_sender = new DMMessageSender(alert_uri, alert_username, alert_password, log_topic);
    }

#ifdef TEST_MODS
    }
#endif

    log_sender->run();

#ifdef TEST_MODS
    if (forward_senders.empty()) {
#endif

    if(dm_system_name == DM) {
	// forward_contributors[eqinfo2gm-contour] = eew.sys.gm-contour.data
	// forward_contributors[eqinfo2gm-map] = eew.sys.gm-map.data
	// forward_senders[eew.sys.gm-contour.data] = DMMessageSender
	map<string, string>::iterator it;
	for(it = forward_contributors.begin(); it != forward_contributors.end(); it++) {
	    forward_senders[it->first] = new DMMessageSender(dmevent_sender->getConnection(), it->second);
	    forward_senders[it->first]->run();
	}
    }

#ifdef TEST_MODS
    }

    HeartBeat hb(use_heartbeat, dmevent_sender, algorithm_receiver,
		 heartbeat_sender, heartbeat_originator, 
		 heartbeat_out_topic, heartbeat_in_topic);
#else

    HBProducer hbp(dmevent_sender->getConnection(), heartbeat_sender, heartbeat_originator, heartbeat_out_topic);
    HBForward hbf(algorithm_receiver->getConnection(), dmevent_sender->getConnection(), heartbeat_sender,
                  hbp.getInterval(), heartbeat_in_topic, heartbeat_out_topic);
#endif

    os << "CISN Decision Module Running" << endl;
    if(use_republish_threshold) {
	os << "  using republish thresholds:" << setprecision(2) << endl;
	os << "    CHANGE_THRESHOLD_MAG:         " << constants.change_threshold_mag << endl;
	os << "    CHANGE_THRESHOLD_MAG_UNCER:   " << constants.change_threshold_mag_uncer << endl;
	os << "    CHANGE_THRESHOLD_LAT:         " << constants.change_threshold_lat << endl;
	os << "    CHANGE_THRESHOLD_LAT_UNCER:   " << constants.change_threshold_lat_uncer << endl;
	os << "    CHANGE_THRESHOLD_LON:         " << constants.change_threshold_lon << endl;
	os << "    CHANGE_THRESHOLD_LON_UNCER:   " << constants.change_threshold_lon_uncer << endl;
	os << "    CHANGE_THRESHOLD_DEPTH:       " << constants.change_threshold_depth << endl;
	os << "    CHANGE_THRESHOLD_DEPTH_UNCER: " << constants.change_threshold_depth_uncer<<endl;
	os << "    CHANGE_THRESHOLD_OTIME:       " << constants.change_threshold_otime << endl;
	os << "    CHANGE_THRESHOLD_OTIME_UNCER: " << constants.change_threshold_otime_uncer<<endl;
	os << "    CHANGE_THRESHOLD_LKLHD:       " << constants.change_threshold_lklhd << endl;
    }
    if(republish_interval > 0) {
	os << "  using republish interval: " << republish_interval << " seconds" << endl;
    }

    cout << os.str() << endl;
    log_sender->sendString(logTime() + "\n" + os.str());
    os.str("");

#ifdef TEST_MODS
    while(loop_countdown) {  // in normal use this is constant -1
#else
      while(true) {
#endif
	cei = algorithm_receiver->receive(100); // wait 100 msecs for next algorithm message

	processMessage(cei);

	discardOldDMEvents();

	discardOldDeleteEvents();

	while( updateAssociations() ) {
	    updateFreeAlgoList();
	}

	publishDMEvents();

	if( !os.str().empty() ) {
	    cout << os.str() << endl;
	    log_sender->sendString(logTime() + "\n" + os.str());
	    os.str("");
	}

	discardOldAlgoEvents();

#ifdef TEST_MODS
	// decrement during testing
	if (loop_countdown > 0) loop_countdown--;
#endif

    }
}

/** Process a new AlgorithmEvent. Reset all DMEvent publish_flags to false. Handle a DELETE
 *  message. Check that the message systemName is in the algorithm_names list from the AlgNames
 *  parameter. Check that the message_id has not already been deleted. Check that the origin
 *  time is not too old. Check for valid event attributes.
 *  @param[in] cei the new algorithm event message object.
 *  @returns true if the message is from a valid algorithm, and the location is within one of the
 *  algorithm's regions, and the message type is DELETE or the category is LIVE and the message values
 *  are acceptable, and a previous version of the message is not already in the delete_messages map.
 */
bool DecisionModule::processMessage(CoreEventInfo *cei)
{
    bool ok;
    // reset the publish_flag to false for all current DMEvents
    for(DMEventIter dm = dm_events.begin(); dm != dm_events.end(); dm++) {
	dm->second->setPublishFlag(false);
    }
    
    if( cei == NULL ) return false;

    string algorithm_name = cei->getSystemNameString();

    bool algorithm_okay = (algorithm_names.find(algorithm_name) != algorithm_names.end());

    if(!algorithm_okay) {
	if(verbose) {
	    os << " Rejected: [" << cei->getSystemNameString() << "," << cei->getID()
			<< "," << cei->getVersion() << "] unknown system" << endl;
	}
	delete cei;
	return false;
    }

    // check if the cei event location is within the region for cei->getSystemNameString(), if a
    // region has been specified { algorithm_regions.find(cei->getSystemNameString() }

    if(!insideAlgRegions(cei, cei)) {
	if(verbose) {
	    os << " Rejected: [" << cei->getSystemNameString() << "," << cei->getID()
			<< "," << cei->getVersion() << "] outside alg regions" << endl;
	}
	delete cei;
	return false;
    }

    bool use_core = (core_contributors.find(cei->getSystemNameString()) != core_contributors.end());
    AlgorithmEvent algo(cei, use_core);

    if(verbose) {
	os << "           " << CoreEventInfo::labelString() << " MSFIT  MXKM  MXDT" << endl;
	os << " Received: " << cei->coreToString() << endl;
    }

    // if the message has type=DELETE, delete all prior messages with same system_name,
    // program_instance and id.
    if( cei->getMessageType() == DELETE ) {
	handleDeleteMsg(algo, cei);
    	return true;
    }

    double age, now = nepoch_to_tepoch(time(NULL));
    // ignore a message if it has invalid attribute values or is LIVE and is too old
    if( !(ok = AlgorithmEvent::valuesOK(cei, constants, verbose, os)) ||
	(cei->getMessageCategory() == LIVE && (age = now-cei->getOriginTime()) > constants.algevent_timeout_secs))
    {
	if(verbose) {
	    os << " Rejected: [" << cei->getSystemNameString() << "," << cei->getID()
			<< "," << cei->getVersion() << "]";
	    if(!ok) {
		os << " invalid values" << endl;
	    }
	    else {
		os << " too old: age = " << age << endl;
	    }
	}
	delete cei;
	return false;
    }

    // ignore a message whose system_name and id are in the delete map
    if( delete_messages.find(algo) != delete_messages.end() ) {
	if(verbose) {
	    os << " Rejected: [" << cei->getSystemNameString() << "," << cei->getID()
			<< "," << cei->getVersion() << "]";
	    os << " id in previous DELETE." << endl;
	}
	delete cei;
	return false;
    }

    // the message type=NEW or UPDATE
    handleAlgoMsg(algo, cei);
    updateFreeAlgoList();

    return true;
}

/** Process a DELETE algorithm message.
 *  @param[in] algo the new AlgorithmEvent object
 *  @param[in] cei the new algorithm event message object (ElarmsMessage, VSMessage,
 *   or OnSiteMessage).
 */
void DecisionModule::handleDeleteMsg(AlgorithmEvent algo, CoreEventInfo *cei)
{
    AlgoMapIter it;

    enum eewSystemName sys_name = cei->getSystemName();

    if(sys_name == DMREVIEW) {
	// delete-event-message from DMReview with the DM event id to be cancelled
	// find a DM event with the same id
	DMEventIter dm;
	if( (dm = dm_events.find(cei->getID())) != dm_events.end() )
	{
	    // Found a current DM event with id == dmreview.id
	    // delete all contributing algorithm events
	    // this will cause the event to be cancelled by publishDMEvents()
	    vector<AlgorithmEvent> tmp = dm->second->getAlgoEvents();

	    for(vector<AlgorithmEvent>::iterator jt = tmp.begin(); jt != tmp.end(); jt++)
	    {
		if( (it = algo_messages.find(*jt)) != algo_messages.end() ) {
		    removeAlgoFromDMEvent(it->first);
		    AlgorithmEvent algo(it->second, true);
		    if( delete_messages.find(algo) == delete_messages.end() ) {
			delete_messages.insert( pair<AlgorithmEvent, CoreEventInfo*>(algo, it->second) );
		    }
		    algo_messages.erase(it); // remove the key from the algo_messages map
		}
	    }
	}
	else {
	    // If there is not a current event with id == dmreview.id, just send out a cancel message here
	    DMEvent *dm = new DMEvent(algo.message_category, dm_system_name, cei->getID(), &algo_messages,
                                        constants, os, verbose);
	    dm->setVersion(0);
	    dm->setMessageType(DELETE);
	    dm->setTimestamp("");
	    dm->setAlgorithmVersion(program_version);
	    dm->setMessageCategory(cei->getMessageCategory());
	    dm->setOriginTime(cei->getOriginTime());
	    dm->setLatitude(cei->getLatitude());
	    dm->setLongitude(cei->getLongitude());
	    dm->setDepth(cei->getDepth());
	    dm->setMagnitude(cei->getMagnitude());

	    dmevent_sender->sendMessage(dm);

	    delete cei;

	    if(verbose) {
		char buf[1000];
		snprintf(buf, sizeof(buf), "%10.10s %15s     %4.4s", "dm",
		    dm->getID().c_str(), dm->getMessageCategoryString().c_str());
		os << "  Deleted: " << string(buf) << endl;
		os << "Published: " << dm->coreToString() << " [event deleted]" << endl;
	    }
	    delete dm;
	}
    }
    else { // received a cancel message from one of the algorithms
	// find a message that has the same category, system_name, program_instance, and id
	if( (it = algo_messages.find(algo)) != algo_messages.end() ) {
	    removeAlgoFromDMEvent(it->first);
	    deleteAlgorithmEvent(it);
	}

	if( delete_messages.find(algo) == delete_messages.end() ) {
	    delete_messages.insert( pair<AlgorithmEvent, CoreEventInfo*>(algo, cei) );
	}
	else {
	    delete cei;
	}
    }
}

void DecisionModule::deleteAlgorithmEvent(AlgoMapIter it)
{
    delete (*it).second;     // the CoreEventInfo object
    algo_messages.erase(it); // remove the key from the algo_messages map
}

/** Continue processing a new algorithm message that is not a DELETE message.
 *  If the message is a valid update to a previous message, set the associated
 *  DM event publish flag to true.
 *  If the message is a new message, check if it can be associated with an
 *  existing DM event or create a new DM event for it.
 *  @param[in] algo the new AlgorithmEvent object
 *  @param[in] cei the new algorithm event message object
 *  @returns true if the message is a valid new message or valid update
 */
bool DecisionModule::handleAlgoMsg(AlgorithmEvent algo, CoreEventInfo *cei)
{
    AlgoMapIter it;
    bool valid = false;

    // correct any invalid uncertainty values
//    AlgorithmEvent::checkCEIUncertainties(cei, verbose); accept all uncertainty values

    // find a message that has the same category, system_name, program_instance, and id
    if( (it = algo_messages.find(algo)) != algo_messages.end() )
    {
	// if the version number is > existing version, update AlgorithmEvent
	// and the associated DMEvent
	if( cei->getVersion() > (*it).second->getVersion() )
	{
	    string s = (*it).second->updateString(*cei);
            delete it->second;
            it->second = cei;
	    if(verbose) {
		os << "  Updated: " << s << endl;
	    }
	    setDMEventPublish(algo);
	    valid = true;
	}
	else if(verbose) {
	    os << " Rejected: [" << cei->getSystemNameString() << "," << cei->getProgramInstance()
			<< "," << cei->getID() << "," << cei->getVersion()
			<<  "," << cei->getMessageCategoryString() << "]";
	    if(cei->getVersion() == (*it).second->getVersion()) {
		os << " repeated version number" << endl;
	    }
	    else {
		os << " lower version number" << endl;
	    }
	}
    }
    else {
	algo_messages.insert( pair<AlgorithmEvent, CoreEventInfo*>(algo, cei) );
	associateAlgorithmEvent(algo);
	valid = true;
    }
    return valid;
}

/** Fill the free_algo_messages map with all AlgorithmEvents in algo_messages that
 *  are not associated with a DMEvent.
 */
void DecisionModule::updateFreeAlgoList()
{
    AlgoMapIter it;
    DMEventIter dm;

    free_algo_messages.clear();

    for(it = algo_messages.begin(); it != algo_messages.end(); it++) {
	// search events to find one that is associated with this message
	for(dm = dm_events.begin(); dm != dm_events.end()
		&& !dm->second->contains(it->first); dm++);

	if( dm == dm_events.end() ) { // algo_message is unassociated
	    free_algo_messages.push_back(it->first);
	}
    }
}

void DecisionModule::removeAlgoFromDMEvent(AlgorithmEvent algo)
{
    for(DMEventIter dm = dm_events.begin(); dm != dm_events.end(); dm++) {
	if( dm->second->contains(algo) ) {
	    dm->second->removeAlgoEvent(algo);
	    dm->second->setPublishFlag(true);
	    dm->second->update();
	}
    }
}

/** Find the DMEvent which is associated to the input AlgorithmEvent, set its
 *  publish_flag=true and recalculate its misfit.
 *  @param[in] algo
 */
void DecisionModule::setDMEventPublish(AlgorithmEvent algo)
{
    for(DMEventIter dm = dm_events.begin(); dm != dm_events.end(); dm++) {
	if( dm->second->contains(algo) ) {
	    dm->second->setPublishFlag(true); // there should only be one
	    dm->second->update();
	}
    }
}

/** Return the CoreEventInfo * "value" from the algo_messages map for the input AlgorithmEvent
 *  key. The CoreEventInfo pointer returned is a pointer to one of the subclasses
 *  ElarmsMessage, VSMessages, or OnSiteMessage.
 *  @param[in] algo
 *  @returns pointer to the algorithm message for the input map key, or NULL if the key is not
 *  found.
 */
CoreEventInfo * DecisionModule::getAlgoMsg(AlgorithmEvent algo)
{
    AlgoMapIter it;

    if( (it = algo_messages.find(algo)) != algo_messages.end() ) { 
	return it->second;
    }
    return NULL;
}

/** Try to associate the input candidate AlgorithmEvent to an existing DMEvent by finding the
 *  best fit (within \link decimod::constants.max_time_to_event constants.max_time_to_event\endlink and
 *  \link decimod::constants.max_distkm_to_event constants.max_distkm_to_event\endlink) among the DMEvents that
 *  don't already contain an AlgorithmEvent from the same system and program instance. If such
 *  a DMEvent cannot be found, create a new DMEvent and associate the input AlgorithmEvent with it.
 *  @param[in] algo
 */
void DecisionModule::associateAlgorithmEvent(AlgorithmEvent algo) 
{
    double best_misfit = NULL_MISFIT;
    double check_misfit, check_delta_time, check_dist_km;
    DMEventIter dm, dm_best;
    bool ok;
    string alg_id = algo.id;
    CoreEventInfo *candidate = getAlgoMsg(algo);
    if(candidate == NULL) {
        os << "associateAlgorithmEvent::geAlgoMsg returned NULL for " << algo.toString() << endl;
        return;
    }
    int alg_ver = candidate->getVersion();
    string alg_str = candidate->getSystemNameString();
    string alg_instance = candidate->getProgramInstance();
    string alg_cat = candidate->getMessageCategoryString();

    dm_best = dm_events.end();
    for(dm = dm_events.begin(); dm != dm_events.end(); dm++)
	if(algo.message_category == dm->second->getMessageCategory() && dm->second->numAlgoEvents() > 0)
    {
	if(verbose) {
	    os << "Associate: [" << alg_str << "," << alg_instance << "," << alg_id << "," << alg_ver
			<< "," << alg_cat << "] with dm " << dm->second->getID();
	}

	// The new algo event can only be associated with an existing event where each algorithm
	// location is within all algorithm regions.
	bool inside_regions = true;
	vector<AlgorithmEvent> dm_algo_events = dm->second->getAlgoEvents();
	for(size_t i = 0; i < dm_algo_events.size(); i++)
	{
	    CoreEventInfo *cei = getAlgoMsg(dm_algo_events[i]);
            if(cei == NULL) {
                os << "associateAlgorithmEvent::geAlgoMsg returned NULL for " << dm_algo_events[i].toString() << endl;
                return;
            }
	    if( !insideAlgRegions(cei, candidate)  // cei is not inside a candidate region and there are candidate regions
	     || !insideAlgRegions(candidate, cei)) // candidate is not inside a cei region and there are cei regions
	    {
		inside_regions = false;
		break;
	    }
	}


	// check for algo regional restrictions.
	// Check for best misfit. Check time distance.

	if( inside_regions
	    && (ok=dm->second->checkNewAlgoMisfit(algo, &check_misfit, &check_delta_time, &check_dist_km))
	    && check_misfit <= constants.max_misfit
	    && check_delta_time <= constants.max_time_to_event
	    && check_dist_km <= constants.max_distkm_to_event)
	{
	    if(best_misfit == NULL_MISFIT || check_misfit < best_misfit) {
		best_misfit = check_misfit;
		dm_best = dm;
	    }
	}
	if(verbose) {
	    if(ok) { // ok=true if an algo from same system is not already in dm
		os << " misfit: " << setprecision(3) << check_misfit << endl;
	    }
	    else {
		os << endl;
	    }
	}
    }

    if (dm_best != dm_events.end()) {
	dm_best->second->addAlgoEvent(algo);
	dm_best->second->setPublishFlag(true);
	if (verbose) {
	    os << "Associate: [" << alg_str << "," << alg_instance << "," << alg_id << "," << alg_ver
		<< "," << alg_cat << "] added to dm " << dm_best->second->getID()
		<< " misfit: " << setprecision(3) << dm_best->second->getMisfit() << endl;
	}
    } 
    else {
	string event_id="";
	// Algo_event does not fit with any DMEvent.
	// If this algorithm was previously associated with a DMEvent
	// that was deleted, and that DMEvent is available for reuse, use it.

	// This event should still be on the dm_events list.
	// Find it.  If it is marked "deleted", reuse it.
	// Otherwise, another previously freed algorithm has already used it.
	for(dm = dm_events.begin(); dm != dm_events.end(); dm++) {
	    if(dm->second->getMessageCategory() == algo.message_category
		&& !dm->second->numAlgoEvents()
		&& dm->second->previouslyContained(algo)) break;
	}
	if (dm != dm_events.end()) {
	    // We can reuse this event. 
	    dm->second->addAlgoEvent(algo);
	    dm->second->setPublishFlag(true);
	    event_id = dm->second->getID();
	    if(verbose) {
		os << "   Reused: " << dm->second->coreToString();
	    }
	}		
	else {
	    event_id = getNewEventID();
	    DMEvent *dmep = new DMEvent(algo.message_category, dm_system_name, event_id, &algo_messages,
                                        constants, os, verbose);
	    dmep->setSystemName(dm_system_name);
	    dmep->addAlgoEvent(algo);
	    dmep->setPublishFlag(true);
	    if(verbose) {
		os << "  Created: " << dmep->coreToString();
	    }
	    dm_events.insert( pair<string, DMEvent *>(event_id, dmep) );
//	    delete dmep;
	}
	if(verbose) {
	    os << " [" << alg_str << "," << alg_instance << "," << alg_id << "," << alg_ver
		<< "," << alg_cat << "]" <<endl;
	}
    }
}

/**
 * Check if any "free" algorithm messages can be associated with an existing DM event. The
 * "free" algorithm messages in free_algo_messages are update messages that have forced
 * the DM event to "split".
 */
bool DecisionModule::updateAssociations()
{
    FreeAlgoIter it;
    bool found_unassociated_algo = false;

    for(it = free_algo_messages.begin(); it != free_algo_messages.end(); it++) {
	associateAlgorithmEvent(*it);
	found_unassociated_algo = true;
    }
    return found_unassociated_algo;
}

/** Discard algorithm messages that are older than constants.algevent_timeout_secs and
 *  are not part of a DMEvent.
 */
void DecisionModule::discardOldAlgoEvents()
{
    AlgoMapIter it, it_tmp;
    DMEventIter dm;
    char buf[1000];
    double now = nepoch_to_tepoch(time(NULL));

    for(it = algo_messages.begin(); it != algo_messages.end(); ) {
        double age = now - (*it).second->getOriginTime();
	if( age > constants.algevent_timeout_secs ) {

            // check if this algorithm event is a member of a dmevent
            for(dm = dm_events.begin(); dm != dm_events.end(); dm++) {
		if( dm->second->contains(it->first) ) break;
            }
            if(dm == dm_events.end()) {
                if(verbose) {
                    snprintf(buf, sizeof(buf), "%10.10s %s %12s %3d %4.4s",
			(*it).second->getSystemNameString().c_str(),
			(*it).second->getProgramInstance().c_str(),
			(*it).second->getID().c_str(), (*it).second->getVersion(),
			(*it).second->getMessageCategoryString().c_str());
                    os << "  Expired: " << string(buf) << " age: "
			<< setprecision(1) << age << " secs" << endl;
                }
                it_tmp = it;
                it++;
                deleteAlgorithmEvent(it_tmp); // remove the old message from the map
                continue;
            }
	}
        it++;
    }
}

/** Discard algorithm DELETE messages that are older than constants.algevent_timeout_secs.
 */
void DecisionModule::discardOldDeleteEvents()
{
    AlgoMapIter it, it_tmp;
    char buf[1000];
    double now = nepoch_to_tepoch(time(NULL));

    for(it = delete_messages.begin(); it != delete_messages.end(); ) {
	double age = now - (*it).second->getOriginTime();
	if( age > constants.algevent_timeout_secs ) {
	    if(verbose) {
		snprintf(buf, sizeof(buf), "%10.10s %s %12s %3d %4.4s",
			(*it).second->getSystemNameString().c_str(),
			(*it).second->getProgramInstance().c_str(),
			(*it).second->getID().c_str(), (*it).second->getVersion(),
			(*it).second->getMessageCategoryString().c_str());
		os << "  Expired: " << string(buf) << " age: "
			<< setprecision(1) << age << " secs" << endl;
	    }
	    it_tmp = it;
	    it++;
	    delete (*it_tmp).second;     // the CoreEventInfo object
	    delete_messages.erase(it_tmp);
	}
	else {
	    it++;
	}
     }
}

/** Discard DM events that are older than constants.dmevent_timeout_secs.
 */
void DecisionModule::discardOldDMEvents()
{
    DMEventIter dm, dm_tmp;
    double now = nepoch_to_tepoch(time(NULL));

    dm = dm_events.begin();
    while(dm != dm_events.end()) {
	double age = now - dm->second->getOriginTime();
	if(age > constants.dmevent_timeout_secs) {
            deleteDMEvent(dm);
            delete dm->second;
	    dm_tmp = dm;
	    dm++;
	    dm_events.erase(dm_tmp);
	} 
	else {
	    dm++;
	}
    }
}

void DecisionModule::deleteDMEvent(DMEventIter dm)
{
    if (verbose) {
        double now = nepoch_to_tepoch(time(NULL));
	double age = now - dm->second->getOriginTime();
        char buf[1000];
        snprintf(buf, sizeof(buf), "%10.10s %15.15s %3d %4.4s",
            dm->second->getSystemNameString().c_str(),
            dm->second->getID().c_str(), dm->second->getVersion(),
            dm->second->getMessageCategoryString().c_str());
        os << "  Expired: " << string(buf) << " age: "
                << setprecision(1) << age << " secs" << endl;
    }

    // remove all associated algo_events
    AlgoMapIter it, it_tmp;
    it = algo_messages.begin();
    while( it != algo_messages.end() ) {
        if( dm->second->contains(it->first) ) {
            it_tmp = it;
            it++;
            deleteAlgorithmEvent(it_tmp);
        }
        else {
            it++;
        }
    }
}

void DecisionModule::publishDMEvents()
{
    DMEventIter dm, dm_tmp;
    bool publish_now;
    char buf[1000];
    double now = nepoch_to_tepoch(time(NULL));

    dm = dm_events.begin();
    while( dm != dm_events.end() ) {
        DMEvent *dmevent = dm->second;
	publish_now = (republish_interval > 0 && dmevent->getTimePublished() > 0. &&
		now - dmevent->getTimePublished() >= republish_interval);

	if(dmevent->numAlgoEvents() == 0) {
	    // Send DELETE message for a DMEvent that has no AlgoEvents and remove it.
	    dmevent->setMessageType(DELETE);
	    dmevent->setVersion(dmevent->getVersion()+1);
	    dmevent->setAlgorithmVersion(program_version);
	    dmevent->setTimestamp("");
	    dmevent_sender->sendMessage(dmevent);
	    if(verbose) {
		snprintf(buf, sizeof(buf), "%10.10s %15.15s     %4.4s", "dm",
		    dmevent->getID().c_str(), dmevent->getMessageCategoryString().c_str());
		os << "  Deleted: " << string(buf) << endl;
		os << "Published: " << dmevent->coreToString() << " [event deleted]" << endl;
	    }

	    // Remove the DMevent
	    dm_tmp = dm;
	    dm++;
	    dm_events.erase(dm_tmp);
	}
	else if(dmevent->getPublishFlag() || publish_now)
        {
            publishDMEvent(dmevent, publish_now);
	    dm++;
        }
	else {
	    dm++;
	}
    }
}

bool DecisionModule::publishDMEvent(DMEvent *dmevent, bool publish_now)
{
    char buf[1000];
    bool obs_or_pred_change = dmevent->loadAlgExtra(constants.fault_info_min_mag);
    bool dm_core_change = dmevent->aboveThreshold();

    if( verbose && !obs_or_pred_change && !dm_core_change ) {
        snprintf(buf, sizeof(buf), "%10.10s %15.15s     %4.4s", "dm",
                dmevent->getID().c_str(), dmevent->getMessageCategoryString().c_str());
        if(dmevent->noChange()) {
            os << "No Change: " << string(buf);
            if(publish_now) { os << " [republish interval]" << endl; }
            else os << endl;
        }
        else { // change, but below threshold values
            os << "  Changed: " << dmevent->changeString();
            if(publish_now) { os << " [republish interval]"; }
            else if(use_republish_threshold) { os << " [not republished]"; }
            os << endl;
        }
    }
    if(dmevent->getVersion() >= 0) {
        dmevent->setMessageType(UPDATE);
        if(verbose) {
            if(dm_core_change) {
		os << "  Changed: " << dmevent->changeString() << endl;
            }
            else if(obs_or_pred_change) {
		snprintf(buf, sizeof(buf), "%10.10s %15.15s     %4.4s", "dm",
                        dmevent->getID().c_str(), dmevent->getMessageCategoryString().c_str());
		os << "  Changed: " << string(buf);
		os << " change to observations or predictions" << endl;
            }
        }
    }

    // always check if need to forward contributors
    if(dm_system_name == DM) {
        forwardContributors(dmevent);
    }

    if(use_republish_threshold && !obs_or_pred_change && !dm_core_change && !publish_now) return false;

    if( !dmevent->containsCoreEventInfo() ) {
        if(verbose) {
            os << "  Holding: " << dmevent->coreToString();
            snprintf(buf, sizeof(buf), " %5.2f %5.1f %5.1f", dmevent->getMisfit(),
                        dmevent->getMaxDistKm(), dmevent->getMaxDeltaTime());
            os << buf << " " << dmevent->contribString() << endl;
        }
        return false;
    }

    dmevent->setAlgorithmVersion(program_version); // The DecisionModule code version

    string reason;
    bool filtered = filter(dmevent, reason);
    bool alert_alone_ok = publishAloneOk(dmevent);
    bool message_changed = messageChanged(dmevent);

    if(!filtered && alert_alone_ok && message_changed) {
        dmevent->setTimestamp("");
        dmevent->setVersion(dmevent->getVersion()+1);
    }

    if(verbose) {
        char buf[100];
        if( (!filtered && message_changed && alert_alone_ok) || publish_now) {
            os << "Published: ";
        }
        else if(filtered) {
            os << reason;
        }
        else if(!alert_alone_ok) {
            os << "  Holding: ";
        }
        else if(!message_changed) {
            os << "No Change: ";
        }
        else {
            os << "  Holding: ";
        }
        os << dmevent->coreToString();
        snprintf(buf, sizeof(buf), " %5.2f %5.1f %5.1f", dmevent->getMisfit(),
                        dmevent->getMaxDistKm(), dmevent->getMaxDeltaTime());
        os << buf << " " << dmevent->contribString() << endl;
    }

    if( (filtered || !message_changed || !alert_alone_ok) && !publish_now) return false;

    dmevent_sender->sendMessage(dmevent);
    double now = nepoch_to_tepoch(time(NULL));
    dmevent->publish(now);
    if(dm_system_name == DM) {
        dmevent->publish_next_contour = true;
        dmevent->publish_next_map = true;
    }

    return true;
}

void DecisionModule::forwardContributors(DMEvent *dmevent)
{
    vector<AlgorithmEvent> contrib = dmevent->getAlgoEvents();
    map<string, CoreEventInfo *> contrib_recent;
    map<string, CoreEventInfo *>::iterator jt;

    // get the most recent message (largest version number) from each distinct forward contributor
    // contrib_recent[eqinfo2gm-contour] = cei
    // contrib_recent[eqinfo2gm-map] = cei
    for(size_t i = 0; i < contrib.size(); i++) {
        AlgoMapIter it = algo_messages.find(contrib[i]);
        CoreEventInfo *cei = it->second;
        string instance = cei->getProgramInstance();

        // get "eqinfo2gm-contour" from "eqinfo2gm-contour@eew-ci-prod1"
        size_t pos = instance.find("@");
        if(pos != string::npos) instance = instance.substr(0, pos);

        // if the instance is eqinfo2gm-contour or eqinfo2gm-map
        if(forward_contributors.find(instance) != forward_contributors.end()) {
            // find the instance (cei) with the largest version
            if((jt = contrib_recent.find(instance)) != contrib_recent.end()) {
                if(cei->getVersion() > jt->second->getVersion()) {
                    contrib_recent[instance] = cei;
                }
            }
            else {
                contrib_recent[instance] = cei;
            }
        }
    }

    // loop over distinct eqinfo2gm-contour and/or eqinfo2gm-map with the largest version number
    for(jt = contrib_recent.begin(); jt != contrib_recent.end(); ++jt) {
        CoreEventInfo *cei = jt->second;
        string instance = jt->first;
        // Example forwarding properties:
        // ForwardContributors eqinfo2gm-contour, eqinfo2gm-map
        // eqinfo2gm-contour.topic eew.sys.gm-contour.data
        // eqinfo2gm-map.topic eew.sys.gm-map.data
        // forward contributors, if requested.
        // forward_contributors[eqinfo2gm-contour] = eew.sys.gm-contour.data
        // forward_contributors[eqinfo2gm-map] = eew.sys.gm-map.data
        //
        map<string, DMMessageSender *>::iterator kt = forward_senders.find(instance);
        if(kt != forward_senders.end()) { // this should always be true
            // save the id, ver and type to restore below. Otherwise the comparison of this
            // contributor with incoming messages might not work correctly
            string id = cei->getID();
            int ver = cei->getVersion();
            enum nudMessageType type = cei->getMessageType();
            if(instance == "eqinfo2gm-contour") {
                if(!dmevent->publish_next_contour || ver <= dmevent->contour_ver_published) continue;
            }
            else if(instance == "eqinfo2gm-map") {
                if(!dmevent->publish_next_map || ver <= dmevent->map_ver_published) continue;
            }

            // renumber the versions starting at zero
            map<string, int>::iterator v = dmevent->forward_version.find(instance);
            if(v == dmevent->forward_version.end()) {
                cei->setVersion(0);
                cei->setMessageType(NEW);
                dmevent->forward_version[instance] = 0;
            }
            else {
                int ver = v->second + 1;
                cei->setVersion(ver);
                cei->setMessageType(UPDATE);
                dmevent->forward_version[instance] = ver;
            }
            dmevent->forward_version[instance] = cei->getVersion();
                
            DMMessageSender *sender = kt->second;
            cei->setID(dmevent->getID());
            cei->setTimestamp(TimeStamp::current_time().ts_as_double(NOMINAL_EPOCH_TIME));
            sender->sendMessage(cei);

            // restore these to the event contributor
            cei->setID(id);
            cei->setVersion(ver);
            cei->setMessageType(type);

            if(instance == "eqinfo2gm-contour") {
                dmevent->contour_ver_published = ver;
                dmevent->publish_next_contour = false;
            }
            else if(instance == "eqinfo2gm-map") {
                dmevent->map_ver_published = ver;
                dmevent->publish_next_map = false;
            }
        }
    }
}

bool DecisionModule::messageChanged(DMEvent *event)
{
    if(!publish_changes_only) return true;

    event->setTimestamp("0"); // prevents timestamp in <event> from updating

    int version = event->getVersion();
    // set version to 0 for comparison
    event->setVersion(0);
    string new_msg = dmevent_sender->getEncodedMessage(event);
    event->setVersion(version);
    event->setTimestamp(""); // causes timestamp to be updated in sendMessage()

    // remove the <contributors> ... </contributors> section before comparison
    size_t start_contrib = new_msg.find("<contributors>");
    if(start_contrib != string::npos) {
        size_t end_contrib = new_msg.find("</contributors>", start_contrib);
        if(end_contrib != string::npos && end_contrib > start_contrib) {
            end_contrib += strlen("</contributors>");
            new_msg = new_msg.substr(0, start_contrib) + new_msg.substr(end_contrib);
        }
    }

    if(event->getVersion() < 0 || event->previous_message.empty()
        || ( compare_num_contrib && event->getNumberContributors() != event->previous_num_contributors)
        || new_msg != event->previous_message)
    {
        event->previous_num_contributors = event->getNumberContributors();
        event->previous_message = new_msg;
        os << "XMLchange: true" << endl;
        return true;
    }
    os << "XMLchange: false" << endl;
    return false;
}

bool DecisionModule::filter(DMEvent *event, string &reason)
{
    char s[50];

    if(!event->publishedOne())
    {
        // The event has not been previously published
	Region *r = NULL;
	bool regional_mag_ok = false;

	for(int i = 0; i < (int)regions.size(); i++) {
	    if(regions[i]->inRegion(event)) {
		r = regions[i];
		if(event->getMagnitude() >= r->min_mag) regional_mag_ok = true;
		break;
	    }
	}
	if(r == NULL) {
	    reason.assign(" Outside all regions.\n No Alert: ");
	    if(!use_gm_contour) return true;
	}
	else if(regional_mag_ok == false) {
	    snprintf(s, sizeof(s), " DM Mag < %.2f in %s region.\n No Alert: ", r->min_mag, r->name.c_str());
	    reason.assign(s);
	    if(!use_gm_contour) return true;
	}
	if(use_gm_contour) {
	    // If there is an eqinfo2gm message with contour attached. Do not alert if the eqinfo2gm
            // contour does not intersect the alert boundary contour.
            if(!insideGMContour(event)) {
                reason.assign("  Holding: ");
                return true;
            }
	}
    }

    if(event->getLikelyhood() < constants.dm_min_lh) {
	snprintf(s, sizeof(s), " Likelihood < %.2f.\n No Alert: ", constants.dm_min_lh);
	reason.assign(s);
	return true;
    }
    else if(!event->passedTwoAlgMag(constants.two_alg_mag)) {
	reason.assign("  Waiting: ");
	return true;
    }
    return false;
}

bool DecisionModule::publishAloneOk(DMEvent *event)
{
    vector<AlgorithmEvent> contributors = event->getAlgoEvents();
    set<string> distinct_core_contributors;

    // check if the number of distinct core contributors is > 1
    // or at least one core contributor can alert alone.
    bool alert_alone = false;

    for(size_t i = 0; i < contributors.size(); i++) {
        AlgoMapIter it = algo_messages.find(contributors[i]);
        CoreEventInfo *cei = (*it).second;
        string name = cei->getSystemNameString();

        // contributors can include algorithms that are not core-contributors
	if(core_contributors.find(name) != core_contributors.end())
        {
            distinct_core_contributors.insert(name);
            if(alertAlgAlone.find(name) != alertAlgAlone.end()) alert_alone = true;
        }
    }
    if(distinct_core_contributors.size() > 1 || alert_alone) {
        return true;
    }
    return false;
}

// Get a new eventid.
string DecisionModule::getNewEventID()
{
    FILE *fp;
    int event_id;

    // Get new DM eventid, and save it to non-volatile storage.
    event_id = next_event_id++;
    if( (fp = fopen(evid_filename.c_str(), "w")) ) {
	fprintf(fp, "%d\n", next_event_id);
	fclose(fp);
    }
    else {
	cerr << "Cannot write to " << evid_filename << endl;
	cerr << "       " << strerror(errno) << endl;
    }
    char s[50];
    snprintf(s, sizeof(s), "%d", event_id);
    return string(s);
}

void DecisionModule::getProperties() throw(string)
{
    ewprop->getProperty("EventIDFile", evid_filename);
    ewprop->getProperty("AlgorithmUser", alg_username);
    ewprop->getProperty("AlgorithmPassword", alg_password);
    ewprop->getProperty("AlgorithmURI", alg_uri); // failover:(tcp://localhost:61616)
    ewprop->getProperty("AlgorithmTopic", alg_topic); // eew.alg.*.data
    ewprop->getProperty("HeartbeatInTopic", heartbeat_in_topic); // eew.alg.*.hb
    ewprop->getProperty("HeartbeatOutTopic", heartbeat_out_topic); // eew.sys.dm.hb
    ewprop->getProperty("HeartbeatSender", heartbeat_sender); // dm.eew-bk-prod1
    ewprop->getProperty("HeartbeatOriginator", heartbeat_originator); // dm.eew-bk-prod1
    ewprop->getProperty("AlertUser", alert_username);
    ewprop->getProperty("AlertPassword", alert_password);
    ewprop->getProperty("AlertURI", alert_uri); // failover:(tcp://localhost:61616)
    ewprop->getProperty("AlertTopic", alert_topic); // eew.sys.dm.data
    ewprop->getProperty("LogURI", log_uri); // failover:(tcp://localhost:61616)
    ewprop->getProperty("LogTopic", log_topic); // eew.sys.dm.log

    ewprop->getProperty("DMEventTimeoutSecs", &constants.dmevent_timeout_secs);
    ewprop->getProperty("AlgEventTimeoutSecs", &constants.algevent_timeout_secs);

    ewprop->getProperty("MaxMisfit", &constants.max_misfit);
    ewprop->getProperty("MaxDistkmToEvent", &constants.max_distkm_to_event);
    ewprop->getProperty("MaxTimeToEvent", &constants.max_time_to_event);
    ewprop->getProperty("DmMinLikelihood", &constants.dm_min_lh);

    ewprop->getProperty("MinMagnitude", &constants.min_mag);
    ewprop->getProperty("MaxMagnitude", &constants.max_mag);
    ewprop->getProperty("MaxMagUncer", &constants.max_mag_uncer);
    ewprop->getProperty("MaxLatUncer", &constants.max_lat_uncer);
    ewprop->getProperty("MaxLonUncer", &constants.max_lon_uncer);

    ewprop->getProperty("MinDepthKm", &constants.min_depth_km);
    ewprop->getProperty("MaxDepthKm", &constants.max_depth_km);
    ewprop->getProperty("MaxDepthUncer", &constants.max_depth_uncer);

    ewprop->getProperty("MinDepthKm", &constants.min_depth_km);
    ewprop->getProperty("MaxDepthKm", &constants.max_depth_km);
    ewprop->getProperty("MaxDepthUncer", &constants.max_depth_uncer);

    ewprop->getProperty("MaxOriginTimeUncer", &constants.max_otime_uncer);

    ewprop->getProperty("MinLikelihood", &constants.min_likelihood);
    ewprop->getProperty("TwoAlgMagnitude", &constants.two_alg_mag);
    ewprop->getProperty("FaultInfoMinMag", &constants.fault_info_min_mag);

    ewprop->getProperty("Verbose", &verbose, 1); // optional
    ewprop->getProperty("PublishChangesOnly", &publish_changes_only, false); // optional
    ewprop->getProperty("CompareNumContrib", &compare_num_contrib, true); // optional
    ewprop->getProperty("UseRepublishThreshold", &use_republish_threshold, false); // optional
    ewprop->getProperty("RepublishInterval", &republish_interval, 0); // optional

    ewprop->getProperty("UseGMContour", &use_gm_contour, false); // optional

    ewprop->getProperty("MaxOriginTimeUncer", &constants.max_otime_uncer);

    ewprop->getProperty("CHANGE_THRESHOLD_MAG", &constants.change_threshold_mag,
                         CHANGE_THRESHOLD_MAG);
    ewprop->getProperty("CHANGE_THRESHOLD_MAG_UNCER", &constants.change_threshold_mag_uncer,
                         CHANGE_THRESHOLD_MAG_UNCER);
    ewprop->getProperty("CHANGE_THRESHOLD_LAT", &constants.change_threshold_lat,
                        CHANGE_THRESHOLD_LAT);
    ewprop->getProperty("CHANGE_THRESHOLD_LAT_UNCER", &constants.change_threshold_lat_uncer,
                         CHANGE_THRESHOLD_LAT_UNCER);
    ewprop->getProperty("CHANGE_THRESHOLD_LON", &constants.change_threshold_lon,
                         CHANGE_THRESHOLD_LON);
    ewprop->getProperty("CHANGE_THRESHOLD_LON_UNCER", &constants.change_threshold_lon_uncer,
                         CHANGE_THRESHOLD_LON_UNCER);
    ewprop->getProperty("CHANGE_THRESHOLD_DEPTH", &constants.change_threshold_depth,
                         CHANGE_THRESHOLD_DEPTH);
    ewprop->getProperty("CHANGE_THRESHOLD_DEPTH_UNCER", &constants.change_threshold_depth_uncer,
                         CHANGE_THRESHOLD_DEPTH_UNCER);
    ewprop->getProperty("CHANGE_THRESHOLD_OTIME", &constants.change_threshold_otime,
                         CHANGE_THRESHOLD_OTIME);
    ewprop->getProperty("CHANGE_THRESHOLD_OTIME_UNCER", &constants.change_threshold_otime_uncer,
                         CHANGE_THRESHOLD_OTIME_UNCER);
    ewprop->getProperty("CHANGE_THRESHOLD_LKLHD", &constants.change_threshold_lklhd,
                         CHANGE_THRESHOLD_LKLHD);
    ewprop->getProperty("CHANGE_THRESHOLD_NUM_STATIONS", &constants.change_threshold_num_stations,
                         CHANGE_THRESHOLD_NUM_STATIONS);

    getDMSystemName();
    getAlgNames();
    getAlgCoreContributors();
    getForwardContributors();
    getRegions();
    getAlgRegions();

    os << "EventIDFile: " << evid_filename << endl;
    os << "AlgorithmUser: " <<  alg_username << endl;
    os << "AlgorithmURI: " <<  alg_uri << endl;
    os << "AlgorithmTopic: " <<  alg_topic << endl;
    os << "AlertUser: " <<  alert_username << endl;
    os << "AlertURI: " <<  alert_uri << endl;
    os << "AlertTopic: " << alert_topic << endl;
    os << "LogURI: " <<  log_uri << endl;
    os << "LogTopic: " << log_topic << endl;
    os << "HeartbeatInTopic: " << heartbeat_in_topic << endl;
    os << "HeartbeatOutTopic: " << heartbeat_out_topic << endl;
    os << "HeartbeatSender: " << heartbeat_sender << endl;
    os << "HeartbeatOriginator: " << heartbeat_originator << endl;

    os << "DMEventTimeoutSecs: " << constants.dmevent_timeout_secs << endl;
    os << "AlgEventTimeoutSecs: " << constants.algevent_timeout_secs << endl;

    os << "MaxMisfit: " << constants.max_misfit << endl;
    os << "MaxDistkmToEvent: " << constants.max_distkm_to_event << endl;
    os << "MaxTimeToEvent: " << constants.max_time_to_event << endl;
    os << "DmMinLikelihood: " << constants.dm_min_lh << endl;

    os << "MinMagnitude: " << constants.min_mag << endl;
    os << "MaxMagnitude: " << constants.max_mag << endl;
    os << "MaxMagUncer: " << constants.max_mag_uncer << endl;
    os << "MaxLatUncer: " << constants.max_lat_uncer << endl;
    os << "MaxLonUncer: " << constants.max_lon_uncer << endl;
    os << "MinDepthKm: " << constants.min_depth_km << endl;
    os << "MaxDepthKm: " << constants.max_depth_km << endl;
    os << "MaxDepthUncer: " << constants.max_depth_uncer << endl;
    os << "MaxOriginTimeUncer: " << constants.max_otime_uncer << endl;
    os << "MinLikelihood: " << constants.min_likelihood << endl;

    os << "Regions: ";
    for(int i = 0; i < (int)regions.size(); i++) {
	os << regions[i]->name;
	if(i < (int)regions.size()-1) os << ",";
    }
    os << endl;
    for(int i = 0; i < (int)regions.size(); i++) os << regions[i]->toString() << endl;

    os << "Verbose: " <<  verbose << endl;
    os << "PublishChangesOnly: " <<  publish_changes_only << endl;
    os << "CompareNumContrib: " <<  compare_num_contrib << endl;
    os << "UseRepublishThreshold: " <<  use_republish_threshold << endl;
    os << "RepublishInterval: " << republish_interval << endl;
    os << "TwoAlgMagnitude: " << constants.two_alg_mag << endl;
    os << "FaultInfoMinMag: " << constants.fault_info_min_mag << endl;
    os << "RepublishInterval: " << republish_interval << endl;
    os << "UseGMContour: " << use_gm_contour << endl;
    os << "DMSystemName: " << dm_name << endl;

    os << "Number of algorithms: " << algorithm_names.size() << endl;
    set<string>::iterator it;
    for(it = algorithm_names.begin(); it != algorithm_names.end(); it++)
    {
	string s = (*it);
	os << s;
	if(core_contributors.find(s) != core_contributors.end()) {
	    os << " core_contributor";
	}
	else {
	    os << " not_core_contributor";
	}
	if(alertAlgAlone.find(s) != alertAlgAlone.end()) {
	    os << " alert_alone=true";
	}
	else {
	    os << " alert_alone=false";
	}
        os << endl;

	map<string, vector<Region *> >::iterator it = algorithm_regions.find(s);

	if(it != algorithm_regions.end()) {
	    os << s << "_regions: ";
	    for(vector<Region *>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
		os << (*jt)->name << " ";
	    }
	    os << endl;
	    for(vector<Region *>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
		os << (*jt)->toString() << endl;
	    }
	}
    }
    if(core_contributors.size() <= 1) {
	os << "Warning: No Alert Possible for M > " << constants.two_alg_mag << " (TwoAlgMagnitude)." << endl;
    }
    os << "Number of ForwardContributors: " << forward_contributors.size() << endl;
    map<string, string>::iterator jt; 
   for(jt = forward_contributors.begin(); jt != forward_contributors.end(); jt++) {
	os << jt->first << " topic: " << jt->second << endl;
    }
}

void DecisionModule::getEventId()
{
    int start_eventid = 1;
    FILE *fp;

    if((fp = fopen(evid_filename.c_str(), "r")) != NULL) {
	fscanf(fp, "%d", &start_eventid);
	fclose(fp);
    }

    if(start_eventid <= 0) {
	next_event_id = 1;
    } else {
	next_event_id = start_eventid;
    }

    if((fp = fopen(evid_filename.c_str(), "w")) != NULL) {
	fprintf(fp, "%d\n", next_event_id);
	fclose(fp);
    }
    else {
	os << "Cannot open file " << evid_filename << endl << strerror(errno) << endl;
    }
    os << "Next eventid: " << next_event_id << endl;
}

class Suppress
{
  public :
    Suppress() { }

    static bool inBayArea(CoreEventInfo *cei)
    {
	// for now, simply hard code.
	// bay area polygon
	Coordinate bay_area[] = {39.0,-124.0, 39.0,-122.0, 36.45,-120.4, 36.45,-122.1, 39.0,-124.0};
	double slope[5];
	int nbay = 5;

	// compute slope for use in determining if a point is inside the polygon
	for(int i = 0; i < nbay-1; i++) {
	    slope[i] = (bay_area[i+1].lat != bay_area[i].lat) ?
			(bay_area[i+1].lon - bay_area[i].lon)/(bay_area[i+1].lat - bay_area[i].lat) : 0.;
	}
	double lat = cei->getLatitude();
	double lon = cei->getLongitude();
	bool in = false;
        for(int i = 0; i < nbay-1; i++) {
            if( ((bay_area[i].lat > lat) != (bay_area[i+1].lat > lat)) &&
		(lon < slope[i]*(lat - bay_area[i].lat) + bay_area[i].lon) )
            {
                in = !in;
            }
        }
        return in;
    }

    static bool inNorthernCA(CoreEventInfo *cei)
    {
	double lat1 = 34.50, lon1 = -121.25, lat2 = 37.43, lon2 = -117.76;
	double lat = cei->getLatitude();
	double lon = cei->getLongitude();
	if(lon < lon1) {
	    return lat >= lat1;
	}
	else if(lon > lon2) {
	    return lat > lat2;
	}
	double slope = (lat2-lat1)/(lon2-lon1);

	double boundary_lat = lat1 + (lon-lon1)*slope;

	return (lat > boundary_lat);
    }
};

string DecisionModule::logTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now = (double)tv.tv_sec + (double)tv.tv_usec*1.e-06;
    double t = round(1000*now)/1000.;
    INT_TIME it = nepoch_to_int(t);
    char *c = time_to_str(it, MONTHS_FMT);
    c[10] = '-';
    string s = string(c);
    return string("GMT: ") + s.substr(0, s.length()-1);
}

Region::Region(string region_name, double minMag, double minPGA, double minPGV, double minMMI, vector<Coordinate> &poly)
{
    name = region_name;
    min_mag = minMag;
    min_pga = minPGA;
    min_pgv = minPGV;
    min_mmi = minMMI;
    npoly = (int)poly.size();
    polygon = new Coordinate[npoly];
    slope = new double[npoly-1];

    for(int i = 0; i < npoly; i++) polygon[i] = poly[i];

    for(int i = 0; i < npoly-1; i++) {
	slope[i] = (polygon[i+1].lat != polygon[i].lat) ?
			(polygon[i+1].lon - polygon[i].lon)/(polygon[i+1].lat - polygon[i].lat) : 0.;
    }
}

Region::~Region()
{
    delete[] polygon;
    delete[] slope;
}

bool Region::inRegion(CoreEventInfo *cei)
{
    double lat = cei->getLatitude();
    double lon = cei->getLongitude();
    bool in = false;
    for(int i = 0; i < npoly-1; i++) {
	if( ((polygon[i].lat > lat) != (polygon[i+1].lat > lat)) &&
	     (lon < slope[i]*(lat - polygon[i].lat) + polygon[i].lon) )
	{
	    in = !in;
	}
    }
    return in;
}

string Region::toString()
{
    char s[50];
    stringstream ss;

    snprintf(s, sizeof(s), "%.2f", min_pga);
    ss << name << "_MinPga: " << s << endl;
    snprintf(s, sizeof(s), "%.2f", min_pgv);
    ss << name << "_MinPgv: " << s << endl;
    snprintf(s, sizeof(s), "%.2f", min_mmi);
    ss << name << "_MinMMI: " << s << endl;
    snprintf(s, sizeof(s), "%.2f", min_mag);
    ss << name << "_MinMag: " << s << endl;
    ss << name << "_LatLon:";
    for(int i = 0; i < npoly; i++) {
	const char *comma = (i < npoly-1) ? "," : "";
	snprintf(s, sizeof(s), " %.4f,%.4f%s", polygon[i].lat, polygon[i].lon, comma);
	ss << s;
    }
    return ss.str();
}

static bool toDouble(const char *s, double *dbl)
{
    char *endptr, last_char;
    const char *c;
    double d;
    int n;

    for(c = s; *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
        d = strtod(c, &endptr);
        last_char = *endptr;
        if(last_char == c[n]) {
	    *dbl = d;
            return true;
        }
    }
    return false;
}

/** Get all optional alert regions
 *  The 'Regions' property specifies one or more regions with varying DM alert minimum magnitude
 *  example:
 *
 *  Regions CA,PNW
 *  CA_MinMag 3.0
 *  CA_latlon 37.43,-117.76,39,-120,42,-120,42,-125,42,-126,40,-126,34.5,-121.25,31.5,...
 *
 *  PNW_MinMag 3.0
 *  PNW_latlon 42.0,-122.70000,42.0,-121.41670,42.0,-120.00000,42.0,-117.00000,49.0,...
 *
 */
void DecisionModule::getRegions() throw(string)
{
    string prop;
    vector<string> region_names;

    ewprop->getProperty("Regions", prop);

    char *tok, *p, *last;
    char *s = strdup(prop.c_str());

    tok = s;
    while((p = strtok_r(tok, ", \t", &last)) != NULL) {
	tok = NULL;
	region_names.push_back(string(p));
    }
    free(s);
    if(region_names.size() <= 0) {
	throw string("Invalid Regions parameter");
    }

    Region *r;

    for(int i = 0; i < (int)region_names.size(); i++) {
	if((r = getRegion(region_names[i])) != NULL) regions.push_back(r);
    }
}

/** Get all algorithm regions.
 *  There can be optional algorithm regions for each of the algorithm names in the AlgNames property
 *  Example:
 *
 *  AlgNames  epic, finder
 *  finder_regions finder_region1, finder_region2
 *  finder_region1_MinMag 3.0    // optional. Equals 0., if not specified.
 *  finder_region1_latlon 42.0,-122.70000,42.0,-121.41670,42.0,-120.00000,42.0,-117.00000,49.0,...
 *  finder_region2_MinMag 4.0    // optional. Equals 0., if not specified.
 *  finder_region2_latlon 37.43,-117.76,39,-120,42,-120,42,-125,42,-126,40,-126,34.5,...
 *
 *  Optional region minimum values: MinPga, MinPgv, and MinMMI.
 *  finder_region1_minPga
 *  finder_region1_minPgv
 *  finder_region1_minMMI
 */
void DecisionModule::getAlgRegions() throw(string)
{
    bool all_alert_alone_found = true;

    set<string>::iterator it;
    for(it = algorithm_names.begin(); it != algorithm_names.end(); it++)
    {
	Region *r;
	string name = (*it) + "_regions";
	string prop;
	ewprop->getProperty(name.c_str(), prop, string(""));

	if(prop.length() > 0) {
	    vector<Region *> regions;
	    char *tok, *p, *last;
	    char *s = strdup(prop.c_str());

	    tok = s;
	    while((p = strtok_r(tok, ", \t", &last)) != NULL) {
		tok = NULL;
		string region_name = string(p);
		if((r = getRegion(region_name)) != NULL) regions.push_back(r);
	    }
	    free(s);

	    if(regions.size() <= 0) {
		throw string("Invalid " + name + " parameter");
	    }
	    algorithm_regions.insert(pair<string, vector<Region *> >(*it, regions) );
	}

	// get alg_alone property
	name = (*it) + "_alert_alone";
	bool alert_alone;
	ewprop->getProperty(name.c_str(), &alert_alone);
	if(alert_alone) {
	    alertAlgAlone.insert(*it);
	}
    }
    if(!all_alert_alone_found) {
	throw string("Missing algorithm_alert_alone parameter(s)");
    }

    // Check if there is one algorithm that is a core contributor with alert_alone=true
    for(it = algorithm_names.begin(); it != algorithm_names.end(); it++) {
	if(core_contributors.find(*it) != core_contributors.end()
	    && alertAlgAlone.find(*it) != alertAlgAlone.end()) break;
    }
    // if not, then need two core contributors
    if(it == algorithm_names.end() && core_contributors.size() <= 1) {
	throw string("Alert NOT possible. See parameters AlgCoreContributors and algorithm_alert_alone.");
    }

    // check if constants.two_alg_mag < 10.0 and only one algorithm can alert
    if(core_contributors.size() <= 1) {
	if(constants.two_alg_mag <= 8.0) {
	    throw string("NO ALERT POSSIBLE for M >= 8.0. See parameters AlgCoreContributors and TwoAlgMagnitude.");
	}
    }
}

/** Regions are specified by counter-clockwise polygons (lat,lon,...)
 */
Region * DecisionModule::getRegion(string region_name) throw(string)
{
    char *tok, *p, *last;
    string prop;
    double minMag, minPGA, minPGV, minMMI;
    vector<Coordinate> poly;

    string prop_minmag(region_name + "_MinMag");
    ewprop->getProperty(prop_minmag.c_str(), &minMag, 0.);

    string prop_minpga(region_name + "_MinPGA");
    ewprop->getProperty(prop_minpga.c_str(), &minPGA, 0.);

    string prop_minpgv(region_name + "_MinPGV");
    ewprop->getProperty(prop_minpgv.c_str(), &minPGV, 0.);

    string prop_minmmi(region_name + "_MinMMI");
    ewprop->getProperty(prop_minmmi.c_str(), &minMMI, 0.);

    string prop_name(region_name + "_latlon");
    ewprop->getProperty(prop_name.c_str(), prop);
    char *s = strdup(prop.c_str());
    tok = s;
    while((p = strtok_r(tok, ", \t", &last)) != NULL) {
	tok = NULL;
	Coordinate c;
	if(!toDouble(p, &c.lat) || (p = strtok_r(tok, ", \t", &last)) == NULL ||
		!toDouble(p, &c.lon))
	{
	    if(p != NULL) {
		throw string("Invalid Region " + region_name + "_latlon: " + p);
	    }
	    else {
		throw string("Invalid Region " + region_name + " " + prop);
	    }
	}
	poly.push_back(c);
    }
    free(s);
    if(poly.size() < 3) {
	throw string("Invalid " + region_name + "_latlon");
    }

    // check if the last point is the same as the first point
    int n = poly.size() - 1;
    if(poly[n].lat != poly[0].lat || poly[n].lon != poly[0].lon)
    {
	    poly.push_back(poly[0]);
    }
    return new Region(region_name, minMag, minPGA, minPGV, minMMI, poly);
}

void DecisionModule::getDMSystemName() throw(string)
{
    ewprop->getProperty("DMSystemName", dm_name);

    if(dm_name == "dm") {
	dm_system_name = DM;
    }
    else if(dm_name == "sa") {
	dm_system_name = SA;
    }
    else {
	throw string("Unknown DMSystemName parameter: " + dm_name + " Must be from 'dm' or 'sa'");
    }
}

void DecisionModule::getAlgNames() throw(string)
{
    string prop;

    ewprop->getProperty("AlgNames", prop);

    char *tok, *p, *last;
    char *s = strdup(prop.c_str());

    tok = s;
    while((p = strtok_r(tok, ", \t", &last)) != NULL) {
	tok = NULL;
	algorithm_names.insert(string(p));
    }
    free(s);
    if(algorithm_names.size() <= 0) {
	throw string("Invalid AlgNames parameter");
    }
}

void DecisionModule::getAlgCoreContributors() throw(string)
{
    string prop;

    ewprop->getProperty("AlgCoreContributors", prop);

    char *tok, *p, *last;
    char *s = strdup(prop.c_str());

    tok = s;
    while((p = strtok_r(tok, ", \t", &last)) != NULL) {
	tok = NULL;
	if(algorithm_names.find(string(p)) == algorithm_names.end()) {
	    throw string("Invalid AlgCoreContributors algorithm: " + string(p));
	}
	core_contributors.insert(string(p));
    }
    free(s);
    if(core_contributors.size() <= 0) {
	throw string("Invalid AlgCoreContributors parameter");
    }
}

void DecisionModule::getForwardContributors() throw(string)
{
    // forward-contributors are specified by the first part of their instance name
    // ie, for instance alg_instance="eqinfo2gm-contour@eew-bk-dev1", the 
    // forward-contributors name is "eqinfo2gm-contour"
    // Example forwarding properties:
    // ForwardContributors eqinfo2gm-contour, eqinfo2gm-map
    // eqinfo2gm-contour.topic eew.sys.gm-contour.data
    // eqinfo2gm-map.topic eew.sys.gm-map.data
    // forward_contributors[eqinfo2gm-contour] = eew.sys.gm-contour.data
    // forward_contributors[eqinfo2gm-map] = eew.sys.gm-map.data
    string prop;

    ewprop->getProperty("ForwardContributors", prop);

    if(!strcasecmp(prop.c_str(), "none")) return;

    char *tok, *p, *last;
    char *s = strdup(prop.c_str());

    tok = s;
    while((p = strtok_r(tok, ", \t", &last)) != NULL) {
	tok = NULL;
	string instance = string(p);
	string name = instance + ".topic";
	ewprop->getProperty(name.c_str(), prop);
	forward_contributors[instance] = prop;
    }
    free(s);
}

bool DecisionModule::insideGMContour(DMEvent *event)
{
    AlgorithmEvent *eqinfo;

    if((eqinfo = event->getContributor(EQINFO2GM)) != NULL) {
        CoreEventInfo *cei = getAlgoMsg(*eqinfo);
        if(cei != NULL) {
            GMMessage *gm = (GMMessage *)cei;

            list<GMContour>::iterator beg = gm->getMultiContourIteratorBegin();
	    list<GMContour>::iterator end = gm->getMultiContourIteratorEnd();
	    for(list<GMContour>::iterator ic = beg; ic != end; ic++) {
	        for(int i = 0; i < (int)regions.size(); i++) {
		    if(regions[i]->overlapsRegion(ic->getLats(), ic->getLons())) {
		        if( ic->getPGA() >= regions[i]->min_pga &&
			    ic->getPGV() >= regions[i]->min_pgv &&
			    ic->getMMI() >= regions[i]->min_mmi) return true;
                    }
		}
	    }
            if(beg != end) {
                // We an eqinfo2gm with contours but the event either does not overlap
                // one of the contours, or the event pga, pgv, or mmi is too low.
                return false;
            }
	}
    }
    // There are no contributing eqinfo2gm messages with contours.
    // Check if the event has a core contributor that can alert alone
    return publishAloneOk(event);
}

/** Check if the input contour overlaps with this Region.
 *  If any of the vertices of the input polygon are inside this Region's polygon,
 *  or any of this Region's vertices are inside the input polygon, then the two
 *  polygons overlap.
 *  @param[in] contour_lat coordinates of a closed polygon
 *  @param[in] contour_lon coordinates of a closed polygon
 *  @returns true if the input polygon overlaps this Region's polygon
 */
bool Region::overlapsRegion(vector<double> contour_lat, vector<double> contour_lon)
{
    int num_contour_vertices = (int)contour_lat.size() - 1;

    /* Test if any of the input polygon vertices (contour_lat[j], contour_lon[j])
     * are inside this Region's polygon.
     */
    for(int j = 0; j < num_contour_vertices; j++) {
	double lat = contour_lat[j];
	double lon = contour_lon[j];

	bool in = false;
	for(int i = 0; i < npoly-1; i++) {
	    if( ((polygon[i].lat > lat) != (polygon[i+1].lat > lat)) &&
		(lon < slope[i]*(lat - polygon[i].lat) + polygon[i].lon) )
	    {
		in = !in;
	    }
	}
	if(in) return true;
    }

    double *contour_slope = new double[num_contour_vertices];
    for(int i = 0; i < num_contour_vertices; i++) {
	contour_slope[i] = (contour_lat[i+1] != contour_lat[i]) ?
		(contour_lon[i+1] - contour_lon[i])/(contour_lat[i+1] - contour_lat[i]) : 0.;
    }

    /* Test if any of this Region's vertices (polygon[j].lat,polygon[j].lon)
     * are inside the input polygon (contour_lat[], contour_lon[])
     */
    for(int j = 0; j < npoly-1; j++) {
	double lat = polygon[j].lat;
	double lon = polygon[j].lon;

	bool in = false;
	for(int i = 0; i < num_contour_vertices; i++) {
	    if( ((contour_lat[i] > lat) != (contour_lat[i+1] > lat)) &&
		(lon < contour_slope[i]*(lat - contour_lat[i]) + contour_lon[i]) )
	    {
		in = !in;
	    }
	}
	if(in) {
	    delete [] contour_slope;
	    return true;
	}
    }

    delete [] contour_slope;
    return false;
}

/** Check if an algorithm event location, loc, is within any of the regions for cei.
 *  Return true if the cei has no regions or loc is inside one of its regions
 *  Return false if the cei has regions and loc is outside all of cei's regions.
 */
bool DecisionModule::insideAlgRegions(CoreEventInfo *loc, CoreEventInfo *cei)
{
    // Get the Regions for the cei algorithm type
    map<string, vector<Region *> >::iterator it = algorithm_regions.find(cei->getSystemNameString());

    // If there are no regions specifed for cei, return true
    if(it == algorithm_regions.end() || (int)it->second.size() == 0) return true;

    for(vector<Region *>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
	// If the event is located within one of the algorithm regions, return true
	if((*jt)->inRegion(loc)) return true;
    }
    if(verbose) {
	char lat[20], lon[20];
	snprintf(lat, sizeof(lat), "%.3lf", loc->getLatitude());
	snprintf(lon, sizeof(lon), "%.3lf", loc->getLongitude());
	os << " Outside " << cei->getSystemNameString() << " Region: ["
		<< loc->getSystemNameString() << "," << loc->getID()
		<< "," << loc->getVersion() << "," << lat << "," << lon << "] " << endl;
    }
    return false; // loc is not inside any of cei's regions
}

