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

#define TEST_ASSERT_BITMAP_PREFIX_1S_THEN_0S(bitmap, ones, total) \
  do {                                                            \
    for (int _i = 0; _i < (total); _i++) {                        \
      uint8_t expected = (_i < (ones)) ? 1 : 0;                   \
      TEST_ASSERT_EQUAL_UINT8(expected, (bitmap)[_i]);            \
    }                                                             \
  } while (0)

#define TEST_ASSERT_BITMAP_ALL_1S(bitmap)                                             \
  do {                                                                                \
    TEST_ASSERT_BITMAP_PREFIX_1S_THEN_0S(bitmap, BLOCKS_PER_GROUP, BLOCKS_PER_GROUP); \
  } while (0)

void setup_test(void) {
  // Initialize the file system if needed
  StatusCode status;
  status = fs_init();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status);

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

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  for (int i = 2; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  ret = fs_add_file("/crc2.txt", msg2, sizeof(msg2), FILETYPE_FILE);
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
    ret = fs_add_file(filenames[i], messages[i], sizeof(messages[i]), FILETYPE_FILE);
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
  ret = fs_add_file(filenames[BLOCKS_PER_GROUP - 1U], messages[BLOCKS_PER_GROUP - 1U], sizeof(messages[BLOCKS_PER_GROUP - 1U]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  for (int i = 0; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[i]);  // Block group 0 should be completely full
  }

  TEST_ASSERT_EQUAL(1, group_1->blockBitmap[0]);  // The content should be stored in block group 1 block 0
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_1->blockBitmap[i]);  // Block group 1 should be empty after block 0
  }

  ret = fs_add_file(filenames[BLOCKS_PER_GROUP], messages[BLOCKS_PER_GROUP], sizeof(messages[BLOCKS_PER_GROUP]), FILETYPE_FILE);
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
  ret = fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = fs_add_file("/crc.txt", (uint8_t *)"CRCPOLY", 8, FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ret);
}

TEST_IN_TASK
void test_add_multi_file_past_block_limit_with_valid_args_expect_return_success(void) {
  /**
   * This test should demonstrate that adding more than (BLOCK_SIZE / sizeof(FileEntry)) files
   * to a directory should expand the folder into a new block
   */
  StatusCode ret = STATUS_CODE_OK;

  // Currently, BLOCK_SIZE = 512 and sizeof(FileEntry) = 44, so we can fit 11 files
  printf("BLOCK_SIZE: %u, sizeof(FileEntry): %lu", BLOCK_SIZE, sizeof(FileEntry));

  const uint8_t messages[][13U] = {
    "CRCPOLY01", "CRCPOLY02", "CRCPOLY03", "CRCPOLY04", "CRCPOLY05", "CRCPOLY06", "CRCPOLY07", "CRCPOLY08", "CRCPOLY09", "CRCPOLY10", "CRCPOLY11", "CRCPOLY12", "CRCPOLY13",
  };
  const char filenames[][13U] = {
    "/a.txt", "/b.txt", "/c.txt", "/d.txt", "/e.txt", "/f.txt", "/g.txt", "/h.txt", "/i.txt", "/j.txt", "/k.txt", "/l.txt", "/m.txt",
  };

  BlockGroup *group_0 = &blockGroups[0];
  BlockGroup *group_1 = &blockGroups[1];

  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);  // These blocks should be empty at first
  }

  for (uint8_t i = 0; i < 10U; i++) {
    ret = fs_add_file(filenames[i], messages[i], sizeof(messages[i]), FILETYPE_FILE);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  }

  /**
   * At this point we have added 10 files, meaning that we have taken up 11 blocks:
   *  1 block for the root directory + 10 blocks for data
   *  Since we have 8 blocks per group, we expect block group 0 to be full and the
   *  first 3 blocks of block group 0 to be full
   */

  TEST_ASSERT_BITMAP_ALL_1S(group_0->blockBitmap);

  TEST_ASSERT_BITMAP_PREFIX_1S_THEN_0S(group_1->blockBitmap, 3, BLOCKS_PER_GROUP);

  ret = fs_add_file(filenames[10], messages[10], sizeof(messages[10]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Since we have added another file past the files per block limit (9), the root directory should
   * now take up 2 blocks, we should expect 12 blocks to be taken up now, 8 + 4
   */

  TEST_ASSERT_BITMAP_PREFIX_1S_THEN_0S(group_1->blockBitmap, 5, BLOCKS_PER_GROUP);

  ret = fs_add_file(filenames[11], messages[11], sizeof(messages[11]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Now there is enough space in the root directory, so this operation should only consume 1 block
   */

  TEST_ASSERT_BITMAP_PREFIX_1S_THEN_0S(group_1->blockBitmap, 6, BLOCKS_PER_GROUP);

  // read everything back and check
  for (uint8_t i = 0; i < 12U; i++) {
    uint8_t message_rc[sizeof(messages[i])] = { 0U };
    ret = fs_read_file(filenames[i], message_rc);
    if (ret != STATUS_CODE_OK) {
      printf("%u\n", i);
    }
    // TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[i], message_rc, sizeof(messages[i]));
  }
}

TEST_IN_TASK
void test_add_file_with_long_content_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t msg[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc et scelerisque lacus. Sed non felis eget ante posuere ultricies ut et mi. Aliquam vulputate blandit dui in convallis. Etiam "
      "tellus diam, ullamcorper ac sapien at, mattis blandit urna. Integer mi velit, malesuada non luctus eget, mattis feugiat orci. Nulla semper blandit orci, et vestibulum ante ultrices at. Mauris "
      "ornare ut velit ac sodales. Donec dapibus, justo sit amet posuere elementum, libero sem venenatis magna, sit amet fermentum est risus integer.";

  BlockGroup *group_0 = &blockGroups[0];
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);  // These blocks should be empty at first
  }

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // The new file content should overflow into block 2 of group 0
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly[sizeof(msg)] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(msg, crc_poly, sizeof(msg));
}

TEST_IN_TASK
void test_read_file_with_nonexistent_name_expect_return_invalid_args(void) {
  StatusCode ret = STATUS_CODE_OK;
  uint8_t crc_poly[8] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ret);
}

