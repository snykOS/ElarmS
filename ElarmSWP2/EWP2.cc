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
#include <stdio.h>
#include <strings.h>
#include "EWP2.h"
#include "Exceptions.h"
#include "PrintLock.h"


bool EWP2::createModules()
{
    try {
	togZ = new ToGroundTSPModule(chanZ);
	togN = new ToGroundTSPModule(chanN);
	togE = new ToGroundTSPModule(chanE);

	gmpeakZ = new GMPeakAmpsTSPModule(chanZ, sender);
	gmpeakN = new GMPeakAmpsTSPModule(chanN, sender);
	gmpeakE = new GMPeakAmpsTSPModule(chanE, sender);

	pickerZ = new PickerTSPModule(chanZ);

	trigpar = new TriggerParamsTSPModule(chanZ, sender);
    }
    catch(Error e) {
	e.print();
	exit(1);
    }

    int bufsize = (int)(trigpar->bufferLength() + 5); // seconds

    Zbuffer = ProcessBuffer(bufsize);
    Nbuffer = ProcessBuffer(bufsize);
    Ebuffer = ProcessBuffer(bufsize);

    packet_start_time = 0.;
    packet_channel = "";

    process_trigger = false;
    return true;
}

EWP2::~EWP2()
{
    LOG_INFO << "epicWP STOPPED";

    delete togZ;
    delete togN;
    delete togE;

    delete gmpeakZ;
    delete gmpeakN;
    delete gmpeakE;
    delete pickerZ;
    delete trigpar;

//    delete [] data;
}

bool EWP2::process(RawPacket &p)
{
    int overlap;
    int ndata=0;
    double first_sample_time;

    packet_start_time = p.start_time; // for reset()
    packet_channel = string(p.ch.channel); // for reset()

    WP::process(p);

    if(packet_channel == string(chanZ.channel)) {
	overlap = Zbuffer.putPacket(p);
	ndata = Zbuffer.getData(data, &first_sample_time, &packet_info);
	if(ndata <= 0) return true;
	togZ->process(ndata, data);
	gmpeakZ->process(ndata, data, first_sample_time, packet_info);
	Zbuffer.updateData(data);
    }
    else if(packet_channel == string(chanN.channel)) {
	overlap = Nbuffer.putPacket(p);
	ndata = Nbuffer.getData(data, &first_sample_time, &packet_info);
	if(ndata <= 0) return true;
	togN->process(ndata, data);
	gmpeakN->process(ndata, data, first_sample_time, packet_info);
	Nbuffer.updateData(data);
    }
    else if(packet_channel == string(chanE.channel)) {
	overlap = Ebuffer.putPacket(p);
	ndata = Ebuffer.getData(data, &first_sample_time, &packet_info);
	if(ndata <= 0) return true;
	togE->process(ndata, data);
	gmpeakE->process(ndata, data, first_sample_time, packet_info);
	Ebuffer.updateData(data);
    } 
    else { // don't recognize channel
	return true;
    }
    if(overlap > 0) warnOverlap(overlap, p);

    if(packet_channel == string(chanZ.channel)) { // Z Channel
	int trigger_sample;
	if(pickerZ->process(ndata, data, first_sample_time, &trigger_sample))
	{
	    // send the EWTrigger
	    double trigger_time = first_sample_time + trigger_sample/packet_info.samplerate;
	    TimeStamp ts(UNIX_TIME, trigger_time);
	    EWTrigger trigger(p, ts);
	    trigger.lat = chanZ.latitude;
            trigger.lon = chanZ.longitude;

	    sender->send(&trigger);

	    trigpar->startTrigger(ts, packet_info, Zbuffer, Nbuffer, Ebuffer);
	    process_trigger = true;
	}
	Zbuffer.updateData(data);
    }
    if(process_trigger) {
	process_trigger = trigpar->updateTrigger(string(p.ch.channel), Zbuffer, Nbuffer, Ebuffer);
    }
//    writeData(p, ndata, data);
    return true;
}

void EWP2::warnOverlap(int overlap, RawPacket &p)
{
    if(overlap >= 1) {
	char s[100];
	snprintf(s, sizeof(s), "%-5.5s %-2.2s %-3.3s %-2.2s samples: %d",
		p.ch.station, p.ch.network, p.ch.channel, p.ch.location, overlap);
	LOG_INFO << "Overlap: " << TimeStamp(UNIX_TIME, p.start_time) << " " << s;
    }
}

void EWP2::reset()
{
    if(packet_channel == string(chanZ.channel)) {
	togZ->restart();
	pickerZ->restart(packet_start_time);
	trigpar->restart(packet_start_time);
	Zbuffer.clear();
    }
    else if(packet_channel == string(chanN.channel)) {
	togN->restart();
	trigpar->restart(packet_start_time);
	Nbuffer.clear();
    }
    else if(packet_channel == string(chanE.channel)) {
	togE->restart();
	trigpar->restart(packet_start_time);
	Ebuffer.clear();
    }
    file_start[packet_channel] = 2;
}

