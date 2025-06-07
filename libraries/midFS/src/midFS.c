#include "midFS.h"

#define SUPERBLOCK_OFFSET 0
#define BLOCKGROUP_OFFSET (SUPERBLOCK_OFFSET + sizeof(SuperBlock))

uint8_t fs_memory[FS_TOTAL_SIZE];

SuperBlock *superBlock;
BlockGroup *blockGroups;

StatusCode fs_init() {
    //setup superBlock and the first block group in memory
    superBlock = (SuperBlock *)&fs_memory[SUPERBLOCK_OFFSET];
    blockGroups = (BlockGroup *)&fs_memory[BLOCKGROUP_OFFSET];

    //super block declaration
    superBlock->magic = 0xC1D1921D;
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
    
    return STATUS_CODE_OK;
}


StatusCode fs_read_file(const char *path) {
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    printf("Path: %s\n", path);
    printf("Path name: %s\n", folderPath);
    printf("Folder name: %s\n", folderName);

    uint32_t parentBlockLocation;

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    StatusCode status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != STATUS_CODE_OK){
        return status;
    }

    printf("%d\n", parentBlockLocation);
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation / BLOCKS_PER_GROUP];
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    FileEntry currentFile;
    currentFile.valid = 0;

    printf("finding file\n");
    
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        printf("%d, %s\n", i, File[i].fileName);
        if(File[i].valid){
        }
        if(File[i].valid && (strcmp(File[i].fileName, folderName) == 0)){
            printf("found file\n");
            currentFile = File[i];
            printf("File i size: %d\n", File[i].size);
            printf("File i name: %s\n", File[i].fileName);
            printf("File i start block index: %d\n", File[i].startBlockIndex);            
            // currentFile.startBlockIndex = 1;
            break;
        }
    }

    if(currentFile.valid == 0){
        return STATUS_CODE_INVALID_ARGS;
    }


    printf("File size: %d\n", currentFile.size);
    printf("File name: %s\n", currentFile.fileName);
    printf("Current file start block index: %d\n", currentFile.startBlockIndex);
        
    BlockGroup fileGroup = blockGroups[currentFile.startBlockIndex / BLOCKS_PER_GROUP];
    uint32_t blocksNeeded = (currentFile.size + BLOCK_SIZE - 1)/ BLOCK_SIZE;

    uint32_t writeSize = currentFile.size;

    for(uint32_t i = 0; i < blocksNeeded; i++){
        uint32_t chunk = (writeSize > BLOCK_SIZE) ? BLOCK_SIZE : writeSize;
        printf("Chunk size: %d\n", chunk);
        printf("File content:\n");
        for(uint32_t j = 0; j < chunk; j++){
            printf("%c", fileGroup.dataBlocks[currentFile.startBlockIndex % BLOCKS_PER_GROUP + i][j]);
        }
        printf("\n");
        
        for(uint32_t i = 0; i < chunk; i++){
            // printf("%c", fileGroup.dataBlocks[currentFile.startBlockIndex][i]);
        }
        printf("\n");

    }

    return STATUS_CODE_OK;
}

