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
#ifndef _E2Trigger_H__
#define	_E2Trigger_H__

#include <iostream>
#include <math.h>
#include "Exceptions.h"
#include "TriggerParams.h"
#include "MaxAmplitudes.h"
#include "FilterBank.h"
#include "NEtoZ.h"
#include "RangePostTrig.h"
#include "ZeroCrossings.h"

typedef enum {
    UNASSOCIATED = 0,
    NEAR_SRCE	 = 1,
    TT_WINDOW	 = 2,
    MULTI_STA  	 = 3,
    TWO_STATN	 = 4,
    ONE_STATN    = 5
} AssocCode;

typedef enum {
    OKAY	 = 0,
    NO_Z_COMP    = 1,
    TP_SMALL     = 2,
    TP_LARGE	 = 3,
    PD_SMALL     = 4,
    PD_LARGE     = 5,
    PV_SMALL     = 6,
    PV_LARGE     = 7,
    PA_SMALL     = 8,
    Z_CROSS	 = 9,
    NE_LARGE	 = 10,
    ACCRANGE	 = 11,
    WAITING	 = 12,
    RMSLARGE	 = 13
} QualityCode;

class E2Trigger {
  private:
    int eventid;
    int update;
    int msg_version;
    std::string station;
    std::string channel;
    std::string network;
    std::string location;
    std::string stachan;

//    bool associated;
    int _sta_id;
    int _stachan_id;
    int version; // event version
    QualityCode quality_code;

    double first_sample_time;
    double taup_output_delay;
    double clip_lev;
    double clip_dur;
    int   az_nsamps;

  public:
    double lat;
    double lon;
    double trigger_time;

    int plength;
    double samprate;
    double toffset;
    double outof_ring;
    double outof_feeder_queue;
    double trigger_found;
    double into_send_queue;
    double outof_send_queue;
    double e2_latency;
    double buf_latency;

    int nsamps;
    int z_recent_sample;
    int n_recent_sample;
    int e_recent_sample;
    int order;
    double distkm;
    double tterr;

    bool magchan_ok;
    bool magdistok;
    bool pdmag_ok;
    bool pdSNRok;
    bool tpmag_ok;
    bool tpSNRok;
    int tele_fbands; // 0: measured at TFMinWindowIndex and is not teleseismic
    		     // 1: measured at TFMinWindowIndex and is teleseismic
    		     // 2: lag < TFMinWindowIndex. Waiting to measure at TFMinWindowIndex
    		     // 3: lag >= TFMinWindowIndex but lead < 30 secs. Cannot measure.
    int tele_window; // largest window index available: 0 <= tele_window <= NUM_FWINDOWS

    double tpmag, pdmag;
    double coslat, sinlat, coslon, sinlon;
    bool tpmag_used, pdmag_used, used, rms_large;
    int teleseismic;
    double minus_tele;
    bool  az_computed;
    double azimuth;
    double incidence;
    double azimuth_error;
    AssocCode assoc_code;
    bool free_with_unassoc;
    int fb_window_printed;

    FilterBank filter_bank;
    MaxAmplitudes max_amplitudes;
    NEtoZ ne_to_z;
    RangePostTrig range_post_trig;
    ZeroCrossings zero_crossings;

    E2Trigger(TriggerParams &tp, int sta_id, int stachan_id);
    ~E2Trigger() { }
    bool updateMeasurements(TriggerParams &tp);

    void incrementUpdate() { update++; }
    int getUpdate() { return update; }

    int staId() { return _sta_id; }
    int staChanId() { return _stachan_id; }
    const char *assocCodeStr();

    bool isAssociated() { return assoc_code != UNASSOCIATED; }
    bool unassociated() { return assoc_code == UNASSOCIATED; }
    /**
     * Disassociate with an Event.
     */    
//    void disassociate() { associated = false; eventid = 0; version = 0; assoc_code = UNASSOCIATED; }
    void disassociate() { eventid = 0; version = 0; assoc_code = UNASSOCIATED; }
    int getEventid() { return eventid; }

