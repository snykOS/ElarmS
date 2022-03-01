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
#ifndef _LogHeaders_h_
#define _LogHeaders_h_
#include <string>

static std::string log_headers = "\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: S:  Station WP Latency Stats: Means and Standard Deviation for the period defined by start_time and duration.\n\
D:     WP Latency = clock time that the raw data packet arrived at the waveform processor minus the time of the last sample in the packet\n\
D: net:	network, sta: station, chan: channel, loc: location, lat: latitude, lon: longitude, U: station is used by E2,\n\
D: last_received_time: clock time that the most recent GMPeak packet was received by E2 for this station,\n\
D: num_packet_sources: the number of EWP2 processes that are sending GMPeak packets to E2,\n\
D: For each computer listed at the end of the S:H line:\n\
D: mean: Mean WP latency for the channel and waveform processor\n\
D: std: Standard Deviation of the WP latency for the channel and waveform processor\n\
D: max:	Maximum value of the WP latency for the channel and waveform processor\n\
D: pacdur: Maximum packet duration for the channel and waveform processor\n\
D: adjmean: Mean WP latency plus 0.5*packet duration for the channel and waveform processor\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: N: New trigger information\n\
D: sta: station, chan: channel, net: network, loc: location, lat: latitude, lon: longitude, time: trigger time, up: update index,\n\
D: c: Component code (Z,E,N),\n\
D: rsp:   The sample offset from the trigger sample of the most recent data available to the waveform processor\n\
D: tsp:   The sample offset from the trigger sample of the maximum taup measurement on the c-component\n\
D: tp:    The maximum taup measurement on the c-component\n\
D: tpsnr: The signal to noise for the maximum taup measurement on the c-component\n\
D: dsp:   The sample offset from the trigger sample of the maximum displacement value on the c-component\n\
D: pd:    The maximum displacement on the c-component\n\
D: pdsnr: The signal to noise for the maximum displacement on the c-component\n\
D: vsp:   The sample offset from the trigger sample of the maximum velocity value on the c-component\n\
D: pv:    The maximum velocity on the c-component\n\
D: pvsnr: The signal to noise for the maximum velocity on the c-component\n\
D: asp:   The sample offset from the trigger sample of the maximum acceleration value on the c-component\n\
D: pa:    The maximum acceleration on the c-component\n\
D: pasnr: The signal to noise for the maximum accelration on the c-component\n\
D: assoc: one of:\n\
D:       UNASSOC: the trigger is unassociated\n\
D:      NO_ZCOMP: no Z-component measurements are available yet\n\
D:      PD_SMALL: trigger cannot be used because log(Pd) < MinPD\n\
D:      PD_LARGE: trigger cannot be used because log(Pd) > MaxPD\n\
D:      PV_SMALL: trigger cannot be used because log(Pv) < MinPV\n\
D:      PV_LARGE: trigger cannot be used because log(Pv) > MaxPV\n\
D:      PA_SMALL: trigger cannot be used because log(Pa) < MinPA\n\
D:      TP_SMALL: trigger cannot be used because log(Taup) < MinTP\n\
D:      TP_LARGE: trigger cannot be used because log(Taup) > MaxTP\n\
D:      NEAR_SRC: trigger is associated to an event using the near-source association criterion\n\
D:      TTWINDOW: trigger is associated to an event using the TT-window association criterion\n\
D:      MULTISTA: trigger is associated to an event using the multiStationEvent method\n\
D:      TWOSTATN: trigger is associated to an event using the twoStationEvent method\n\
D:      ONESTATN: trigger is associated to an event using the singleStationEvent method\n\
D: tel: 1 - trigger time is within a teleseismic event window, (window_start-teleseismic_secs <= trigger_time < window_end+teleseismic_secs)\n\
D:      2 - trigger time is within +/- teleseismic_sec of the predicted phase arrival time, computed on distance to the trigger station.\n\
D:      0 - trigger time has not been identified as teleseismic.\n\
D: tsec: If tel is 1 or 2, this is the trigger time minus predicted teleseismic arrival time.\n\
D: plen: the number of samples in the packet that contains the trigger sample\n\
D: sps: the sample rate for this channel\n\
D: toffset: the offset of the trigger (seconds) from the first sample in the packet\n\
D: arrtime: the arrival time of the trigger-packet at the waveform processor (packet-arrival-time) minus the trigger time.\n\
D: protime: the time that the trigger-packet was passed to the WP processing thread minus the packet-arrival-time\n\
D: fndtime: the time that the trigger was detected minus the packet-arrival-time\n\
D: quetime: the time that the trigger entered the WP message-send-queue minus the packet-arrival-time\n\
D: sndtime: the time that the trigger message was passed to the activemq send function minus the packet-arrival-time\n\
D: e2time:  the time that trigger message arrived at the Event Associator and was placed in the trigger-buffer minus the packet-arrival-time\n\
D: buftime: the time that the trigger was removed from the trigger-buffer and inserted into the trigger pool minus the packet-arrival-time\n\
D: zc: Minimum number of zero-crossings\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: U: Updated trigger information\n\
D: sta: station, chan: channel, net: network, loc: location, lat: latitude, lon: longitude, time: trigger time, up: update index,\n\
D: c: Component code (Z,E,N),\n\
D: rsp:   The sample offset from the trigger sample of the most recent data available to the waveform processor\n\
D: tsp:   The sample offset from the trigger sample of the maximum taup measurement on the c-component\n\
D: tp:    The maximum taup measurement on the c-component\n\
D: tpsnr: The signal to noise for the maximum taup measurement on the c-component\n\
D: dsp:   The sample offset from the trigger sample of the maximum displacement value on the c-component\n\
D: pd:    The maximum displacement on the c-component\n\
D: pdsnr: The signal to noise for the maximum displacement on the c-component\n\
D: vsp:   The sample offset from the trigger sample of the maximum velocity value on the c-component\n\
D: pv:    The maximum velocity on the c-component\n\
D: pvsnr: The signal to noise for the maximum velocity on the c-component\n\
D: asp:   The sample offset from the trigger sample of the maximum acceleration value on the c-component\n\
D: pa:    The maximum acceleration on the c-component\n\
D: pasnr: The signal to noise for the maximum accelration on the c-component\n\
D: assoc: see above.\n\
D: zc: Minimum number of zero-crossings\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: F: Trigger Teleseismic Multi-window/Multi-band Filter Measurement\n\
D:      sta chan net loc      lat       lon            trigger-time  U C  lead  lag npts  Pgv1  T1  Pgv2 T2  Pgv3 T3  Pgv4 T4  Pgv5 T5  Pgv6 T6  Pgv7 T7  Pgv8 T8  Pgv9 T9 TTest TF\n\
D:|F:   BC3  HLZ  CI  --  33.6551 -115.4537 1999-10-16T09:46:21.040  5 Z 30.00 0.30 3031 -3.978 T -3.958 T -3.949 T -3.800 T -3.399 T -3.546 T -3.504 T -3.620 T -3.586 T  0.032 T\n\
D:   The test for a teleseismic trigger is T1 to T9 are true and TTest is > 0., where TTest is logPgv[TFLow] - (TFM2*logPgv[TFHigh] + TFB2);\n\
D: U: trigger update index\n\
D: C: component\n\
D: lead: the start of the data window for the IIR filter in seconds before the trigger time\n\
D: lag: the end of the data window for the IIR filter in seconds after the trigger time\n\
D:      the 11 lag values are: 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 and 2.0 seconds.\n\
D: npts: the number of samples in the data window for the IIR filter\n\
D: PgvN: Log10 Peak ground velocity amplitude measure for this time window and the N'th frequency band.\n\
D: TN: T/F - true of false - is the PgvN value between the amplitude limits for a teleseismic signal. Amplitude limits are in tfparam.dat\n\
D: TTest: logPgv[TFLow] - (TFM2*logPgv[TFHigh] + TFB2)\n\
D: TF: If all T2 through T9 are 'T' and TTest (logPgv[TFLow] - (TFM2*logPgv[TFHigh] + TFB2)) is > 0, the trigger is classified as teleseismic\n\
D:  The 9 frequency bands for three sample rate intervals are:\n\
D:  if sample rate < 32 {\n\
D:	low 	high\n\
D:	6. 	8.\n\
D:	6.	8.\n\
D:	6.	8.\n\
D:	3.	6.\n\
D:	1.5	3.\n\
D:	0.75	1.5\n\
D:	0.375	0.75\n\
D:	0.1875	0.375\n\
D:	0.09375 0.1875\n\
D:  }\n\
D:  else if sample rate < 96  {\n\
D:	low 	high\n\
D:	12.	16.\n\
D:	8.	12.\n\
D:	6.	8.\n\
D:	3.	6.\n\
D:	1.5	3.\n\
D:	0.75	1.5\n\
D:	0.375	0.75\n\
D:	0.1875	0.375\n\
D:	0.09375 0.1875\n\
D:  }\n\
D:  else {\n\
D:	low 	high\n\
D:	24.	48.\n\
D:	12.	24.\n\
D:	6.	12.\n\
D:	3.	6.\n\
D:	1.5	3.\n\
D:	0.75	1.5\n\
D:	0.375	0.75\n\
D:	0.1875	0.375\n\
D:	0.09375 0.1875\n\
D:  }\n\
D: The window lengths and filter band limits are set in TriggerParamsTSPModule.cc\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: E:I:H: Event Information Header\n\
D: eventid: Event id: < 0 for events that are never alerted; > 0 for event that are alerted\n\
D: ver:	 Event version (update) number\n\
D: evlat: latitude, evlon: longitude, dep: depth, mag: magnitude, time: origin time, latu: latitude uncertainty, lonu: longitude uncertainty\n\
D: depu: depth uncertainty, magu: magnitude uncertainty, timeu: origin time uncertainty, lk: event likelihood\n\
D: nTb: number of triggers in the build\n\
D: nSb: number of stations in the build\n\
D: nT:	number of triggers in the current alert\n\
D: nS:	number of stations in the current alert\n\
D: ave:	 average station travel time error, rms: root-mean-square of station travel time error, fitok: (ave < MaxMisfit)\n\
D: TF: (on the event line) event teleseismic code from the multi-data-window/multi-filter-band discriminator\n\
D:      0 - event is NOT teleseismic. TFNumStaRequired stations measured NOT teleseismic at the data lag length TFMinWindowIndex secs\n\
D:	1 - event is teleseismic. TFNumStaRequired stations measured teleseismic at the data lag length TFMinWindowIndex secs\n\
D:	2 - less than TFNumStaRequired stations with data lag length TFMinWindowIndex seconds. waiting to measure\n\
D: splitok: 1: this is not a split event, 0: this event was rejected because it is a split event\n\
D: near:    number of near stations, counting clusters as one station\n\
D: statrig: number of near stations that triggered, counting clusters as one station\n\
D: active:  number of near stations that are active, counting individual stations\n\
D: inact:   number of near stations that are inactive, counting individual stations\n\
D: nsta:    number of near active plus inactive stations\n\
D: percnt:  statrig / near,  prcntok: (percnt >= TrigStaPercent)\n\
D: mindist: minimum associated station distance, maxdist: maximum associated station distance, distok: (min_dist < MaxMagDistkm)\n\
D: azspan:  maximum station azimuth minus minimum station azimuth\n\
D: Mok:	 (AlertMinMag < magnitude < AlertMaxMag)\n\
D: nSok: (nT >= AlertMinTrigs && nS >= AlertMinStats && (if within a AlertMinStaRegion: nS >= region_min_sta))\n\
D: Lok:	 (current location is not on the edge of the search grid)\n\
D: Tdif:  log10(pdaverage) - (-3.728 + 2.817 * log10(tpaverage))\n\
D: tpave: log10(tpaverage)\n\
D: pdave: log10(pdaverage)\n\
D: TF: trigger teleseismic code from the multi-window/filter-band teleseismic discriminator\n\
D:	0 - trigger is NOT teleseismic\n\
D:	1 - trigger is teleseismic\n\
D:	2 - waiting for window length of TFMinWindowIndex\n\
D:	3 - insufficient data before the trigger for the filter window.\n\
D: Tok:	 (log10(pdaverage) < (-3.728 + 2.817 * log10(tpaverage))\n\
D: Azok: Event azimuth span >= minAzSpan\n\
D: Aok:	Alert passed all criteria and can be sent\n\
D: Ast:	Alert actually sent (alerts are not sent more that once per second)\n\
D: alert_time: Time that the alert criteria were checked\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: E:I:T:H: Event Information Trigger Header\n\
D: eventid: Event id\n\
D: ver: Event version (update) number\n\
D: update: Trigger update. 0 for initial trigger values\n\
D: order: The order that the triggers were associated with the event\n\
D: sta: station name, chan: channel name, net: network name, loc: location name, lat: station latitude, lon: station longitude\n\
D: trigger_time: trigger time\n\
D: rsmp: the number of data samples more recent than the trigger sample that were available to the waveform processor at detection time\n\
D: tsmp: the sample offset from the trigger of the maximum value of Taup.\n\
D: log_taup: log(maximum Taup), taup_snr: SNR at the time of the maximum Taup\n\
D: taup_snr: Taup signal-to-noise at tsmp\n\
D: dsmp: the sample offset from the trigger of the maximum value of displacement (Pd)\n\
D: log_pd: log(abs(maximum displacement)), pd_snr: SNR at the time of the maximum displacement\n\
D: pd_snr: Pd signal-to-noise at dsmp\n\
D: assoc: association code (see above)\n\
D: tpmag: Magnitude computed from log_taup measurements only\n\
D: utpm: tpmag is used\n\
D: pdmag: Magnitude computed from log_pd measurements only\n\
D: updm: pdmag is used\n\
D: uch: channel is okay for magnitude computation (net==\"CI\" || net==\"AZ\" || chan_2nd_char='L' || chan_2nd_char='N' || chan_2nd_char=='H')\n\
D: ukm: trigger distance is okay for magnitude computation (see E2Magnitude.cc)\n\
D: upd: (Pdmag > 0 && Pdmag < 9)\n\
D: ups: (Pd-SNR >= MinPdSNR)\n\
D: utp: (Tpmag > 0 && Tpmag < 9)\n\
D: uts: (Tp-SNR >= MinTaupSNR)\n\
D: tel: 1 - trigger time is within a teleseismic event window, (window_start-teleseismic_secs <= trigger_time < window_end+teleseismic_secs)\n\
D:      2 - trigger time is within +/- teleseismic_sec of the predicted phase arrival time, computed using distance to the trigger station.\n\
D:      0 - trigger time has not been identified as teleseismic.\n\
D: tsec: If tel is 1 or 2, this is the trigger time minus predicted teleseismic arrival time.\n\
D: distkm: trigger distance, azimuth: trigger azimuth, tterr: travel-time error\n\
D: azimuth: the event to station azimuth\n\
D: TF: trigger teleseismic code from multi-window/filter-band discriminator\n\
D:      0 - trigger is NOT teleseismic\n\
D:      1 - trigger is teleseismic\n\
D:      2 - waiting for window length of TFMinWindowIndex\n\
D:      3 - insufficient data before the trigger for the filter window.\n\
D: tterr: trigger time minus predicted trigger time using the current event location\n\
D: plen: trigger packet length, sps: channel sample rate\n\
D: sps: sampling rate for this channel\n\
D: toffset: the offset of the trigger (seconds) from the first sample in the packet\n\
D: arrtime: the arrival time of the trigger-packet at the waveform processor (packet-arrival-time) minus the trigger time.\n\
D: protime: the time that the trigger-packet was passed to the WP processing thread minus the packet-arrival-time\n\
D: fndtime: the time that the trigger was detected minus the packet-arrival-time\n\
D: quetime: the time that the trigger entered the WP message-send-queue minus the packet-arrival-time\n\
D: sndtime: the time that the trigger message was passed to the activemq send function minus the packet-arrival-time\n\
D: e2time:  the time that trigger message arrived at the Event Associator and was placed in the trigger-buffer minus the packet-arrival-time\n\
D: buftime: the time that the trigger was removed from the trigger-buffer and inserted into the trigger pool minus the packet-arrival-time\n\
D: alert:   the time of the alert minus the packet-arrival-time\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: B:H: Event Build Summary\n\
D: B:N: starting station\n\
D: loop: search algorithm loop (1 or 2), ord: station-added index; net, sta, ch, loc: station tested\n\
D: evlat, evlon, dep, evtime: current event location, ttok: tt accepted, tt: station travel time (must be <= MultiStaMaxDist/PVelocity)\n\
D: dok: distance accepted, dist: station-to-event distance (must be <= MultiStaMaxDist)\n\
D: msta: station with max station-to-station tt-residual; maxd: distance to this sta; mttres: tt-residual to this sta; mttok: mttres accepted\n\
D: trig: add trigger, ns: number of stations, nns: num stations with trigger, nt: num triggers, lo: located,\n\
D: avefit: location average error, rmsfit: rms error\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: E:S:H Station Count Summary\n\
D: eventid, ver, evlat, evlon, time: counting triggered stations for this event\n\
D: mindist: minimum event-to-station distance, maxdist: maximum station distance\n\
D: percnt: the percentage of active stations that triggered (sta_trig_cnt/near_sta_cnt)\n\
D: near_sta_cnt: number of active stations, counting a cluster as one station\n\
D: sta_trig_cnt: number of triggered stations, counting a triggered cluster as one station\n\
D: active: number of activem stations, inactive: number of inactive stations, nsta: total number of stations\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: E:C:H Station Count Details\n\
D: eventid, ver: counting triggered stations for this event\n\
D: sta, net, lat, lon: a station whose distance to the event is less than the distance to the fartherest triggered station\n\
D: cluster: station is a member of a cluster, dist: station to event distance, tt: predicted travel time to station\n\
D: time: last_data_time = time of most recent gmpeak packet from the station\n\
D: time_check: last_data_time - (event_time + tt - MaxStationDelay). If this is positive, the station is counted as active\n\
D: active: counted as active, trig: triggered, clu: part of a cluster, ctrig: the cluster triggered (40% of cluster stations triggered)\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: L:E:H: Location Algorithm Header\n\
D: eventid: event located, ver: event version, nT: number of triggers, s: where location routine was called in the code (1,2,or 3)\n\
D: lat0, lon0, dep0, time0: initial values; lat, lon, dep, time: new location, ddist: distance(km) from old to new location\n\
D: avefit: average trigger travel-time error, rmsfit: route-mean-square of the travel time error, nT: number of triggers, nS: number of stations\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: L:T:H: Location Triggers\n\
D: U: trigger was used in the location, dist: trigger distance to new location, tt: travel-time to new location, tterr: travel-time error\n\
D: ---------------------------------------------------------------------------------------------------------------------------------\n\
D: TE:H: Teleseismic Event Message Header\n\
D: src,eventid,origin_time,lat,lon,depth,mag: PDL Product Event Source, EventId, EventTime, Latitude, Longitude, Depth, and Magnitude.\n\
D: window_start, window_end: the earliest and latest arrival times for the phases and lat,lon points specified by the EEWTeleseism\n\
D: program configuration parameters \"phase\" and \"region\".\n\
D: TE:A:H: phase,distdeg,arrival_time: arrival name, distance in degrees, and travel time in seconds.";
#endif
