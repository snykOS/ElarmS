#include <string.h>
#include "nscl.h"


char* mapLC(char* network,char* lc,const int d){
    if(!lc || !network)
	return NULL;
    
    if(strlen(lc)==0 || strcmp(lc," ")==0 || strcmp(lc,"  ")==0 || strcmp(lc,"--")==0){
	//	if(strcmp(network,"BK")==0){
	    if(d == BINARY)
		strcpy(lc,"  ");
	    if(d == MEMORY)     
		strcpy(lc,"--");
	    if(d == ASCII)
		strcpy(lc,"--");
	    if(d == FILENAME)
		strcpy(lc,"");
	    if(d == DATABASE)
		strcpy(lc,"  ");
/* 	} */
/* 	else{ */
/* 	    if(d == BINARY) */
/* 		strcpy(lc,"01"); */
/* 	    if(d == MEMORY)      */
/* 		strcpy(lc,"01"); */
/* 	    if(d == ASCII) */
/* 		strcpy(lc,"01"); */
/* 	    if(d == FILENAME) */
/* 		strcpy(lc,"01"); */
/* 	    if(d == DATABASE) */
/* 		strcpy(lc,"01"); */
/* 	} */
    }
    return lc;
}
