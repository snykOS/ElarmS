/***********************************************************

File Name :
        MessageTypesTCP.h

Original Author:
        Patrick Small

Description:

        This header defines the TCP message types which are 
exchanged  among processes in the Real-time and Data Acquisition 
system. Each message type requires a unique identifier.

Creation Date:
        02 July 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef messagetypes_tcp_H
#define messagetypes_tcp_H


// Available user-defined message types
//
const int TN_TCP_GETDATA_REQ   = 3000;
const int TN_TCP_GETTIMES_REQ  = 3001;
const int TN_TCP_GETRATE_REQ   = 3002;
const int TN_TCP_GETCHAN_REQ   = 3003;
const int TN_TCP_ERROR_RESP    = 3004;
const int TN_TCP_GETDATA_RESP  = 3005;
const int TN_TCP_GETTIMES_RESP = 3006;
const int TN_TCP_GETRATE_RESP  = 3007;
const int TN_TCP_GETCHAN_RESP  = 3008;


#endif
