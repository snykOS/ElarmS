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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <plog/Log.h>
#include "ProcessBuffer.h"

using namespace std;

ArrayStruct::ArrayStruct()
{
    ndata = 0;
    k1 = 0;
    i1 = 0;
    raw = (int *)NULL;
    t = (float *)NULL;
    d = (float *)NULL;
    v = (float *)NULL;
    a = (float *)NULL;
    s = (float *)NULL;
    n = (float *)NULL;
    r = (float *)NULL;
    sta = (float *)NULL;
    lta = (float *)NULL;
    size_array = 0;
}

ArrayStruct::~ArrayStruct()
{
    deleteArrays();
}

void ArrayStruct::deleteArrays()
{
    if(raw != NULL) { delete [] raw; raw = NULL; }
    if(t != NULL)   { delete [] t;     t = NULL; }
    if(d != NULL)   { delete [] d;     d = NULL; }
    if(v != NULL)   { delete [] v;     v = NULL; }
    if(a != NULL)   { delete [] a;     a = NULL; }
    if(s != NULL)   { delete [] s;     s = NULL; }
    if(n != NULL)   { delete [] n;     n = NULL; }
    if(r != NULL)   { delete [] r;     r = NULL; }
    if(sta != NULL) { delete [] sta; sta = NULL; }
    if(lta != NULL) { delete [] lta; lta = NULL; }
}

void ArrayStruct::setSize(int size)
{
    if(size <= size_array)  return;

    deleteArrays();

    raw = new int[size];
    t = new float[size];
    d = new float[size];
    v = new float[size];
    a = new float[size];
    s = new float[size];
    n = new float[size];
    r = new float[size];
    sta = new float[size];
    lta = new float[size];
    size_array = size;
};


ProcessBuffer::ProcessBuffer()
{
    samplerate = 0.;
    max_duration = MAXBUFSIZE;
    next_sample = 0;
}

ProcessBuffer::ProcessBuffer(int size_secs)
{
    samplerate = 0.;
    max_duration = size_secs;
    next_sample = 0;
}

ProcessBuffer::ProcessBuffer(const ProcessBuffer & sb)
{
    samplerate = sb.samplerate;
    max_duration = sb.max_duration;
    clear();
}

ProcessBuffer & ProcessBuffer::operator=(const ProcessBuffer &sb)
{
    samplerate = sb.samplerate;
    max_duration = sb.max_duration;
    clear();
    return *this;
}

int ProcessBuffer::numRemaining()
{
    int i, index_in_packet, num=0, npackets = (int)dataq.size();

    if((i = currentPacket(next_sample, &index_in_packet)) >= 0) {
	num = dataq[i]->length - index_in_packet;
	for(++i; i < npackets; i++) num += dataq[i]->length;
    }
    return num;
}

int ProcessBuffer::getData(ArrayStruct &data, double *sample_time, PacketInfo *packet_info)
{
    int j, k, k1, index_in_packet, ndata=0, npackets = (int)dataq.size();

    data.ndata = 0;
    data.k1 = 0;
    data.i1 = 0;
    if((k1 = currentPacket(next_sample, &index_in_packet)) >= 0) {
	ndata = dataq[k1]->length - index_in_packet;
	for(k = k1+1; k < npackets; k++) ndata += dataq[k]->length;
    }
    if(ndata <= 0) {
	*sample_time = 0.0;
	return 0;
    }
    else {
	data.setSize(ndata);
    }

    *sample_time = dataq[k1]->start_time + (double)index_in_packet/dataq[k1]->samplerate;

    j = 0;
    for(int i = index_in_packet; i < dataq[k1]->length; i++, j++) {
	data.raw[j] = dataq[k1]->data[i].raw;
	data.t[j]   = dataq[k1]->data[i].t;
	data.d[j]   = dataq[k1]->data[i].d;
	data.v[j]   = dataq[k1]->data[i].v;
	data.a[j]   = dataq[k1]->data[i].a;
	data.s[j]   = dataq[k1]->data[i].s;
	data.n[j]   = dataq[k1]->data[i].n;
	data.r[j]   = dataq[k1]->data[i].r;
	data.sta[j] = dataq[k1]->data[i].sta;
	data.lta[j] = dataq[k1]->data[i].lta;
    }

    for(k = k1+1; k < npackets; k++) {
	for(int i = 0; i < dataq[k]->length; i++, j++) {
	    data.raw[j] = dataq[k]->data[i].raw;
	    data.t[j]   = dataq[k]->data[i].t;
	    data.d[j]   = dataq[k]->data[i].d;
	    data.v[j]   = dataq[k]->data[i].v;
	    data.a[j]   = dataq[k]->data[i].a;
	    data.s[j]   = dataq[k]->data[i].s;
	    data.n[j]   = dataq[k]->data[i].n;
	    data.r[j]   = dataq[k]->data[i].r;
	    data.sta[j] = dataq[k]->data[i].sta;
	    data.lta[j] = dataq[k]->data[i].lta;
	}
    }

    next_sample += ndata;

    packet_info->length = dataq[k1]->length;
    packet_info->samplerate = dataq[k1]->samplerate;
    packet_info->start_time = dataq[k1]->start_time;
    packet_info->outof_ring = dataq[k1]->outof_ring;
    packet_info->outof_feeder_queue = dataq[k1]->outof_feeder_queue;
    packet_info->trigger_found = 0.0;
    packet_info->into_send_queue = 0.0;
    packet_info->outof_send_queue = 0.0;

    data.k1 = k1;
    data.i1 = index_in_packet;
    data.ndata = ndata;

    return ndata;
}

