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
#include <unistd.h>
#include <vector>
#include <plog/Log.h>
#include "E2TriggerManager.h"
#include "E2Trigger.h"
#include "E2Event.h"
#include "E2AMQReader.h"
#include "E2Reader.h"
#include "E2ModuleManager.h"
#include "E2Prop.h"
#include "TimeStamp.h"
#include "deltaz.h"
#include "E2Region.h"

map<string, int> E2TriggerManager::netsta_map;
map<int, string> E2TriggerManager::sta_id_map;
map<string, int> E2TriggerManager::stachan_map;

bool E2TriggerManager::replay_mode = false;
double E2TriggerManager::tfMaxStaDist = 260.;
int E2TriggerManager::tfMinWindowIndex = 1;
int E2TriggerManager::tfLow = 6;
int E2TriggerManager::tfHigh = 2;
double E2TriggerManager::tfM2 = 1.25; // slope of (logpgv[tfHigh],logpgv[tfLow]) cutoff line
double E2TriggerManager::tfB2 = 1.40; // intercept of (logpgv[tfHigh],logpgv[tfLow]) cutoff line
// test if above line: logPgv[tfLow] > tfM2*logPgv[tfHigh] + tfB2
bool E2TriggerManager::tfChannelReverse = false;

double E2TriggerManager::minTP;  // minimum log10(TauPmax) value for any event 
double E2TriggerManager::maxTP;  // maximum log10(TauPmax) value for any event 
double E2TriggerManager::minPD;  // minimum log10(Pd) value for any event 
double E2TriggerManager::maxPD;  // maximum log10(Pd) value for any event 
double E2TriggerManager::minPV;  // minimum log10(Pv) value for any event 
double E2TriggerManager::maxPV;  // maximum log10(Pv) value for any event 
double E2TriggerManager::minPA;  // minimum log10(Pa) value for any event 
bool E2TriggerManager::zCrossVelAlways = false; // If true, count using velocity even when the channel is acceleration. If false, count vel or accel
int E2TriggerManager::zCrossMin = 0;  // minimum number of post-trig zero-crossings
// range_post_trig = maximum change in the length of the acceleration vector in the
// time period from startRangePostTrig to endRangePostTrig seconds after the trigger
// if range_post_trig < minRangePostTrig, do not use the trigger
double E2TriggerManager::maxNEtoZ; // maximum ratio of largest (N,E) to Z component amplitude
// If rangeAccelAlways is true, always use the acceleration vector for the MinRangePostTrig, otherwise use vel or accel
bool E2TriggerManager::rangeAccelAlways = true;

double E2TriggerManager::minRangePostTrigAccel;
double E2TriggerManager::minRangePostTrigVel;


#define DEFAULT_TRIGGER_TIMEOUT 30
static int verbose;

#define PLOCK  PrintLock::lock()
#define PUNLOCK PrintLock::unlock()

static bool getNextLine(FILE *fp, char *line, int len);
static int readTFParams(string file, stringstream &ss);
static int readTFChannelType(FILE *fp, double *min, double *max);
static void printTFParams(stringstream &ss, string name, double *min, double *max);

