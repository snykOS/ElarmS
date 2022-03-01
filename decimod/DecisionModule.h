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
#ifndef __DecisionModule_h
#define __DecisionModule_h

#include <time.h>
#include <math.h>
#include <map>
#include <set>
#include <sstream>
#include "qlib2.h"
#include "DMEvent.h"
#include "DMMessageReceiver.h"
#include "DMMessageSender.h"
#include "AlgorithmEvent.h"
#include "DMconsts.h"

/** @mainpage Intro
 *
 *  @section decimod_section ShakeAlert Solution Aggregator and Decision Module Program
 *  The \ref decimod_page "Decision Module Program", decimod, receives messages from the EPIC,
 *  and FinDer event detection systems and generates EventMessages
 *  for users, such as the UserDisplay program. It also received messages from eqInfo2GM associates them with the appropriate event
 *  and forwards them to the message broker. The Decision Module attempts to associate
 *  algorithm event messages from the two different algorithm and various different systems (EPIC, FinDer, UCB, UW, CalTech)
 *  together and compute a combined event location, magnitude, time and likelihood. The combined
 *  event is referred to as a DMEvent. There are rules for how close the algorithm events must be
 *  in time and space, before they will be combined into a DMEvent. For Message Formats, check out the  \ref dm_lib_section .
 *
 *  @section dm_lib_section ShakeAlert Messaging Library
 *  The <a href="../dmlib/index.html">ShakeAlert Messaging Library (dmlib)</a> is the message handling API for the ShakeAlert EEW
 *  System. It is used to communicate event detection information between components of
 *  the system. It utilizes the Apache ActiveMQ open source messaging software for the
 *  actual data transmission and the Apache Xerces C++ open source API for encoding and
 *  decoding XML formatted messages. The Xerces DOMDocument class is used to
 *  encode XML messages and XercesDOMParser class is used to decode them.
 */

