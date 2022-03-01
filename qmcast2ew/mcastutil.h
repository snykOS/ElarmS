#ifndef SOCKET_MCAST_H
#define SOCKET_MCAST_H

#include <iostream>
#include "onesec_pkt.h"

#define RECV_BUF_NUM_MCAST_PACKETS 100 /* the maximum number of mcast packets to buffer in recv() */
#define	MAX_SIZE_MCAST_PACKET	sizeof(struct onesec_pkt)


class SocketMcast {
private:
	char mcastbuf[MAX_SIZE_MCAST_PACKET];
	int socket_fd;

public:
        class connection_exception {
                private:
                        const char * err_string;
                public:
                        connection_exception(const char  *str) {
                                err_string = str;
                        };
                        ~connection_exception() {};
                        void display() {
                                std::cerr << err_string << std::endl;
                        };
        };

	SocketMcast(char *address, char *mcast_addr, short port) throw (connection_exception);
	~SocketMcast();
	char * getDataPacket();	// non-blocking returns NULL if no  packet available from recv()
	int getSocketFD();
};
#endif