E2TriggerManager::E2TriggerManager(bool replayMode) throw(Error) :
		reader(NULL), new_input(false), teleseismic_secs(5.0),
		num_stations(0), station_array(NULL), stop(false)
{
    string file, file_start, file_end, e2log;
    stringstream ss;
    char c[20];

    replay_mode = replayMode;
    try {
        unassoc_trigger_timeout = E2Prop::getInt("UnassocTriggerTimeout");
    } catch (Error e) { unassoc_trigger_timeout = DEFAULT_TRIGGER_TIMEOUT; }

    verbose = E2Prop::getInt("Verbose");
    useGMPeak = E2Prop::getBool("UseGMPeak");
    maxStationDelay = E2Prop::getDouble("MaxStationDelay");
    pVelocity = E2Prop::getDouble("PVelocity");
    try {
	teleseismic_secs = E2Prop::getDouble("TeleseismicSecs");
    } catch(Error e) { teleseismic_secs = 5.0; }
    minTP = E2Prop::getDouble("MinTP");
    maxTP = E2Prop::getDouble("MaxTP");
    minPD = E2Prop::getDouble("MinPD");
    maxPD = E2Prop::getDouble("MaxPD");
    minPV = E2Prop::getDouble("MinPV");
    maxPV = E2Prop::getDouble("MaxPV");
    minPA = E2Prop::getDouble("MinPA");
    zCrossVelAlways = E2Prop::getBool("ZeroCrossVelAlways");
    zCrossMin = E2Prop::getInt("ZeroCrossMin");
    maxNEtoZ = E2Prop::getDouble("MaxNEtoZ");
    rangeAccelAlways = E2Prop::getBool("RangeAccelAlways");
    minRangePostTrigAccel = E2Prop::getDouble("MinRangePostTrigAccel");
    minRangePostTrigVel = E2Prop::getDouble("MinRangePostTrigVel");
    useTF = E2Prop::getBool("UseTF");
    if(useTF) {
	tfFile = E2Prop::getString("TFFile");
	tfMaxStaDist = E2Prop::getDouble("TFMaxStaDist");
	tfMinWindowIndex = E2Prop::getInt("TFMinWindowIndex");
	if(tfMinWindowIndex < 0 || tfMinWindowIndex >= 11) throw(Error("Invalid TFMinWindowIndex"));
	tfLow = E2Prop::getInt("TFLow");
	if(tfLow < 0 || tfLow >= 9) throw(Error("Invalid TFLow"));
	tfHigh = E2Prop::getInt("TFHigh");
	if(tfHigh < 0 || tfHigh >= 9) throw(Error("Invalid TFHigh"));
	tfM2 = E2Prop::getDouble("TFM2");
	tfB2 = E2Prop::getDouble("TFB2");
	tfChannelReverse = E2Prop::getBool("TFChannelReverse");
	int ret = readTFParams(tfFile, ss);
	snprintf(c, sizeof(c), "%d", ret);
	if(ret == -1) throw(Error("Cannot open file " + tfFile));
	else if(ret > 0) throw(Error("Read error file " + tfFile + " line " + string(c)));
    }
    E2ModuleManager::param_str << "P: E2TriggerManager.UnassocTriggerTimeout: " << unassoc_trigger_timeout << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.UseGMPeak: " << useGMPeak << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MaxStationDelay: " << maxStationDelay << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.PVelocity: " << pVelocity << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.TeleseismicSecs: " << teleseismic_secs << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MinTP: " << minTP << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MaxTP: " << maxTP << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MinPD: " << minPD << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MaxPD: " << maxPD << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MinPV: " << minPV << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MaxPV: " << maxPV << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MinPA: " << minPA << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.ZeroCrossVelAlways: " << zCrossVelAlways << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.ZeroCrossMin: " << zCrossMin << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MaxNEtoZ: " << maxNEtoZ << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.RangeAccelAlways: " << rangeAccelAlways << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MinRangePostTrigAccel: " << minRangePostTrigAccel << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.MinRangePostTrigVel: " << minRangePostTrigVel << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.UseTF: " << useTF << endl;
    if(useTF) {
	E2ModuleManager::param_str << "P: E2TriggerManager.TFFile: " << tfFile << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFMaxStaDist: " << tfMaxStaDist << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFMinWindowIndex: " << tfMinWindowIndex << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFLow: " << tfLow << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFHigh: " << tfHigh << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFM2: " << tfM2 << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFB2: " << tfB2 << endl;
	E2ModuleManager::param_str << "P: E2TriggerManager.TFChannelReverse: " << tfChannelReverse << endl;
	E2ModuleManager::param_str << ss.str();
    }

    pthread_mutex_init(&stations_lock, NULL);
    pthread_mutex_init(&teleseisms_lock, NULL);

    getClusters();

    trigger_connection = NULL;

    amq_user = E2Prop::getString("AmqUser");
    amq_password = E2Prop::getString("AmqPassword");
    trigger_amq_uri = E2Prop::getString("TriggerURI");
    trigger_amq_topic = E2Prop::getString("TriggerTopic");
    E2ModuleManager::param_str << "P: E2TriggerManager.AmqUser: " << amq_user << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.TriggerURI: " << trigger_amq_uri << endl;
    E2ModuleManager::param_str << "P: E2TriggerManager.TriggerTopic: " << trigger_amq_topic << endl;
    reader = new E2AMQReader(amq_user, amq_password, trigger_amq_topic, trigger_amq_uri, replay_mode, this);
    trigger_connection = ((E2AMQReader *)reader)->getConnection();
}

void E2TriggerManager::run()
{
    reader->runReader();
}

void E2TriggerManager::stopReader()
{
    reader->stopReader();
}

int E2TriggerManager::read(int last_evid)
{
    new_input = false;
    if( reader == NULL ) {
	stringstream msg_str;
	msg_str << "W: E2TriggerManager: NULL reader.";
	LOG_INFO << msg_str.str();
	msg_str << endl;
	E2ModuleManager::sendLogMsg(msg_str.str());
	return -1;
    }
    return reader->read(last_evid);
}