/**
 * @defgroup decimod ShakeAlert Solution Aggregator and Decision Module Program
 * @page decimod_page ShakeAlert Solution Aggregator and Decision Module Program
 * The Decision Module Program, decimod, receives "algorithm" messages from the Elarms,
 * Virtual Seismologist, and OnSite event detection systems and generates EventMessages
 * for user processes, such as the UserDisplay program. The Decision Module attempts to associate
 * algorithm event messages from the three different systems (Elarms, OnSite, and Virtual Seismologist)
 * together and compute a combined event location, magnitude, time and likelihood. The combined
 * event is referred to as a DMEvent. There are rules for how close the algorithm events must be
 * in time and space, before they will be combined into a DMEvent.
 *
 * \ref decimod_classes
 * 
 * \ref storage
 *
 * \ref processing_loop
 * 
 * \ref decimod_output
 * 
 * @section decimod_classes Program Classes
 * The \ref decimod "program classes" are listed \ref decimod "here". The DecisionModule
 * class creates the message handling objects DMMessageReceiver and DMMessageSender, and it
 * contains the main message processing loop. The DecisionModule creates a DMEvent class
 * object, which is a subclass of EventMessage, for each new combined event calculation
 * that is sent to the message broker. The DecisionModule class is described below.
 *
 * @section storage DecisionModule Storage maps and lists
 *
 * The message classes ElarmsMessage, VSMessage, and OnSiteMessage are referred to in the
 * decimod code as algorithm messages. These three classes are all subclasses of the
 * CoreEventInfo class.  Decimod uses two C++ map objects to store all of the messages that
 * it receives. These maps are both defined with a
 *  key=(\link CoreEventInfo::MessageCategory message_category\endlink,
 *  \link CoreEventInfo::eewSystemName system_name\endlink,
 *  \link CoreEventInfo::getID message_id\endlink) and a value=(CoreEventInfo *). The
 *  \link CoreEventInfo::MessageCategory message_category\endlink is LIVE or TEST. The
 *  \link CoreEventInfo::eewSystemName system_name\endlink is ELARMS, VS, or ONSITE. The message_id is
 * the algorithm event_id. The CoreEventInfo pointer is just a pointer to an ElarmsMessage,
 * VSMessage, or OnSiteMessage. There is a map to store non-delete messages and another map
 * to store the delete messages.
 * 
 * <pre>
 * 	map: \link DecisionModule::algo_messages algo_messages\endlink   : all algorithm messages, except type=DELETE
 * 	map: \link DecisionModule::delete_messages delete_messages\endlink : all algorithm messages with type=DELETE
 * </pre>
 * 
 * There is a list of unassociated algorithm messages called
 * <pre>
 * 	list: \link DecisionModule::free_algo_messages free_algo_messages\endlink
 * </pre>
 * All of the keys in the \link DecisionModule::free_algo_messages free_algo_messages\endlink map will also be in the \link DecisionModule::algo_messages algo_messages\endlink map.
 * (The key type (\link CoreEventInfo::MessageCategory message_category\endlink,
 *  \link CoreEventInfo::eewSystemName system_name\endlink,
 *  \link CoreEventInfo::getID message_id\endlink) is an AlgorithmEvent object).
 * 
 * There is one map to hold the DMEvent objects that decimod creates. This map has a
 * key=(\link DMEvent::getID dm_event_id\endlink) and value=(DMEvent object)
 * <pre>
 * 	map: \link DecisionModule::dm_events dm_events\endlink : all current\link DMEvent DMEvents\endlink
 * </pre>
 * 
 * One to three algorithm event messages, of unique type, contribute to a DMEvent object.
 * The contributing alogrithm messages are referenced in the DMEvent object through their
 * AlgorithmEvent key. The DMEvent class has a vector of contributing \link DMEvent::algo_events AlgorithmEvent keys\endlink.
 *
 * @section processing_loop Description of the DecisionModule Message Processing Loop
 * 
 * <pre>
 * Loop forever {
 *     Listen for a message for\link decimod::MESSAGE_WAIT MESSAGE_WAIT\endlink msecs
 * 
 *     set all DMEvent \link DMEvent::setPublishFlag publish_flags\endlink to false
 * 
 *     if (message received) {
 * 
 *         if (\link CoreEventInfo::getMessageType message type\endlink = \link nudMessageType.DELETE DELETE\endlink) {
 *             if (\link DecisionModule::algo_messages algo_messages\endlink contains the (\link CoreEventInfo::MessageCategory message_category\endlink,\link CoreEventInfo::eewSystemName system_name\endlink,\link CoreEventInfo::getID id\endlink)) {
 *                  find associated DMEvent, remove AlgorithmEvent from it, recalculate misfit and set publish_flag=true
 *                  delete the AlgorithmEvent key from the algo_messages map
 *             }
 *             save the message in the \link DecisionModule::delete_messages delete_messages\endlink map
 *         }
 *         else if (all message attribute values are within \link AlgorithmEvent::valuesOK acceptable\endlink ranges,
 *             and the message origin_time is not \link decimod::ALGOEVENT_TIMEOUT too old\endlink,
 *             and the message (message_category,system_name,id) is not in the delete_messages map)
 *         {
 *             \link AlgorithmEvent::checkCEIUncertainties correct\endlink any invalid uncertainty values
 *
 *             force magnitude uncertainty and likelihood to have constant values
 *
 *             if (algo_messages already contains (message_category,system_name,id)) {
 *                 if (new_message_version > current_message_version) {
 *                     merge in new AlgorithmEvent info into the existing message object
 *                     find associated DMEvent and set publish_flag=true
 *                     recompute misfit for associated DMEvent
 *                 }
 *             }
 *             else {
 *                 save new message in the algo_messages map
 * 
 *                 call associateAlgorithmEvent(new message)
 *             }
 *             update the free_algo_messages list (look for unassociated algorithm messages)
 *         }
 *     }
 * 
 *     remove any \link decimod::ALGOEVENT_TIMEOUT old\endlink algo_delete_events from the delete_messages map
 *     remove any \link decimod::ALGOEVENT_TIMEOUT old\endlink AlgorithmEvents and associated DMEvents from the algo_messages map
 * 
 *     if the misfit for any DMEvent is > \link decimod::MAX_MISFIT MAX_MISFIT\endlink, remove all AlgorithmEvents
 *            from that DMEvent and update the free_algo_messages list
 * 
 *     while ( free_algo_messages is not empty ) {
 *         call associateAlgorithmEvent(AlgorithmEvent)
 *         if the misfit for any DMEvent is > MAX_MISFIT, remove all AlgorithmEvents from the DMEvent
 *         update the free_algo_messages list
 *     }
 * 
 *     remove any old DMEvents and remove their associated AlgorithmEvents
 * 
 *     publish DMEvents ( all events that have publish_flag=true )
 * 
 *     publish delete message for empty DMEvents and remove them
 * }
 * 
 * associateAlgorithmEvent(AlgorithmEvent)
 * {
 *     try to associate the AlgorithmEvent to a DMEvent with the same message_category by finding the
 * 	 best fit (within \link decimod::MAX_TIME_TO_EVENT MAX_TIME_TO_EVENT\endlink and \link decimod::MAX_DISTANCE_TO_EVENT MAX_DISTANCE_TO_EVENT\endlink) among the DMEvents that
 *	 don't already contain an AlgorithmEvent from the same system
 * 
 *     if( can associate to a DMEvent ) {
 *         add the AlgorithmEvent key (message_category,system_name,id) to the DMEvent and set publish_flag=true
 *         recompute misfit for the DMEvent
 *     }
 *     else {
 *         look for a DMEvent that currently contains no AlgorithmEvents, but previously contained this AlgorithmEvent
 *         if ( find such a DMEvent ) {
 *             add this AlgorithmEvent to it and set publish_flag=true
 *             recompute misfit for the DMEvent
 *         }
 *         else {
 *             create a new DMEvent with this AlgorithmEvent and set publish_flag=true
 *         }
 *     }
 * }
 * </pre>
 *
 * Algorithm Event Attribute Ranges
 *
 * <pre>
 * \link decimod::VALID_MAG_MIN_MW VALID_MAG_MIN_MW\endlink	<= Magnitude	<= \link decimod::VALID_MAG_MAX_MW VALID_MAG_MAX_MW\endlink
 * \link decimod::VALID_LAT_MIN_DEG VALID_LAT_MIN_DEG\endlink	<= Latitude	<= \link decimod::VALID_LAT_MAX_DEG VALID_LAT_MAX_DEG\endlink
 * \link decimod::VALID_LON_MIN_DEG VALID_LON_MIN_DEG\endlink	<= Longitude	<= \link decimod::VALID_LON_MAX_DEG VALID_LON_MAX_DEG\endlink
 * \link decimod::VALID_LIKELIHOOD_MIN VALID_LIKELIHOOD_MIN\endlink	<= Likelihood	<= \link decimod::VALID_LIKELIHOOD_MAX VALID_LIKELIHOOD_MAX\endlink
 * \link decimod::VALID_DEPTH_MIN_KM VALID_DEPTH_MIN_KM\endlink	<= Depth	<= \link decimod::VALID_DEPTH_MAX_KM VALID_DEPTH_MAX_KM\endlink
 * \link decimod::VALID_O_TIME_MIN VALID_O_TIME_MIN\endlink	< OriginTime
 * </pre>
 *
 *  Correct algorithm uncertainty values
 *
 * <pre>
 * if( MagUncer < 0.0 or MagUncer > \link decimod::VALID_MAG_UNCER_MW VALID_MAG_UNCER_MW\endlink ) {
 *	MagUncer = \link decimod::DEFAULT_MAG_UNCER_MW DEFAULT_MAG_UNCER_MW\endlink
 * }
 * if( LatUncer < 0.0 or LatUncer > \link decimod::VALID_LAT_UNCER_DEG VALID_LAT_UNCER_DEG\endlink ) {
 *	LatUncer = \link decimod::DEFAULT_LAT_UNCER_DEG DEFAULT_LAT_UNCER_DEG\endlink
 * }
 * if( LonUncer < 0.0 or LonUncer > \link decimod::VALID_LON_UNCER_DEG VALID_LON_UNCER_DEG\endlink ) {
 *	LonUncer = \link decimod::DEFAULT_LON_UNCER_DEG DEFAULT_LON_UNCER_DEG\endlink
 * }
 * if( DepthUncer < 0.0 or DepthUncer > \link decimod::VALID_DEPTH_UNCER_KM VALID_DEPTH_UNCER_KM\endlink ) {
 *	DepthUncer = \link decimod::DEFAULT_DEPTH_UNCER_KM DEFAULT_DEPTH_UNCER_KM\endlink
 * }
 * if( OTUncer < 0.0 or OTUncer > \link decimod::VALID_O_TIME_UNCER VALID_O_TIME_UNCER\endlink ) {
 *	OTUncer = \link decimod::DEFAULT_O_TIME_UNCER DEFAULT_O_TIME_UNCER\endlink
 * }
 * </pre>
 *
 *  Force magnitude uncertainty and likelihood to have constant values
 *
 * <pre>
 * MagUncer = \link decimod::DEFAULT_MAG_UNCER_MW DEFAULT_MAG_UNCER_MW\endlink
 *
 * if( Likelihood >= 0.4 ) {
 *	Likelihood = \link decimod::DEFAULT_LIKELIHOOD DEFAULT_LIKELIHOOD\endlink
 * }
 * </pre>
 *
 * <table>
 * <tr><td><b>Parameter</b></td> <td><b>Value</b></td></tr>
 * <tr><td>MESSAGE_WAIT</td> <td>100msec</td></tr>
 * <tr><td>DMEVENT_TIMEOUT</td> <td>600sec</td></tr>
 * <tr><td>ALGOEVENT_TIMEOUT</td> <td>600sec</td></tr>
 * <tr><td>MAX_MISFIT</td> <td>2.0</td></tr>
 * <tr><td>MAX_DISTANCE_TO_EVENT</td> <td>100.0</td></tr>
 * <tr><td>MAX_TIME_TO_EVENT</td> <td>30.0</td></tr>
 * <tr><td>VALID_MAG_MIN_MW</td> <td>0.0</td></tr>
 * <tr><td>VALID_MAG_MAX_MW</td> <td>10.0</td></tr>
 * <tr><td>VALID_MAG_UNCER_MW</td> <td>0.5</td></tr>
 * <tr><td>DEFAULT_MAG_UNCER_MW</td> <td>0.5</td></tr>
 * <tr><td>VALID_LAT_MIN_DEG</td> <td>30.0</td></tr>
 * <tr><td>VALID_LAT_MAX_DEG</td> <td>45.0</td></tr>
 * <tr><td>VALID_LAT_UNCER_DEG</td> <td>0.5</td></tr>
 * <tr><td>DEFAULT_LAT_UNCER_DEG</td> <td>0.5</td></tr>
 * <tr><td>VALID_LON_MIN_DEG</td> <td>-128.0</td></tr>
 * <tr><td>VALID_LON_MAX_DEG</td> <td>-112.0</td></tr>
 * <tr><td>VALID_LON_UNCER_DEG</td> <td>0.5</td></tr>
 * <tr><td>DEFAULT_LON_UNCER_DEG</td> <td>0.5</td></tr>
 * <tr><td>VALID_O_TIME_MIN</td> <td>0</td></tr>
 * <tr><td>VALID_O_TIME_UNCER</td> <td>20</td></tr>
 * <tr><td>DEFAULT_O_TIME_UNCER</td> <td>20</td></tr>
 * <tr><td>VALID_DEPTH_MIN_KM</td> <td>-5.0</td></tr>
 * <tr><td>VALID_DEPTH_MAX_KM</td> <td>50.0</td></tr>
 * <tr><td>VALID_DEPTH_UNCER_KM</td> <td>50.0</td></tr>
 * <tr><td>DEFAULT_DEPTH_UNCER_KM</td> <td>50.0</td></tr>
 * <tr><td>VALID_LIKELIHOOD_MIN</td> <td>0.0</td></tr>
 * <tr><td>VALID_LIKELIHOOD_MAX</td> <td>1.0</td></tr>
 * <tr><td>DEFAULT_LIKELIHOOD</td> <td>0.7</td></tr>
 * </table>
 *
 * DMEvent Calculation
 *
 * @f$ {\displaystyle Magnitude}_{ \displaystyle dm} ~~=~~ \displaystyle \frac{ \displaystyle \sum_{\displaystyle algevent} \displaystyle \frac{{\displaystyle Mag}_{\displaystyle alg}}{{{\displaystyle MagUncer}_{\displaystyle alg}}^{\displaystyle 2}} } { \displaystyle \sum_{\displaystyle algevent}  \displaystyle \frac{\displaystyle 1}{{{\displaystyle \displaystyle MagUncer}_{\displaystyle alg}}^{\displaystyle 2}}  } @f$
 *
 * @f$ {\displaystyle OriginTime}_{ \displaystyle dm} ~~=~~ \displaystyle \frac{ \displaystyle \sum_{\displaystyle algevent} \displaystyle \frac{{\displaystyle OT}_{\displaystyle alg}}{{{\displaystyle OTUncer}_{\displaystyle alg}}^{\displaystyle 2}} } { \displaystyle \sum_{\displaystyle algevent}  \displaystyle \frac{\displaystyle 1}{{{\displaystyle \displaystyle OTUncer}_{\displaystyle alg}}^{\displaystyle 2}}  } @f$
 *
 * @f$ {\displaystyle Latitude}_{ \displaystyle dm} ~~=~~ \displaystyle \frac{ \displaystyle \sum_{\displaystyle algevent} \displaystyle \frac{{\displaystyle Lat}_{\displaystyle alg}}{{{\displaystyle LatUncer}_{\displaystyle alg}}^{\displaystyle 2}} } { \displaystyle \sum_{\displaystyle algevent}  \displaystyle \frac{\displaystyle 1}{{{\displaystyle \displaystyle LatUncer}_{\displaystyle alg}}^{\displaystyle 2}}  } @f$
 *
 * @f$ {\displaystyle Longitude}_{ \displaystyle dm} ~~=~~ \displaystyle \frac{ \displaystyle \sum_{\displaystyle algevent} \displaystyle \frac{{\displaystyle Lon}_{\displaystyle alg}}{{{\displaystyle LonUncer}_{\displaystyle alg}}^{\displaystyle 2}} } { \displaystyle \sum_{\displaystyle algevent}  \displaystyle \frac{\displaystyle 1}{{{\displaystyle \displaystyle LonUncer}_{\displaystyle alg}}^{\displaystyle 2}}  } @f$
 *
 * @f$ {\displaystyle Depth}_{ \displaystyle dm} ~~=~~ \displaystyle \frac{ \displaystyle \sum_{\displaystyle algevent} \displaystyle \frac{{\displaystyle Depth}_{\displaystyle alg}}{{{\displaystyle DepthUncer}_{\displaystyle alg}}^{\displaystyle 2}} } { \displaystyle \sum_{\displaystyle algevent}  \displaystyle \frac{\displaystyle 1}{{{\displaystyle \displaystyle DepthUncer}_{\displaystyle alg}}^{\displaystyle 2}}  } @f$
 *
 * Misfit Calculation
 *
 * DMEvent misfit = @f$ \displaystyle \sum_{ \displaystyle algevent} ( \displaystyle \frac{|~{ \displaystyle OriginTime}_{ \displaystyle alg} - { \displaystyle OriginTime}_{ \displaystyle dm}~|}{ \displaystyle MaxTimeToEvent} ~~+~~ \displaystyle \frac{ \displaystyle Distance_{ \displaystyle alg}}{ \displaystyle MaxDistanceToEvent}) @f$
 *
 *
 * @section decimod_output Log Output
 * The Decision Module program, decimod, can be executed with the "-d 1" command line argument to generate a log of the message processing activity.
 * An example of the information that is printed to standard output is shown \ref example_log "below".
 *
 * The lines in the output start with prefixes that are explained in the following table.
 * <table>
 * <tr><td><b>Prefix</b></td> <td><b>Line Description</b></td></tr>
 * <tr><td>Received</td> <td>An algorithm event message has been received. The message values are displayed: SYSTEM=\link vs_page vs\endlink,
 *  \link elarms_page elarms\endlink, or \link onsite_page onsite\endlink, ID=unigue event id,
 *  VER=version number, which must increase for repeated ID's, TYPE=new, update, or delete, MAG=magnitude, MAGU=magnitude uncertainty,
 *  LAT=latitude, LATU=latitude uncertainty, LON=longitude, LONU=longitude uncertainty, DEP=depth, DEPU=depth uncertainty,
 *  TIME=origin time (UTC), TIMEU= origin time uncertainty, LH=likelihood \ref dm_extra "(1)"</td></tr>
 * <tr><td>Corrected</td> <td>The NULL uncertainty values in the algorithm event message have been replaced with the \ref uncertainty_table "default values" shown.
 *    (Messages with type=delete are not corrected.)</td></tr>
 * <tr><td>Created</td> <td>A new Decision Module event (DMEvent object) has been created to contain the current algorithm event.</td></tr>
 * <tr><td>Published</td> <td>The Decision Module event (DMEvent object) has been published. An \link dm_page XML message\endlink has been sent to the destination "eew.sys.dm.data".</td></tr>
 * <tr><td>Rejected</td> <td>An algorithm event message has been rejected because of one or more of the following reasons: an invalid \ref core_parameter_table "core value" (magnitude,
 *  latitude, longitude, depth, origin-time, and likelihood), a repeated message id with a repeated or lower version number, the origin time is older
 *  than \link decimod::ALGOEVENT_TIMEOUT ALGOEVENT_TIMEOUT\endlink, or the message id has been previously deleted. \ref delete_messages "(2)" </td></tr>
 * <tr><td>Updated</td> <td>A type=update algorithm event message has been processed. The algorithm event parameter values that have been changed are listed.</td></tr>
 * <tr><td>Changed</td> <td>A algorithm event update message, or the association of an addition algorithm event has caused the DMEvent parameters values to change. Only the
 *  parameters that have been changed are listed and the amount of the change in the parameter is shown, not the parameter value. If the program was executed with the "-m 1" option,
 *  these parameter changes will be compared to the \ref threshold_table "change-threshold values". The altered DMEvent will only be republished if a
 *  parameter change exceeds its change-threshold value (or there are changes to the attached station observations or predictions). This line will end with [not republished],
 *  if the updated DMEvent will not be republished.</td></tr>
 * <tr><td>Associate</td> <td>An attempt was made to associate an unassociated algorithm event with the indicated DMEvent. The DMEvent parameters (location, time, magnitude,
 * and likelihood) were recalculated with the unassociated algorithm event included. The absolute value of the time difference between each algorithm event and
 * the recalculated DMEvent time is listed as sec:#####. The distance from each algorithm event location to the recalculated DMEvent location is listed as km:######. The total misfit
 * for the recalculated DMEvent is listed as misfit:#####. The trial association was rejected, if the time difference is greater than
 * \link decimod::MAX_TIME_TO_EVENT MAX_TIME_TO_EVENT\endlink, or the distance is greater than \link decimod::MAX_DISTANCE_TO_EVENT MAX_DISTANCE_TO_EVENT\endlink, or the misfit
 *  is greater than \link decimod::MAX_MISFIT MAX_MISFIT\endlink. There will be successive "Associate:" lines for each DMEvent currently available (each DMEvent that has
 * not expired or been deleted). If there were one or more trail associations that were acceptable, the last "Associate:" line will indicate that the
 * unassociated algorithm has been added to the DMEvent for which the trail association had the smallest misfit value. If no trail associations were acceptable,
 * a "Created" line will follow that indicates that a new DMEvent was created from the unassociated algorithm event.</td></tr>
 * <tr><td>Deleted</td> <td>This line indicates that a DMEvent was deleted. This happens when a delete algorithm event message deletes the only algorithm event
 * that contributes to the DMEvent. A Published line will always follow the Deleted dm line to show that a dm with type=delete message that was sent.</td></tr>
 * <tr><td>Broke up</td> <td>This line indicates that an update of an algorithm event has caused the associated
 * DMEvent to become invalid. This means that one of the following conditions is no longer true: the DMEvent
 * misfit <= \link decimod::MAX_MISFIT MAX_MISFIT\endlink, the maximum absolute difference between the
 * algorithm event times and the DMEvent time is <= \link decimod::MAX_TIME_TO_EVENT MAX_TIME_TO_EVENT\endlink,
 * and the maximum distance from the algorithm event origins to the DMEvent origin is <=
 * \link decimod::MAX_DISTANCE_TO_EVENT MAX_DISTANCE_TO_EVENT\endlink. When this happens, the algorithm events
 * that are associated with this DMEvent become free to associate with another DMEvent, or if that is not
 * possible, to create a new DMEvent.</td></tr>
 * <tr><td>Reused</td><td>This line is printed after a "Broke up" line to indicate that one of the
 * algorithm events remains associated with the DMEvent ID that was broken up.</td></tr>
 * <tr><td>No Change</td><td>The indicated DMEvent has not changed since the last time that it was published. That is, there are no core parameter changes that are
 * greater than the corresponding \ref threshold_table "change threshold", and there are no changes or additions to the attached station observations or predictions.</td></tr>
 * <tr><td>Expired</td> <td>Algorithm messages and DMEvent objects are removed after \link decimod::ALGOEVENT_TIMEOUT ALGOEVENT_TIMEOUT\endlink seconds.</td></tr>
 * </table>
 *
 * \anchor dm_extra (1) Log lines that report information about a DMEvent can also contain values for the columns MSFIT, MXKM, and MXDT. MSFIT is the DMevent misfit. MXKM is the
 * maximum of the distances between each associated algorithm event location and the DMEvent combined location. MXDT is the maximum of the absolute value of the time differences between each
 * associated algorithm origin time and the DMEvent combined origin time.
 *
 * \anchor delete_messages (2) Delete messages (type=delete) are processed separately. All of the fields in a type=delete message are ignored,
 * except the system, id, and type.  Delete messages are not rejected for invalid core values, version numbers, or origin time. Messages with
 * type=delete will delete a previous or future algorithm event message with the indicated system and id. Delete messages are time stamped and
 * retained by the program for \link decimod::ALGOEVENT_TIMEOUT ALGOEVENT_TIMEOUT\endlink seconds. An example algorithm delete message is shown below.
 * \code
           SYSTEM           ID   VER  CAT   TYPE    MAG  MAGU     LAT  LATU      LON  LONU    DEP   DEPU           TIME  TIMEU    LH MSFIT  MXKM  MXDT
 Received: onsite         9234     0 live delete -9.900 -9999 -99.900 -9999 -999.900 -9999  -9.90 -99999 1308084100.000 -99999 -9.90
   \endcode
 *
 * \anchor uncertainty_table
 * Valid ranges and default values for event uncertainty parameters.
 * <table>
 * <tr><td><b>Uncertainty Parameter</b></td> <td><b>Valid Range</b></td> <td><b>Default Value</b></td></tr>
 * <tr><td>magnitude uncertainty</td> <td>0. to  \link decimod::VALID_MAG_UNCER_MW VALID_MAG_UNCER_MW\endlink</td> <td>\link decimod::DEFAULT_MAG_UNCER_MW DEFAULT_MAG_UNCER_MW\endlink</td></tr>
 * <tr><td>latitude uncertainty</td> <td>0. to  \link decimod::VALID_LAT_UNCER_DEG VALID_LAT_UNCER_DEG\endlink</td> <td>\link decimod::DEFAULT_LON_UNCER_DEG DEFAULT_LON_UNCER_DEG\endlink</td></tr>
 * <tr><td>longitude uncertainty</td> <td>0. to  \link decimod::VALID_LON_UNCER_DEG VALID_LON_UNCER_DEG\endlink</td> <td>\link decimod::DEFAULT_LON_UNCER_DEG DEFAULT_LON_UNCER_DEG\endlink</td></tr>
 * <tr><td>depth uncertainty</td> <td>0. to  \link decimod::VALID_DEPTH_UNCER_KM VALID_DEPTH_UNCER_KM\endlink</td> <td>\link decimod::DEFAULT_DEPTH_UNCER_KM DEFAULT_DEPTH_UNCER_KM\endlink</td></tr>
 * <tr><td>origin time uncertainty</td> <td>0. to  \link decimod::VALID_O_TIME_UNCER VALID_O_TIME_UNCER\endlink</td> <td>\link decimod::DEFAULT_O_TIME_UNCER DEFAULT_O_TIME_UNCER\endlink</td></tr>
 * </table>
 * \anchor core_parameter_table
 * Valid ranges for event core parameters.
 * <table>
 * <tr><td><b>Event Parameter</b></td> <td><b>Minimum</b></td> <td><b>Maximum</b></td></tr>
 * <tr><td>magnitude</td> <td>\link decimod::VALID_MAG_MIN_MW VALID_MAG_MIN_MW\endlink</td> <td>\link decimod::VALID_MAG_MAX_MW VALID_MAG_MAX_MW\endlink</td></tr>
 * <tr><td>latitude</td> <td>\link decimod::VALID_LAT_MIN_DEG VALID_LAT_MIN_DEG\endlink</td> <td>\link decimod::VALID_LAT_MAX_DEG VALID_LAT_MAX_DEG\endlink</td></tr>
 * <tr><td>longitude</td> <td>\link decimod::VALID_LON_MIN_DEG VALID_LON_MIN_DEG\endlink</td> <td>\link decimod::VALID_LON_MAX_DEG VALID_LON_MAX_DEG\endlink</td></tr>
 * <tr><td>depth</td> <td>\link decimod::VALID_DEPTH_MIN_KM VALID_DEPTH_MIN_KM\endlink</td> <td>\link decimod::VALID_DEPTH_MAX_KM VALID_DEPTH_MAX_KM\endlink</td></tr>
 * <tr><td>origin time</td> <td>\link decimod::VALID_O_TIME_MIN VALID_O_TIME_MIN\endlink</td> <td> - </td></tr>
 * <tr><td>likelihood</td> <td>\link decimod::VALID_LIKELIHOOD_MIN VALID_LIKELIHOOD_MIN\endlink</td> <td>\link decimod::VALID_LIKELIHOOD_MAX VALID_LIKELIHOOD_MAX\endlink</td></tr>
 * </table>
 *
 * \anchor threshold_table
 * Update DM message threshold values. When executing in "-m 1" mode, a DMEvent type=update message will be sent only when a change to a core value is
 * greater than the threshold values shown here (or there are changes or additions to the attached algorithm station observations or predictions).
 * <table>
 * <tr><td><b>DMEvent Parameter</b></td> <td><b>Change Threshold</b></td></tr>
 * <tr><td>magnitude</td> <td>\link decimod::CHANGE_THRESHOLD_MAG CHANGE_THRESHOLD_MAG\endlink</td> </tr>
 * <tr><td>magnitude uncertainty</td> <td>\link decimod::CHANGE_THRESHOLD_MAG_UNCER CHANGE_THRESHOLD_MAG_UNCER\endlink</td> </tr>
 * <tr><td>latitude</td> <td>\link decimod::CHANGE_THRESHOLD_LAT CHANGE_THRESHOLD_LAT\endlink</td> </tr>
 * <tr><td>latitude uncertainty</td> <td>\link decimod::CHANGE_THRESHOLD_LAT_UNCER CHANGE_THRESHOLD_LAT_UNCER\endlink</td> </tr>
 * <tr><td>longitude</td> <td>\link decimod::CHANGE_THRESHOLD_LON CHANGE_THRESHOLD_LON\endlink</td> </tr>
 * <tr><td>longitude uncertainty</td> <td>\link decimod::CHANGE_THRESHOLD_LON_UNCER CHANGE_THRESHOLD_LON_UNCER\endlink</td> </tr>
 * <tr><td>depth</td> <td>\link decimod::CHANGE_THRESHOLD_DEPTH CHANGE_THRESHOLD_DEPTH\endlink</td> </tr>
 * <tr><td>depth uncertainty</td> <td>\link decimod::CHANGE_THRESHOLD_DEPTH_UNCER CHANGE_THRESHOLD_DEPTH_UNCER\endlink</td> </tr>
 * <tr><td>origin time</td> <td>\link decimod::CHANGE_THRESHOLD_OTIME CHANGE_THRESHOLD_OTIME\endlink</td> </tr>
 * <tr><td>origin time uncertainty</td> <td>\link decimod::CHANGE_THRESHOLD_OTIME_UNCER CHANGE_THRESHOLD_OTIME_UNCER\endlink</td> </tr>
 * <tr><td>likelihood</td> <td>\link decimod::CHANGE_THRESHOLD_LKLHD CHANGE_THRESHOLD_LKLHD\endlink</td> </tr>
 * </table>
 *
 * \anchor delete_dm An example of a "Deleted" line followed by a Published dm delete line.
 * \code
  Deleted:     dm         8959     live
Published:     dm         8959  18 live delete  3.500  1.00  34.230  0.34 -120.200  0.36   9.10   0.79 11-08-11 22:08:04.812  20.00  0.85 [event deleted]
   \endcode

 * \anchor example_log
 * Example log information that is printed to standard output when the "-d 1" decimod options are used.
 * \include example_log
 */

