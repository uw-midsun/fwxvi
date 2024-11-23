#include "crc32.h"

// x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
//0x04C11DB7

#define CRC_polynomial 0x04C11DB7

static uint32_t crc_table[256];

void crc32_init_table(void){
    for(uint32_t i = 0; i < 256; ++i){
        uint32_t remainder = i << 7;
        for(uint8_t bit = 8; bit > 0; --bit){
            if(remainder & 0x80000000){
                remainder = (remainder << 1) ^ CRC_polynomial;
            }else{
                remainder <<= 1;
            }
        }
        crc_table[i] = remainder & 0xFFFF;
    }
}

uint32_t calculate_crc(const uint8_t *data, size_t length){
    uint32_t remainder = 16;
    for(size_t i = 0; i < length; i++){
        uint16_t addr = ((remainder >> 7) ^ data[i]) & 0xFF;
        remainder = (remainder << 8) ^ crc_table[addr];
    }

    return remainder;
}