E2Trigger *E2TriggerManager::insertTrigger(TriggerParams &tp)
{
    static int doy = -1;
    static const string labels = "N:H: sta chan net loc      lat       lon                    time up c  rsp  tsp            tp         tpsnr  dsp            pd         pdsnr  vsp            pv         pvsnr  asp            pa         pasnr    assoc tel tsec plen    sps toffset arrtime protime fndtime quetime sndtime  e2time buftime zc ne_to_z  acc_range";

    if(verbose >= 2) { // Print the trigger label line only once per day
	if(TimeStamp::current_time().day_of_year() != doy) {
	    doy = TimeStamp::current_time().day_of_year();
	    LOG_INFO << labels;
	    E2ModuleManager::sendLogMsg(labels);
	}
    }
    // cha = channel with last character removed
    // allow only one trigger in the map from a sta.cha.net.loc with trigger time rounded to the nearest second
    string channel(tp.chan);
    int len = channel.length();
    string cha = (len > 1) ? channel.substr(0,len-1) : channel;

    // For the map key, round the trigger time to the nearest second
    stringstream snclt;
    snclt << string(tp.sta) << "." << string(cha) << "." << string(tp.net) << "." << string(tp.loc) << "." << (int)(tp.getTime()+.5);

    TriggerMap::iterator it = trigmap.find(snclt.str());

    E2Trigger *t = NULL;

    if( !trigmap.empty() && it != trigmap.end() )
    {
	t = (*it).second;
	t->setBufLatency(TimeStamp::current_time().ts_as_double(UNIX_TIME) - tp.outof_ring);
	if(!t->updateMeasurements(tp)) {
	    return NULL; // no change in trigger values
	}
	t->trigger_time = t->getTime();
        testTeleseismicPgv(t);

        if(verbose >= 2) {
            stringstream msg_str;
	    printUpdateString(msg_str, t, tp.chan);
            printTeleseismicPgv(msg_str, t);
            LOG_INFO << msg_str.str();
            E2ModuleManager::sendLogMsg(msg_str.str());
        }
	updated_trigs.push_back(t);
    }
    else
    {
	int sta_id, stachan_id;

	string netsta = string(tp.net) + "." + string(tp.sta);
	sta_id = staId(netsta);
	stachan_id = stachanId(netsta + "." + string(tp.chan).substr(0,2));

	if(stations.find(sta_id) == stations.end()) {
	    addStation(netsta, &tp);
	}

	t = new E2Trigger(tp, sta_id, stachan_id);
	t->trigger_time = t->getTime();
	t->setBufLatency(TimeStamp::current_time().ts_as_double(UNIX_TIME) - tp.outof_ring);
        testTeleseismicPgv(t);

	trigmap[snclt.str()] = t;
	new_input = true;

	if(verbose >= 2) {
            char s[200];
	    stringstream msg_str;
	    MaxAmplitudes *m = &t->max_amplitudes;
	    msg_str << "N: " << t->toShortString();
	    snprintf(s, sizeof(s), " %2d Z %4d %4d %13.6e %13.6e %4d %13.6e %13.6e %4d %13.6e %13.6e %4d %13.6e %13.6e %s",
		t->getUpdate(), t->z_recent_sample, m->tp_sample, m->tp, m->tp_snr, m->pd_sample, m->pd, m->pd_snr,
		m->pv_sample, m->pv, m->pv_snr, m->pa_sample, m->pa, m->pa_snr, t->assocCodeStr());
	    msg_str << s;
	    double outof_ring = (!E2TriggerManager::replay_mode) ? t->outof_ring - t->trigger_time : 0.;
	    double min_range;
	    double range = t->rangePostTrig(&min_range);

	    snprintf(s, sizeof(s), " %3d %4.1f %4d %6.1f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %2d %7.2f %10.3e",
		t->teleseismic, t->minus_tele, t->getPlength(), t->getSamprate(), t->getToffset(), outof_ring,
		t->outof_feeder_queue - t->trigger_time,
		t->trigger_found - t->trigger_time,
		t->into_send_queue - t->trigger_time,
		t->outof_send_queue - t->trigger_time,
		t->getE2Latency(), t->getBufLatency(), t->numZeroCross(), t->ne_to_z.ne_to_z(), range);
	    msg_str << s;
            printTeleseismicPgv(msg_str, t);
	    LOG_INFO << msg_str.str();
	    E2ModuleManager::sendLogMsg(msg_str.str());
	}
    }
    return t;
}

void E2TriggerManager::printUpdateString(stringstream &msg_str, E2Trigger *t, char *chan)
{
    char s[200];
    MaxAmplitudes *m = &t->max_amplitudes;
    t->incrementUpdate();
    double min_range;
    double range = t->rangePostTrig(&min_range);
    if(t->range_post_trig.getStatus() != 1) range = 0.;
    int n = (int)strlen(chan);
    char *comp = chan+n-1;
    msg_str << "U: " << t->toShortString(comp);
    snprintf(s, sizeof(s), " %2d %s %4d %4d %13.6e %13.6e %4d %13.6e %13.6e %4d %13.6e %13.6e %4d %13.6e %13.6e %s %10.3e",
	    t->getUpdate(), comp, t->z_recent_sample, m->tp_sample, m->tp, m->tp_snr, m->pd_sample, m->pd, m->pd_snr,
	    m->pv_sample, m->pv, m->pv_snr, m->pa_sample, m->pa, m->pa_snr, t->assocCodeStr(), range);
    msg_str << s;
}

static double hhz_min[NUM_FBANDS*NUM_FWINDOWS];
static double hhz_max[NUM_FBANDS*NUM_FWINDOWS];
static double hnz_min[NUM_FBANDS*NUM_FWINDOWS];
static double hnz_max[NUM_FBANDS*NUM_FWINDOWS];
static double b40_min[NUM_FBANDS*NUM_FWINDOWS];
static double b40_max[NUM_FBANDS*NUM_FWINDOWS];
static double b20_min[NUM_FBANDS*NUM_FWINDOWS];
static double b20_max[NUM_FBANDS*NUM_FWINDOWS];

static int lineno=0;

static int readTFParams(string file, stringstream &ss)
{
    FILE *fp;
    lineno = 1;

    if((fp = fopen(file.c_str(), "r")) == NULL) {
	return -1;
    }
    if( readTFChannelType(fp, hhz_min, hhz_max) ||
	readTFChannelType(fp, hnz_min, hnz_max) ||
	readTFChannelType(fp, b40_min, b40_max) ||
	readTFChannelType(fp, b20_min, b20_max) )
    {
	fclose(fp);
	return lineno;
    }
    fclose(fp);

    printTFParams(ss, "HHZ", hhz_min, hhz_max);
    printTFParams(ss, "HNZ", hnz_min, hnz_max);
    printTFParams(ss, "B40", b40_min, b40_max);
    printTFParams(ss, "B20", b20_min, b20_max);

    return 0;
}

static void printTFParams(stringstream &ss, string name, double *min, double *max)
{
    char buf[200];
    const char *time_windows[] =
	{"0.10", "0.20", "0.30", "0.40", "0.50", "0.60", "0.70", "0.80", "0.90", "1.00", "2.00"};

    ss << "P: TF: Name: " << name << endl;
    for(int i = 0; i < NUM_FWINDOWS; i++) {
	snprintf(buf, sizeof(buf), "P: TF: %s", time_windows[i]);
	ss << buf;
	for(int j = 0; j < NUM_FBANDS; j++) {
	    snprintf(buf, sizeof(buf), " (%6.3f %6.3f)", min[j*NUM_FWINDOWS+i], max[j*NUM_FWINDOWS+i]);
	    ss << buf;
	}
	ss << endl;
    }
}

