#ifndef _INETHEADER_H
#define _INETHEADER_H

#define ETHER_ADDR_LEN	6

struct ethernethdr{
  u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
  u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
  u_short ether_type; /* IP? ARP? RARP? etc */
};

/*
ip_hl: the ip header length in 32bit octets. this means a value of 5
 for the hl means 20 bytes (5 * 4). values other than 5 only need to
 be set it the ip header contains options (mostly used for routing)

ip_v: the ip version is always 4 (maybe I'll write a IPv6 tutorial later;)

ip_tos: type of service controls the priority of the packet. 0x00 is
 normal. the first 3 bits stand for routing priority, the next 4 bits
 for the type of service (delay, throughput, reliability and cost).

ip_len: total length must contain the total length of the ip datagram.
 this includes ip header, icmp or tcp or udp header and payload size in bytes.

ip_id: the id sequence number is mainly used for reassembly of fragmented IP
 datagrams. when sending single datagrams, each can have an arbitrary ID.

ip_off: the fragment offset is used for reassembly of fragmented datagrams.
 the first 3 bits are the fragment flags, the first one always 0, the second
 the do-not-fragment bit (set by ip_off |= 0x4000) and the third the more-flag
 or more-fragments-following bit (ip_off |= 0x2000). the following 13 bits is
 the fragment offset, containing the number of 8-byte big packets already sent.

ip_ttl: time to live is the amount of hops (routers to pass) before the packet
 is discarded, and an icmp error message is returned. the maximum is 255.

ip_p: the transport layer protocol. can be tcp (6), udp(17), icmp(1), or
 whatever protocol follows the ip header. look in /etc/protocols for more.

ip_sum: the datagram checksum for the whole ip datagram. every time anything
 in the datagram changes, it needs to be recalculated, or the packet will
 be discarded by the next router. see V. for a checksum function.

ip_src and ip_dst: source and destination IP address, converted to long
 format, e.g. by inet_addr(). both can be chosen arbitrarily.
*/

#define IP_P_UDP 17

struct iphdr {
 unsigned char ip_hl:4, ip_v:4; /* this means that each member is 4 bits */
 unsigned char ip_tos;
 unsigned short int ip_len;
 unsigned short int ip_id;
 unsigned short int ip_off;
 unsigned char ip_ttl;
 unsigned char ip_p;
 unsigned short int ip_sum;
 unsigned int ip_src;
 unsigned int ip_dst;
}; /* total ip header length: 20 bytes (=160 bits) */


struct icmphdr {
 unsigned char icmp_type;
 unsigned char icmp_code;
 unsigned short int icmp_cksum;
 /* The following data structures are ICMP type specific */
 unsigned short int icmp_id;
 unsigned short int icmp_seq;
}; /* total icmp header length: 8 bytes (=64 bits) */

struct udphdr {
 unsigned short int uh_sport;
 unsigned short int uh_dport;
 unsigned short int uh_len;
 unsigned short int uh_check;
}; /* total udp header length: 8 bytes (=64 bits) */

#endif
