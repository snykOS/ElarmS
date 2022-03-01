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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <map>
#include <set>

#include <plog/Log.h>
#include "GMPeak.h"
#include "TimeStamp.h"
#include "CoreEventInfo.h"
#include "DMMessageDecoder.h"

#include <activemq/transport/DefaultTransportListener.h>
#include <cms/ExceptionListener.h>

using namespace std;

class Trigger
{
  public:
    string sta;
    string chan;
    string loc;
    string net;
    string src;
    double t;
    Trigger(const char *station, const char *channel, const char *location, const char *network,
		const char *packet_src, double trigger) :
	sta(station), chan(channel), loc(location), net(network), src(packet_src), t(trigger) {}
};

class Latency {
  public:
    string net;
    string sta;
    timeval trigger_time;
    float latency;
};

class PeakData {
  public:
    string sta;
    string chan;
    int npts;
    double last_sec;
    vector<double> start_time;
    vector<int> start_index;
    int size_data;
    float *data;
    PeakData(GMPeak &p);
};

template <typename T> class Segment {
  public:
    T	    *data;
    double  beg;
    double  del;
    int     data_length;
    double  Calib, Calper;

    Segment() : data(NULL), beg(0.), del(0.), data_length(0), Calib(1.), Calper(1.) { }
    Segment(int seg_data_length, double tstart, double dt, double calibration, double calperiod) :
	data(NULL), beg(tstart), del(dt), data_length(seg_data_length), Calib(calibration),
	Calper(calperiod)
    {
	if(seg_data_length > 0) {
	    data = (T *)malloc(seg_data_length*sizeof(T));
	    if(!data) { cerr << "malloc failed for seg_data_length: " << seg_data_length << endl; }
	}
    }

    Segment(int seg_data_length, T *input, double tstart, double dt, double calibration,
	double calperiod) : data(NULL), beg(tstart), del(dt), data_length(seg_data_length),
	Calib(calibration), Calper(calperiod)
    {
	if(seg_data_length > 0) {
	    data = (T *)malloc(seg_data_length*sizeof(T));
	    if(!data) { cerr << "malloc failed for seg_data_length: " << seg_data_length << endl; }
	    memcpy(data, input, seg_data_length*sizeof(T));
	}
    }

    Segment(Segment<T> *s) : data(NULL), beg(s->beg), del(s->del), data_length(s->data_length),
		Calib(s->Calib), Calper(s->Calper)
    {
	if(s->data_length > 0) {
	    data = (T *)malloc(s->data_length*sizeof(T));
	    if(!data) { cerr << "malloc failed for seg_data_length: " << data_length << endl; }
	    memcpy(data, s->data, s->data_length*sizeof(T));
	}
    }
    ~Segment() { if(data) free(data); }
    double tbeg() { return beg; }
    double tend() { return beg + (data_length-1)*del; }
    double tdel() { return del; }
    int length() { return data_length; }
    void setTdel(double tdel) { del = tdel; }
    double calib() { return Calib; }
    double calper() { return Calper; }

    void truncate(int i1, int i2)
    {
	if(i1 >= data_length || i2 < 0 || i1 > i2) return;
	if(i1 < 0) i1 = 0;
	if(i2 >= data_length) i2 = data_length-1;
	beg = beg + i1*del;
	data_length = i2 - i1 + 1;
	for(int i = i1, j = 0; i <= i2; i++) data[j++] = data[i];
	if(data_length > 0) {
	    data = (T *)realloc(data, data_length*sizeof(T));
	}
	else {
	    data = (T *)realloc(data, sizeof(T));
	}
    }
};

class EWP2Receiver : public cms::ExceptionListener, public activemq::transport::DefaultTransportListener
{
  private:
    map<string, PeakData *> peak_data_map;
    map<string, Trigger *> trigger_map;
    map<string, CoreEventInfo *> event_map;
    DMMessageDecoder decoder;
    bool saving_ewdata;
    bool transport_ok;
    bool read_txt_msg;
    string in, topic, user, passwd, fmt, out, otopic, ouser, opasswd;
    string ofmt, pfmt, sta, chan, toffset;
    int out_sockfd;
    struct sockaddr_in out_addr;
    typedef map <char, int> IdMap;
    timeval last_time;
    int arid;
    double time_offset;
    set<string> packet_src_set;
    string msg_src;

    vector<Latency> latencies;

    void writeTriggers(string out);
    void writeEvents(string out);

    virtual void onException(const cms::CMSException &ex AMQCPP_UNUSED);
    virtual void transportInterrupted();
    virtual void transportResumed();

  public:
    EWP2Receiver() {}
    bool run();

    void handleMessage(char *msg, int len, double msg_latency, string pfmt, string sta, string chan);
    void processEventMessage(string msg);
    bool getarg(int argc, char **argv, string name, string &value);
    bool getArgs(int argc, char **argv, string &in, string &topic, string &user, string &passwd,
		string &fmt, string &out, string &otopic, string &ouser, string &opasswd,
		string &ofmt, string &pfmt, string &sta, string &chan, string &toffset);
    void joinGMPeak(GMPeakPacket &packet);
    void writeOrigin(FILE *fp, string id, double latitude, double longitude, double odepth, double otime, double mag, string src);
    void writeArrival(FILE *fp, string sta, string chan, string loc, string net, string src, double time, int arid);
    void flipBytes(float *data, int npts);
    void flipIBytes(int *data, int npts);
};
