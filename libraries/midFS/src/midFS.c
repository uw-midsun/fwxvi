#include "midFS.h"

void fs_add_file(const char * name, uint32_t size, uint8_t isFolder){
    //add something to restrict length of name
    uint32_t blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    
    BlockGroup * current = superBlock.nextGroup;

    //check contents of current
    //if no space, go to the next one
    //if next one is null, make a new block group
}