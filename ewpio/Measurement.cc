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
#include <iostream>
#include <string>
#include "Measurement.h"
#include "EWPacket.h"

Measurement::Measurement(std::string name) : status(MEASUREMENT_WAITING_ON_DATA)
{
    measurement_name = name;
    if(measurement_name.length() > MEASUREMENT_NAME_SIZE) {
	measurement_name.resize(MEASUREMENT_NAME_SIZE);
    }
}

int Measurement::serialize(char * &packet, int length) throw(std::string)
{
    (void)packet; (void)length;
    return 0;
}
int Measurement::deserialize(char * &packet, int maxlength) throw(std::string)
{
    (void)packet; (void)maxlength;
    return 0;
}
int Measurement::packetSize()
{
    return 0;
}

void Measurement::messageType(char *packet, int packet_len, std::string &name, int *length) throw(std::string)
{
    int len = MEASUREMENT_NAME_SIZE + sizeof(int);
    if(packet_len < len) {
        std::string s = std::string("Measurement.messageType packet length < " + len);
        std::cerr << s << std::endl;
	throw s;
    }
    char s[MEASUREMENT_NAME_SIZE+1];
    memset(s, 0, sizeof(s));
    char *p = packet;
    getFromPacket(s, MEASUREMENT_NAME_SIZE, p);
    name.assign(s);
    int size;
    getFromPacket(size, p);
    *length = size;
}

void Measurement::startSerialize(char * &p, int length) throw(std::string)
{
    int size = packetSize();
    if(length < size) {
	char s[100];
	snprintf(s, sizeof(s), "%s: length(%d) < packetSize(%d)",
		measurement_name.c_str(), length, size);
	std::cerr << s << std::endl;
        throw std::string(s);
    }
    char name[MEASUREMENT_NAME_SIZE];
    memset(name, 0, sizeof(name));
    strncpy(name, measurement_name.c_str(), MEASUREMENT_NAME_SIZE);
    addToPacket(name, MEASUREMENT_NAME_SIZE, p);
    addToPacket(size, p);
}

void Measurement::startDeserialize(char * &p, int length) throw(std::string)
{
    int size = packetSize();
    if(length < size) {
	char s[100];
	snprintf(s, sizeof(s), "%s.deserialize: length(%d) < packetSize(%d)",
			measurement_name.c_str(), length, size);
	std::cerr << s << std::endl;
        throw std::string(s);
    }

    char name[MEASUREMENT_NAME_SIZE+1];
    memset(name, 0, sizeof(name));
    getFromPacket(name, MEASUREMENT_NAME_SIZE, p);
    if(strcmp(name, measurement_name.c_str())) {
	char s[100];
	snprintf(s, sizeof(s), "%s.deserialize: invalid name: %s",
		measurement_name.c_str(), name);
	std::cerr << s << std::endl;
        throw std::string(s);
    }
    int siz;
    getFromPacket(siz, p);
    if(siz != size) {
	char s[100];
	snprintf(s, sizeof(s), "%s.deserialize: invalid packet size: %d",
		measurement_name.c_str(), siz);
	std::cerr << s << std::endl;
        throw std::string(s);
    }
}

std::string Measurement::toString()
{
    char s[100];
    const char *state;
    if(status == MEASUREMENT_WAITING_ON_DATA) state = "WAITING  ";
    else if(status == MEASUREMENT_UPDATING)   state = "UPDATING ";
    else if(status == MEASUREMENT_COMPLETE)   state = "COMPLETE ";
    else if(status == MEASUREMENT_INSUFFICIENT_LEAD) state = "SHORTLEAD ";
    else                                      state = "GAP      ";
    snprintf(s, sizeof(s), "%13s %s ", measurement_name.c_str(), state);
    return std::string(s);
}