static int readTFChannelType(FILE *fp, double *min, double *max)
{
    char line[1024];
    for(int j = 0; j < NUM_FBANDS; j++) {
	if(!getNextLine(fp, line, sizeof(line)-1)) return lineno;
	int i = j*NUM_FWINDOWS;
	if(sscanf(line, "%le %le %le %le %le %le %le %le %le %le %le",
	    &min[i+0], &min[i+1], &min[i+2], &min[i+3], &min[i+4], &min[i+5], &min[i+6], &min[i+7],
	    &min[i+8], &min[i+9], &min[i+10]) != NUM_FWINDOWS) { fclose(fp); return lineno; }
    }
    for(int j = 0; j < NUM_FBANDS; j++) {
	if(!getNextLine(fp, line, sizeof(line)-1)) return lineno;
	int i = j*NUM_FWINDOWS;
	if(sscanf(line, "%le %le %le %le %le %le %le %le %le %le %le",
	    &max[i+0], &max[i+1], &max[i+2], &max[i+3], &max[i+4], &max[i+5], &max[i+6], &max[i+7],
	    &max[i+8], &max[i+9], &max[i+10]) != NUM_FWINDOWS) { fclose(fp); return lineno; }
    }
    return 0;
}

static bool getNextLine(FILE *fp, char *line, int len)
{
    while(fgets(line, len, fp) != NULL) {
	lineno++;
	int i;
	for(i = 0; line[i] != '\0' && isspace(line[i]); i++);
	if(line[i] != '\0' && line[i] != '#') return true;
    }
    return false;
}

#ifdef _NOT_USING_FILE
// HHZ samplerate >= 100
// window lag:  0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1 2 

static double hhz_min[NUM_FBANDS][NUM_FWINDOWS] = {
    {-6.273,-6.644,-6.534,-6.500,-6.074,-6.440,-6.422,-6.419,-6.399,-6.025,-6.008},
    {-6.066,-6.270,-6.172,-6.128,-5.809,-6.071,-6.055,-6.048,-6.032,-5.756,-5.740},
    {-5.877,-5.956,-5.899,-5.833,-5.641,-5.765,-5.756,-5.744,-5.738,-5.622,-5.576},
    {-5.503,-5.491,-5.399,-5.342,-5.257,-5.251,-5.254,-5.233,-5.227,-5.209,-5.106},
    {-5.247,-5.140,-5.025,-4.865,-4.753,-4.618,-4.584,-4.563,-4.561,-4.558,-4.499},
    {-5.090,-4.782,-4.630,-4.571,-4.558,-4.513,-4.394,-4.233,-4.124,-4.103,-4.099},
    {-5.499,-5.185,-4.879,-4.635,-4.505,-4.485,-4.532,-4.538,-4.538,-4.442,-4.083},
    {-5.203,-5.225,-5.119,-4.971,-4.784,-4.656,-4.614,-4.584,-4.607,-4.621,-4.244},
    {-5.254,-5.289,-5.267,-5.245,-5.147,-5.104,-5.046,-4.959,-4.937,-4.917,-4.957},
};
static double hhz_max[NUM_FBANDS][NUM_FWINDOWS] = {
    {-3.669,-3.693,-3.670,-3.660,-3.707,-3.624,-3.623,-3.599,-3.602,-3.675,-3.685},
    {-3.684,-3.741,-3.662,-3.640,-3.678,-3.583,-3.577,-3.556,-3.549,-3.605,-3.609},
    {-3.849,-3.776,-3.516,-3.448,-3.451,-3.313,-3.295,-3.261,-3.254,-3.275,-3.268},
    {-3.771,-3.586,-3.263,-3.001,-2.939,-2.779,-2.716,-2.669,-2.662,-2.645,-2.631},
    {-3.389,-3.075,-2.863,-2.587,-2.401,-2.359,-2.253,-2.022,-1.985,-1.949,-1.922},
    {-3.523,-3.115,-2.777,-2.453,-2.168,-1.981,-1.950,-1.919,-1.883,-1.683,-1.444},
    {-3.684,-3.327,-2.956,-2.624,-2.300,-1.999,-1.800,-1.651,-1.613,-1.616,-1.268},
    {-3.859,-3.751,-3.472,-3.092,-2.758,-2.459,-2.224,-2.004,-1.852,-1.737,-1.668},
    {-3.964,-4.005,-3.905,-3.671,-3.349,-3.007,-2.751,-2.525,-2.351,-2.201,-1.848},
};

// HNZ samplerate >= 100
// window lag: .1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1 2 