void ProcessBuffer::updateData(ArrayStruct &data)
{
    int npackets = (int)dataq.size();
    int k1 = data.k1;
    int j = 0;
    for(int i = data.i1; i < dataq[k1]->length && j < data.ndata; i++, j++) {
	dataq[k1]->data[i].t = data.t[j];
	dataq[k1]->data[i].d = data.d[j];
	dataq[k1]->data[i].v = data.v[j];
	dataq[k1]->data[i].a = data.a[j];
	dataq[k1]->data[i].s = data.s[j];
	dataq[k1]->data[i].n = data.n[j];
	dataq[k1]->data[i].r = data.r[j];
	dataq[k1]->data[i].sta = data.sta[j];
	dataq[k1]->data[i].lta = data.lta[j];
    }
    for(int k = k1+1; k < npackets; k++) {
	for(int i = 0; i < dataq[k]->length && j < data.ndata; i++, j++) {
	    dataq[k]->data[i].t = data.t[j];
	    dataq[k]->data[i].d = data.d[j];
	    dataq[k]->data[i].v = data.v[j];
	    dataq[k]->data[i].a = data.a[j];
	    dataq[k]->data[i].s = data.s[j];
	    dataq[k]->data[i].n = data.n[j];
	    dataq[k]->data[i].r = data.r[j];
	    dataq[k]->data[i].sta = data.sta[j];
	    dataq[k]->data[i].lta = data.lta[j];
	}
    }
}

int ProcessBuffer::getSamples(double start, ArrayStruct &data)
{
    data.k1 = 0;
    data.i1 = 0;
    data.ndata = 0;
    if(dataq.size() > 0) {
	int j, k, k1, index_in_packet, num=0, npackets = (int)dataq.size();
	int sample_index = (int)((start - dataq.front()->start_time)*(double)samplerate + 0.5);

	if((k1 = currentPacket(sample_index, &index_in_packet)) >= 0) {
	    num = dataq[k1]->length - index_in_packet;
	    for(k = k1+1; k < npackets; k++) num += dataq[k]->length;
	}
	if(num == 0) {
	    return 0;
	}
	else {
	    data.setSize(num);
        }

	j = 0;
	for(int i = index_in_packet; i < dataq[k1]->length && j < num; i++, j++) {
	    data.raw[j] = dataq[k1]->data[i].raw;
	    data.t[j]   = dataq[k1]->data[i].t;
	    data.d[j]   = dataq[k1]->data[i].d;
	    data.v[j]   = dataq[k1]->data[i].v;
	    data.a[j]   = dataq[k1]->data[i].a;
	    data.s[j]   = dataq[k1]->data[i].s;
	    data.n[j]   = dataq[k1]->data[i].n;
	    data.r[j]   = dataq[k1]->data[i].r;
	    data.sta[j] = dataq[k1]->data[i].sta;
	    data.lta[j] = dataq[k1]->data[i].lta;
	}

	for(k=k1+1; k < npackets && j < num; k++) {
	    for(int i = 0; i < dataq[k]->length && j < num; i++, j++) {
		data.raw[j] = dataq[k]->data[i].raw;
		data.t[j]   = dataq[k]->data[i].t;
		data.d[j]   = dataq[k]->data[i].d;
		data.v[j]   = dataq[k]->data[i].v;
		data.a[j]   = dataq[k]->data[i].a;
		data.s[j]   = dataq[k]->data[i].s;
		data.n[j]   = dataq[k]->data[i].n;
		data.r[j]   = dataq[k]->data[i].r;
		data.sta[j] = dataq[k]->data[i].sta;
		data.lta[j] = dataq[k]->data[i].lta;
	    }
	}
	data.k1 = k1;
	data.i1 = index_in_packet;
	data.ndata = num;
	return num;
    }
    return 0;
}

