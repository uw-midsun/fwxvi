/************************************************************************************************
 * @file   can_hw.c
 *
 * @brief  Source file for CAN HW Interface
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <string.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "can_hw.h"


StatusCode can_hw_init(const CanQueue* rx_queue, const CanSettings *settings) {
    return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode can_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended) {
    return STATUS_CODE_UNIMPLEMENTED;
}

CanHwBusStatus can_hw_bus_status(void) {
    return CAN_HW_BUS_STATUS_OK;
}


StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
    return STATUS_CODE_UNIMPLEMENTED;
}

bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len) {
    return true;
}
