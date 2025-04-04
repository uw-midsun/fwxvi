
#include "stm32l433xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal_rcc.h"

#include <string.h>
#include "packet_manager.h"
#include "network_buffer.h"
#include "fota_error.h"


static CRC_HandleTypeDef s_crc_handle;

FotaError packet_manager_init(PacketManager *packet_manager) {
    __HAL_RCC_CRC_CLK_ENABLE();
    s_crc_handle.Instance = CRC;
    s_crc_handle.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    s_crc_handle.Init.CRCLength = CRC_POLYLENGTH_32B;
    s_crc_handle.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
    s_crc_handle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    s_crc_handle.Init.InitValue = 0xFFFFFFFFU;
    s_crc_handle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    s_crc_handle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  
    if (HAL_CRC_Init(&s_crc_handle) != HAL_OK) {
      return FOTA_ERROR_INTERNAL;
    }
  
    return FOTA_ERROR_SUCCESS;
  

} 

FotaError crc_check(NetworkBuffer *net_buf, uint32_t received_crc){
    if(net_buf==NULL) return FOTA_ERROR_INVALID_ARGS;
    uint32_t computed_crc = HAL_CRC_Calculate(&s_crc_handle, (uint32_t *)net_buf->data, net_buf->num_items);


    if (computed_crc != received_crc) {
      return FOTA_ERROR_CRC_MISMATCH;
    }

    return FOTA_ERROR_SUCCESS;
}


FotaError packet_manager_encode(Packet *packet, NetworkBuffer *net_buff){
    return encode_packet(packet, net_buff);
}

FotaError packet_manager_decode(Packet *packet, NetworkBuffer *net_buff){
    return decode_packet(packet, net_buff);
}
FotaError transmit_packet(PacketManager *packet_manager){
//TODO
}