StatusCode fs_add_file(const char * path, uint8_t* content, uint32_t size, uint8_t isFolder){
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);
    printf("Path: %s\n", path);
    printf("Path name: %s\n", folderPath);
    printf("Folder name: %s\n", folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation;
    
    StatusCode status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != STATUS_CODE_OK){
        return status;
    }

    printf("Parent block location: %d\n", parentBlockLocation);

    if(parentBlockLocation == FS_INVALID_BLOCK) return STATUS_CODE_INVALID_ARGS;

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
    // newFile.startBlockIndex = 1;

    uint32_t fileLocation = UINT32_MAX;

    //search for the first empty spot and copy in file data
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(!File[i].valid){
            fileLocation = i;
            File[i] = newFile;
            printf("%s\n", File[i].fileName);
            printf("%d\n", File[i].startBlockIndex);
            printf("%d\n", File[i].valid);
            printf("Copied name\n");
            break;
        }
    }

    if(fileLocation == UINT32_MAX){
        return STATUS_CODE_OUT_OF_RANGE;
    }
    
    //determine how many blocks we need
    uint32_t blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    
    printf("Blocks needed: %d\n", blocksNeeded);
    
    //first block
    BlockGroup current;
    uint32_t currentBlockGroupIndex = superBlock->nextBlockGroup;
    
    uint32_t incomingBlockAddress = UINT32_MAX; //invalid marker
    uint32_t space = 0;
    // File[fileLocation].startBlockIndex = 1;
    
    while(1){
        // printf("Checking for empty space w/ current block group index of %d\n", currentBlockGroupIndex);
        fs_read_block_group(currentBlockGroupIndex, &current);
        for(uint32_t i = 0; i < BLOCKS_PER_GROUP; i++){
            printf("%d\n", current.blockBitmap[i]);
            if(current.blockBitmap[i] == 0){
                while(i + space < BLOCKS_PER_GROUP && current.blockBitmap[i + space] == 0){
                    space++;
                    if(space == blocksNeeded){
                        printf("yo\n");
                        // File[fileLocation].startBlockIndex = i;
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

        printf("Incoming block address: %d\n", incomingBlockAddress);
        printf("File location of start block index: %d\n", File[fileLocation].startBlockIndex);
        if(incomingBlockAddress != UINT32_MAX) break; //we found an address, break


        if(current.nextBlockGroup == FS_NULL_BLOCK_GROUP){ //next block does not exist
            uint32_t newBlockGroupIndex;
            StatusCode status = fs_create_block_group(&newBlockGroupIndex);
            if(status != STATUS_CODE_OK){
                return status;
            }
            current.nextBlockGroup = newBlockGroupIndex;
            fs_write_block_group(currentBlockGroupIndex, &current); //write new data to actual block group
            currentBlockGroupIndex = current.nextBlockGroup;
        }else{//next block exists, go to it
            currentBlockGroupIndex = current.nextBlockGroup;
        }

    }

    for (uint32_t i = 0; i < blocksNeeded; i++) current.blockBitmap[incomingBlockAddress + i] = 1; //update the block bitmap
    
    newFile.startBlockIndex = incomingBlockAddress;
    File[fileLocation].startBlockIndex = incomingBlockAddress;
    printf("File[filelocation] start block address: %d\n", File[fileLocation].startBlockIndex);

    fs_write_block_group(parentBlockLocation, parentGroup);
    
    if(blocksNeeded == 1){
        memcpy(&current.dataBlocks[incomingBlockAddress][0], content, size); //copy in content
        for(uint32_t i = 0; i < size; i++){
            printf("%c", current.dataBlocks[incomingBlockAddress][i]);
        }
        printf("\n");
    }else{
        memcpy(&current.dataBlocks[incomingBlockAddress][0], content, BLOCK_SIZE); //copy content into first block
        uint32_t copied = BLOCK_SIZE;
        for(uint32_t block = 1; (block < blocksNeeded) && (copied < size); block++){
            uint32_t remaining = size - copied;
            uint32_t copySize = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
            memcpy(&current.dataBlocks[incomingBlockAddress + block][0], &content[copied], copySize);
            copied += copySize;
        }
    }
    
    fs_write_block_group(currentBlockGroupIndex, &current); //update the current block
    
    printf("New file name: %s\n", newFile.fileName);
    printf("New file size: %d\n", newFile.size);
    printf("New file start block index: %d\n", newFile.startBlockIndex);
    
    BlockGroup fileGroup = blockGroups[newFile.startBlockIndex / BLOCKS_PER_GROUP];
    for(uint32_t i = 0; i < newFile.size; i++){
        printf("%c", fileGroup.dataBlocks[newFile.startBlockIndex % BLOCKS_PER_GROUP][i]);
    }

    printf("\n");
    printf("End of add file function\n");

    return STATUS_CODE_OK;
}

StatusCode fs_delete_file(const char* path){
    //add something to restrict length of name
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation; 

    StatusCode status = fs_resolve_path(path, &parentBlockLocation);
    if(status != STATUS_CODE_OK){
        return status;
    }

    if(parentBlockLocation == FS_INVALID_BLOCK) return STATUS_CODE_INVALID_ARGS;

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    //initialize them to invalid markers
    uint16_t fileStartBlockIndex = UINT16_MAX;
    uint32_t fileSize = UINT32_MAX;

    //search for the first empty spot
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(strncmp(File[i].fileName, folderName, MAX_FILENAME_LENGTH)){ //if the file name is the same as our path name

            //store the index and size
            fileStartBlockIndex = File[i].startBlockIndex;
            fileSize = File[i].size;

            //clear the file metadata
            File[i].valid = 0;
            memset(&File[i].fileName, 0, MAX_FILENAME_LENGTH);
            File[i].size = 0;
            File[i].startBlockIndex = 0;
            break;
        }
    }

    if(fileSize == UINT32_MAX || fileStartBlockIndex == UINT16_MAX){
        return STATUS_CODE_INVALID_ARGS;
    }

    BlockGroup *fileContentGroup = &blockGroups[fileStartBlockIndex/BLOCKS_PER_GROUP];

    //determine how many blocks this file takes up
    uint32_t blocksNeeded = (fileSize + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division

    //clear the blocks
    //update the bitmap
    for(uint32_t i = 0; i < blocksNeeded; i++){
        memset(fileContentGroup->dataBlocks[(fileStartBlockIndex%BLOCKS_PER_GROUP) + i], 0, BLOCK_SIZE);
        fileContentGroup->blockBitmap[(fileStartBlockIndex%BLOCKS_PER_GROUP) + i] = 1;
    }
    return STATUS_CODE_OK;
}

StatusCode fs_write_file(const char * path, uint8_t *content, uint32_t contentSize){
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation;

    StatusCode status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != STATUS_CODE_OK){
        return status;
    }

    if(parentBlockLocation == FS_INVALID_BLOCK) return STATUS_CODE_INVALID_ARGS;

    //locate the block group
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    //array of files in block group
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    FileEntry oldFile;
    oldFile.valid = 0;

    //search for the first empty spot
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(File[i].valid && strncmp(File[i].fileName, folderName, MAX_FILENAME_LENGTH)){
            //if the name of this file matches the file we're trying to find
            oldFile = File[i];
            break;
        }
    }

    if(oldFile.valid == 0){
        return STATUS_CODE_INVALID_ARGS;
    }

    uint32_t oldBlocksNeeded = (oldFile.size + BLOCK_SIZE - 1) / BLOCK_SIZE; //ceiling division
    uint32_t newBlocksNeeded = (oldFile.size + contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE;

    uint32_t canWriteInPlace = 1;

    //i starts at the last block of the old file
    //i ends at the index of (start of file + newBlocksNeeded)
    for(uint32_t i = (parentBlockLocation % BLOCKS_PER_GROUP) + oldBlocksNeeded; i < (parentBlockLocation % BLOCKS_PER_GROUP) + newBlocksNeeded; i++){
        if(parentGroup->blockBitmap[i] == 1){
            canWriteInPlace = 0;
        }
    }

    if(canWriteInPlace == 1){ //we have sufficient contiguous blocks after our old file to write in place
        if(newBlocksNeeded == 1){
            memcpy(&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP][oldFile.size], content, contentSize); //copy in new content after old content
        }else{
            memcpy(&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP][oldFile.size], content, BLOCK_SIZE - oldFile.size); //copy content into block with the end of oldFile content
            uint32_t copied = BLOCK_SIZE - oldFile.size;
            for(uint32_t block = 1; (block < newBlocksNeeded - oldBlocksNeeded) && (copied < contentSize); block++){
                uint32_t remaining = contentSize - copied;
                uint32_t copySize = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
                memcpy(&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP + block][0], &content[copied], copySize);
                copied += copySize;
            }
        }

        //update size of file entry
        oldFile.size = oldFile.size + contentSize;
        fs_write_block_group(parentBlockLocation/BLOCKS_PER_GROUP, parentGroup); //update the current block

    }else{ //if we cannot write in place we must find more space (search for contiguous space of length newBlocksNeeded)
        BlockGroup current;
        uint32_t currentBlockGroupIndex = superBlock->nextBlockGroup; //start searching from the first block
        uint32_t incomingBlockAddress = UINT32_MAX;
        uint32_t space = 0;

        while(1){
            fs_read_block_group(currentBlockGroupIndex, &current);

            //locate contiguous blocks of size newBlocksNeeded, if found store it in incomingBlockAddress
            for(uint32_t i = 0; i < BLOCKS_PER_GROUP; i++){
                if(current.blockBitmap[i] == 0){
                    space = 1;
                    while((i + space < BLOCKS_PER_GROUP) && current.blockBitmap[i + space] == 0){
                        space++;
                        if(space == newBlocksNeeded){
                            incomingBlockAddress = i;
                            break;
                        }
                    }
                    if(incomingBlockAddress != UINT32_MAX) break;
                    i += space;
                }
            }

            if(incomingBlockAddress != UINT32_MAX) break;

            //if we reach this point that means we couldnt find sufficient contiguous space in the current block group
            if(current.nextBlockGroup == FS_NULL_BLOCK_GROUP){ //if this is the final block group
                //initialize a new block group
                uint32_t newBlockGroupIndex;
                StatusCode status = fs_create_block_group(&newBlockGroupIndex);
                if(status != STATUS_CODE_OK){
                    return status;
                }
                current.nextBlockGroup = newBlockGroupIndex;
                fs_write_block_group(currentBlockGroupIndex, &current);
                currentBlockGroupIndex = newBlockGroupIndex;
            }else{
                //go to the next block group
                currentBlockGroupIndex = current.nextBlockGroup;
            }
        }

        //create a new array to store old content + new content
        uint8_t fullContent[oldFile.size + contentSize];

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
            current.blockBitmap[incomingBlockAddress + i] = 1;
        }

        copied = 0;

        //copy data in
        for(uint32_t i = 0; i < newBlocksNeeded; i++){
            uint32_t chunk = (oldFile.size + contentSize - copied > BLOCK_SIZE) ? BLOCK_SIZE : (oldFile.size + contentSize - copied);
            memcpy(&current.dataBlocks[incomingBlockAddress + i], &fullContent[copied], chunk);
            copied += chunk;
        }

        //clear the old blocks and set old bitmap values to 0
        for(uint32_t i = 0; i < oldBlocksNeeded; i++){
            memset(&blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP].dataBlocks[oldFile.startBlockIndex % BLOCKS_PER_GROUP + i][0], 0, BLOCK_SIZE);
            blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP].blockBitmap[oldFile.startBlockIndex % BLOCKS_PER_GROUP + i] = 0;
        }

        //update file metadata
        oldFile.size = oldFile.size + contentSize;
        oldFile.startBlockIndex = incomingBlockAddress;

        //update block groups
        fs_write_block_group(parentBlockLocation / BLOCKS_PER_GROUP, parentGroup);
        fs_write_block_group(currentBlockGroupIndex, &current);
        fs_write_block_group(oldFile.startBlockIndex / BLOCKS_PER_GROUP, &blockGroups[oldFile.startBlockIndex / BLOCKS_PER_GROUP]);

    }

    return STATUS_CODE_OK;
}

