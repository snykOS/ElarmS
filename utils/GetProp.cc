#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>     // gethostname for predefined SHOST

#include "RetCodes.h" // TN_ notifications
#include "Configuration.h" 

// logging stuff
#include <plog/Log.h>
#include "ExternalMutexConsoleAppender.h"
#include "SeverityFormatter.h"

#include "GetProp.h"

using namespace std;


const string RCSID_GetProp_cc = "$Id: GetProp.cc $";

// NOTE:  MAXSTR was originally defined in AQMS/GenLimits.h as 256 but if a config
// file contained a long line, this caused a buffer overflow which either resulted
// in the parameter getting lost or corrupted or could cause the program to crash.
// This constant was increased to 1024 but library may still be broken
// Increased to 2048 here in case compiling with an old AQM library.

const int MAXSTR = 2048;    // max size of key, value or line being parsed

GetProp* handle = NULL;
GetProp* GetProp::getInstance() {
    if (handle == NULL) {
        handle = new GetProp();
    }
    return handle;
}

void GetProp::init(std::string config_file) throw(Error)
{
	init(config_file, 0, NULL);
} // GetProp::init
 
void GetProp::init(std::string config_file, int argc, char* argv[]) throw(Error)
{
    // save config_file name
    this->config_file = config_file;

    // set predefined word PROGRAM
    this->pkvm["PROGRAM"] = "unknown";

    // set predefined word SHOST
    string hostname;
    static char buffer[MAXSTR];
    (void)memset((void*)buffer, '\0', sizeof(buffer));
    if (gethostname(buffer, sizeof(buffer) - 1) == 0) {
        char* cp = strchr(buffer, '.');
        if (cp != NULL)
            *cp = '\0';      // truncate at first period
        hostname = buffer;
    } else {
        hostname = "localhost";
    }
    this->pkvm["SHOST"] = hostname;

    // now process config file
    fstream file;
    file.open(config_file.c_str(),fstream::in);
    if(file.fail()) {
        throw Error("Unable to open property file: " + config_file);
    }
    file.close();

    // Populate KeyValue Map
    char key[MAXSTR];
    char value[MAXSTR];

    Configuration cfg(config_file.c_str());

    if (!cfg) {
        throw Error("Invalid config file: " + config_file);
    }

    while(cfg.next(key,value) == TN_SUCCESS)
    {
        if(kvm[key] != "") {
            throw Error(config_file + ": Duplicate parameter found: " + key);
        }

        // ignore comments
        char* comment = strchr(value, '#');
        if (comment != 0) *comment = '\0';

        // check for trailing spaces in case there was a comment
        int space = (int)strlen(value);
        while(space > 0 && isspace((int)value[space-1])) space--;
        if (space != 0) value[space] = '\0';

        kvm[key] = value;
    }

    // update predefined word PROGRAM from argv[0]
    if (argc > 0) {
        string filename = argv[0];
        this->pkvm["PROGRAM"] = filename.substr(filename.find_last_of("/") + 1);
    }


    // now add and override with command line arguments
    for(int i = 1; i < argc; i++) {
        char *c = strstr(argv[i], "=");
        if(c) {
            int n = (int)(c - argv[i]);
            if(n > MAXSTR-1) {
                throw Error("command-line parameter name is too long.");
            }
            strncpy(key, argv[i], n);
            key[n] = '\0';
            if(*(c+1) != '\0') {
                c++;
                n = (int)strlen(argv[i]) - (int)(c - argv[i]);
                if(n > MAXSTR-1) {
                    throw Error("command-line parameter value is too long.");
                }
                strncpy(value, c, n);
                value[n] = '\0';
                kvm[key] = value;
            }
        }
    }
} // GetProp::init
	
 
// collection methods

int GetProp::getPropertyCount() const {
    return kvm.size();
}

std::vector<std::string> GetProp::getConfigListNames()
{
    std::vector<std::string> retList;
    for(std::map<std::string, std::string>::iterator it = kvm.begin(); it != kvm.end(); it++) {
        retList.push_back(it->first);
    }
    return retList;
}

bool GetProp::contains(const string name)
{
    return kvm.find(name) != kvm.end();
}

string GetProp::toString(string prefix)
{
    stringstream s;

    for(std::map<std::string, std::string>::iterator it = kvm.begin(); it != kvm.end(); it++) {
        s << prefix << (*it).first << " " << (*it).second << std::endl;
    }
    return s.str();
}


// get property values