void EWP2::writeData(RawPacket &p, int ndata, ArrayStruct &data)
{
    // start = 1: open new file and write seed header
    // start = 2: write seed header (gap)
    // start = 0: same file, continue writing data

    map<string, int>::iterator it = file_start.find(packet_channel);
    int start = (it != file_start.end()) ? (*it).second : 1;

    writeComponent("raw_",  p, ndata, data, start);
    writeComponent("disp_", p, ndata, data, start);
    writeComponent("vel_",  p, ndata, data, start);
    writeComponent("acc_",  p, ndata, data, start);

    if(packet_channel == string(chanZ.channel)) {
	writeComponent("filtvel_",  p, ndata, data, start);
	writeComponent("lta_",  p, ndata, data, start);
	writeComponent("sta_",  p, ndata, data, start);
	writeComponent("ratio_",  p, ndata, data, start);
    }

    file_start[packet_channel] = 0;
}

void EWP2::writeComponent(string prefix, RawPacket &p, int ndata, ArrayStruct &data, int start)
{
    string file = prefix + string(p.ch.network) + "." + string(p.ch.station)
			+ "." + packet_channel + "." + string(p.ch.location);
    FILE *fp;
    const char *mode = (start == 1) ? "w" : "a";
    if((fp = fopen(file.c_str(), mode)) == NULL) {
	LOG_INFO << "Cannot open data file " << file << strerror(errno);
	exit(1);
    }
    int type = (prefix == "raw_") ? -1 : -2;
    if(start > 0) {
	LOG_INFO << "Start: " << p.ch.network << "." << p.ch.station << "." << packet_channel << "."
			<< p.ch.location << " " << TimeStamp(UNIX_TIME, p.start_time);
	writeHeader(fp, p, type);
    }

    int n = ndata;
    wordorder_master(n);
    fwrite(&n, sizeof(int), 1, fp);

    if(prefix == "raw_") {
	int *d = new int[ndata];
	for(int i = 0; i < ndata; i++) { d[i] = data.raw[i]; wordorder_master(d[i]); }
	fwrite(d, sizeof(int), ndata, fp);
	delete[] d;
    }
    else {
	float *f = new float[ndata];
	if(prefix == "disp_") {
	    for(int i = 0; i < ndata; i++) { f[i] = data.d[i]; wordorder_master(f[i]); }
	}
	else if(prefix == "vel_") {
	    for(int i = 0; i < ndata; i++) { f[i] = data.v[i]; wordorder_master(f[i]); }
	}
	else if(prefix == "acc_") {
	    for(int i = 0; i < ndata; i++) { f[i] = data.a[i]; wordorder_master(f[i]); }
	}
	else if(prefix == "filtvel_") {
	    for(int i = 0; i < ndata; i++) { f[i] = data.r[i]; wordorder_master(f[i]); }
	}
	else if(prefix == "lta_") {
	    for(int i = 0; i < ndata; i++) { f[i] = data.lta[i]; wordorder_master(f[i]); }
	}
	else if(prefix == "sta_") {
	    for(int i = 0; i < ndata; i++) { f[i] = data.sta[i]; wordorder_master(f[i]); }
	}
	else if(prefix == "ratio_") {
	    for(int i = 0; i < ndata; i++) {
		f[i] = (data.lta[i] != 0.) ? data.sta[i]/data.lta[i] : 0.;
		wordorder_master(f[i]);
	    }
	}
	fwrite(f, sizeof(float), ndata, fp);
	delete[] f;
    }
    fclose(fp);
}

void EWP2::writeHeader(FILE *fp, RawPacket &p, int header_flag)
{
    double start_time = p.start_time;
    double samplerate = p.samplerate;
    wordorder_master(header_flag);	// -1 int, -2 float
    wordorder_master(start_time);
    wordorder_master(samplerate);
    fwrite(&header_flag, sizeof(int), 1, fp);
    fwrite(&start_time, sizeof(double), 1, fp);
    fwrite(&samplerate, sizeof(double), 1, fp);
    char c[10];
    memset(c, 0, sizeof(c));
    strcpy(c, p.ch.network);
    fwrite(c, sizeof(char), 10, fp);
    memset(c, 0, sizeof(c));
    strcpy(c, p.ch.station);
    fwrite(c, sizeof(char), 10, fp);
    memset(c, 0, sizeof(c));
    strcpy(c, p.ch.channel);
    fwrite(c, sizeof(char), 10, fp);
    memset(c, 0, sizeof(c));
    strcpy(c, p.ch.location);
    fwrite(c, sizeof(char), 10, fp);
}
