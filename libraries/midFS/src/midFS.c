#include "midFS.h"

#define SUPERBLOCK_OFFSET 0
#define BLOCKGROUP_OFFSET (SUPERBLOCK_OFFSET + sizeof(SuperBlock))

uint8_t fs_memory[FS_TOTAL_SIZE];

SuperBlock *superBlock;
BlockGroup *blockGroups;

void fs_init() {
    //setup superBlock and the first block group in memory
    superBlock = (SuperBlock *)&fs_memory[SUPERBLOCK_OFFSET];
    blockGroups = (BlockGroup *)&fs_memory[BLOCKGROUP_OFFSET];

    //super block declaration
    superBlock->magic = 0xC1D1921D;
    superBlock->blockSize = BLOCK_SIZE;
    superBlock->blocksPerGroup = BLOCKS_PER_GROUP;
    superBlock->nextBlockGroup = &blockGroups[0];

    blockGroups->nextBlockGroup = FS_NULL_BLOCK_GROUP;

    //init the first block group
    memset(&blockGroups[0], 0, sizeof(BlockGroup));

    blockGroups[0].blockBitmap[0] = 1; //block 0 in group 0 is used for the root folder

    //root folder declaration
    strncpy(superBlock->rootFolderMetadata.fileName, "/", MAX_FILENAME_LENGTH);
    superBlock->rootFolderMetadata.type = FILETYPE_FOLDER;
    superBlock->rootFolderMetadata.valid = 1;
    superBlock->rootFolderMetadata.startBlockIndex = 0;
    superBlock->rootFolderMetadata.size = 0;
}


uint32_t read_file(const char *path) {
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation = fs_resolve_path(folderPath); 

    return parentBlockLocation;
}

void extract_file(uint8_t *addr, uint32_t size){
    for (uint32_t i=0; i<size; i++){
        putchar(addr[i]);
        //just read out the contents
    }
}

void fs_add_file(const char * path, uint8_t* content, uint32_t size, uint8_t isFolder){
    //add something to restrict length of name
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, *folderPath, *folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation = fs_resolve_path(folderPath); 

    if(parentBlockLocation == FS_INVALID_BLOCK) return;

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    //update the bitmap
    parentGroup->blockBitmap[parentBlockLocation % BLOCKS_PER_GROUP] = 1;

    //Array of files
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];
    
    //our new file
    FileEntry newFile = {
        .size = size,
        .valid = 1,
        .type = isFolder ?  FILETYPE_FOLDER : FILETYPE_FILE
    };

    //copy in the name
    strncpy(newFile.fileName, folderName, MAX_FILENAME_LENGTH);
    newFile.fileName[MAX_FILENAME_LENGTH - 1] = '\0';
    
    //search for the first empty spot
    for(int i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(!File[i].valid){
            File[i] = newFile;
            break;
        }
    }

    //determine how many blocks we need
    uint32_t blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    
    //first block
    BlockGroup current;
    uint32_t currentBlockGroupIndex = superBlock->nextBlockGroup;
    
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
    
    newFile.startBlockIndex = incomingBlockAddress,

    strncpy(newFile.fileName, path, MAX_FILENAME_LENGTH);

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

void fs_delete_file(const char* path){
    //add something to restrict length of name
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, *folderPath, *folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation = fs_resolve_path(folderPath); 

    if(parentBlockLocation == FS_INVALID_BLOCK) return;

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    uint16_t fileStartBlockIndex;
    uint32_t fileSize;

    //search for the first empty spot
    for(int i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(File[i].fileName == folderName){ //if the file name is the same as our path name

            //store the index and size
            fileStartBlockIndex = File[i].startBlockIndex;
            fileSize = File[i].size;

            //clear the file
            File[i].valid = 0;
            memset(File[i].fileName, 0, MAX_FILENAME_LENGTH);
            File[i].size = 0;
            File[i].startBlockIndex = 0;
        }
    }

    BlockGroup *fileContentGroup = &blockGroups[fileStartBlockIndex/BLOCKS_PER_GROUP];

    //determine how many blocks this file takes up
    uint32_t blocksNeeded = (fileSize + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division

    //clear the blocks
    for(int i = 0; i < blocksNeeded; i++){
        memset(fileContentGroup->dataBlocks[(fileStartBlockIndex%BLOCKS_PER_GROUP) + i], 0, BLOCK_SIZE);        
    }

    //update the bitmap


}