string GetProp::resolveName(const string name)
{
    // first, check for reference to existing property
    if (contains(name))
        return getString(name);

    // check environment
    char* value = getenv(name.c_str());
    if (value != NULL)
        return value;

    // check predefined list
    if (pkvm.find(name) != pkvm.end()) {
        return pkvm[name];
    }

    // otherwise return empty string
    return "";
}

const string REFERENCE_START = "$";
const string REFERENCE_END = "$";

// list of keywords that should be left unchanged such as RCS keywords which are also brackted by dollar signs
std::string word_list[] = {"Id:"};
vector<string> special_words(word_list, word_list + sizeof(word_list)/sizeof(word_list[0]));

string GetProp::getString(const string name) throw(Error)
{
    if (config_file.empty())
        throw Error("GetProp::getString called before init().");

    if (contains(name)) {

        string value = kvm[name];
        std::size_t start;
        while ((start = value.find(REFERENCE_START)) != value.npos) {

            // skip any keywords that should be left as is
            for (std::vector<string>::const_iterator iter = special_words.begin(); iter != special_words.end(); ++iter) {
                if (value.find(*iter, start) != value.npos)
                    return value;
            }

            std::size_t end = value.find(REFERENCE_END, start + 1);
            if (end == value.npos)
                throw Error(config_file + ": Paramaeter " + name + " not $ termindated: " + value);
            string var = value.substr(start + 1, end - start - 1);
            string val = resolveName(var);
            if (val == "")
                throw Error(config_file + ": Paramaeter " + name + " value not resolved: " + var);
            value.replace(start, end - start + 1, val);
        }
        return value;
    } else {
        throw Error(config_file + ": Parameter " + name + " not found.");
    }
}

int GetProp::getInt(const string name) throw(Error)
{
    return toInt(getString(name));
}
double GetProp::getDouble(const string name) throw(Error)
{
    return toDouble(getString(name));
}
bool GetProp::getBool(const string name) throw(Error)
{
    return toBool(getString(name));
}
void GetProp::getDoubleArray(const string name, int num, double *values) throw(Error)
{
    toDoubleArray(getString(name), num, values);
}
template<typename T>
std::vector<T> GetProp::getVector(const std::string name) throw(Error)
{
    return toVector<T>(getString(name));
}
template std::vector<int> GetProp::getVector<int>(const std::string s);
template std::vector<double> GetProp::getVector<double>(const std::string s);
template std::vector<std::string> GetProp::getVector<std::string>(const std::string s);

// utility functions
template<typename T>
std::vector<T> GetProp::toVector(string s) throw(Error) {
    char *tok, *tmp;
    std::vector<T> out;

    tmp = strdup(s.c_str());
    tok = strtok(tmp, ",");
    while (tok != NULL) {
        try {
            std::string strTok(tok);
            fillVector(&out, strTok);
        }
        catch(Error e){
	        free(tmp);
	        throw e;
	    }
        tok = strtok(NULL, ",");
    }
    free(tmp);

    return out;
}
template std::vector<int> GetProp::toVector<int>(string s);
template std::vector<double> GetProp::toVector<double>(string s);
template std::vector<std::string> GetProp::toVector<std::string>(string s);

void GetProp::fillVector(std::vector<int>* in, std::string x) {
    in->push_back(toInt(x));
}
void GetProp::fillVector(std::vector<double>* in, std::string x) {
    in->push_back(toDouble(x));
}
void GetProp::fillVector(std::vector<std::string>* in, std::string x) {
    in->push_back(x);
}

int GetProp::toInt(string s) throw(Error)
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
    throw Error("Invalid int");
}
 
double GetProp::toDouble(string s) throw(Error)
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
    throw Error("Invalid double");
}

void GetProp::toDoubleArray(string s, int num, double *values) throw(Error)
{
    char *c, *tok, *last, *tmp;
    tmp = strdup(s.c_str());
    tok = tmp;
    int i = 0;
    while(i < num && (c = strtok_r(tok, ",", &last)) != NULL) {
	tok = NULL;
        try {
	    values[i] = toDouble(c);
        }
        catch(Error e){
	    free(tmp);
	    throw e;
	}
	i++;
    }
    free(tmp);
    if(i != num) {
	throw Error("Invalid double array");
    }
}

bool GetProp::toBool(string s) throw(Error)
{
    const char *c;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c == '\0') {
        throw Error("Invalid bool");
    }

    if(!strncasecmp(c, "t", n) || !strncasecmp(c, "true", n) ||
            !strncmp(c, "1", n)) {
        return true;
    }
    else if(!strncasecmp(c, "f", n) || !strncasecmp(c, "false", n) ||
            !strncmp(c, "0", n)) {
        return false;
    }
    throw Error("Invalid bool");
}
 

