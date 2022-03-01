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
#ifndef __E2ModuleManager_H__
#define __E2ModuleManager_H__

#include "DMMessageSender.h"
#include "HBProducer.h"
#include "Exceptions.h" // for Error

class E2TriggerManager;
class E2EventManager;
class E2Magnitude;
class E2Associator;
class E2Location;
class E2Alert;

class E2ModuleManager
{
 private:
    static E2ModuleManager *handle;
 
    bool initialized;

    E2TriggerManager *tm;
    E2EventManager   *em;
    E2Associator *assoc;
    E2Location *loc;
    E2Magnitude *mag;
    E2Alert *alert;

    string event_amq_uri;
    string event_amq_topic;
    string heartbeat_topic;
    string heartbeat_sender;
    string log_uri;
    string log_topic;
    static bool send_log;
    double min_mag_change;

    static DMMessageSender *log_sender;
    DMMessageSender *sender;
    HBProducer *hb;
    E2ModuleManager();
    ~E2ModuleManager();
    void updateEvents(int evid);
    void cleanup();

 public:
    void init(bool replay_mode) throw(Error);
    void run();
    DMMessageSender *getMessageSender() { return sender; }

    static E2ModuleManager* getInstance() throw(Error);
    static bool sendLogMsg(string msg);
    static bool sendLogMsg(list<string> &msg);
    static bool sendLogMsg(string line_label, list<string> &msg);
    static E2Location *getLocator() { return getInstance()->loc; }
    static std::stringstream param_str;
};

#endif
