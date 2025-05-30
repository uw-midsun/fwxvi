#include "midFS.h"


void fs_add_file(const char * name, uint8_t* content, uint32_t size, uint8_t isFolder){
    //add something to restrict length of name
    if(strlen(name) > MAX_FILENAME_LENGTH) return;

    //determine how many blocks we need
    uint32_t blocksNeeded = (FILE_ENTRY_SIZE + size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    
    //first block
    BlockGroup current;
    uint32_t currentBlockGroupIndex = superBlock.nextBlockGroup;
    
    uint32_t incomingBlockAddress = UINT32_MAX; //invalid marker
    uint32_t space = 0;
    
    while(1){
        fs_read_block_group(currentBlockGroupIndex, &current);
        for(uint32_t i = 0; i < BLOCKS_PER_GROUP; i++){
            if(current.blockBitmap[i] == 0){
                space++;
                while(i + space < BLOCKS_PER_GROUP && current.blockBitmap[i + space] == 0){
                    space++;
                    if(space == blocksNeeded){
                        incomingBlockAddress = i;
                        break;
                    }
                }
                i += space;
            }
            space = 0;
            if(incomingBlockAddress != UINT32_MAX){
                break;
            }
        }
        //if there is no space and we havent found free space, go to the next block

        if(incomingBlockAddress != UINT32_MAX) break; //we found an address, break

        if(current.nextBlockGroup == FS_NULL_BLOCK_GROUP){ //next block does not exist
            uint32_t newBlockGroup = fs_create_block_group();
            current.nextBlockGroup = newBlockGroup;
            fs_write_block_group(currentBlockGroupIndex, &current); //write new data to actual block group
            currentBlockGroupIndex = current.nextBlockGroup;
        }else{//next block exists, go to it
            currentBlockGroupIndex = current.nextBlockGroup;
        }

    }

    for (uint32_t i = 0; i < blocksNeeded; i++) current.blockBitmap[incomingBlockAddress + i] = 1; //update the block bitmap

    FileEntry newFile = {
        .size = size,
        .startBlockIndex = incomingBlockAddress,
        .valid = 1,
        .type = isFolder ?  FILETYPE_FOLDER : FILETYPE_FILE
    };

    strncpy(newFile.fileName, name, MAX_FILENAME_LENGTH);

    memcpy(&current.dataBlocks[incomingBlockAddress], &newFile, FILE_ENTRY_SIZE); //copy in fileEntry

    if(blocksNeeded == 1){
        memcpy(&current.dataBlocks[incomingBlockAddress][FILE_ENTRY_SIZE], content, size); //copy in content
    }else{
        memcpy(&current.dataBlocks[incomingBlockAddress][FILE_ENTRY_SIZE], content, BLOCK_SIZE - FILE_ENTRY_SIZE); //copy content into first block w/ metadata
        size_t copied = BLOCK_SIZE - FILE_ENTRY_SIZE;
        for(uint32_t block = 1; (block < blocksNeeded) && (copied < size); block++){
            uint32_t remaining = size - copied;
            uint32_t copySize = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
            memcpy(&current.dataBlocks[incomingBlockAddress + block][0], &content[copied], copySize);
            copied += copySize;
        }
    }

    fs_write_block_group(currentBlockGroupIndex, &current); //update the current block

    return;
}

void fs_delete_file(const char* name){
    
}