// unit tests

void GetProp::Internal_unit_tests() {
    string filename = "'this file does not exist'";
    GetProp* prop = NULL;

    LOGI << std::endl << "Case 0a: Attemping to use uninitialized prop.";
    try {
        prop = new GetProp();
        prop->getString("blah zip");
        LOGE << "You should NOT see this message!";
        exit(1);
    } catch (Error& e) {
        LOGI << "Caught Error exception. " << e.str() << std::endl
            << "You should have gotten an exception saying property structure not initialized!";
    }

    LOGI << std::endl << "Case 0b: Attemping to open non existent file " << filename;
    try {
        GetProp::getInstance()->init(filename);
        throw Error("TEST OF MISSING FILE EXCEPTION");
        LOGE << "You should NOT see this message!";
        exit(2);
    } catch (Error& e) {
        LOGI << "Caught Error exception: " << e.str() << std::endl
            << "You should have gotten an exception trying to open a non existent file";
    }

    filename = "GetProp.h";
    LOGI << std::endl << "Case 0c: Attemping to open bad file " << filename;
    try {
        GetProp::getInstance()->init(filename);
        throw Error("TEST OF BAD FILE EXCEPTION");
        LOGE << "You should NOT see this message!";
        exit(3);
    } catch (Error& e) {
        LOGI << "Caught Error exception: " << e.str() << std::endl
            << "You should have gotten an exception trying to parse a bad file";
    }

} // internal_unit_tests

void GetProp::File_unit_tests(GetProp* prop) {

    LOGI << std::endl << "Case 1: Walk through list of properties";
    try {
        LOGI << "Number of properties read in: " << prop->getPropertyCount();
        vector<string> list = prop->getConfigListNames();
        for(vector<string>::iterator it = list.begin(); it != list.end(); it++) {
            const char* name = it->c_str();
            std::ostringstream ostrm;
            ostrm << "Trying to get property " << name << "...";
            ostrm << " " << prop->getString(name);
            ostrm << "  asInt: "; try { ostrm << prop->getInt(name); } catch (Error &e) { ostrm << "nan"; }
            ostrm << "  asDouble: "; try { ostrm << prop->getDouble(name); } catch (Error &e) { ostrm << "nan"; }
            ostrm << "  asBool: "; try { ostrm << prop->getBool(name); } catch (Error &e) { ostrm << "n/a"; }
            LOGI << ostrm.str();
        }
        LOGI << "You should see this message if the file is valid";
    } catch (Error& e) {
        LOGI << std::endl << "Caught Error exception: " << e.str() << std::endl
            << "You should see this message if the file is invalid";
    }

} // File_unit_tests


// unit test of GetProp

int GetProp::main(int argc, char* argv[]) {

    int rc = 0;         // return code - assume OK

    // logging stuff
    static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
    static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&print_lock);

    plog::init(plog::verbose, &Severity_appender);

    LOGN << "Program: " << argv[0] << " -- unit test for " << RCSID_GetProp_cc << std::endl;
    LOGN << "Modules:";
    LOGN << "  " << RCSID_Exceptions_h;
    LOGN << "  " << RCSID_Exceptions_cc;
    LOGN << "  " << RCSID_GetProp_h;
    LOGN << "  " << RCSID_GetProp_cc;
    LOGN;

    LOGI << "Begin unit test for GetProp";

    if(argc < 2) {
        LOGI << "Usage: " << argv[0] << " <config file>" << std::endl;

        LOGI << "No file specified";
        try {
            LOGI << "Trying GetProp::Internal_unit_Tests...";
            GetProp::Internal_unit_tests();

        } catch (Error& e) {
            rc++;
            LOGE << "Error running GetProp::Internal_unit_tests. " << e.str();
            return rc;
        }
        LOGI << std::endl << "End unit test for GetProp. returning " << rc;
        exit(rc);
    }

    try {
        GetProp* prop = GetProp::getInstance();
        string filename = argv[1];
        // LOGD << "Attempting to open property file " << filename;
        prop->init(filename, argc, argv);

        LOGI << std::endl << "Trying GetProp::File_unit_tests...";
        GetProp::File_unit_tests(prop);

    } catch (Error& e) {
        rc++;
        LOGE << "Error running GetProp File_unit_tests. " << e.str();
        return rc;
    }

    LOGI << std::endl << "End unit test for GetProp. returning " << rc;
    return rc;
} // GetProp::main

// end of fille: GetProp.cc
