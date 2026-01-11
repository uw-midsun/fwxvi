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

  // Additional setup can be done here
  printf("Test setup complete.\n");
}

void teardown_test(void) {}

TEST_IN_TASK
void test_superblock_and_first_block_setup(void) {
  TEST_ASSERT_NOT_NULL(superBlock);
  TEST_ASSERT_EQUAL_HEX32(0xC1D1921D, superBlock->magic);
  TEST_ASSERT_EQUAL(BLOCK_SIZE, superBlock->blockSize);
  TEST_ASSERT_EQUAL(BLOCKS_PER_GROUP, superBlock->blocksPerGroup);

  // Test offset is correct
  TEST_ASSERT_EQUAL_UINT32(sizeof(SuperBlock), (uint32_t)((uint8_t *)blockGroups - (uint8_t *)superBlock));

  FileEntry *root = &superBlock->rootFolderMetadata;
  TEST_ASSERT_EQUAL_STRING("/", root->fileName);
  TEST_ASSERT_EQUAL(FILETYPE_FOLDER, root->type);
  TEST_ASSERT_EQUAL(1, root->valid);
  TEST_ASSERT_EQUAL_UINT16(0, root->startBlockIndex);
  TEST_ASSERT_EQUAL_UINT32(0, root->size);

  BlockGroup *group_0 = &blockGroups[0];
  TEST_ASSERT_EQUAL(FS_NULL_BLOCK_GROUP, group_0->nextBlockGroup);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }
  // All data blocks should be zeroed
  for (int i = 0; i < BLOCKS_PER_GROUP; i++) {
    for (int j = 0; j < BLOCK_SIZE; j++) {
      TEST_ASSERT_EQUAL_UINT8(0, group_0->dataBlocks[i][j]);
    }
  }
}

TEST_IN_TASK
void test_add_file_with_valid_args_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t msg[] = "CRCPOLY";
  const uint8_t msg2[] = "CRCPOLY2";

  BlockGroup *group_0 = &blockGroups[0];
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);  // These blocks should be empty at first
  }

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), 0);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  for (int i = 2; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  ret = fs_add_file("/crc2.txt", msg2, sizeof(msg2), 0);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // The new file content should be stored in block 2 of group 0
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly[sizeof(msg)] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(msg, crc_poly, sizeof(msg));

  uint8_t crc_poly2[sizeof(msg2)] = { 0 };

  ret = fs_read_file("/crc2.txt", crc_poly2);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(msg2, crc_poly2, sizeof(msg2));
}

TEST_IN_TASK
void test_add_multi_file_with_valid_args_expect_return_success(void) {
  /**
   * This test should demonstrate that adding more than (BLOCKS_PER_GROUP - 1) files to the filesystem
   * should start adding data to the next block group
   */
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t messages[][BLOCKS_PER_GROUP + 1U] = {
    "CRCPOLY1", "CRCPOLY2", "CRCPOLY3", "CRCPOLY4", "CRCPOLY5", "CRCPOLY6", "CRCPOLY7", "CRCPOLY8", "CRCPOLY9",
  };
  const char filenames[][BLOCKS_PER_GROUP + 1U] = {
    "/a.txt", "/b.txt", "/c.txt", "/d.txt", "/e.txt", "/f.txt", "/g.txt", "/h.txt", "/i.txt",
  };

  BlockGroup *group_0 = &blockGroups[0];
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);  // These blocks should be empty at first
  }

  for (uint8_t i = 0; i < BLOCKS_PER_GROUP - 1U; i++) {
    ret = fs_add_file(filenames[i], messages[i], sizeof(messages[i]), 0);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

    for (int j = 1; j <= i + 1; j++) {
      TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[j]);
    }
    for (int j = i + 2; j < BLOCKS_PER_GROUP; j++) {
      TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[j]);
    }
  }

  // At this point, block group 0 should be full, and now for any files we add, the content should be written to block group 1

  BlockGroup *group_1 = &blockGroups[1];
  ret = fs_add_file(filenames[BLOCKS_PER_GROUP - 1U], messages[BLOCKS_PER_GROUP - 1U], sizeof(messages[BLOCKS_PER_GROUP - 1U]), 0);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  for (int i = 0; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[i]);  // Block group 0 should be completely full
  }

  TEST_ASSERT_EQUAL(1, group_1->blockBitmap[0]);  // The content should be stored in block group 1 block 0
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_1->blockBitmap[i]);  // Block group 1 should be empty after block 0
  }

  ret = fs_add_file(filenames[BLOCKS_PER_GROUP], messages[BLOCKS_PER_GROUP], sizeof(messages[BLOCKS_PER_GROUP]), 0);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  for (int i = 0; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[i]);  // Block group 0 should be completely full
  }

  TEST_ASSERT_EQUAL(1, group_1->blockBitmap[1]);  // The content should be stored in block group 1 block 1
  for (int i = 2; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_1->blockBitmap[i]);  // Block group 1 should be empty after block 0
  }

  // read everything back and check

  for (uint8_t i = 0; i < BLOCKS_PER_GROUP + 1U; i++) {
    uint8_t message_rc[sizeof(messages[i])] = { 0U };
    ret = fs_read_file(filenames[i], message_rc);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[i], message_rc, sizeof(messages[i]));
  }
}

TEST_IN_TASK
void test_add_file_with_duplicate_name_expect_return_invalid_args(void) {
  StatusCode ret = STATUS_CODE_OK;
  ret = fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, 0);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, 0);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ret);
}

TEST_IN_TASK
void test_read_file_with_nonexistent_name_expect_return_unreachable(void) {
  StatusCode ret = STATUS_CODE_OK;
  uint8_t crc_poly[8] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_UNREACHABLE, ret);
}