/** @ingroup decimod */
typedef map<string, DMEvent *> DMEventMap;
/** @ingroup decimod */
typedef map<string, DMEvent *>::iterator DMEventIter;
/** @ingroup decimod */
typedef list<AlgorithmEvent>::iterator FreeAlgoIter;

typedef struct {
    double lat;
    double lon;
} Coordinate;

class Region
{
  protected:
    int npoly;
    Coordinate *polygon;
    double *slope;

  public:
    Region(string name, double minMag, double minPGA, double minPGV, double minMMI, vector<Coordinate> &poly);
    ~Region();
    bool inRegion(CoreEventInfo *cei);
    bool overlapsRegion(vector<double> contour_lat, vector<double> contour_lon);
    string toString();
    string name;
    double min_mag;
    double min_pga;
    double min_pgv;
    double min_mmi;
};

/** @ingroup decimod */
class DecisionModule
{
  private:
    string evid_filename;
    string alg_username, alg_password, alg_uri;
    string alg_topic, heartbeat_in_topic, heartbeat_out_topic;
    string heartbeat_sender, heartbeat_originator;
    string alert_username, alert_password, alert_uri;
    string alert_topic;
    string log_uri, log_topic;
    string dm_name;
    enum eewSystemName dm_system_name;
    int verbose;
    bool publish_changes_only;
    bool compare_num_contrib;
    int next_event_id;
    bool use_republish_threshold;
    bool use_gm_contour;
    int republish_interval; // not implemented yet

