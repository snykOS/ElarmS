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
#include "Configuration.h"
#include "GetProp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <iostream>
#include <sstream>
#include <fstream>

#include "GenLimits.h"
#include "RetCodes.h"


GetProp * GetProp::createInstance(string config_file) throw(string)
{
    return createInstance(config_file, 0, NULL);
}

GetProp * GetProp::createInstance(string config_file, int argc, char **argv) throw(string)
{
    GetProp *prop = new GetProp(config_file);
    try {
	prop->init(argc, argv);
    }
    catch(string &e) {
	delete prop;
	throw(e);
    }
    return prop;
}

int GetProp::toInt(string s) throw(string)
{
    char *endptr, last_char;
    const char *c;
    long l;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
	l = strtol(c, &endptr, 10);
	last_char = *endptr;
	if(last_char == c[n]) {
	    return (int)l;
	}
    }
    throw string("Invalid int");
}

double GetProp::toDouble(string s) throw(string)
{
    char *endptr, last_char;
    const char *c;
    double d;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
	d = strtod(c, &endptr);
	last_char = *endptr;
	if(last_char == c[n]) {
	    return d;
	}
    }
    throw string("Invalid double");
}

bool GetProp::toBool(string s) throw(string)
{
    const char *c;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c == '\0') {
        throw string("Invalid bool");
    }

    if(!strncasecmp(c, "t", n) || !strncasecmp(c, "true", n) ||
                !strncmp(c, "1", n)) {
        return true;
    }
    else if(!strncasecmp(c, "f", n) || !strncasecmp(c, "false", n) ||
                !strncmp(c, "0", n)) {
        return false;
    }
    throw string("Invalid bool");
}

void GetProp::getProperty(const char *name, string &value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	throw string(config_file + ": Parameter " + name + " not found.");
    }
    else {
	value = kvm[name];
    }
}

void GetProp::getProperty(const char *name, string &value, string default_value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	value = default_value;
    }
    else {
	value = kvm[name];
    }
}

void GetProp::getProperty(const char *name, int *value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	throw string(config_file + ": Parameter " + name + " not found.");
    }
    else {
	try {
	    *value = toInt(kvm[name]);
	}
	catch(string& e){
	    throw string(config_file + ": "+ name + " is invalid");
	}
    }
}
void GetProp::getProperty(const char *name, int *value, int default_value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	*value = default_value;
    }
    else {
	try {
	    *value = toInt(kvm[name]);
	}
	catch(string& e){
	    throw string(config_file + ": "+ name + " is invalid");
	}
    }
}

void GetProp::getProperty(const char *name, double *value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	throw string(config_file + ": Parameter " + name + " not found.");
    }
    else {
	try {
	    *value = toDouble(kvm[name]);
	}
	catch(string& e){
	    throw string(config_file + ": "+ name + " is invalid");
	}
    }
}
void GetProp::getProperty(const char *name, double *value, double default_value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	*value = default_value;
    }
    else {
	try {
	    *value = toDouble(kvm[name]);
	}
	catch(string& e){
	    throw string(config_file + ": "+ name + " is invalid");
	}
    }
}

void GetProp::getProperty(const char *name, bool *value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	throw string(config_file + ": Parameter " + name + " not found.");
    }
    else {
	try {
	    *value = toBool(kvm[name]);
	}
	catch(string& e){
	    throw string(config_file + ": "+ name + " is invalid");
	}
    }
}
void GetProp::getProperty(const char *name, bool *value, bool default_value) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == "") {
	*value = default_value;
    }
    else {
	try {
	    *value = toBool(kvm[name]);
	}
	catch(string& e){
	    throw string(config_file + ": "+ name + " is invalid");
	}
    }
}

void GetProp::getProperty(const char *name, string &dest_addr,
				int *dest_port) throw(string)
{
    if(config_file.empty()) {
	throw string("GetProp::getProperty called before init().");
    }
    if(kvm[name] == ""){
	throw string(config_file + ": Parameter " + name + " not found.");
    }
    else {
	string ip_port = kvm[name];
 	const char *c, *ips, *port_str;
	char *endptr;

	ips = ip_port.c_str();

	if( (c = strstr(ips, ":")) ) {
	    port_str = c+1;
	}
	else {
	    port_str = ips;
	}
	if(c && c != ips) {
	    dest_addr.assign(ips, c-ips);
	}
	else {
	    dest_addr.assign("localhost");
	}
	*dest_port = strtol(port_str, &endptr, 10);
	if(endptr - port_str != (int)strlen(port_str))  {
	    throw string(config_file + ": " + name +
			" is invalid. Enter 'ip-address:port'");
	}
	if( dest_addr.empty() ) {
	    throw string(config_file + ": port component of " + name
			+ " not specified.");
	}
    }
}

void GetProp::init(int argc, char **argv) throw(string)
{
    fstream file;

    file.open(config_file.c_str(),fstream::in);
    if(file.fail()) {
        throw string("Unable to open " + config_file);
    }
    file.close();
    
    // Populate KeyValue Map
    char key[MAXSTR];
    char value[MAXSTR];
    
    Configuration cfg(config_file.c_str());
   
    while(cfg.next(key,value) == TN_SUCCESS)
    {
        if(kvm[key] != "") {
            throw string(config_file + ": Duplicate parameter found: " + key);
        }
        kvm[key] = value;
    }
    // add and override with command line arguments
    for(int i = 1; i < argc; i++) {
	char *c = strstr(argv[i], "=");
	if(c) {
	    int n = (int)(c - argv[i]);
	    if(n > MAXSTR-1) {
		throw string("command-line parameter name is too long.");
	    }
	    strncpy(key, argv[i], n);
	    key[n] = '\0';
	    if(*(c+1) != '\0') {
		c++;
		n = (int)strlen(argv[i]) - (int)(c - argv[i]);
		if(n > MAXSTR-1) {
		    throw string("command-line parameter value is too long.");
		}
		strncpy(value, c, n);
		value[n] = '\0';
		kvm[key] = value;
	    }
	}
    }
}

bool GetProp::getarg(int argc, char **argv, string name, string &value)
{
    bool found = false;
    string s = name + "=";
    int len = s.length(), j=0;

    for(int i = 1; i < argc; i++) {
	if(!strncmp(argv[i], s.c_str(), len)) {
	    if( found ) {
		j = i;
	    }
            found = true;
            value.assign(argv[i]+len);
        }
    }
    if(j > 0) {
	cerr << "warning: multiple command line " << name
		<< " arguments. Using " << argv[j] << endl;
    }
    return found;
}

string GetProp::toString()
{
    stringstream s;

    for(KeyValueMap::iterator it = kvm.begin(); it != kvm.end(); it++) {
	s << (*it).first << " " << (*it).second << endl;
    }
    return s.str();
}
