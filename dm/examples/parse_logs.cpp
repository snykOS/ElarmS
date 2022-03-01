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
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "DMMessageDecoder.h"
#include "qlib2.h"

class Event
{
  public:
    EventMessage *first;
    EventMessage *last;
    Event(EventMessage *f, EventMessage *l) : first(f), last(l) {}
};

void parseLogs(int argc, char *argv[]);

int main(int argc , char *argv[])
{
    if(argc < 3 || strncmp(argv[1], "sys=", 4)) {
	cerr <<  "usage: parse_logs sys=(elarms,vs,onsite,or dm) LOG_DIRECTORY_OR_FILE [...]" << endl;
	exit(0);
    }
    parseLogs(argc, argv);
}

static bool sort_logs(string a, string b)
{
    return (strcmp(a.c_str(), b.c_str()) < 0);
}

static bool sort_by_log_time(Event a, Event b)
{
    return (a.first->getLogTime() < b.first->getLogTime());
}

static bool sort_by_id(CoreEventInfo *a, CoreEventInfo *b)
{
    // Some vs ids are reused in the old logfiles. For example:
    // [elarms@eew2 logs]$ grep -l "vs id: 10008263" test_alg_*
    // test_alg_receiver_20110401.log
    // test_alg_receiver_20110507.log
    // test_alg_receiver_20110614.log
    // need to sort by id, logtime, version
    // logtime: 2011-04-01 19:22:18:380

    double a_logtime, b_logtime;

    if(a->getID() != b->getID()) return (a->getID() < b->getID());

    a_logtime = a->getDepthUncertainty();

    b_logtime = b->getDepthUncertainty();
    
    // ids are not reused within a 10 minute period
    if(b_logtime - a_logtime > 10.*60.) return true;
    if(a_logtime - b_logtime > 10.*60.) return false;

    return (a->getVersion() < b->getVersion());
}

