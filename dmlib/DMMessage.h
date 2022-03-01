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
#ifndef __DM_Message_h
#define __DM_Message_h

/** \page dm_page Event Example XML Message
 *  \section dm_sec1 An Example Event Message
 *  One or more of the nodes <b>elarms_info</b>, <b>onsite_info</b>, or <b>vs_info</b> will not
 *  be present when that algorithm did not contribute to the Decision Module event, or when it did
 *  not contain any observations or predictions.
 *  \include dm.xml
 */
#include <vector>
#include "AlgMessage.h"
#include "FiniteFaultMessage.h"

class DMContributor
{
  private:
    enum eewSystemName algorithm_name;
    string algorithm_version;
    string algorithm_instance;
    string event_id;
    int version;
    enum MessageCategory message_category;

  public:
    DMContributor(enum eewSystemName alg_name, string instance, string alg_version, string id, int ver,
		enum MessageCategory category)
    {
	algorithm_name = alg_name;
	algorithm_instance = instance;
    	algorithm_version = alg_version;
	event_id = id;
	version = ver;
	message_category = category;
    }
    enum eewSystemName setAlgorithmName(enum eewSystemName alg_name) {
	algorithm_name = alg_name; return algorithm_name; }
    string setAlgorithmVersion(string alg_version) {
	algorithm_version = alg_version; return algorithm_version; }
    string setEventID(string id) { event_id = id; return event_id; }
    int setVersion(int ver) { version = ver; return version; }
    enum MessageCategory setMessageCategory(enum MessageCategory category) {
	message_category = category; return message_category; }

    enum eewSystemName getAlgorithmName() { return algorithm_name; }
    string getAlgorithmNameString() {
	return eewSystemNameString[algorithm_name];
    }
    string getAlgorithmInstance() { return algorithm_instance; }
    string getAlgorithmVersion() { return algorithm_version; }
    string getEventID() { return event_id; }
    int getVersion() { return version; }
    enum MessageCategory getMessageCategory() { return message_category; }
    string getMessageCategoryString() {
	return MessageCategoryString[message_category]; }
};

/** A CoreEventInfo subclass for the Decision Module output event messages.
 *  @ingroup dm_lib
 */
class DMMessage : public AlgMessage
{
  protected: //note changed from private
    list<DMContributor> contributors;
    list<FiniteFaultMessage> finite_faults;

  public:

    DMMessage(string i,
		double mag=-9.9,
		double mag_uncer=-9.9,
		double lat=-99.9,
		double lat_uncer=-99.9,
		double lon=-999.9,
		double lon_uncer=-999.9,
		double dep=-9.9,
		double dep_uncer=-9.9,
		double o_time=-99999.99,
		double o_time_uncer=-9.9,
		double lklyhd=-9.9,
		enum nudMessageType type = NEW,
		int ver = 0,
		enum MessageCategory category = LIVE,
		string time_stamp = "",
		string alg_ver = "-",
		string instance = "",
		int num_stations = 0,
		string mag_units = "Mw",
		string mag_uncer_units = "Mw",
		string lat_units = "deg",
		string lat_uncer_units = "deg",
		string lon_units = "deg",
		string lon_uncer_units = "deg",
		string dep_units = "km",
		string dep_uncer_units = "km",
		string o_time_units = "UTC",
		string o_time_uncer_units = "sec");
    DMMessage(int i,
		double mag=-9.9,
		double mag_uncer=-9.9,
		double lat=-99.9,
		double lat_uncer=-99.9,
		double lon=-999.9,
		double lon_uncer=-999.9,
		double dep=-9.9,
		double dep_uncer=-9.9,
		double o_time=-99999.99,
		double o_time_uncer=-9.9,
		double lklyhd=-9.9,
		enum nudMessageType type = NEW,
		int ver = 0,
		enum MessageCategory category = LIVE,
		string time_stamp = "",
		string alg_ver = "-",
		string instance = "",
		int num_stations = 0,
		string mag_units = "Mw",
		string mag_uncer_units = "Mw",
		string lat_units = "deg",
		string lat_uncer_units = "deg",
		string lon_units = "deg",
		string lon_uncer_units = "deg",
		string dep_units = "km",
		string dep_uncer_units = "km",
		string o_time_units = "UTC",
		string o_time_uncer_units = "sec");

    virtual ~DMMessage();

    void addContributor(DMContributor contributor) { contributors.push_back(contributor); }
    void addContributor(AlgMessage *alg) {
	addContributor(DMContributor(alg->getSystemName(), alg->getProgramInstance(), alg->getAlgorithmVersion(),
			alg->getID(), alg->getVersion(), alg->getMessageCategory()));
    }
    int getNumberContributors() { return contributors.size(); }
    list<DMContributor>::iterator getContributorBegin() { return contributors.begin(); }
    list<DMContributor>::iterator getContributorEnd() { return contributors.end(); }

    void addFiniteFaultMessage(FiniteFaultMessage ff) { finite_faults.push_back(ff); }
    int getNumberFiniteFaults() { return finite_faults.size(); }
    list<FiniteFaultMessage>::iterator getFiniteFaultIteratorBegin() { return finite_faults.begin(); }
    list<FiniteFaultMessage>::iterator getFiniteFaultIteratorEnd() { return finite_faults.end(); }

    bool loadAlgMessages(vector<AlgMessage *> &alg, bool load_fault_info=true);

    virtual void coutPrint(bool long_form=false) { cout << toString(long_form) << endl; }
    virtual string toString(bool long_form=false);
    string updateFrom(const CoreEventInfo &cei);
};

typedef list<DMContributor>::iterator DMContributorIter;
typedef list<FiniteFaultMessage>::iterator FFMsgIter;

#endif
