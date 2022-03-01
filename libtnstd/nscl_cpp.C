#include <cstring>
#include "nscl.h"


char* mapLC(nscl* s,const int d){
    return mapLC(s->network,s->location,d);
}