void parseLogs(int argc, char *argv[])
{
    int i, j, filename_len, prefix_len, len;
    char *log_dir, c[50];
    INT_TIME *nt;
    double logtime;
    FILE *fp;
    DIR *dirp;
    struct dirent *dp;
    vector<string> logs;
    vector<CoreEventInfo *> messages;
    vector<Event> events;
    ostringstream os;
    string path, sys, prefix;
    DMMessageDecoder decoder;

    sys.assign(argv[1]+4);

    if(sys == "dm") {
	prefix.assign("test_receiver_");
	filename_len = (int)strlen("test_receiver_20110822.log");
    }
    else if(sys == "elarms" || sys == "vs" || sys == "onsite") {
	prefix.assign("test_alg_receiver_");
	filename_len = (int)strlen("test_alg_receiver_20110822.log");
    }
    else {
	cerr << "invalid sys argument:" << endl;
	cerr << "usage: parse_logs sys=(elarms,vs,onsite,or dm) LOG_DIRECTORY_OR_FILE [more]" << endl;
	return;
    }
    prefix_len = prefix.length();

    for(int k = 2; k < argc; k++) {
	log_dir = argv[k];
	if( (dirp = opendir(log_dir)) ) { // directory
	    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) if(dp->d_name[0] != '.')
	    {
		// test_receiver_20110822.log
		len = (int)strlen(dp->d_name);
		if(!strncmp(dp->d_name, prefix.c_str(), prefix.length())
		    && len == filename_len
		    && !strcmp(dp->d_name+len-4, ".log"))
		{
		    logs.push_back(string(log_dir) + "/" + string(dp->d_name));
		}
	    }
	    closedir(dirp);
	}
	else {
	    string s(log_dir);
	    if( (i = (int)s.find_last_of('/')) != string::npos && s.length()-i-1 != filename_len) {
		cerr << "Invalid file name. Must be like: " << prefix << "20110822.log" << endl;
	    }
	    else {
		logs.push_back(s);
	    }
	}
    }

    sort(logs.begin(), logs.end(), sort_logs);

    for(vector<string>::iterator s = logs.begin(); s != logs.end(); s++) {
	path = *s;
	string date;
	if( (i = (int)path.find_last_of('/')) != string::npos && path.length()-i-1 == filename_len) {
	    date = path.substr(i+prefix_len+1, 4);
	    date.append("-");
	    date.append(path.substr(i+prefix_len+5, 2));
	    date.append("-");
	    date.append(path.substr(i+prefix_len+7, 2));
	}
	else {
	    date = path.substr(prefix_len, 4);
	    date.append("-");
	    date.append(path.substr(prefix_len+4, 2));
	    date.append("-");
	    date.append(path.substr(prefix_len+6, 2));
	}

	if( (fp = fopen(path.c_str(), "r")) ) {
	    int m = (int)messages.size();
	    int n = decoder.decodeAllLogMessages(fp, messages);
	    cout << path << " " << n << " event messages" << endl;
	    for(int i = m; i < (int)messages.size(); i++) {
		// insert date string before time string
		messages[i]->setLogTime(date + " " + messages[i]->getLogTime());
	    }
	    fclose(fp);
	}
	else {
	    cerr << "Cannot open " << path.c_str() << endl;
	    cerr << strerror(errno) << endl;
	}
    }

    // keep only messages with system == sys
    for(i = 0; i < (int)messages.size() && messages[i]->getSystemNameString() == sys; i++);

    int n = 0;
    if(i < (int)messages.size()) {
	vector<CoreEventInfo *> v(messages);
	messages.clear();
	for(i = 0; i < (int)v.size(); i++) {
	    if(v[i]->getSystemNameString() == sys) messages.push_back(v[i]);
	}
	n = (int)(v.size() - messages.size());
    }
    cout << endl << "Total " << sys << " messages read: " << messages.size() << endl;

    // use the depth_uncertainty member to store true-epoch logtime
    for(i = 0; i < (int)messages.size(); i++) {
	strncpy(c, messages[i]->getLogTime().c_str(), sizeof(c)-1);
	c[sizeof(c)-1] = '\0';
        // logtime: 2011-04-01 19:22:18:380
	// convert to 2011/04/01/19:22:18.380 for qlib2
	c[4] = '/';
	c[7] = '/';
	c[10] = '/';
	c[19] = '.';
	if( !(nt = parse_date(c)) ) {
	    cerr << "Cannot parse logtime: " << messages[i]->getLogTime() << endl;
	    exit(1);
	}
	logtime = int_to_tepoch(*nt);
	messages[i]->setDepthUncertainty(logtime);
    }

    // sort by id,version and remove duplicate id.version messages. Get duplicates from the old
    // files that have an xml message followed by a core value printout. The oldest log files have
    // only the core printout. The newer files have only the xml message.
    sort(messages.begin(), messages.end(), sort_by_id);

    n = 0;
    for(i = 1; i < (int)messages.size(); i++) {
	if(messages[i]->getID() == messages[i-1]->getID() &&
	    messages[i]->getVersion() == messages[i-1]->getVersion()) n++;
    }
    if(n > 0) {
	vector<CoreEventInfo *> v(messages);
	messages.clear();
	messages.push_back(v[0]);
	for(i = 1; i < (int)v.size(); i++) {
	    if(v[i]->getID() != v[i-1]->getID() ||
		v[i]->getVersion() != v[i-1]->getVersion()) messages.push_back(v[i]);
else cout << "Duplicate: id: " << v[i]->getID() << " version: " << v[i]->getVersion() << endl;
	}
	n = (int)(v.size() - messages.size());
    }
    cout << "Duplicate messages:  " << n << endl;
    cout << "Total valid messages: " << messages.size() << endl;

    // print all messages in id order
    if(1) {
	stringstream os;
	os << sys << ".messages";
	FILE *fp = fopen(os.str().c_str(), "w");
	fprintf(fp, "%7.7s ID  ver              Report Time               Origin Time      Mag      lat       lon     depth    lklh   rlag\n",
	    sys.c_str());
	int nrlag = 0;
	double rlag, aver_rlag = 0.;
	for(i = 0; i < (int)messages.size(); i++) {
	    CoreEventInfo *cei = messages[i];
	    INT_TIME it = tepoch_to_int(cei->getOriginTime());
	    rlag = cei->getDepthUncertainty() - cei->getOriginTime();
	    fprintf(fp, "%10d %4d  %s  %s  %7.4f  %7.4f %9.4f  %8.4f  %6.2f %6.2f\n",
		cei->getID(), cei->getVersion(), cei->getLogTime().c_str(), time_to_str(it, MONTHS_FMT), cei->getMagnitude(),
		cei->getLatitude(), cei->getLongitude(), cei->getDepth(), cei->getLikelyhood(), rlag);
	    if(cei->getVersion() == 0 && rlag < 60.) { // a few bad datum
		aver_rlag += rlag;
		nrlag++;
	    }
	}
	if(nrlag) aver_rlag /= nrlag;
	fprintf(fp, "average version 0 report time - origin time: %.2f\n", aver_rlag);
	fclose(fp);
    }

    // keep the first and last version for each event id
    for(i = 0; i < (int)messages.size(); i = j) {
	for(j = i+1; j < (int)messages.size(); j++) {
	    if(messages[j]->getID() != messages[i]->getID()) break;
	    // using depth_uncertainty to store epoch logtime.
	    // Don't associate messages if logtime difference > 10 min.
	    if(fabs(messages[j]->getDepthUncertainty()
		  - messages[i]->getDepthUncertainty()) > 10.*60) break;
	}
	events.push_back(Event((EventMessage *)messages[i], (EventMessage *)messages[j-1]));
    }
    cout << "Total events: " << events.size() << endl;

    sort(events.begin(), events.end(), sort_by_log_time);

    printf("%7.7s ID              Report Time               Origin Time  Time-n   Mag-0   Mag-n    lat-0     lon-0  depth-0    lat-n     lon-n  depth-n  lklh-0 lklh-n\n",
	    sys.c_str());

    for(i = 0; i < (int)events.size(); i++) {

	EventMessage *first = events[i].first;
	EventMessage *last = events[i].last;

	INT_TIME it_first = tepoch_to_int(first->getOriginTime());
	double dif = last->getOriginTime() - first->getOriginTime();

	printf("%10d  %s  %s  %6.2f %7.4f %7.4f  %7.4f %9.4f %8.4f  %7.4f %9.4f %8.4f  %6.2f %6.2f",
		first->getID(), first->getLogTime().c_str(),
		time_to_str(it_first, MONTHS_FMT), dif,
		first->getMagnitude(), last->getMagnitude(), 
		first->getLatitude(), first->getLongitude(), first->getDepth(),
		last->getLatitude(), last->getLongitude(), last->getDepth(),
		first->getLikelyhood(), last->getLikelyhood());
/**/
	if(last->getSystemName() == DM) {
	    for(int k = 0; k < (int)last->contributors.size(); k++) {
		if(k == 0) printf(" %s %d", eewSystemNameString[last->contributors[k].system_name].c_str(), last->contributors[k].event_id);
		else printf(",%s %d", eewSystemNameString[last->contributors[k].system_name].c_str(), last->contributors[k].event_id);
	    }
	}
/**/
	printf("\n");
    }
}