static double hnz_min[NUM_FBANDS][NUM_FWINDOWS] = {
	{-5.764,-5.705,-5.659,-5.626,-5.582,-5.527,-5.520,-5.502,-5.500,-5.501,-5.493},
	{-5.898,-5.789,-5.728,-5.678,-5.644,-5.585,-5.577,-5.545,-5.548,-5.541,-5.529},
	{-5.446,-5.262,-5.200,-5.118,-5.018,-4.941,-4.922,-4.919,-4.905,-4.900,-4.850},
	{-4.964,-4.859,-4.752,-4.592,-4.473,-4.401,-4.382,-4.381,-4.352,-4.346,-4.268},
	{-4.162,-3.942,-3.865,-3.770,-3.632,-3.519,-3.444,-3.434,-3.390,-3.364,-3.288},
	{-4.604,-4.039,-3.698,-3.570,-3.595,-3.562,-3.407,-3.283,-3.201,-3.174,-3.138},
	{-5.042,-4.485,-4.022,-3.639,-3.413,-3.352,-3.360,-3.373,-3.346,-3.271,-3.120},
	{-4.908,-4.860,-4.603,-4.155,-3.822,-3.599,-3.465,-3.423,-3.403,-3.407,-3.235},
	{-4.783,-4.760,-4.767,-4.678,-4.467,-4.181,-3.975,-3.862,-3.780,-3.732,-3.689},
};
static double hnz_max[NUM_FBANDS][NUM_FWINDOWS] = {
	{-3.897,-3.903,-3.906,-3.874,-3.864,-3.885,-3.854,-3.886,-3.878,-3.880,-3.867},
	{-3.321,-3.341,-3.363,-3.336,-3.309,-3.333,-3.313,-3.350,-3.330,-3.341,-3.320},
	{-3.382,-3.353,-3.325,-3.279,-3.256,-3.270,-3.257,-3.263,-3.252,-3.263,-3.261},
	{-3.067,-2.843,-2.723,-2.703,-2.608,-2.578,-2.552,-2.542,-2.539,-2.540,-2.555},
	{-3.010,-2.844,-2.601,-2.500,-2.453,-2.310,-2.195,-2.180,-2.181,-2.182,-2.182},
	{-2.393,-2.349,-2.255,-2.079,-1.896,-1.817,-1.825,-1.733,-1.569,-1.490,-1.442},
	{-2.559,-2.396,-2.249,-2.144,-2.037,-1.900,-1.794,-1.752,-1.738,-1.754,-1.340},
	{-2.738,-2.649,-2.500,-2.404,-2.304,-2.198,-2.086,-1.981,-1.898,-1.851,-1.629},
	{-2.593,-2.605,-2.581,-2.585,-2.566,-2.542,-2.471,-2.379,-2.282,-2.208,-2.012},
};

// BHZ samplerate = 40.
// window lag: .1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1 2 

static double b40_min[NUM_FBANDS][NUM_FWINDOWS] = {
    {-6.012,-5.906,-5.804,-5.736,-5.682,-5.654,-5.628,-5.632,-5.642,-5.627,-5.560},
    {-5.882,-5.788,-5.692,-5.629,-5.575,-5.543,-5.521,-5.527,-5.535,-5.521,-5.450},
    {-5.684,-5.602,-5.503,-5.431,-5.373,-5.348,-5.328,-5.330,-5.333,-5.313,-5.215},
    {-5.516,-5.438,-5.336,-5.256,-5.195,-5.154,-5.129,-5.130,-5.131,-5.107,-5.003},
    {-5.189,-5.034,-4.949,-4.794,-4.644,-4.546,-4.472,-4.474,-4.480,-4.477,-4.426},
    {-5.110,-4.753,-4.570,-4.508,-4.505,-4.465,-4.276,-4.153,-4.073,-4.054,-4.056},
    {-5.291,-5.198,-4.948,-4.693,-4.501,-4.404,-4.356,-4.370,-4.396,-4.406,-3.990},
    {-5.147,-5.126,-5.025,-4.873,-4.685,-4.546,-4.455,-4.439,-4.452,-4.462,-4.099},
    {-5.195,-5.198,-5.175,-5.140,-5.081,-4.997,-4.899,-4.836,-4.800,-4.777,-4.776},
};
static double b40_max[NUM_FBANDS][NUM_FWINDOWS] = {
    {-3.984,-3.889,-3.719,-3.653,-3.624,-3.569,-3.513,-3.512,-3.497,-3.503,-3.478},
    {-4.012,-3.892,-3.676,-3.579,-3.544,-3.484,-3.418,-3.410,-3.398,-3.402,-3.374},
    {-3.934,-3.790,-3.514,-3.355,-3.303,-3.215,-3.129,-3.123,-3.114,-3.113,-3.080},
    {-3.798,-3.648,-3.377,-3.143,-3.063,-2.975,-2.846,-2.838,-2.825,-2.823,-2.780},
    {-3.426,-3.175,-2.992,-2.747,-2.560,-2.518,-2.409,-2.204,-2.170,-2.140,-2.086},
    {-3.517,-3.139,-2.852,-2.592,-2.347,-2.185,-2.101,-2.062,-2.032,-1.850,-1.635},
    {-3.883,-3.640,-3.337,-3.026,-2.744,-2.499,-2.280,-2.130,-2.038,-1.982,-1.693},
    {-3.928,-3.801,-3.552,-3.220,-2.925,-2.679,-2.456,-2.289,-2.172,-2.082,-1.967},
    {-4.048,-4.024,-3.953,-3.776,-3.487,-3.212,-2.964,-2.778,-2.637,-2.512,-2.213},
};

// BHZ samplerate = 20.
// window lag: .1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1 2 

