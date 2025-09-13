/************************************************************************************************
 * @file   bootstrap.c
 *
 * @brief  Source file for bootstrap application
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

#include "bootstrap.h"
uint32_t crc32_table[256];

void jump_to(uint32_t addr){
    uint32_t sp = *((volatile uint32_t *)addr);
    uint32_t reset_vector = *((volatile uint32_t *)(addr + 4));

    __disable_irq();
    __set_MSP(sp);
    ((EntryPoint)reset_vector)();
}

void crc32_init(void){
    for(uint32_t i = 0; i < 256; i++){
        uint32_t crc = i;
        for(int j = 0; j < 8; j++){
            if(crc & 1){
                crc = (crc >> 1) ^ CRC32_POLY;
            }else{
                crc >>= 1;
            }
            crc32_table[i] = crc;
        }
    }
}

void compute_crc32(const uint8_t *data, size_t length, uint32_t *crc_dest){
    uint32_t crc = 0xFFFFFFFF;

    for(size_t i = 0; i < length; i++){
        uint8_t byte = data[i];
        crc = (crc>>8) ^ crc32_table[(crc ^ byte) & 0xFF];
    }

    *crc_dest = ~crc;
}

FsStatus read_crc32(const char *file_path, size_t length, uint32_t *crc_dest){
    return fs_read_file(file_path, crc_dest);
}

void bootstrap_main(void){
    printf("Initializing file system\n\r");
    fs_init();
    fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, 0);

    printf("Computing CRC\n\r");
    uint32_t computed_crc = 0;
    crc32_init();
    compute_crc32((uint8_t*)BOOTLOADER_ADDR, BOOTLOADER_SIZE, &computed_crc);
    
    printf("reading stored CRC\n\r");
    uint32_t stored_crc = 0;
    read_crc32(CRC_FILE_PATH, CRC_SIZE, &stored_crc);

    if(computed_crc == stored_crc){
        printf("CRC match. Jumping to bootloader\n\r");
        jump_to(BOOTLOADER_ADDR);
    }else{
        printf("CRC does not match. Jumping to application\n\r");
        jump_to(APPLICATION_ADDR);
    }

}