TEST_IN_TASK
void test_write_file_with_valid_conditions_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t msg[] = "CRCPOLY";
  const uint8_t newmsg[] = "Some new message...";

  BlockGroup *group_0 = &blockGroups[0];

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  for (int i = 2; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly[sizeof(msg)] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(msg, crc_poly, sizeof(msg));

  ret = fs_write_file("/crc.txt", newmsg, sizeof(newmsg));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  // Check that blocks are still the same
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  for (int i = 2; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t full[sizeof(msg) + sizeof(newmsg)] = { 0 };
  uint8_t expected[sizeof(msg) + sizeof(newmsg)] = { 0 };
  memcpy(&expected[0], msg, sizeof(msg));
  memcpy(&expected[sizeof(msg)], newmsg, sizeof(newmsg));

  ret = fs_read_file("/crc.txt", full);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, full, sizeof(expected));
}

TEST_IN_TASK
void test_write_file_to_overflow_with_valid_conditions_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t msg[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc et scelerisque lacus. Sed non felis eget ante posuere ultricies ut et mi. Aliquam vulputate blandit dui in convallis. Etiam "
      "tellus diam, ullamcorper ac sapien at, mattis blandit urna. Integer mi velit, malesuada non luctus eget, mattis feugiat orci. Nulla semper blandit orci, et vestibulum ante ultrices at. Mauris "
      "ornare ut velit ac sodales. Donec dapibus, justo sit amet posuere elementum, libero sem venenatis magna, sit amet";
  const uint8_t newmsg[] = "fermentum est risus integer. CRCPOLY";

  BlockGroup *group_0 = &blockGroups[0];

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  for (int i = 2; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly[sizeof(msg)] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(msg, crc_poly, sizeof(msg));

  ret = fs_write_file("/crc.txt", newmsg, sizeof(newmsg));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // The new file content should now overflow to block 2 of group 0
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t full[sizeof(msg) + sizeof(newmsg)] = { 0 };
  uint8_t expected[sizeof(msg) + sizeof(newmsg)] = { 0 };
  memcpy(&expected[0], msg, sizeof(msg));
  memcpy(&expected[sizeof(msg)], newmsg, sizeof(newmsg));

  ret = fs_read_file("/crc.txt", full);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, full, sizeof(expected));
}

TEST_IN_TASK
void test_write_file_to_overflow_with_no_contiguous_memory_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t msg[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc et scelerisque lacus. Sed non felis eget ante posuere ultricies ut et mi. Aliquam vulputate blandit dui in convallis. Etiam "
      "tellus diam, ullamcorper ac sapien at, mattis blandit urna. Integer mi velit, malesuada non luctus eget, mattis feugiat orci. Nulla semper blandit orci, et vestibulum ante ultrices at. Mauris "
      "ornare ut velit ac sodales. Donec dapibus, justo sit amet posuere elementum, libero sem venenatis magna, sit amet";
  const uint8_t newmsg[] = "fermentum est risus integer. CRCPOLY";
  const uint8_t bufmsg[] = "I am blocking the way...";

  BlockGroup *group_0 = &blockGroups[0];

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = fs_add_file("/buff.txt", bufmsg, sizeof(bufmsg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The new file content should be stored in block 1 of group 0
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // The new file content should now overflow to block 2 of group 0
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  ret = fs_write_file("/crc.txt", newmsg, sizeof(newmsg));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Since crc.txt is located before buff.txt and fs_write_file(newmsg) causes the crc.txt to take up 2 blocks instead of 1.
   * buff.txt is blocking the contiguous write so crc.txt must be relocated to blocks 3 & 4, leaving block 1 empty
   */

  TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[1]);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[3]);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[4]);
  for (int i = 5; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  /**
   * Check that the content wrote successfully
   */
  uint8_t full[sizeof(msg) + sizeof(newmsg)] = { 0 };
  uint8_t expected[sizeof(msg) + sizeof(newmsg)] = { 0 };
  memcpy(&expected[0], msg, sizeof(msg));
  memcpy(&expected[sizeof(msg)], newmsg, sizeof(newmsg));

  ret = fs_read_file("/crc.txt", full);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, full, sizeof(expected));

  /**
   * Write another file, 1 block wide, verify that it fills the space in block 1
   */
  ret = fs_add_file("/bufftwo.txt", bufmsg, sizeof(bufmsg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[3]);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[4]);
  for (int i = 5; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }
}

