#include "bootstrap.h"

void jump_to(uint32_t addr){
    uint32_t sp = *((volatile uint32_t *)addr);
    uint32_t reset_vector = *((volatile uint32_t *)(addr + 4));

    __disable_irq();
    __set_MSP(sp);
    ((EntryPoint)reset_vector)();
}

void compute_crc32(const uint8_t *data, size_t length, uint32_t *crc_dest){

}

void read_crc32(const char *file_path, size_t length, uint32_t *crc_dest){
}