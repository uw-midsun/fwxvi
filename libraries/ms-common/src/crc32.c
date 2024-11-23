#include "crc32.h"

// x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
//10000010011100001111000110100111
//0x8270F1A7

#define CRC_polynomial 0x8270F1A7

uint32_t calculate_crc(const uint8_t *data, size_t length){
    uint32_t crc = 0xFFFFFFFF;
    for(size_t i = 0; i < length; i++){
        crc ^= (uint32_t)(data[i]) << 24;
        for(int j = 0; j < 8; j++){
            if(crc & 0x80000000){
                crc = (crc<<1) ^ CRC_polynomial;
            }else{
                crc <<= 1;
            }
        }
    }
    return ~crc;
}