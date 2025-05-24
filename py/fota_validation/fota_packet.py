## @file    fota_packet_sender.py
#  @date    2025-05-14
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Packet class for fota_packet_sender
#
#  @ingroup fota_validation

import serial
from crc32 import CRC32

# Endianness for byte conversion
BYTE_ORDER = 'little'  # TODO: Assumed from can_datagram, so confirm

crc32 = CRC32(STANDARD_CRC32_POLY)

class FotaPacket():
    """
    @brief Defines serialized format of FOTA packet 
    """
    SOF = 0xAA
    EOF = 0xBB
    MAX_PAYLOAD_BYTES = 64  # TODO: Placeholder, so confirm

    def __init__(self, packet_type: int, datagram_id: int, sequence_num: int, payload: bytes):
        """Initialize FotaPacket class"""

        # Validate byte sizes
        if not (0 <= packet_type <= 0xFF):
            raise ValueError("packet_type must be a single byte")
        
        if not (0 <= sequence_num <= 0xFF):
            raise ValueError("sequence_num must be a single byte")
        
        if not (0 <= datagram_id <= 0xFFFFFFFF):
            raise ValueError("datagram_id must fit in 4 bytes")
        
        if len(payload) > FotaPacket.MAX_PAYLOAD_BYTES:
            raise ValueError("payload too long")

        self.sof = FotaPacket.SOF
        self.packet_type = packet_type
        self.datagram_id = datagram_id
        self.sequence_num = sequence_num
        self.payload = payload
        self.payload_len = len(payload)
        self.crc32_value = crc32.calculate(payload)
        self.eof = FotaPacket.EOF

    def pack(self) -> bytearray:
        """Serialize packet values"""
        packet = bytearray()

        packet.append(self.sof)
        packet.append(self.packet_type)
        packet += self.datagram_id.to_bytes(4, BYTE_ORDER)
        packet.append(self.sequence_num)
        packet += self.payload_len.to_bytes(2, BYTE_ORDER)
        packet += self.payload
        packet += self.crc32_value.to_bytes(4, BYTE_ORDER)
        packet.append(self.eof)

        return packet

    def create_fota_packet(packet_type: int, datagram_id: int, sequence_num: int, payload: bytes) -> FotaPacket:
        return FotaPacket(packet_type, datagram_id, sequence_num, payload)