static double b20_min[NUM_FBANDS][NUM_FWINDOWS] = {
    {-7.074,-7.074,-7.074,-7.021,-6.942,-6.945,-6.911,-6.938,-6.927,-6.928,-6.860},
    {-6.911,-6.911,-6.911,-6.890,-6.810,-6.813,-6.797,-6.823,-6.814,-6.816,-6.733},
    {-6.702,-6.702,-6.702,-6.702,-6.636,-6.654,-6.665,-6.676,-6.667,-6.657,-6.534},
    {-6.515,-6.515,-6.515,-6.547,-6.497,-6.486,-6.520,-6.530,-6.524,-6.507,-6.379},
    {-6.123,-6.123,-6.123,-6.040,-5.924,-5.772,-5.726,-5.861,-5.880,-5.909,-5.874},
    {-5.597,-5.597,-5.597,-5.665,-5.813,-5.847,-5.603,-5.428,-5.308,-5.401,-5.559},
    {-5.909,-5.909,-5.909,-5.697,-5.560,-5.557,-5.618,-5.739,-5.820,-5.889,-5.415},
    {-5.907,-5.907,-5.907,-5.875,-5.749,-5.675,-5.672,-5.756,-5.831,-5.918,-5.424},
    {-5.918,-5.918,-5.918,-5.962,-6.037,-6.089,-6.073,-6.095,-6.106,-6.160,-6.369},
};
static double b20_max[NUM_FBANDS][NUM_FWINDOWS] = {
    {-2.429,-2.429,-2.429,-2.354,-2.354,-2.271,-2.236,-2.201,-2.197,-2.200,-2.203},
    {-2.444,-2.444,-2.444,-2.307,-2.303,-2.211,-2.154,-2.117,-2.108,-2.114,-2.123},
    {-2.305,-2.305,-2.305,-2.071,-2.036,-1.903,-1.808,-1.784,-1.768,-1.784,-1.802},
    {-2.192,-2.192,-2.192,-1.838,-1.757,-1.634,-1.470,-1.447,-1.419,-1.440,-1.445},
    {-1.821,-1.821,-1.821,-1.491,-1.274,-1.282,-1.169,-0.828,-0.756,-0.723,-0.683},
    {-1.832,-1.832,-1.832,-1.428,-1.036,-0.790,-0.784,-0.791,-0.784,-0.517,-0.181},
    {-2.379,-2.379,-2.379,-2.016,-1.683,-1.337,-1.031,-0.767,-0.601,-0.515,-0.316},
    {-2.676,-2.676,-2.676,-2.208,-1.856,-1.540,-1.250,-0.978,-0.782,-0.642,-0.687},
    {-3.219,-3.219,-3.219,-2.951,-2.529,-2.117,-1.800,-1.525,-1.319,-1.142,-0.675},
};
#endif

void E2TriggerManager::testTeleseismicPgv(E2Trigger *t)
{
    bool tele[NUM_FBANDS];
    double min[NUM_FBANDS*NUM_FWINDOWS], max[NUM_FBANDS*NUM_FWINDOWS];

    if(!useTF) {
	t->tele_fbands = 0;
	return;
    }

    getTeleFBLimits(t, min, max);

    // tfMinWindowIndex = the index in the window arrays, assuming 11 window lengths from EWP2
    // 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 2.0

    // allow t->tele_fbands to change if trigger is not already teleseismic (tele_fbands==1)
    // or tfChannelReverse == true
    if(t->tele_fbands != 1 || tfChannelReverse) {
	t->tele_window = 0;
	FilterWindow *fw = t->filter_bank.fw;
	for(int i = 0; i < NUM_FWINDOWS; i++) if( fw[i].isMeasured() ) {
	    bool is_tele = true;
	    for(int j = 0; j < NUM_FBANDS; j++) {
		tele[j] = (min[j*NUM_FWINDOWS+i] <= fw[i].pgv[j] && fw[i].pgv[j] <= max[j*NUM_FWINDOWS+i]);
		if(!tele[j]) is_tele = false;
	    }
	    // test if above line logPgv[tfLow] = tfM2*logPgv[tfHigh] + tfB2
	    is_tele = (is_tele && fw[i].pgv[tfLow] > tfM2*fw[i].pgv[tfHigh] + tfB2);
	    if(i >= tfMinWindowIndex) {
		t->tele_fbands = is_tele ? 1 : 0;
	    }
	    t->tele_window = i+1;
	}
	if(!fw[tfMinWindowIndex].isMeasured() && fw[tfMinWindowIndex].window_lag > 0.) {
	    t->tele_fbands = 3; // lead is too short to measure
	}
    }
}

void E2TriggerManager::printTeleseismicPgv(stringstream &msg_str, E2Trigger *t)
{
    bool tele[NUM_FBANDS];
    double min[NUM_FBANDS*NUM_FWINDOWS], max[NUM_FBANDS*NUM_FWINDOWS];
    char buf[200];

    getTeleFBLimits(t, min, max);

    FilterWindow *fw = t->filter_bank.fw;
    for(int i = t->fb_window_printed+1; i < NUM_FWINDOWS; i++) if( fw[i].isMeasured() ) {
        bool is_tele = true;
        for(int j = 0; j < NUM_FBANDS; j++) {
            tele[j] = (min[j*NUM_FWINDOWS+i] <= fw[i].pgv[j] && fw[i].pgv[j] <= max[j*NUM_FWINDOWS+i]);
            if(!tele[j]) is_tele = false;
        }
        snprintf(buf, sizeof(buf), "F: %s %2d Z ", t->toShortString().c_str(), t->getUpdate());
        msg_str << endl << buf << fw[i].toString(tele);

        // test if above line logPgv[tfLow] = tfM2*logPgv[tfHigh] + tfB2
        is_tele = (is_tele && fw[i].pgv[tfLow] > tfM2*fw[i].pgv[tfHigh] + tfB2);

        snprintf(buf, sizeof(buf), " %6.3f", fw[i].pgv[tfLow] - (tfM2*fw[i].pgv[tfHigh] + tfB2));
        msg_str << buf << (is_tele ? " T" : " F");
        t->fb_window_printed = i;
    }
}

