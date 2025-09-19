/************************************************************************************************
 * @file   midFS.c
 *
 * @brief  Source file for file system library
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

#include "midFS.h"

#define SUPERBLOCK_OFFSET 0
#define BLOCKGROUP_OFFSET (SUPERBLOCK_OFFSET + sizeof(SuperBlock))

uint8_t fs_memory[FS_TOTAL_SIZE] = {0};

SuperBlock *superBlock=NULL;
BlockGroup *blockGroups=NULL;


FsStatus fs_init() {
    printf("File system init\n\r");

    //pull memory from HAL
    fs_hal_read(FS_HAL_ADDRESS, fs_memory, FS_TOTAL_SIZE);

    //setup superBlock and the first block group in memory
    superBlock = (SuperBlock *)&fs_memory[SUPERBLOCK_OFFSET];
    blockGroups = (BlockGroup *)&fs_memory[BLOCKGROUP_OFFSET];

    //super block declaration
    superBlock->magic = 0xC1D1921D;
    printf("Magic number from init: %#lx\n\r", superBlock->magic);

    superBlock->blockSize = BLOCK_SIZE;
    superBlock->blocksPerGroup = BLOCKS_PER_GROUP;
    superBlock->nextBlockGroup = 0;

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
    printf("End of file system init\n\r");
    return STATUS_CODE_OK;
}

StatusCode fs_pull(){

    //pull memory from HAL
    fs_hal_read(FS_HAL_ADDRESS, fs_memory, FS_TOTAL_SIZE);

    //setup superblock and block groups
    superBlock = (SuperBlock *)&fs_memory[SUPERBLOCK_OFFSET];
    blockGroups = (BlockGroup *)&fs_memory[BLOCKGROUP_OFFSET];

    //check if magic number matches
    if(superBlock->magic != 0xC1D1921D){
        printf("FS magic number does not match\n\r");
        return STATUS_CODE_INTERNAL_ERROR;
    }else{
        printf("FS magic number matches\n\r");
    }
    printf("Magic number from pull: %#lx\n\r", superBlock->magic);
    printf("Data from pull: %c\n\r", blockGroups->dataBlocks[0][0]);

    return STATUS_CODE_OK;
}

StatusCode fs_commit(){
    printf("Magic number from commit pre-wipe: %#lx\n\r", superBlock->magic);
    printf("Data: %c\n\r", blockGroups->dataBlocks[0][0]);
    fs_hal_write(FS_HAL_ADDRESS, fs_memory, FS_TOTAL_SIZE);
    
    // Wipe everything for testing
    // memset(fs_memory, 0, sizeof(fs_memory));
    // printf("Data POST WIPE: %c\n\r", blockGroups->dataBlocks[0][0]);
    // printf("Magic number from commit post-wipe: %#lx\n\r", superBlock->magic);
    
    return FS_STATUS_OK;
}

FsStatus fs_read_file(const char *path, uint8_t *content) {
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    printf("fs_read_file command:\n\r");
    printf("\tPath: %s\n\r", path);
    printf("\tPath name: %s\n\r", folderPath);
    printf("\tFolder name: %s\n\r", folderName);

    uint32_t parentBlockLocation;

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    FsStatus status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != FS_STATUS_OK){
        return status;
    }

    printf("Located parent block at index: %ld\n\r", parentBlockLocation);
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation / BLOCKS_PER_GROUP];
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    FileEntry currentFile;
    currentFile.valid = 0;

    printf("Finding file...\n\r");
    
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(i == (BLOCK_SIZE / sizeof(FileEntry)) - 1){
            if(File[i].valid){
                BlockGroup *nestedFileGroup = &blockGroups[File[i].startBlockIndex / BLOCKS_PER_GROUP];
                File = (FileEntry *)&nestedFileGroup->dataBlocks[File[i].startBlockIndex % BLOCKS_PER_GROUP];
                i = 0;
            }else{
                break;
            }
        }

        printf("File at %ld name: %s\n\r", i, File[i].fileName);

        if(File[i].valid && (strcmp(File[i].fileName, folderName) == 0)){
            currentFile = File[i];
            printf("Found file:\n\r");
            printf("\tFile size: %ld\n\r", File[i].size);
            printf("\tFile name: %s\n\r", File[i].fileName);
            printf("\tFile start block index: %d\n\r", File[i].startBlockIndex);            
            break;
        }
    }

    if(currentFile.valid == 0){
        printf("Could not find file\n\r");
        return FS_STATUS_INVALID_ARGS;
    }
        
    BlockGroup fileGroup = blockGroups[currentFile.startBlockIndex / BLOCKS_PER_GROUP];
    uint32_t blocksNeeded = (currentFile.size + BLOCK_SIZE - 1)/ BLOCK_SIZE;

    uint32_t writeSize = currentFile.size;

    printf("Blocks needed: %ld\n\r", blocksNeeded);

    printf("File content: \"");
    uint32_t copied = 0;
    for(uint32_t i = 0; i < blocksNeeded; i++){
        uint32_t chunk = (writeSize > BLOCK_SIZE) ? BLOCK_SIZE : writeSize;
        memcpy(&content[copied], fileGroup.dataBlocks[currentFile.startBlockIndex % BLOCKS_PER_GROUP + i], chunk);
        copied += chunk;
        writeSize -= chunk;
        // printf("%ld. chunk size: %ld\n\r", i, chunk);
        for(uint32_t j = 0; j < chunk; j++){
            if((j+1) % 100 == 0){
                printf("\n\r");
            }
            printf("%c", fileGroup.dataBlocks[currentFile.startBlockIndex % BLOCKS_PER_GROUP + i][j]);
        }
    }
    printf("\"\n\r");

    printf("End of fs_read_file_command\n\r");

    return FS_STATUS_OK;
}

FsStatus fs_add_file(const char * path, uint8_t* content, uint32_t size, uint8_t isFolder){
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    printf("fs_add_file command\n\r");
    
    fs_split_path(path, folderPath, folderName);
    
    printf("\tPath: %s\n\r", path);
    printf("\tPath name: %s\n\r", folderPath);
    printf("\tFolder name: %s\n\r", folderName);
    
    uint8_t doesFileExist;
    fs_does_file_exist(path, &doesFileExist); //check if a file of this name already exists in this directory
    if(doesFileExist){
        printf("A file of name %s already exists in directory %s. Skipping fs_add_file command\n\r", folderName, folderPath);
        return FS_STATUS_INVALID_ARGS;
    }


    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation;
    
    FsStatus status = fs_resolve_path(folderPath, &parentBlockLocation);
    printf("finished first resolve path function\n\r");

    if(status != FS_STATUS_OK){
        return status;
    }

    printf("Located parent block at index: %ld\n\r", parentBlockLocation);

    if(parentBlockLocation == FS_INVALID_BLOCK) return FS_STATUS_INVALID_ARGS;

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

    //initialize as a invalid marker
    uint32_t fileLocation = UINT32_MAX;

    //search for the first empty spot and copy in file data
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(i == (BLOCK_SIZE / sizeof(FileEntry)) - 1){
            //we have reached the end of the block, meaning this file will be a folder that points to the next block
            if(!File[i].valid){
                // printf("empty last index, expanding file\n\r");
                uint32_t nestedFileIndex;
                fs_locate_memory(1, &nestedFileIndex);
                File[i].valid = 1;
                File[i].type = FILETYPE_FOLDER;
                File[i].startBlockIndex = nestedFileIndex;
                // printf("More memory found at: %d\n\r", File[i].startBlockIndex);
                char *lastSlash;
                if(strchr(folderPath, '/') == NULL){
                    lastSlash = folderPath;
                }else{
                    lastSlash = strrchr(folderPath, '/');
                    lastSlash++;
                }
                if (lastSlash != NULL && *(lastSlash) != '\0') {
                    strncpy(File[i].fileName, lastSlash, MAX_FILENAME_LENGTH);
                    File[i].fileName[MAX_FILENAME_LENGTH - 1] = '\0';  // null-terminate
                }     
            }
            //the block group of our new file
            BlockGroup *nestedFileGroup = &blockGroups[File[i].startBlockIndex / BLOCKS_PER_GROUP];
            //set the bitmap
            nestedFileGroup->blockBitmap[File[i].startBlockIndex % BLOCKS_PER_GROUP] = 1;
            File = (FileEntry *)&nestedFileGroup->dataBlocks[File[i].startBlockIndex % BLOCKS_PER_GROUP];
            fs_write_block_group(File[i].startBlockIndex / BLOCKS_PER_GROUP, nestedFileGroup);
            // for(uint32_t j = 0; j < BLOCK_SIZE / sizeof(FileEntry); j++){
            //     File[j].valid = 0;
            // }
            
            //reset the loop, it will run one more time and the file will be added to the new block group
            i = 0;
        }

        //look for an empty spot
        if(!File[i].valid){
            //copy in the data
            fileLocation = i;
            File[i] = newFile;
            break;
        }
    }

    //we couldn't find a spot
    if(fileLocation == UINT32_MAX){
        return FS_STATUS_OUT_OF_SPACE;
    }

    if(isFolder){
        uint32_t incomingFolderBlockAddress = UINT32_MAX;
        fs_locate_memory(1, &incomingFolderBlockAddress);
        File[fileLocation].startBlockIndex = incomingFolderBlockAddress;
        //get the block group that our incoming block address is in 
        BlockGroup *current = &blockGroups[incomingFolderBlockAddress / BLOCKS_PER_GROUP];
        current->blockBitmap[incomingFolderBlockAddress] = 1; //update the block bitmap
        printf("Added folder at with block address: %ld\n\r", incomingFolderBlockAddress);
        return FS_STATUS_OK;
    }
    
    //determine how many blocks we need
    uint32_t blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    
    uint32_t incomingBlockAddress = UINT32_MAX; //invalid marker
    
    fs_locate_memory(blocksNeeded, &incomingBlockAddress);
    
    printf("Blocks needed: %ld\n\r", blocksNeeded);
    printf("Located sufficient memory at address: %ld\n\r", incomingBlockAddress);

    //get the block group that our incoming block address is in 
    BlockGroup *current = &blockGroups[incomingBlockAddress / BLOCKS_PER_GROUP];

    for (uint32_t i = 0; i < blocksNeeded; i++) current->blockBitmap[(incomingBlockAddress % BLOCKS_PER_GROUP) + i] = 1; //update the block bitmap
    
    File[fileLocation].startBlockIndex = incomingBlockAddress;

    fs_write_block_group(parentBlockLocation, parentGroup);
    
    if(blocksNeeded == 1){
        memcpy(&current->dataBlocks[incomingBlockAddress % BLOCKS_PER_GROUP][0], content, size); //copy in content
        printf("Copying in content: \"");
        for(uint32_t i = 0; i < size; i++){
            printf("%c", current->dataBlocks[incomingBlockAddress % BLOCKS_PER_GROUP][i]);
        }
        printf("\"\n\r");
    }else{
        memcpy(&current->dataBlocks[incomingBlockAddress % BLOCKS_PER_GROUP][0], content, BLOCK_SIZE); //copy content into first block
        printf("Copying in content that is larger than 1 block\"");
        
        uint32_t copied = BLOCK_SIZE;
        for(uint32_t block = 1; (block < blocksNeeded) && (copied < size); block++){
            uint32_t remaining = size - copied;
            uint32_t copySize = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
            memcpy(&current->dataBlocks[(incomingBlockAddress % BLOCKS_PER_GROUP) + block][0], &content[copied], copySize);
            copied += copySize;
        }
    }
    
    fs_write_block_group(incomingBlockAddress / BLOCKS_PER_GROUP, current); //update the current block

    BlockGroup fileGroup = blockGroups[newFile.startBlockIndex / BLOCKS_PER_GROUP];

    printf("End of fs_add_file function\n\r");

    return FS_STATUS_OK;
}

FsStatus fs_delete_file(const char* path){
    printf("fs_delete_file command\n\r");

    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    printf("\tfile name: %s\n\r", folderName);
    printf("\tfile path: %s\n\r", folderPath);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation; 

    FsStatus status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != FS_STATUS_OK){
        return status;
    }

    printf("Located parent block at index: %ld\n\r", parentBlockLocation);

    if(parentBlockLocation == FS_INVALID_BLOCK) return FS_STATUS_INVALID_ARGS;

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    //initialize them to invalid markers
    uint16_t fileStartBlockIndex = UINT16_MAX;
    uint32_t fileSize = UINT32_MAX;

    //search for the first empty spot
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(i == (BLOCK_SIZE / sizeof(FileEntry)) - 1){ //if we reach the end of a block group
            if(File[i].valid){ //nested folder exists
                //relocate to nested folder
                BlockGroup *nestedFileGroup = &blockGroups[File[i].startBlockIndex / BLOCKS_PER_GROUP];
                File = (FileEntry *)&nestedFileGroup->dataBlocks[File[i].startBlockIndex % BLOCKS_PER_GROUP];
                i = 0; //reset to 0 and keep looking
            }else{//nested folder doesnt exist
                break;
            }
        }

        if(File[i].valid && (strcmp(File[i].fileName, folderName) == 0)){ //if the file name is the same as our path name
            //store the index and size
            fileStartBlockIndex = File[i].startBlockIndex;
            fileSize = File[i].size;

            printf("Found file:\n\r");
            printf("\tFile size: %ld\n\r", File[i].size);
            printf("\tFile name: %s\n\r", File[i].fileName);
            printf("\tFile start block index: %d\n\r", File[i].startBlockIndex);    

            //clear the file metadata
            File[i].valid = 0;
            memset(&File[i].fileName, 0, MAX_FILENAME_LENGTH);
            File[i].size = 0;
            File[i].startBlockIndex = 0;
            printf("Deleted file\n\r");
            break;
        }
    }

    if(fileSize == UINT32_MAX || fileStartBlockIndex == UINT16_MAX){
        return FS_STATUS_INVALID_ARGS;
    }

    BlockGroup *fileContentGroup = &blockGroups[fileStartBlockIndex/BLOCKS_PER_GROUP];

    //determine how many blocks this file takes up
    uint32_t blocksNeeded = (fileSize + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division

    for(uint32_t i = 0; i < blocksNeeded; i++){
        //clear the blocks
        memset(fileContentGroup->dataBlocks[(fileStartBlockIndex%BLOCKS_PER_GROUP) + i], 0, BLOCK_SIZE);
        //update the bitmap
        fileContentGroup->blockBitmap[(fileStartBlockIndex%BLOCKS_PER_GROUP) + i] = 1;
    }
    return FS_STATUS_OK;
}

FsStatus fs_write_file(const char * path, uint8_t *content, uint32_t contentSize){
    FsStatus fs_status = FS_STATUS_OK;

    printf("fs_write_file command\n\r");
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation;

    fs_status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(fs_status != FS_STATUS_OK){
        return fs_status;
    }

    if(parentBlockLocation == FS_INVALID_BLOCK){
        return FS_STATUS_INVALID_ARGS;
    }

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    //array of files in block group
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    FileEntry oldFile;
    oldFile.valid = 0;
    uint32_t fileLocation = UINT32_MAX;


    //search for the first empty spot
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(i == (BLOCK_SIZE / sizeof(FileEntry)) - 1){
            if(File[i].valid){
                BlockGroup *nestedFileGroup = &blockGroups[File[i].startBlockIndex / BLOCKS_PER_GROUP];
                File = (FileEntry *)&nestedFileGroup->dataBlocks[File[i].startBlockIndex % BLOCKS_PER_GROUP];
                i = 0;
            }else{
                break;
            }
        }

        if(File[i].valid && (strcmp(File[i].fileName, folderName) == 0)){
            //if the name of this file matches the file we're trying to find
            fileLocation = i;
            oldFile = File[i];
            printf("Found file:\n\r");
            printf("\tFile size: %ld\n\r", File[i].size);
            printf("\tFile name: %s\n\r", File[i].fileName);
            printf("\tFile start block index: %d\n\r", File[i].startBlockIndex);    
            break;
        }
    }

    if(oldFile.valid == 0){
        return FS_STATUS_INVALID_ARGS;
    }

    uint32_t oldBlocksNeeded = (oldFile.size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    uint32_t newBlocksNeeded = (oldFile.size + contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE;

    printf("Old blocks needed: %ld\n\r", oldBlocksNeeded);
    printf("New blocks needed: %ld\n\r", newBlocksNeeded);

    uint32_t canWriteInPlace = 1;

    //i starts at the last block of the old file
    //i ends at the index of (start of file + newBlocksNeeded)
    for(uint32_t i = (parentBlockLocation % BLOCKS_PER_GROUP) + oldBlocksNeeded; i < (parentBlockLocation % BLOCKS_PER_GROUP) + newBlocksNeeded; i++){
        if(parentGroup->blockBitmap[i] == 1){
            canWriteInPlace = 0;
        }
    }

    printf("Can write in place: %ld\n\r", canWriteInPlace);

    if(canWriteInPlace == 1){ 
        //we have sufficient contiguous blocks after our old file to write in place
        BlockGroup * oldFileGroup = &blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP];
        memcpy(&oldFileGroup->dataBlocks[oldFile.startBlockIndex % BLOCKS_PER_GROUP][oldFile.size], content, contentSize); //copy in new content after old content
        File[fileLocation].size = oldFile.size + contentSize; //update size of file entry
        fs_write_block_group(parentBlockLocation/BLOCKS_PER_GROUP, parentGroup); //update the current block

    }else{ 
        //if we cannot write in place we must find more space (search for contiguous space of length newBlocksNeeded)
        uint32_t incomingBlockAddress = UINT32_MAX;
        
        fs_status = fs_locate_memory(newBlocksNeeded, &incomingBlockAddress);
        if(fs_status != FS_STATUS_OK){
            return fs_status;
        }
        
        BlockGroup *current = &blockGroups[incomingBlockAddress / BLOCKS_PER_GROUP];

        //create a new array to store old content + new content
        uint8_t fullContent[MAX_FILE_CONTENT_POST_WRITE];

        uint32_t copied = 0;

        //copy old content in
        for(uint32_t i = 0; i < oldBlocksNeeded; i++){
            uint32_t blockOffset = (oldFile.startBlockIndex % BLOCKS_PER_GROUP) + i;
            uint32_t copySize = (oldFile.size - copied > BLOCK_SIZE) ?BLOCK_SIZE : oldFile.size - copied;
            memcpy(&fullContent[copied], &blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP].dataBlocks[blockOffset][0], copySize);
            copied += copySize;
        }

        //copy new content in
        memcpy(&fullContent[copied], content, contentSize);

        //update bitmap
        for(uint32_t i = 0; i < newBlocksNeeded; i++){
            current->blockBitmap[(incomingBlockAddress % BLOCKS_PER_GROUP) + i] = 1;
        }

        copied = 0;

        //copy data in
        for(uint32_t i = 0; i < newBlocksNeeded; i++){
            uint32_t chunk = (oldFile.size + contentSize - copied > BLOCK_SIZE) ? BLOCK_SIZE : (oldFile.size + contentSize - copied);
            memcpy(&current->dataBlocks[(incomingBlockAddress % BLOCKS_PER_GROUP) + i], &fullContent[copied], chunk);
            copied += chunk;
        }

        //clear the old blocks and set old bitmap values to 0
        for(uint32_t i = 0; i < oldBlocksNeeded; i++){
            memset(&blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP].dataBlocks[oldFile.startBlockIndex % BLOCKS_PER_GROUP + i][0], 0, BLOCK_SIZE);
            blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP].blockBitmap[oldFile.startBlockIndex % BLOCKS_PER_GROUP + i] = 0;
        }

        //update file metadata
        File[fileLocation].size = oldFile.size + contentSize;
        File[fileLocation].startBlockIndex = incomingBlockAddress;

        //update block groups
        fs_write_block_group(parentBlockLocation / BLOCKS_PER_GROUP, parentGroup);
        fs_write_block_group(incomingBlockAddress / BLOCKS_PER_GROUP, current);
        fs_write_block_group(oldFile.startBlockIndex / BLOCKS_PER_GROUP, &blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP]);

    }

    return FS_STATUS_OK;
}

FsStatus fs_create_block_group(uint32_t *index){
    for(uint32_t i = 0; i < NUM_BLOCK_GROUPS; i++){
        if(
            blockGroups[i].nextBlockGroup == FS_NULL_BLOCK_GROUP &&
            memcmp(&blockGroups[i], &(BlockGroup){0}, sizeof(BlockGroup)) == 0 //block group is empty
        ){
            memset(&blockGroups[i], 0, sizeof(BlockGroup)); //clear it just to be safe
            *index = i;
            return FS_STATUS_OK;
        }
    }

    *index = FS_INVALID_BLOCK;
    return FS_STATUS_OUT_OF_SPACE; //no more space
}

FsStatus fs_read_block_group(uint32_t blockIndex, BlockGroup *dest){
    if(blockIndex >= NUM_BLOCK_GROUPS) return STATUS_CODE_OUT_OF_RANGE; //block index is too high
    memcpy(dest, &blockGroups[blockIndex], sizeof(BlockGroup));
    return FS_STATUS_OK;
}

FsStatus fs_write_block_group(uint32_t blockIndex, BlockGroup *src){
    if(blockIndex >= NUM_BLOCK_GROUPS) return STATUS_CODE_OUT_OF_RANGE; //block index is too high
    memcpy(&blockGroups[blockIndex], src, sizeof(BlockGroup));
    return FS_STATUS_OK;
}

FsStatus fs_split_path(char *path, char *folderPath, char *fileName){
    const char *lastSlash = strrchr(path, '/');

    if(!lastSlash || lastSlash == path){
        snprintf(folderPath, MAX_PATH_LENGTH, "%s", "/");
        snprintf(fileName, MAX_FILENAME_LENGTH, "%s", (lastSlash) ? lastSlash + 1 : path);
    }else{
        uint32_t folderLen = lastSlash - path;
        strncpy(folderPath, path, folderLen);
        folderPath[folderLen] = '\0';
        snprintf(fileName, MAX_FILENAME_LENGTH, "%s", lastSlash + 1);
    }

    return FS_STATUS_OK;
}

FsStatus fs_resolve_path(const char* folderPath, uint32_t* path){
    printf("fs_resolve_path function:\n\r");
    printf("searching for path: %s\n\r", folderPath);

    if (strcmp(folderPath, "/") == 0){
        *path = 0;
        printf("resolved root path: %ld\n\r", *path);
        return STATUS_CODE_OK;  
    }  //return the root directory

    char copy[MAX_PATH_LENGTH];
    strncpy(copy, folderPath, MAX_PATH_LENGTH);
    copy[MAX_PATH_LENGTH-1] = '\0';

    char *saveptr;
    char *currentFile = strtok_r(copy, "/", &saveptr);
    uint32_t currentBlock = 0;    


    //start at root folder, get first file, and search all fileentries
    while(currentFile != NULL){
        printf("searching for: %s\n\r", currentFile);
        BlockGroup *group = &blockGroups[currentBlock/BLOCKS_PER_GROUP];
        FileEntry *File = (FileEntry *) &group->dataBlocks[currentBlock % BLOCKS_PER_GROUP];
        int found = 0;
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
            printf("index: %ld, name: %s\n\r", i, File[i].fileName);
            if (File[i].valid && strcmp(File[i].fileName, currentFile) == 0){
                currentBlock = File[i].startBlockIndex;
                found = 1;
                break;
            }
        }
        if (!found){
            *path = FS_INVALID_BLOCK;
            printf("could not resolve path. exiting... \n\r");
            return FS_STATUS_PATH_NOT_FOUND;
        }
        currentFile = strtok_r(NULL, "/", &saveptr);
    }
    *path = currentBlock;

    printf("resolved path\n\r: %ld", *path);
    return FS_STATUS_OK;
}

FsStatus fs_list(const char *path){
    printf("Start of fs_list command\n\r");

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation; 
    FsStatus status = fs_resolve_path(path, &parentBlockLocation);

    printf("%d\n\r", status);

    if(status != FS_STATUS_OK){
        return status;
    }

    printf("%d\n\r", parentBlockLocation == FS_INVALID_BLOCK);

    if(parentBlockLocation == FS_INVALID_BLOCK) return FS_STATUS_INVALID_ARGS;

    // printf("Located parent block at index: %d\n\r", parentBlockLocation);

    //the block group our directory is located in
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    //Store the files in our folder block in an array of files
    //might need to add a try catch or something for this line in case the path doesnt lead to a folder
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    //loop through files
    printf("Files in %s: \n\r", path);
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(i == (BLOCK_SIZE / sizeof(FileEntry)) - 1){
            if(File[i].valid){
                BlockGroup *nestedFileGroup = &blockGroups[File[i].startBlockIndex / BLOCKS_PER_GROUP];
                File = (FileEntry *)&nestedFileGroup->dataBlocks[File[i].startBlockIndex % BLOCKS_PER_GROUP];
                i = 0;
            }else{
                break;
            }
        }
        printf("|--");
        if(File[i].valid){
            printf("\t%s", File[i].fileName);
        }
        printf("\n\r");
    }

    return FS_STATUS_OK;
}

FsStatus fs_locate_memory(const uint32_t blocksNeeded, uint32_t *incomingBlockAddress){

    //first block
    BlockGroup current;
    uint32_t currentBlockGroupIndex = superBlock->nextBlockGroup;
    
    uint32_t space = 0;
    uint8_t memoryFound = 0;
    *incomingBlockAddress = 0;
    
    while(1){
        fs_read_block_group(currentBlockGroupIndex, &current);
        for(uint32_t i = 0; i < BLOCKS_PER_GROUP; i++){
            if(current.blockBitmap[i] == 0){
                while(i + space < BLOCKS_PER_GROUP && current.blockBitmap[i + space] == 0){
                    space++;
                    if(space == blocksNeeded){
                        *incomingBlockAddress += i;
                        memoryFound = 1;
                        break;
                    }
                }
                i += space;
            }
            space = 0;
            if(memoryFound){
                break;
            }
        }
        //if there is no space and we havent found free space, go to the next block

        if(memoryFound) break; //we found an address, break

        //if we still have not found an address, we need to go to the next block group

        //increment incomingBlockAddress by BLOCKS_PER_GROUP to keep with our global indexing
        *incomingBlockAddress += BLOCKS_PER_GROUP;

        if(current.nextBlockGroup == FS_NULL_BLOCK_GROUP){ //next block does not exist
            uint32_t newBlockGroupIndex;
            FsStatus status = fs_create_block_group(&newBlockGroupIndex);
            if(status != FS_STATUS_OK){
                return status;
            }
            current.nextBlockGroup = newBlockGroupIndex;
            fs_write_block_group(currentBlockGroupIndex, &current); //write new data to actual block group
            currentBlockGroupIndex = current.nextBlockGroup;
        }else{//next block exists, go to it
            currentBlockGroupIndex = current.nextBlockGroup;
        }

    }

    return FS_STATUS_OK;
    
}

FsStatus fs_does_file_exist(const char *path, uint8_t *doesFileExist){
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation;

    FsStatus status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != FS_STATUS_OK){
        return status;
    }

    if(parentBlockLocation == FS_INVALID_BLOCK) return FS_STATUS_INVALID_ARGS;

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    //array of files in block group
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(i == (BLOCK_SIZE / sizeof(FileEntry)) - 1){
            if(File[i].valid){
                BlockGroup *nestedFileGroup = &blockGroups[File[i].startBlockIndex / BLOCKS_PER_GROUP];
                File = (FileEntry *)&nestedFileGroup->dataBlocks[File[i].startBlockIndex % BLOCKS_PER_GROUP];
                i = 0;
            }else{
                break;
            }
        }
        if(File[i].valid == 1 && strcmp(File[i].fileName, folderName) == 0){
            *doesFileExist = 1;
            return FS_STATUS_OK;
        }
    }

    *doesFileExist = 0;
    return FS_STATUS_OK;
}
