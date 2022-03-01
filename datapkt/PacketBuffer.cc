#include <stdlib.h>
#include <string.h>
#include "PacketBuffer.h"

PacketBuffer::PacketBuffer(){
    memset(buf,0,PACKETBUF_SIZE);
    size = 0;
    type = UNKNOWN_PACKET;
}

PacketBuffer::~PacketBuffer(){
}

char* PacketBuffer::get(){
    return buf;
}

PacketBuffer::PacketBuffer(const PacketBuffer& src){
    memset(buf,0,PACKETBUF_SIZE);
    memcpy(buf,src.buf,PACKETBUF_SIZE);
    size = src.size;
    type = src.type;
    arrivaltime = src.arrivaltime;
}

PacketBuffer& PacketBuffer::operator=(const PacketBuffer& src){
    memset(buf,0,PACKETBUF_SIZE);
    memcpy(buf,src.buf,PACKETBUF_SIZE);
    size = src.size;
    type = src.type;
    arrivaltime = src.arrivaltime;
    return (*this);
}
