#include "network_buffer.h"

static packet data[MAX_PACKETS];
static NetworkBuffer circular_buffer;
static bool init = false;

returnStatus networkBuffer_init() {
    if (init) 
        return ALREADY_INITALISED;
    
    memset(data, 0, MAX_PACKETS * PACKET_BYTES);
    circular_buffer.data =data;
    circular_buffer.size = MAX_PACKETS;
    circular_buffer.head = 0;
    circular_buffer.insert = 0;
    return SUCCESS;
}

returnStatus network_buffer_insert(packet *data){

}

returnStatus networkBuffer_read(packet *loc){

}