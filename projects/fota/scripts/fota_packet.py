# @file    fota_packet.py
#  @date    2025-05-14
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Packet class for fota_packet_sender
#
#  @ingroup FOTA_Python

from crc32 import CRC32

# Note: Stolen from can_datagram/bootloader_id, so check for correct code
STANDARD_CRC32_POLY = 0x04C11DB7

# Endianness for byte conversion
BYTE_ORDER = 'little'  # Note: Assumed from can_datagram, so confirm

CRC32_INST = CRC32(STANDARD_CRC32_POLY)


class FotaPacket():
    """
    @brief Defines serialized format of a FOTA packet
    """
    SOF = 0xAA
    EOF = 0xBB
    MAX_PAYLOAD_BYTES = 64  # Note: Placeholder, so confirm

    def __init__(self, packet_type: int, datagram_id: int, sequence_num: int, payload: bytes) -> None:
        """
        @brief Initialize FotaPacket object
        """
        self._check_args(packet_type, datagram_id, sequence_num, payload)

        self._packet_type = packet_type
        self._datagram_id = datagram_id
        self._sequence_num = sequence_num
        self._payload = payload
        self._payload_len = len(payload)
        self._crc32_value = CRC32_INST.calculate(payload)

    def __repr__(self) -> str:
        """
        @brief Readable representation of FotaPacket object
        """
        return (
            f"<FotaPacket "
            f"SOF=0x{FotaPacket.SOF:02X}, "
            f"Type=0x{self.packet_type:02X}, "
            f"DatagramID=0x{self.datagram_id:08X}, "
            f"Seq=0x{self.sequence_num:02X}, "
            f"Len={self.payload_len}, "
            f"CRC32=0x{self.crc32_value:08X}, "
            f"EOF=0x{FotaPacket.EOF:02X}>"
        )

    def pack(self) -> bytearray:
        """
        @brief Serialize packet values for transmission
        """
        packet = bytearray()

        packet.append(FotaPacket.SOF)
        packet.append(self.packet_type)
        packet += self.datagram_id.to_bytes(4, BYTE_ORDER)
        packet.append(self.sequence_num)
        packet += self.payload_len.to_bytes(2, BYTE_ORDER)
        packet += self.payload
        packet += self.crc32_value.to_bytes(4, BYTE_ORDER)
        packet.append(FotaPacket.EOF)

        return packet

    @property
    def packet_type(self):
        """
        @brief Packet type
        """
        return self._packet_type

    @property
    def datagram_id(self):
        """
        @brief Datagram ID number
        """
        return self._datagram_id

    @property
    def sequence_num(self):
        """
        @brief Packet sequence number
        """
        return self._sequence_num

    @property
    def payload_len(self):
        """
        @brief Packet payload length
        """
        return self._payload_len

    @property
    def payload(self):
        """
        @brief Packet payload
        """
        return self._payload

    @property
    def crc32_value(self):
        """
        @brief Packet CRC32 value
        """
        return self._crc32_value

    @staticmethod
    def _check_args(packet_type: int, datagram_id: int, sequence_num: int, payload: bytes) -> None:
        """
        @brief Validate byte sizes of object parameters
        """
        if not 0 <= packet_type <= 0xFF:
            raise ValueError("packet_type must be a single byte")

        if not 0 <= datagram_id <= 0xFFFFFFFF:
            raise ValueError("datagram_id must fit in 4 bytes")

        if not 0 <= sequence_num <= 0x07:
            raise ValueError("sequence_num must be 3 bits")

        if len(payload) > FotaPacket.MAX_PAYLOAD_BYTES:
            raise ValueError("payload exceeds user-defined limit")
