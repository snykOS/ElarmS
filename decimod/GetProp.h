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
#ifndef _GetProp_h__ 
#define _GetProp_h__ 

#include <string>
#include <map>

using namespace std;

typedef map<string,string> KeyValueMap;

class GetProp
{
 private:
    string config_file;
    KeyValueMap kvm;
    GetProp(string file) { config_file = file; }
    void init(int argc, char **argv) throw(string);
    static int toInt(string) throw(string);
    static double toDouble(string) throw(string);
    static bool toBool(string) throw(string);

 public:
    static GetProp *createInstance(string config_file) throw(string);
    static GetProp *createInstance(string config_file, int argc, char **argv) throw(string);
    bool getarg(int argc, char **argv, string name, string &value);

    void getProperty(const char *name, string &value) throw(string);
    void getProperty(const char *name, int *value) throw(string);
    void getProperty(const char *name, double *value) throw(string);
    void getProperty(const char *name, bool *value) throw(string);

    void getProperty(const char *name, string &value, string default_value) throw(string);
    void getProperty(const char *name, int *value, int default_value) throw(string);
    void getProperty(const char *name, double *value, double default_value) throw(string);
    void getProperty(const char *name, bool *value, bool default_value) throw(string);
    void getProperty(const char *name, string &dest_addr, int *dest_port) throw(string);

/*
    void getProperty(string name, string &value) throw(string) {
	getProperty(name.c_str(), value);
    }
*/

    string getString(const char *name) throw(string) {
	string s;
	getProperty(name, s);
	return s;
    }
    string getString(const char *name, string default_value) throw(string) {
	string s;
	getProperty(name, s, default_value);
	return s;
    }
    int getInt(const char *name) throw(string) {
	int value;
	getProperty(name, &value);
	return value;
    }
    int getInt(const char *name, int default_value) throw(string) {
	int value;
	getProperty(name, &value, default_value);
	return value;
    }
    double getDouble(const char *name) throw(string) {
	double value;
	getProperty(name, &value);
	return value;
    }
    double getDouble(const char *name, double default_value) throw(string) {
	double value;
	getProperty(name, &value, default_value);
	return value;
    }
    bool getBool(const char *name) throw(string) {
	bool value;
	getProperty(name, &value);
	return value;
    }
    bool getBool(const char *name, bool default_value) throw(string) {
	bool value;
	getProperty(name, &value, default_value);
	return value;
    }

    string toString();

    KeyValueMap keyValueMap() { return kvm; }
};

#endif