StatusCode fs_create_block_group(uint32_t *index){
    for(uint32_t i = 0; i < FS_TOTAL_BLOCK_GROUPS; i++){
        if(
            blockGroups[i].nextBlockGroup == FS_NULL_BLOCK_GROUP &&
            memcmp(&blockGroups[i], &(BlockGroup){0}, sizeof(BlockGroup)) == 0 //block group is emptys
        ){
            memset(&blockGroups[i], 0, sizeof(BlockGroup)); //clear it just to be safe
            *index = i;
            return STATUS_CODE_OK;
        }
    }


    *index = FS_INVALID_BLOCK;
    return STATUS_CODE_INCOMPLETE; //no more space
}

StatusCode fs_read_block_group(uint32_t blockIndex, BlockGroup *dest){
    if(blockIndex >= FS_TOTAL_BLOCK_GROUPS) return STATUS_CODE_OUT_OF_RANGE; //block index is too high
    memcpy(dest, &blockGroups[blockIndex], sizeof(BlockGroup));
    return STATUS_CODE_OK;
}

StatusCode fs_write_block_group(uint32_t blockIndex, BlockGroup *src){
    if(blockIndex >= FS_TOTAL_BLOCK_GROUPS) return STATUS_CODE_OUT_OF_RANGE; //block index is too high
    memcpy(&blockGroups[blockIndex], src, sizeof(BlockGroup));
    return STATUS_CODE_OK;
}

