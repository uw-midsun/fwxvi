#include "crc32.h"

// x^16 + x^12 + x^5 + 1
//0x1021
#define CRC_polynomial 0x1021

static uint16_t crc_table[256];

void crc16_init_table(void){
    for(uint16_t i = 0; i < 256; ++i){
        uint16_t remainder = i << 7;
        for(uint8_t bit = 8; bit > 0; --bit){
            if(remainder & 0x8000){
                remainder = (remainder << 1) ^ CRC_polynomial;
            }else{
                remainder <<= 1;
            }
        }
        crc_table[i] = remainder & 0xFFFF;
    }
}

uint16_t calculate_crc16(const uint8_t *data, size_t length){
    uint16_t remainder = 16; //test this
    for(size_t i = 0; i < length; i++){
        uint16_t addr = ((remainder >> 7) ^ data[i]) & 0xFF;
        remainder = (remainder << 8) ^ crc_table[addr];
    }

    return remainder;
}