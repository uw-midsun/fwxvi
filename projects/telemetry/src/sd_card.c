

#include "sd_card.h"


StatusCode sd_card_init(SpiPort p) {

  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode sd_read_blocks(SpiPort p, uint8_t *dst, uint32_t lba, uint32_t count) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode sd_write_blocks(SpiPort p, const uint8_t *src, uint32_t lba, uint32_t count) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode sd_is_initialized(SpiPort p) {
  return STATUS_CODE_UNIMPLEMENTED;
}
