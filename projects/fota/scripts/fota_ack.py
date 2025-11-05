# @file    fota_ack.py
#  @date    2025-11-05
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Core module for fota_python
#
#  @details
#
#  @ingroup FOTA_Python

import struct

FOTA_ERROR_MSG_LENGTH = 256
ACK_STRUCT_FORMAT = f"<BBI{FOTA_ERROR_MSG_LENGTH}s"

"""
Represents a FOTA Acknowledgement packet:
    typedef struct {
    uint8_t  ack_status;        // 0 == ACK, 1 == NACK
    uint8_t  response_to_type;  // Response to FotaDatagramType
    uint32_t error_code;        // FotaError enum value
    char     error_msg[256];    // Optional null-terminated message
    } FotaDatagramPayload_Acknowledgement;
"""

class FotaAck:
    def __init__(self, raw_bytes: bytes):
        expected_len = struct.calcsize(ACK_STRUCT_FORMAT)
        if len(raw_bytes) < expected_len:
            raise ValueError(f"Incomplete ACK recieved!")

        ack_status, response_to_type, err_code, err_msg = struct.unpack(
            ACK_STRUCT_FORMAT, raw_bytes[:expected_len]
        )

        self._ack_status = ack_status                      # 0 == ACK, 1 == NACK
        self._response_to_type = response_to_type
        self._err_code = err_code
        self._err_msg = err_msg.decode('utf-8').rstrip('\x00')

    @property
    def ack_status(self):
        return self._ack_status

    @property
    def response_to_type(self):
        return self._response_to_type
    
    @property
    def err_code(self):
        return self._err_code
    
    @property
    def err_msg(self):
        return self._err_msg
    

    def is_ack(self):
        return self._ack_status == 0