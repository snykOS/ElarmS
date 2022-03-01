#include "Exceptions.h"

#include <iostream>         // std::cout

// logging stuff
#include <plog/Log.h>
#include "ExternalMutexConsoleAppender.h"
#include "SeverityFormatter.h"

using namespace std;

const string RCSID_Exceptions_cc = "$Id: Exceptions.cc $";

Error::Error(){}

Error::Error(const char* str){
    addMessage(str);
}

Error::Error(const std::string& str){
    addMessage(str.c_str());
}

Error::~Error(){
    msgstack.clear();
}

void Error::addMessage(const char* msg){
    if(!msg) return;
    msgstack.push_back(string(msg));
}

std::string Error::str(void){
    ostringstream ostrm; 
    int line = 0;
    for(StrList::iterator it=msgstack.begin(); it!=msgstack.end(); it++){
        for(int i = 0; i < line; i++){
            ostrm << "\t ";
        }
        line++;
        ostrm << (*it).c_str();
    }
    return ostrm.str();
}

std::string Error::printStr(void) { return str(); }

void Error::print(void){
    LOGE << str();
}

LogicalError::LogicalError(){}
LogicalError::LogicalError(const char* str) : Error(str){}

CriticalError::CriticalError(){}
CriticalError::CriticalError(const char* str) : Error(str){}

OutofMemoryError::OutofMemoryError(){}

DatabaseError::DatabaseError(): LogicalError(){}
DatabaseError::DatabaseError(const char* str): LogicalError(str){}

FileFormatError::FileFormatError(){}
FileFormatError::FileFormatError(const char* str): LogicalError(str){}

FileIOError::FileIOError(){}
FileIOError::FileIOError(const char* str): LogicalError(str){}


// unit test of Exceptions

int Error::main(int argc, char* argv[]) {

    int rc = 0;         // return code - assume OK

    // logging stuff
    static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
    static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&print_lock);

    plog::init(plog::verbose, &Severity_appender);

    LOGN << "Program: " << argv[0] << " -- unit test for " << RCSID_Exceptions_cc << std::endl;
    LOGN << "Modules:";
    LOGN << "  " << RCSID_Exceptions_h;
    LOGN << "  " << RCSID_Exceptions_cc;
    LOGN;

    LOGI << "Begin unit test for Exceptions";

    if(argc < 2) {
        LOGN << "Usage: " << argv[0] << " [flag] ";
        LOGN << "where each extra argument will cause a throw to be skipped to test error reporting";
        LOGN;
    }
    int skip = argc;    // number of throws to skip
    
    LOGI << std::endl << "Starting simple exception test";
    try {
        LOGI << "About to throw Error...";
        if (! (--skip > 0))
            throw Error("TEST OF ERROR EXCEPTION");
        rc++;
        LOGE << std::endl << "You should not see this message" << std::endl;
    } catch (Error& e) {
        LOGI << "Caught Error exception: " << e.str();
        LOGI << "^^^^ You should have seen the text 'TEST OF ERROR EXCEPTION'";
    }

    LOGI << std::endl << "Starting exception test with multiple messages";
    try {
        LOGI << "About to throw Error...";
        Error e;
        e.addMessage("THIS IS THE FIRST MESSAGE");
        e.addMessage("THIS IS A SECOND MESSAGE");
        e.addMessage("THIS IS A THIRD MESSAGE");
        if (!(--skip > 0))
            throw e;
        rc++;
        LOGE << std::endl << "You should not see this message" << std::endl;
    } catch (Error& e) {
        LOGI << "Caught Error exception: " << e.str();
        LOGI << "^^^^ You should have seen three indented messages";
    }

    LOGI << std::endl << "End unit test for Exceptions. returning " << rc;
    return rc;
} // Error::main

// end of file: Exceptions.cc
