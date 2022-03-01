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
#ifndef __RawPacket_h
#define __RawPacket_h

#include <stdlib.h>
#include <string.h>
#include "Channel.h"

class RawPacket {
 public:
    Channel ch;
    double start_time;
    double samplerate;
    int nsamps;
    int *data;
    double outof_ring; // time packet taken from EW ring
    double outof_feeder_queue; // time packet taken from ewfeeder queue
    
    RawPacket(Channel channel, int nsamp, int *d, double start, double samprate, double outof_ewring,
		double outof_queue) {
	(void)BINARY; (void)MEMORY; (void)ASCII; (void)FILENAME; (void)DATABASE; // avoid compiler used warning
	ch = channel;
	nsamps = nsamp;
	data = new int[nsamps];
	memcpy(data, d, nsamps*sizeof(int));
	start_time = start;
	samplerate = samprate;
	outof_ring = outof_ewring;
	outof_feeder_queue = outof_queue;
    }
    ~RawPacket() { delete[] data; }

    RawPacket & operator=(const RawPacket &src) {
	ch = src.ch;
	nsamps = src.nsamps;
	data = src.data;
	start_time = src.start_time;
	samplerate = src.samplerate;
	outof_ring = src.outof_ring;
	outof_feeder_queue = src.outof_feeder_queue;
	return (*this);
    }

    double endtime() {
	return (samplerate > 0.) ? start_time + (double)(nsamps-1)/samplerate : 0.0;
    }
};

#endif
