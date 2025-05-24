## @file    fota_packet.py
#  @date    2025-05-14
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Packet class for fota_packet_sender
#
#  @ingroup fota_validation

from crc32 import CRC32  # TODO: Resolve import error

# TODO: Stolen from can_datagram/bootloader_id, so check for correct code
STANDARD_CRC32_POLY = 0x04C11DB7

# Endianness for byte conversion
BYTE_ORDER = 'little'  # TODO: Assumed from can_datagram, so confirm

crc32 = CRC32(STANDARD_CRC32_POLY)

class FotaPacket():
    """
    @brief Defines serialized format of a FOTA packet 
    """
    SOF = 0xAA
    EOF = 0xBB
    MAX_PAYLOAD_BYTES = 64  # TODO: Placeholder, so confirm

    def __init__(self, packet_type: int, datagram_id: int, sequence_num: int, payload: bytes):
        """
        @brief Initialize FotaPacket object
        """

        # Validate byte sizes
        if not (0 <= packet_type <= 0xFF):
            raise ValueError("packet_type must be a single byte")
        
        if not (0 <= sequence_num <= 0x07):
            raise ValueError("sequence_num must be 3 bits")
        
        if not (0 <= datagram_id <= 0xFFFFFFFF):
            raise ValueError("datagram_id must fit in 4 bytes")
        
        if len(payload) > FotaPacket.MAX_PAYLOAD_BYTES:
            raise ValueError("payload exceeds user-defined limit")

        self.sof = FotaPacket.SOF
        self.packet_type = packet_type
        self.datagram_id = datagram_id
        self.sequence_num = sequence_num
        self.payload = payload
        self.payload_len = len(payload)
        self.crc32_value = crc32.calculate(payload)
        self.eof = FotaPacket.EOF

    def __repr__(self) -> str:
        """
        @brief 
        """
        return(
            f"<FotaPacket "
            f"SOF=0x{self.sof:02X}, "
            f"Type=0x{self.packet_type:02X}, "
            f"DatagramID=0x{self.datagram_id:08X}, "
            f"Seq=0x{self.sequence_num:02X}, "
            f"Len={self.payload_len}, "
            f"CRC32=0x{self.crc32_value:08X}, "
            f"EOF=0x{self.eof:02X}>"
        )

    def pack(self) -> bytearray:
        """
        @brief Serialize packet values for transmission
        """
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




