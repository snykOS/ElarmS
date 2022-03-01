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
#ifndef __ProcessBuffer_h
#define __ProcessBuffer_h

#include <iostream>
#include <deque>
#include "RawPacket.h"
#include "TimeStamp.h"

using namespace std;

const int MAXBUFSIZE = 60;

typedef struct
{
    int length;
    double samplerate;
    double start_time;
    double outof_ring;
    double outof_feeder_queue;
    double trigger_found;
    double into_send_queue;
    double outof_send_queue;
} PacketInfo;

class SampleStruct
{
  public:
    int raw;	// Raw data
    float t;	// Predominant period (TauP)
    float d;	// Displacement
    float v;	// Velocity
    float a;	// Acceleration
    float s;	// Signal level
    float n;	// Noise level
    float r;	// High-pass Velocity for picker
    float sta;	// short-term average
    float lta;	// long-term average
};

class ArrayStruct
{
  public:
    ArrayStruct();
    ~ArrayStruct();
    void deleteArrays();
    void setSize(int size);

    int ndata;
    int k1;
    int i1;
    int *raw;	// Raw data
    float *t;	// Predominant period (TauP)
    float *d;	// Displacement
    float *v;	// Velocity
    float *a;	// Acceleration
    float *s;	// Signal level
    float *n;	// Noise level
    float *r;	// High-pass Velocity for picker
    float *sta;	// short-term average
    float *lta;	// long-term average
    int size_array;
};

class SamplePacket
{
  private:
    SamplePacket() {
	data = NULL;
	length = 0;
	start_time = 0.0;
	samplerate = 0.0;
	outof_ring = 0.0;
	outof_feeder_queue = 0.0;
    }
  public:
    SampleStruct *data;
    int length;
    double start_time;
    double samplerate;
    double outof_ring;
    double outof_feeder_queue;

    ~SamplePacket() {
	if(data) delete[] data;
    }
    double endtime() {
	return (samplerate > 0.) ? start_time + (double)(length-1)/samplerate : 0.0;
    }
    static void addPacket(RawPacket &raw, int start_sample, deque<SamplePacket *> &dataq) {
	if(start_sample >= 0 && start_sample < raw.nsamps)  {
	    SamplePacket *p = new SamplePacket();
	    p->length = raw.nsamps - start_sample;
	    p->data = new SampleStruct[p->length];
	    for(int i = 0; i < p->length; i++) {
		p->data[i].raw = raw.data[start_sample+i];
		p->data[i].a = 0.0;
		p->data[i].v = 0.0;
		p->data[i].d = 0.0;
		p->data[i].t = 0.0;
		p->data[i].n = 0.0;
		p->data[i].s = 0.0;
		p->data[i].r = 0.0;
		p->data[i].sta = 0.0;
		p->data[i].lta = 0.0;
	    }
	    p->samplerate = raw.samplerate;
	    p->start_time = raw.start_time + (double)start_sample/p->samplerate;
	    p->outof_ring = raw.outof_ring;
	    p->outof_feeder_queue = raw.outof_feeder_queue;
	    dataq.push_back(p);
	}
    }
};

class ProcessBuffer
{
  private:
    double samplerate;
    int max_duration;
    int next_sample;
    deque<SamplePacket *> dataq;

  public:
    ProcessBuffer();
    ProcessBuffer(int size_secs);
    ProcessBuffer(const ProcessBuffer& sb);
    ~ProcessBuffer() {}

    ProcessBuffer& operator=(const ProcessBuffer& sb);

    void setSizeInSeconds(int seconds) { max_duration = seconds; }
    int putPacket(RawPacket &p);
    int currentPacket(int sample_index, int *index_in_packet);
    int numRemaining();
    int getNumSamples(double start);
    int getData(ArrayStruct &data, double *sample_time, PacketInfo *packet_info);
    void updateData(ArrayStruct &data);
    int getSamples(double start, ArrayStruct &data);
    double nextSampleTime();
    double startTime();
    double endTime();
    double duration();
    void clear();
    void print();
    double sampleRate() { return samplerate; }
};

#endif
