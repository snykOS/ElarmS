#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mcastutil.h"

static char err_string[1024];

SocketMcast::SocketMcast(char *mcastif, char * mcast_addr, short port) throw (SocketMcast::connection_exception)
{
int flags;
int status;
struct sockaddr_in serv_addr;
struct in_addr out_addr;
struct ip_mreq mreq;
int recv_buf_bytes;
	socket_fd=-1;

	// this logic is "borrowed" from mserv (p maechlling and p friberg)

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	out_addr.s_addr = inet_addr(mcastif);
	status = setsockopt(socket_fd,IPPROTO_IP,IP_MULTICAST_IF,
               (char *)&out_addr,sizeof(out_addr));

	if ( status == -1 )
	{
		sprintf(err_string,"Error setting mcast socket option interface with errno : %d\n",errno);
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	}

	/**** MODIFICATION TO ORIGINAL */
	/* allow multiple sockets to use the same PORT number */
	unsigned int yes = 1;
	if (setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
		sprintf(err_string,"Error setting mcast socket option for REUSEADDR failed with errno : %d\n",errno);
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	}
	/*** END OF MODIFICATION TO ORIGINAL */

	memset ((char*) &serv_addr, '\0',sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( (unsigned short)port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(socket_fd, (const sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		sprintf (err_string, "Could not bind socket to local address\n");
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	}
	/* this next bit was added by paulf to increase the recv() buffer to handle a slew of packets */
	/* packets while the program is processing clients */
	recv_buf_bytes = MAX_SIZE_MCAST_PACKET * RECV_BUF_NUM_MCAST_PACKETS;
	status = setsockopt(socket_fd,SOL_SOCKET,SO_RCVBUF, &recv_buf_bytes,sizeof(recv_buf_bytes));
	if ( status == -1 )
	{
		sprintf(err_string,"Attempting to set SO_RCVBUF socket opt errno=%d\n",errno);
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	} 

	/* This will set the multicast address to which we will subscribe. This */
	/* should be in the multicast address range 224.xxxxx to xxxxxx */
	/* Fill in server's socket address structure */

	/* Add multicast interface specification */
	mreq.imr_multiaddr.s_addr=inet_addr(mcast_addr);
	mreq.imr_interface.s_addr=serv_addr.sin_addr.s_addr;
	status = setsockopt(socket_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, (char *)&mreq,sizeof(mreq));
	
	if ( status == -1 )
	{
		sprintf(err_string,"Error setting socketopt for mcast interface errno : %d\n",errno);
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	}
	
	/* This was an important setting. The comserv code uses FNDELAY */
	/* which is phased out but is essentially the same option as O_NONBLOCK. */
	/* The idea here is that mserv will not sit on a socket read. If no data */
	/* is available, it will exit the recev, with an errno, and continue. */
	/* If this option is not set, then the recv will hang, and the comserv */
	/* clients will not receive service in a timely manner. */
	
	flags = fcntl(socket_fd,F_GETFL,0);
	if (flags < 0) 
	{
		sprintf(err_string, "FATAL ERROR: Failure to get flags using fcntl()\n");
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	} 
	flags = flags | O_NONBLOCK;
	flags = fcntl(socket_fd,F_SETFL,flags);
	if (flags == -1) 
	{
		sprintf(err_string, "FATAL ERROR: Failure to set O_NONBLOCK using fcntl()\n");
		close(socket_fd);
		socket_fd=-1;
		throw(connection_exception(err_string));
	} 
}


SocketMcast::~SocketMcast()
{
	if (socket_fd != -1) 
	{
		close(socket_fd);
	}
}

// non-blocking returns NULL if no  packet available from recv()
char * SocketMcast::getDataPacket()
{
int bytes_returned;
	
	memset (mcastbuf, 0, sizeof(mcastbuf));
	bytes_returned = recv(socket_fd, mcastbuf, MAX_SIZE_MCAST_PACKET, 0);
	if (bytes_returned < 0)
	{
		if (errno == EWOULDBLOCK) 
		{
			return NULL;
		}
		else
		{
			// error message later on!
			return NULL;
		}
	}
	if (bytes_returned <= 0) 
	{
		return NULL;
	}
	return  mcastbuf;
}

// returns socket file descriptor for use in select () call.
// if socket is not configured properly it returns -1
int SocketMcast::getSocketFD()
{
	return socket_fd;
}