    bool associate(int event_id, int event_version, int msg_ver, AssocCode assocCode) {
//	if(associated) {
	if(isAssociated()) {
	    std::cout << toShortString("W") << " already associated. eventid: " << eventid << std::endl;
	    return false;
	}
	eventid = event_id;
	version = event_version;
	msg_version = msg_ver;
	assoc_code = assocCode;
//	associated = true;
	return true;
    }
    void setEventVersion(int ver) { version = ver; }

    char chanSecondChar() {
	if(channel.length() > 1) {
	    return channel.at(1);
	}
	return ' ';
    }
    std::string twoCharChan() {
	if(channel.length() > 1) {
	    return channel.substr(0,2);
	}
	return std::string("");
    }
    std::string getStaChan() {
	return stachan;
    }

    std::string &getSta() { return station; }
    std::string &getChan() { return channel; }
    std::string &getNet() { return network; }
    std::string &getLoc() { return location; }
    std::string getNetSta() { return network + "." + station; }
    double getLat()  { return lat; }
    double getLon()  { return lon; }
    double getTime() { return trigger_time; }
    double getTaup() { return max_amplitudes.tp; }
    double getTaupSnr() { return max_amplitudes.tp_snr; }
    double getPd()    { return max_amplitudes.pd; }
    double getPdSnr() { return max_amplitudes.pd_snr; }
    double getPv() { return max_amplitudes.pv; }
    double getPvSnr() { return max_amplitudes.pv_snr; }
    double getPa() { return max_amplitudes.pa; }
    double getPaSnr() { return max_amplitudes.pa_snr; }
    double getLogTaup() { return (max_amplitudes.tp != 0.) ? log10(fabs(max_amplitudes.tp)) : 1.e-60; }
    double getLogPd() { return (max_amplitudes.pd != 0.) ? log10(fabs(max_amplitudes.pd)) : 1.e-60; }
    double getLogPv() { return (max_amplitudes.pv != 0.) ? log10(fabs(max_amplitudes.pv)) : 1.e-60; }
    double getLogPa() { return (max_amplitudes.pa != 0.) ? log10(fabs(max_amplitudes.pa)) : 1.e-60; }
    int getPlength() { return plength; }
    double getSamprate() { return samprate; }
    double getToffset() { return toffset; }
    double getE2Latency() { return e2_latency; }
    double getBufLatency() { return buf_latency; }
    void setBufLatency(double latency) { buf_latency = latency; }

    int numZeroCross();
    double rangePostTrig(double *min_range);

    FilterWindow *getPgv() { return filter_bank.fw; }
    std::string toString();
    std::string toString(int evid, int version, int update, int order);
    std::string toEmailString(char group);
    std::string toShortString();
    std::string toShortString(const char *comp);
    std::string timeString();
    int msgVersion() { return msg_version; }
    static double stringToDouble(const std::string &s, const std::string name) throw(Error);
    static int stringToInt(const std::string &s, const std::string name) throw(Error);
    static double stringToTime(const std::string &s) throw(Error);

    static std::string labels() {
	return std::string(
"   eventid ver update order   sta chan net loc      lat       lon            trigger_time rsmp tsmp      log_taup      taup_snr dsmp        log_pd        pd_snr    assoc  tpmag utpm  pdmag updm uch ukm upd ups utp uts tel tsec distkm azimuth");
    }

    QualityCode getQualityCode();
    AssocCode getAssocCode() { return assoc_code; }
};

struct trigger_compare {
    bool operator() (E2Trigger * lhs, E2Trigger * rhs) const
    {
 	if(lhs->msgVersion() != rhs->msgVersion()) {
	    return lhs->msgVersion() < rhs->msgVersion();
	}
	if(lhs->trigger_time != rhs->trigger_time) return lhs->trigger_time < rhs->trigger_time;
	return strcasecmp(lhs->getStaChan().c_str(), rhs->getStaChan().c_str());
    }
};
#endif