void E2TriggerManager::getTeleFBLimits(E2Trigger *t, double *min, double *max)
{
    if(t->getSamprate() >= 96) {
        if(t->chanSecondChar() == 'H' || t->chanSecondChar() == 'h') {
            memcpy(min, hhz_min, sizeof(hhz_min));
            memcpy(max, hhz_max, sizeof(hhz_max));
        }
        else if(t->chanSecondChar() == 'N' || t->chanSecondChar() == 'n') {
            memcpy(min, hnz_min, sizeof(hnz_min));
            memcpy(max, hnz_max, sizeof(hnz_max));
        }
        else if(t->chanSecondChar() == 'L' || t->chanSecondChar() == 'l') {
            memcpy(min, hnz_min, sizeof(hnz_min));
            memcpy(max, hnz_max, sizeof(hnz_max));
        }
    }
    else if(t->getSamprate() >= 32) {
        memcpy(min, b40_min, sizeof(b40_min));
        memcpy(max, b40_max, sizeof(b40_max));
    }
    else {
        memcpy(min, b20_min, sizeof(b20_min));
        memcpy(max, b20_max, sizeof(b20_max));
    }
}

void E2TriggerManager::removeOldTriggers(E2EventManager *em) 
{
    TriggerMap::iterator it, it_tmp;
    E2Trigger *p;
    time_t now = reader->currentTime();
    time_t unassoc_time = now - unassoc_trigger_timeout;

    for(it = trigmap.begin(); it != trigmap.end(); )
    {
	// if the trigger is not associated or free_with_unassoc = true
        if( (!(*it).second->isAssociated() || (*it).second->free_with_unassoc)
		&& (*it).second->trigger_time < unassoc_time )
        {
            it_tmp = it;
            it++;
            p = (*it_tmp).second;
            trigmap.erase(it_tmp);
            em->removeTrigger(p);
            delete p;
        }
        else {
            it++;
        }
    }
}

// Cleanup the memory
void E2TriggerManager::cleanup()
{
    for(TriggerMap::iterator it = trigmap.begin();it != trigmap.end(); it++)
    {
	delete (*it).second;
    }
}

void E2TriggerManager::updateStationArray()
{
    pthread_mutex_lock(&stations_lock);
    num_stations = stations.size();
    free(station_array);
    station_array = (Station **)malloc(num_stations*sizeof(Station *));
    int i = 0;
    for(map<int, Station>::iterator it = stations.begin(); it != stations.end(); it++, i++) {
	station_array[i] = &(*it).second;
    }
    pthread_mutex_unlock(&stations_lock);
}

map<int, Station>::iterator E2TriggerManager::addStation(string netsta, TriggerParams *tp)
{
    char line[100];
    map<int, Station>::iterator it;
    int sta_id = staId(netsta);
    if((it = stations.find(sta_id)) == stations.end()) {
	stringstream msg_str;
	Station s(string(tp->net), string(tp->sta), tp->lat, tp->lon);
	s.sta_id = sta_id;
	s.replay_count = replay_mode;
	stations[sta_id] = s;
	snprintf(line, sizeof(line), "M: Adding station: %-4.4s %-6.6s %8.4f %9.4f",
			tp->net, tp->sta, tp->lat, tp->lon);
	msg_str << line;
	LOG_INFO << msg_str.str();
	E2ModuleManager::sendLogMsg(msg_str.str());
	updateStationArray();
	updateClusters(s);
	it = stations.find(sta_id);
    }
    return it;
}

map<int, Station>::iterator E2TriggerManager::addStation(string net, string sta, double lat, double lon)
{
    char line[100];
    map<int, Station>::iterator it;
    int sta_id = staId(net + "." + sta);

    if((it = stations.find(sta_id)) == stations.end()) {
	stringstream msg_str;
	Station s(net, sta, lat, lon);
	s.sta_id = sta_id;
	stations[sta_id] = s;
	snprintf(line, sizeof(line), "M: Adding station: %4s %6s", net.c_str(), sta.c_str());
	msg_str << line;
	LOG_INFO << msg_str.str();
	E2ModuleManager::sendLogMsg(msg_str.str());
	updateStationArray();
	updateClusters(s);
	it = stations.find(sta_id);
    }
    return it;
}

int E2TriggerManager::getClusters()
{
    char prop_name[20];

    for(int i = 1; ; i++) {
	snprintf(prop_name, sizeof(prop_name), "Cluster%d", i);
	string cluster_string = "";
	try {
	    cluster_string = E2Prop::getString(prop_name);
	}
	catch(Error e) {
	    break;
	}
	if(!cluster_string.empty()) {
	    Cluster c(prop_name);
	    char *cs = strdup(cluster_string.c_str());
	    char *last, *s, *tok = cs;
	    if(	(s = strtok_r(tok, ", ", &last)) == NULL  || sscanf(s, "%lf", &c.center_lat) != 1 ||
		(s = strtok_r(NULL, ", ", &last)) == NULL || sscanf(s, "%lf", &c.center_lon) != 1 ||
		(s = strtok_r(NULL, ", ", &last)) == NULL || sscanf(s, "%lf", &c.radiuskm) != 1 ||
		(s = strtok_r(NULL, ", ", &last)) == NULL || sscanf(s, "%lf", &c.min_distkm) != 1)
	    {
		E2ModuleManager::param_str << "M: Invalid cluster: " << cluster_string << endl;
		free(cs);
		break;
	    }
	    while( (s = strtok_r(NULL, ", ", &last)) != NULL ) {
		c.sta_ids.insert(E2TriggerManager::staId(s));
	    }
	    free(cs);
	    pthread_mutex_lock(&stations_lock);
	    clusters[prop_name] = c;
	    pthread_mutex_unlock(&stations_lock);
	    E2ModuleManager::param_str << "P: E2TriggerManager." << prop_name << ": " << cluster_string << endl;
	}
	else {
	    break;
	}
    }

    map<int, Station>::iterator it;
    for(it = stations.begin(); it != stations.end(); it++) {
	updateClusters((*it).second);
    }
    return (int)clusters.size();
}

