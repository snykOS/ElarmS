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
#include "StaNetChanLoc.h"

StaNetChanLoc::StaNetChanLoc(string sta, string net, string chan, string loc) :
		station(sta), network(net), channel(chan), location(loc) {}

StaNetChanLoc::~StaNetChanLoc() {}

string StaNetChanLoc::setNetwork(string net) { network=net; return network; }
string StaNetChanLoc::setStation(string sta) { station=sta; return station; }
string StaNetChanLoc::setChannel(string chan) { channel=chan; return chan; }
string StaNetChanLoc::setLocation(string loc) { location=loc; return loc; }
string StaNetChanLoc::getNetwork() const { return network; } 
string StaNetChanLoc::getStation() const { return station; }
string StaNetChanLoc::getChannel() const { return channel; }
string StaNetChanLoc::getLocation() const { return location; }
string StaNetChanLoc::getSNCL() const {
    return station + "." + network + "." + channel + "." + location;
}

bool StaNetChanLoc::operator==(const StaNetChanLoc &A) const
{
    return ( A.network == network && A.station == station &&
	     A.channel == channel && A.location == location );
}

bool StaNetChanLoc::operator!=(const StaNetChanLoc &A) const { return !(*this==A); }

bool StaNetChanLoc::operator<(const StaNetChanLoc &A) const
{
    int ic;
    ic = station.compare(A.station);
    if(ic != 0) return (ic < 0) ? true : false;

    ic = network.compare(A.network);
    if(ic != 0) return (ic < 0) ? true : false;

    ic = channel.compare(A.channel);
    if(ic != 0) return (ic < 0) ? true : false;

    ic = location.compare(A.location);
    if(ic != 0) return (ic < 0) ? true : false;
    return false;
}

bool StaNetChanLoc::operator>(const StaNetChanLoc &A) const
{
    int ic;
    ic = station.compare(A.station);
    if(ic != 0) return (ic > 0) ? true : false;

    ic = network.compare(A.network);
    if(ic != 0) return (ic > 0) ? true : false;

    ic = channel.compare(A.channel);
    if(ic != 0) return (ic > 0) ? true : false;

    ic = location.compare(A.location);
    if(ic != 0) return (ic > 0) ? true : false;
    return false;
}