int ProcessBuffer::getNumSamples(double start)
{
    int num = 0;
    if(dataq.size() > 0) {
	int k, k1, index_in_packet, npackets = (int)dataq.size();
	int sample_index = (int)((start - dataq.front()->start_time)*(double)samplerate + 0.5);

	if((k1 = currentPacket(sample_index, &index_in_packet)) >= 0) {
	    num = dataq[k1]->length - index_in_packet;
	    for(k = k1+1; k < npackets; k++) num += dataq[k]->length;
	}
    }
    return num;
}

double ProcessBuffer::nextSampleTime()
{
    int k, index_in_packet;
    if((k = currentPacket(next_sample, &index_in_packet)) >= 0) {
	return dataq[k]->start_time + (double)index_in_packet/dataq[k]->samplerate;
    }
    return 0.0;
}

int ProcessBuffer::currentPacket(int sample_index, int *index_in_packet)
{
    int i, nbeg, nend, npackets = (int)dataq.size();

    *index_in_packet = -1;
    nbeg = 0;
    nend = 0;
    for(i = 0; i < npackets; i++) {
	nend += dataq[i]->length;
	if(nbeg <= sample_index && sample_index < nend) {
	    *index_in_packet = sample_index - nbeg;
	    return i;
	}
	nbeg += dataq[i]->length;
    }
    return -1;
}

int ProcessBuffer::putPacket(RawPacket &p)
{
    int ret = 0;

    if(p.nsamps <= 0 || p.samplerate <= 0.) {
	return ret;
    }
    samplerate = p.samplerate;

    double input_end = p.start_time + (double)(p.nsamps-1)/samplerate;
    double current_end = (dataq.size() > 0) ? dataq.back()->endtime() : 0.;

    // return if no new data: current data endtime >= input data endtime
    if(current_end > input_end - 0.5/samplerate) return -1;

    // check for overlap
    int start_sample = 0;
    if(p.start_time < current_end + 0.5/samplerate) {
	// get index of first new sample (starting at zero)
	start_sample = (int)((current_end - p.start_time)*samplerate + 0.5) + 1;
	ret = start_sample;
	if(start_sample >= p.nsamps) return ret;
    }

    SamplePacket::addPacket(p, start_sample, dataq);

    while(dataq.size() > 1 && dataq.back()->endtime() - dataq.front()->endtime() > max_duration) {
	next_sample -= dataq.front()->length;
	delete dataq.front();
	dataq.pop_front();
    }
    if(next_sample < 0) next_sample = 0;
    
    return ret;
}

void ProcessBuffer::clear() {
    int n = (int)dataq.size();
    for(int i = 0; i < n; i++) {
	delete dataq[i];
    }
    dataq.clear();
    next_sample = 0;
}

void ProcessBuffer::print() {
    char s[200];

    int n = (int)dataq.size();
    for(int i = 0; i < n; i++) {
	snprintf(s, sizeof(s), "%4d %5d %5.1f  ", i+1, dataq[i]->length, dataq[i]->samplerate);
	LOG_INFO << s << TimeStamp(UNIX_TIME, dataq[i]->start_time) << "  " << TimeStamp(UNIX_TIME, 
			dataq[i]->start_time + (dataq[i]->length-1)/dataq[i]->samplerate);
    }
}

double ProcessBuffer::startTime() {
    if(dataq.size() > 0) {
	return dataq.front()->start_time;
    }
    return 0.;
}

double ProcessBuffer::duration() {
    if(dataq.size() > 0) {
	return dataq.back()->endtime() - dataq.front()->start_time;
    }
    return 0.;
}

double ProcessBuffer::endTime() {
    if(dataq.size() > 0) {
	return dataq.back()->endtime();
    }
    return 0.;
}
