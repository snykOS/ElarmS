#ifndef SOCKET_MCAST_H
#define SOCKET_MCAST_H

#define RECV_BUF_NUM_MSEED_PACKETS 100 /* the maximum number of mseed packets to buffer in recv() */

#include <iostream>

class SocketMcast {
private:
	char mseedbuf[1024];
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
	char * getMseedPacket();	// non-blocking returns NULL if no  packet available from recv()
	int getSocketFD();
};
#endif