void E2TriggerManager::updateClusters(Station &s)
{
    char line[100];
    stringstream msg_str;
    map<string, Cluster>::iterator it;

    for(it = clusters.begin(); it != clusters.end(); it++) {
	int netsta = staId(s.net + "." + s.sta);
	if((*it).second.sta_ids.find(netsta) == (*it).second.sta_ids.end()) {
	    double distkm = distancekm((*it).second.center_lat, (*it).second.center_lon, s.lat, s.lon);
	    if(distkm <= (*it).second.radiuskm) {
		pthread_mutex_lock(&stations_lock);
		(*it).second.sta_ids.insert(netsta);
		pthread_mutex_unlock(&stations_lock);
		snprintf(line, sizeof(line), "M: Adding to cluster %s: %4s %6s",
			(*it).second.name.c_str(), s.net.c_str(), s.sta.c_str());
		if(!msg_str.str().empty()) msg_str << endl;
		msg_str << line;
	    }
	}
    }
    if(!msg_str.str().empty()) {
	LOG_INFO << msg_str.str();
	E2ModuleManager::sendLogMsg(msg_str.str());
    }
}

bool E2TriggerManager::isActiveStation(E2Event *event, int sta_id)
{
    map<int, Station>::iterator it;

    if((it = stationsFind(sta_id)) != stationsEnd()) {
	double distkm = distancekm((*it).second.lat, (*it).second.lon, event->lat, event->lon);
	double tt = distkm/pVelocity;
	if(!useGMPeak || (*it).second.last_data_time > event->time + tt - maxStationDelay || (*it).second.replay_count) {
	    return true;
	}
    }
    return false;
}

int E2TriggerManager::staId(string netsta)
{
    map<string, int>::iterator it;

    if((it = netsta_map.find(netsta)) != netsta_map.end()) {
	return (*it).second;
    }
    int id = netsta_map.size() + 1;
    netsta_map[netsta] = id;
    sta_id_map[id] = netsta;
    return id;
}

string E2TriggerManager::netsta(int sta_id)
{
    map<int, string>::iterator it;
    if((it = sta_id_map.find(sta_id)) != sta_id_map.end()) return (*it).second;
    return string("-");
}

int E2TriggerManager::stachanId(string stachan)
{
    map<string, int>::iterator it;

    if((it = stachan_map.find(stachan)) != stachan_map.end()) {
	return (*it).second;
    }
    int id = stachan_map.size() + 1;
    stachan_map[stachan] = id;
    return id;
}

void E2TriggerManager::checkTeleseisms(void)
{
    stringstream msg_str;

    pthread_mutex_lock(&teleseisms_lock);
    // first remove old teleseisms
    if(trigmap.size() > 0 && E2AMQReader::teleseisms.size() > 0) {
	double time = (*trigmap.begin()).second->trigger_time - 300.;
	for(list<Teleseism *>::iterator it = E2AMQReader::teleseisms.begin(); it != E2AMQReader::teleseisms.end(); ) {
	    if((*it)->window_end < time) {
		if(!msg_str.str().empty()) msg_str << endl;
		msg_str << "TE:R: removing: " << (*it)->toShortString();
		delete (*it);
		it = E2AMQReader::teleseisms.erase(it);
	    }
	    else {
		it++;
	    }
	}
    }
    pthread_mutex_unlock(&teleseisms_lock);

    if(!msg_str.str().empty()) {
	LOG_INFO << msg_str.str();
	E2ModuleManager::sendLogMsg(msg_str.str());
    }
	
    pthread_mutex_lock(&teleseisms_lock);
    for(list<Teleseism *>::iterator it = E2AMQReader::teleseisms.begin(); it != E2AMQReader::teleseisms.end(); it++) {
	double wbeg = (*it)->window_start - teleseismic_secs;
	double wend = (*it)->window_end + teleseismic_secs;
	for(TriggerMap::iterator jt = trigmap.begin(); jt != trigmap.end(); jt++) if((*jt).second->teleseismic == 0) {
	    if((*jt).second->trigger_time > wbeg && (*jt).second->trigger_time < wend) {
		(*jt).second->teleseismic = 1;
		double distdeg = getDelta((*it)->lat, (*it)->lon, (*jt).second->getLat(), (*jt).second->getLon());
		map<double, Arrival>::iterator kt = (*it)->arrivals.lower_bound(distdeg);
		if(kt != (*it)->arrivals.end()) {
		    double t1 = (*kt).second.time;
		    double d1 = (*kt).first;
		    double t = t1;
		    kt++;
		    if(kt != (*it)->arrivals.end()) {
			double t2 = (*kt).second.time;
			double d2 = (*kt).first;
			double dd = d2 - d1;
			if(dd) t = t1 + (t2-t1)*(distdeg-d1)/dd;
		    }
		    (*jt).second->minus_tele = (*jt).second->trigger_time - t;
		    if(fabs((*jt).second->minus_tele) < teleseismic_secs) (*jt).second->teleseismic = 2;
		}
	    }
	}
    }
    pthread_mutex_unlock(&teleseisms_lock);
}
