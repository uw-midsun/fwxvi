## @file    fota_packet_sender.py
#  @date    2025-05-14
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Packet class for fota_packet_sender
#
#  @ingroup fota_validation

import serial
from crc32 import CRC32

# Frame markers
SOF = 0xAA
EOF = 0xBB

# Endianness for byte conversion
BYTEORDER = 'little'  # TODO: Assumed from can_datagram, so confirm

# Size limit for payload
MAX_PAYLOAD_BYTES = 64  # TODO: Confirm

crc32 = CRC32(STANDARD_CRC32_POLY)

class FotaPacket():
    """
    @brief Defines serialized format of FOTA packet 
    """

    def __init__(self, packet_type: int, datagram_id: int, sequence_num: int, payload: bytes):
        """Initialize FotaPacket class"""
        self.sof = FotaPacket.SOF
        self.packet_type = packet_type
        self.datagram_id = datagram_id
        self.sequence_num = sequence_num
        self.payload = payload
        self.payload_len = len(payload)
        self.crc32 = crc32.calculate(payload)
        self.eof = FotaPacket.EOF

    def serialize(self) -> bytearray:
        packet = bytearray()

        packet.append(self.sof)
        packet.append(self.packet_type)
        packet += self.datagram_id.to_bytes(4, byteorder='little')  # Assumed little endian from can_datagram
        packet.append(self.sequence_num)
        packet += self.payload_len.to_bytes(2, byteorder='little')
        packet += self.payload
        packet += self.crc32.to_bytes(4, byteorder='little')
        packet.append(self.eof)

        return packet