    DMConstants constants;
    set<string> algorithm_names;
    set<string> core_contributors;
    map<string, string> forward_contributors;
    map<string, DMMessageSender *> forward_senders;
    set<string> alertAlgAlone;
    vector<Region *> regions;
    map<string, vector<Region *> > algorithm_regions;

    AlgoMap algo_messages;
    AlgoMap delete_messages;
    list<AlgorithmEvent> free_algo_messages;
    DMEventMap dm_events;

    DMMessageReceiver *algorithm_receiver;
    DMMessageSender *dmevent_sender;
    DMMessageSender *log_sender;

#ifdef TEST_MODS
    bool use_heartbeat;  // flag to disable HB when testing
    int loop_countdown;  // if +ve, number of message loops to run
    // common initialisation across constructors
    // (cannot use constructor chaining before C11)
    void construct(int message_loop_limit, bool heartbeat, 
		   DMMessageReceiver *algorithm,
		   DMMessageSender *event,
		   DMMessageSender *log);
#endif

    void getProperties() throw(string);
    void getDMSystemName() throw(string);
    void getRegions() throw(string);
    Region *getRegion(string region_name) throw(string);
    void getAlgRegions() throw(string);
    void getAlgNames() throw(string);
    void getAlgCoreContributors() throw(string);
    void getForwardContributors() throw(string);
    bool insideGMContour(DMEvent *event);
    bool insideAlgRegions(CoreEventInfo *loc, CoreEventInfo *cei);

