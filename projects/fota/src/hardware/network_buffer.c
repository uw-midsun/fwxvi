#include "network_buffer.h"

static packet data[MAX_PACKETS];
static NetworkBuffer circular_buffer;
static bool init = false;

returnStatus network_buffer_init() {
    if (init) 
        return ALREADY_INITALISED;
    
    memset(data, 0, MAX_PACKETS * PACKET_BYTES);
    circular_buffer.data = data;
    circular_buffer.size = MAX_PACKETS;
    circular_buffer.head = 0;
    circular_buffer.insert = 0;
    return SUCCESS;
}


returnStatus network_buffer_insert(packet *data){
    if (!init)
        return NOT_INITALIZED;
    
    memcpy(data[circular_buffer.insert].internal, data, PACKET_BYTES);

    circular_buffer.insert = (circular_buffer.insert + 1) % circular_buffer.size;

    return SUCCESS;
}

returnStatus network_buffer_read(packet *loc){
    if (!init) 
        return NOT_INITALIZED;

    memcpy(loc, data[circular_buffer.head].internal, PACKET_BYTES);

    circular_buffer.head = (circular_buffer.head + 1) % circular_buffer.size;

    return SUCCESS;
}