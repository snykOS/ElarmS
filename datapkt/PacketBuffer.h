#ifndef __PacketBuffer_h
#define __PacketBuffer_h

#include "TimeStamp.h"

const int PACKETBUF_SIZE = 4096;

enum PacketType {QMA_PACKET,SEED_PACKET,COMMAND_PACKET,UNKNOWN_PACKET};

class PacketBuffer{
 private:
  char buf[PACKETBUF_SIZE];

 public:
  int size;

  PacketType type;
  TimeStamp arrivaltime;

  PacketBuffer();
  ~PacketBuffer();
  
  PacketBuffer(const PacketBuffer&);
  PacketBuffer& operator=(const PacketBuffer&);
  
  char* get();
};

#endif