    void getEventId();
    bool processMessage(CoreEventInfo *cei);
    void handleDeleteMsg(AlgorithmEvent algo, CoreEventInfo *cei);
    bool handleAlgoMsg(AlgorithmEvent algo, CoreEventInfo *cei);
    void deleteAlgorithmEvent(AlgoMapIter it);
    void deleteDMEvent(DMEventIter dm);
    void setDMEventPublish(AlgorithmEvent);
    bool valuesOK(CoreEventInfo *);
//    bool checkCEIUncertainties(CoreEventInfo* ceip);
    CoreEventInfo *getAlgoMsg(AlgorithmEvent);
    void associateAlgorithmEvent(AlgorithmEvent algo);
    void removeAlgoFromDMEvent(AlgorithmEvent algo);
    bool updateAssociations();
    void updateFreeAlgoList();
    void discardOldAlgoEvents();
    void discardOldDeleteEvents();
    void discardOldDMEvents();
    void processDMEvents();
    void publishDMEvents();
    bool publishDMEvent(DMEvent *dmevent, bool publish_now);
    void forwardContributors(DMEvent *dmevent);
    bool filter(DMEvent *event, string &reason);
    string getNewEventID();
    string logTime();
    bool messageChanged(DMEvent *event);
    bool publishAloneOk(DMEvent *event);
    
  public:
    DecisionModule();
    virtual ~DecisionModule();

    void run() throw (string);

    static stringstream os;

#ifdef TEST_MODS
    // constructor for testing (with mock AMQ classes)
    DecisionModule(int message_loop_limit, 
		   DMMessageReceiver *algorithm, 
		   DMMessageSender *event,
		   DMMessageSender *log, 
		   map<string, DMMessageSender*> &forward);
    // access for testing
    DMEventMap getDmEvents();
#endif
};

#endif
