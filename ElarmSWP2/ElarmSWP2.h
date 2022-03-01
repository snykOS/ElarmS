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
#ifndef __ELARMSWP2_H__
#define __ELARMSWP2_H__

#include <unistd.h>
#include <activemq/library/ActiveMQCPP.h>

#include "EWP2.h"
#include "EWP2Sender.h"
#include "HBProducer.h"

using namespace std;

class EWP2ReaderFactory : public WPFactory 
{
  private:
    EWP2Sender *sender;
    vector<EWP2 *> _channelreaders;

  public:
    EWP2ReaderFactory(EWP2Sender *Sender) : sender(Sender) { }
    ~EWP2ReaderFactory() {
	for (size_t i = 0; i < _channelreaders.size(); i++) {
	    EWP2 * p = _channelreaders.at(i);
	    if(NULL != p) {
		delete p;
	    }
	}
	_channelreaders.clear();
    }

    EWP2 *createInstance(Channel Z, Channel E, Channel N) {
	EWP2 *p = new EWP2(Z, E, N, sender);
	_channelreaders.push_back(p);
	return p;
    }
};

class ElarmSWP2
{
  private:
    EWP2Sender *sender;
    string uri;
    string user;
    string passwd;

  public:
    ElarmSWP2(const string &amq_uri, const string &amq_user, const string &amq_password) :
		sender(NULL), uri(amq_uri), user(amq_user), passwd(amq_password) { }

    virtual ~ElarmSWP2() { if(sender) delete sender; }
    virtual void run();
    void stop() { if(sender) sender->stop(); }

    EWP2Sender *getSender() { return sender; }

    static std::stringstream param_str;
};

#endif
