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
#ifndef __AlgorithmEvent_h
#define __AlgorithmEvent_h

#include <map>
#include "CoreEventInfo.h"
#include "DMconsts.h"

/** @ingroup decimod */
class AlgorithmEvent
{
  public:
    enum eewSystemName system_name;
    enum MessageCategory message_category;
    string system_name_string;
    string program_instance;
    string id;
    bool use_core_info;

    AlgorithmEvent(CoreEventInfo *cei, bool use_core);
    bool useCoreEventInfo() { return use_core_info; }
    std::string toString();
    static bool valuesOK(CoreEventInfo *cei, DMConstants& c, bool verbose, stringstream &os);
    static bool checkCEIUncertainties(CoreEventInfo* cei, bool verbose=false);

    virtual bool operator==(const AlgorithmEvent &A) const;
    virtual bool operator!=(const AlgorithmEvent &A) const;
};

/** @ingroup decimod */
class AlgoLessThan
{
  public:
    bool operator() (const AlgorithmEvent &algo1, const AlgorithmEvent &algo2) const
    {
	if(algo1.message_category < algo2.message_category) return true;
	else if(algo1.message_category > algo2.message_category) return false;
	else if(algo1.system_name < algo2.system_name) return true;
	else if(algo1.system_name > algo2.system_name) return false;
	else if(algo1.program_instance.compare(algo2.program_instance) < 0) return true;
	else if(algo1.program_instance.compare(algo2.program_instance) > 0) return false;
	else return (algo1.id < algo2.id);
    }
};

/** @ingroup decimod */
typedef map<AlgorithmEvent, CoreEventInfo *, AlgoLessThan> AlgoMap;
/** @ingroup decimod */
typedef map<AlgorithmEvent, CoreEventInfo *, AlgoLessThan>::iterator AlgoMapIter;

#endif