StatusCode fs_split_path(char *path, char *folderPath, char *fileName){
    const char *lastSlash = strrchr(path, '/');

    if(!lastSlash || lastSlash == path){
        strcpy(folderPath, "/");
        strcpy(fileName, (lastSlash) ? lastSlash + 1 : path);
    }else{
        uint32_t folderLen = lastSlash - path;
        strncpy(folderPath, path, folderLen);
        folderPath[folderLen] = '\0';
        strcpy(fileName, lastSlash + 1);
    }

    return STATUS_CODE_OK;
}

StatusCode fs_resolve_path(const char* folderPath, uint32_t* path){
    printf("comparing: %s\n", folderPath);

    if (strcmp(folderPath, "/") == 0){
        printf("folder is root folder\n");
        *path = 0;
        return STATUS_CODE_OK;  
    }  //return the root directory
    char copy[MAX_PATH_LENGTH];
    strncpy(copy, folderPath, MAX_PATH_LENGTH);
    copy[MAX_PATH_LENGTH-1]='\0';

    char *currentFile = strtok(copy, "/");
    uint32_t currentBlock=0;    


    //start at root folder, get first file, and search all fileentries
    while(currentFile != NULL){
        BlockGroup *group = &blockGroups[currentBlock/BLOCKS_PER_GROUP];
        FileEntry *files = (FileEntry *) &group->dataBlocks[currentBlock % BLOCKS_PER_GROUP];
        int found=0;
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
            if (files[i].valid && strncmp(files[i].fileName, currentFile, MAX_FILENAME_LENGTH)==0){
                currentBlock=files[i].startBlockIndex;
                found=1;
                break;
            }
        }
        if (!found){
            *path = FS_INVALID_BLOCK;
            return STATUS_CODE_INVALID_ARGS;
        }
        currentFile=strtok(NULL, "/");
    }
    *path = currentBlock;
    return STATUS_CODE_OK;
}

