#include <stdint.h>
#include "midFS_types.h"

SuperBlock superBlock = {
    .magic = 0xC1D1921D,
    .maxFiles = MAX_FILES_PER_GROUP * BLOCKS_PER_GROUP * TOTAL_BLOCKS,
    .blockSize = BLOCK_SIZE,
    .blocksPerGroup = BLOCKS_PER_GROUP,
    .totalSize = superBlock.maxFiles,
    .rootIndex = 0
};