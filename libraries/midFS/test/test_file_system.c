/************************************************************************************************
 * @file   test_file_system.c
 *
 * @brief  Test file for file_system
 *
 * @date   2025-06-05
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "midFS.h"
#include "test_helpers.h"
#include "unity.h"
/* External library Headers */

/* Intra-component Headers */

StatusCode status;

void setup_test(void) {
  // Initialize the file system if needed
  status = fs_init();
  printf("%d", status);

  // Additional setup can be done here
  printf("Test setup complete.\n");
}

void teardown_test(void) {}

void test_superblock_and_first_block_setup(void) {
  TEST_ASSERT_NOT_NULL(superBlock);
  TEST_ASSERT_EQUAL_HEX32(0xC1D1921D, superBlock->magic);
  TEST_ASSERT_EQUAL(BLOCK_SIZE, superBlock->blockSize);
  TEST_ASSERT_EQUAL(BLOCKS_PER_GROUP, superBlock->blocksPerGroup);

  // Test offset is correct
  TEST_ASSERT_EQUAL_UINT8(*(fs_memory + sizeof(SuperBlock)), ((uint8_t *)blockGroups)[0]);

  FileEntry *root = &superBlock->rootFolderMetadata;
  TEST_ASSERT_EQUAL_STRING("/", root->fileName);
  TEST_ASSERT_EQUAL(FILETYPE_FOLDER, root->type);
  TEST_ASSERT_EQUAL(1, root->valid);
  TEST_ASSERT_EQUAL_UINT16(0, root->startBlockIndex);
  TEST_ASSERT_EQUAL_UINT32(0, root->size);

  BlockGroup *group0 = &blockGroups[0];
  TEST_ASSERT_EQUAL_UINT8(0, group0->nextBlockGroup);

  TEST_ASSERT_EQUAL_UINT8(1, group0->blockBitmap[0]);
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group0->blockBitmap[i]);
  }
  // All data blocks should be zeroed
  for (int i = 0; i < BLOCKS_PER_GROUP; i++) {
    for (int j = 0; j < BLOCK_SIZE; j++) {
      TEST_ASSERT_EQUAL_UINT8(0, group0->dataBlocks[i][j]);
    }
  }
}
