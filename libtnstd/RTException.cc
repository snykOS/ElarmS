#include "RTException.h"

RTException::RTException(){
    msg = "<No Message>";
    errcode = 0;
}

RTException::RTException(string _msg){
    msg = _msg;
}

RTOutOfMemoryException::RTOutOfMemoryException(string msg) : RTException(msg){}

RTIOException::RTIOException(string msg) : RTException(msg){}

RTFileNotFoundException::RTFileNotFoundException(string msg) : RTException(msg){}

RTIllegalArgumentException::RTIllegalArgumentException(string msg) : RTException(msg){}

RTIllegalFileFormatException::RTIllegalFileFormatException(string msg) : RTException(msg){}

RTFileCorruptedError::RTFileCorruptedError(string msg) : RTException(msg){}