TEST_IN_TASK
void test_write_file_with_nonexistent_name_expect_return_invalid_args(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t newmsg[] = "fermentum est risus integer. CRCPOLY";
  ret = fs_write_file("/crc.txt", newmsg, sizeof(newmsg));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ret);
}

TEST_IN_TASK
void test_add_and_delete_file_with_valid_args_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t msg[] = "CRCPOLY";
  const uint8_t msg2[] = "CRCPOLY2";

  BlockGroup *group_0 = &blockGroups[0];
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);  // These blocks should be empty at first
  }

  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = fs_add_file("/crc2.txt", msg2, sizeof(msg2), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = fs_delete_file("/crc.txt");
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[1]);  // crc.txt block should be empty now since it was deleted
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // crc.txt block should still be full
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly[sizeof(msg)] = { 0 };
  ret = fs_read_file("/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ret);

  uint8_t crc_poly2[sizeof(msg2)] = { 0 };

  ret = fs_read_file("/crc2.txt", crc_poly2);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  /**
   * Add back crc.txt, 1 block wide, verify that it fills the space in block 1
   */
  ret = fs_add_file("/crc.txt", msg, sizeof(msg), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // crc.txt block should be empty now since it was deleted
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // crc.txt block should still be full
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }
}

TEST_IN_TASK
void test_delete_file_with_nonexistent_name_expect_return_invalid_args(void) {
  StatusCode ret = STATUS_CODE_OK;
  ret = fs_delete_file("/crc.txt");
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ret);
}

TEST_IN_TASK
void test_nested_directories_and_recursive_delete_expect_return_success(void) {
  StatusCode ret = STATUS_CODE_OK;
  const uint8_t messages[][BLOCKS_PER_GROUP + 1U] = {
    "CRCPOLY1", "CRCPOLY2", "CRCPOLY3", "CRCPOLY4", "CRCPOLY5", "CRCPOLY6", "CRCPOLY7", "CRCPOLY8", "CRCPOLY9",
  };

  BlockGroup *group_0 = &blockGroups[0];
  for (int i = 1; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);  // These blocks should be empty at first
  }

  ret = fs_add_file("/crcs", NULL, 0, FILETYPE_FOLDER);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  ret = fs_add_file("/crcs/crc.txt", messages[0], sizeof(messages[0]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[1]);  // The crcs directory is stored here
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[2]);  // crc.txt should be stored here
  for (int i = 3; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  ret = fs_add_file("/crc2.txt", messages[1], sizeof(messages[1]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[3]);  // The new file content should be stored in block 3 of group 0
  for (int i = 4; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly[sizeof(messages[0])] = { 0 };
  ret = fs_read_file("/crcs/crc.txt", crc_poly);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[0], crc_poly, sizeof(messages[0]));

  uint8_t crc_poly2[sizeof(messages[1])] = { 0 };

  ret = fs_read_file("/crc2.txt", crc_poly2);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[1], crc_poly2, sizeof(messages[1]));

  ret = fs_add_file("/crcs/my_crcs", NULL, 0, FILETYPE_FOLDER);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[4]);  // The my_crcs directory should be stored here
  for (int i = 5; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  ret = fs_add_file("/crcs/my_crcs/crc.txt", messages[2], sizeof(messages[2]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);

  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[5]);  // The content of crc.txt should be stored here
  for (int i = 6; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  uint8_t crc_poly3[sizeof(messages[2])] = { 0 };

  ret = fs_read_file("/crcs/my_crcs/crc.txt", crc_poly3);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(messages[2], crc_poly3, sizeof(messages[2]));

  ret = fs_add_file("/crc3.txt", messages[3], sizeof(messages[3]), FILETYPE_FILE);
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[6]);  // The new file content should be stored in block 3 of group 0
  for (int i = 7; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }

  ret = fs_delete_file("/crcs");
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ret);
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[0]);  // The new file metadata should be stored in the root directory
  TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[1]);  // The crcs directory is stored here
  TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[2]);  // crc.txt should be stored here
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[3]);  // crc2.txt was not deleted and is stored here
  TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[4]);  // The my_crcs directory should be stored here
  TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[5]);  // The content of crc.txt should be stored here
  TEST_ASSERT_EQUAL_UINT8(1, group_0->blockBitmap[6]);  // The new file content should be stored in block 3 of group 0

  for (int i = 7; i < BLOCKS_PER_GROUP; i++) {
    TEST_ASSERT_EQUAL_UINT8(0, group_0->blockBitmap[i]);
  }
}