StatusCode fs_list(const char *path){
    char folderPath[MAX_PATH_LENGTH];
    char folderName[MAX_FILENAME_LENGTH];

    fs_split_path(path, folderPath, folderName);

    //returns a index in global space, meaning the blockgroup is given by parentBlockLocation / BLOCKS_PER_GROUP, the block index is given by parentBlockLocation % BLOCKS_PER_GROUP
    uint32_t parentBlockLocation; 
    StatusCode status = fs_resolve_path(folderPath, &parentBlockLocation);
    if(status != STATUS_CODE_OK){
        return status;
    }

    if(parentBlockLocation == FS_INVALID_BLOCK) return STATUS_CODE_INVALID_ARGS;

    //the block group our directory is located in
    BlockGroup *parentGroup = &blockGroups[parentBlockLocation/BLOCKS_PER_GROUP];

    //Store the files in our folder block in an array of files
    //might need to add a try catch or something for this line in case the path doesnt lead to a folder
    FileEntry *File = (FileEntry *)&parentGroup->dataBlocks[parentBlockLocation % BLOCKS_PER_GROUP];

    //loop through files
    for(uint32_t i = 0; i < BLOCK_SIZE / sizeof(FileEntry); i++){
        if(File[i].valid){
            printf("%s\n", File[i].fileName);
        }
    }

    return STATUS_CODE_OK;
}