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
#include <plog/Log.h>
#include <ExternalMutexConsoleAppender.h>
#include <SeverityFormatter.h>

#include "DecisionModule.h"
#include "GetProp.h"

GetProp *ewprop = NULL;

using namespace std;

#ifdef TEST_MODS
string program_version="2.3.21 2019-06-20_TEST_MODS";
#else
string program_version="2.3.21 2019-06-20";
#endif
const string BUILD_INFO = " (Built " + (string)__DATE__ + " " + __TIME__ + " by " + BUILDER + ")";

using namespace eew;

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static ExternalMutexConsoleAppender<SeverityFormatter> mtx_console_appender(&print_lock);


int main(int argc, char *argv[]) 
{
    // initialize plog library per GitLab issue #69
    plog::init(plog::verbose, &mtx_console_appender);

    DecisionModule::os << "dm version " << program_version << BUILD_INFO << std::endl;

    if(argc < 2) {
        LOGN << "dm version " << program_version << BUILD_INFO;
        LOGN << DMLib::getVersion();
        LOGN << "Usage: dm param_file";
        return 1;
    }

    // force program info to output before we start initializing internals
    LOGI << DecisionModule::os.str();
    DecisionModule::os.str("");

    try {
        ewprop = GetProp::createInstance(argv[1], argc, argv);
    }
    catch (string e) {
        LOGE << "problem reading properties" << std::endl << e;
        return -1;
    }

    activemq::library::ActiveMQCPP::initializeLibrary();

    try {
        DecisionModule dm;
        dm.run();
    }
    catch (string e) {
        LOGE << "problem starting up" << std::endl << e;
        return -1;
    }

    activemq::library::ActiveMQCPP::shutdownLibrary();

    return 0;
}
