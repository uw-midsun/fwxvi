# @file    fota_datagram.py
#  @date    2025-09-12
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Core datagram generator for client side
#
#  @details
#
#  @ingroup FOTA_Python

from crc32 import CRC32
from fota_packet import FotaPacket


STANDARD_CRC32_POLY = 0x04C11DB7
BYTE_ORDER = 'little' 
CRC32_INST = CRC32(STANDARD_CRC32_POLY)


class FotaDatagramHeader():
    """
    @brief Defined format for the datagram header
    """
    def __init__(self, target_node, length, num_packets, datagram_crc32, datagram_id):
        self._target_node = target_node
        self._type = 0
        self._length = length
        self._num_packets = num_packets
        self._datagram_crc32 = datagram_crc32
        self._datagram_id = datagram_id
        self._header_packet = None

        
    @property
    def target_node(self):
        return self._target_node

    @property
    def type(self):
        return self._type

    @property
    def length(self):
        return self._length

    @property
    def num_packets(self):
        return self._num_packets

    @property
    def datagram_crc32(self):
        return self._datagram_crc32

    @property
    def datagram_id(self):
        return self._datagram_id

    @property
    def header_packet(self):
        return self._header_packet
    
    def create_header_packet(self, datagram_crc32):
        # Placeholder payload with 0 for packet_crc32
        header_payload = (
            self._target_node.to_bytes(1, BYTE_ORDER) +          # target_node_id (1 byte)
            self._type.to_bytes(1, BYTE_ORDER) +                 # type (1 byte)
            self._num_packets.to_bytes(2, BYTE_ORDER) +          # num_packets (2 bytes)
            self._datagram_id.to_bytes(4, BYTE_ORDER) +          # datagram_id (4 bytes)
            self._length.to_bytes(4, BYTE_ORDER) +               # total_length (4 bytes)
            datagram_crc32.to_bytes(4, BYTE_ORDER) +             # datagram_crc32 (4 bytes)
            (0).to_bytes(4, BYTE_ORDER)                          # packet_crc32 placeholder (4 bytes)
        )

        # Compute CRC32 over the payload excluding the last 4 bytes (packet_crc32)
        packet_crc32 = CRC32_INST.calculate(header_payload[:-4])

        # Rebuild the header payload with actual packet_crc32
        header_payload = (
            self._target_node.to_bytes(1, BYTE_ORDER) +
            self._type.to_bytes(1, BYTE_ORDER) +
            self._num_packets.to_bytes(2, BYTE_ORDER) +
            self._datagram_id.to_bytes(4, BYTE_ORDER) +
            self._length.to_bytes(4, BYTE_ORDER) +
            datagram_crc32.to_bytes(4, BYTE_ORDER) +
            packet_crc32.to_bytes(4, BYTE_ORDER)
        )

        # Create the header FotaPacket
        self._header_packet = FotaPacket(
            packet_type=0,       
            datagram_id=self._datagram_id,
            sequence_num=0,         
            payload=header_payload
        )

    
class FotaDatagram():
    """
    @brief Defined serialized format for the datagrams
    """

    def __init__(self, datagram_header = None):
        """_packet_list does not contain the datagram header packet"""
        
        self._datagram_header = datagram_header
        self._packet_list = []
        self._packet_num = 0
        self._complete_send = False


    def bin_to_datagram(self, path, datagram_id, node_id) -> None:
        with open(path, "rb") as file:
            data = file.read(128)
            seq_num = 1
            total_len = 0

            while data:
                packet = FotaPacket(0, datagram_id, seq_num, data)
                
                self._packet_list.append(packet)
                seq_num += 1
                total_len += packet.payload_len

                data = file.read(128)

        self._packet_num = len(self._packet_list)
        

        datagram_crc = self.calculate_datagram_crc32()

        if self._datagram_header == None:
            self._datagram_header = FotaDatagramHeader(node_id, total_len, self._packet_num, datagram_crc, datagram_id)
        else:
            self._datagram_header._target_node = node_id
            self._datagram_header._type = 0
            self._datagram_header._length = total_len
            self._datagram_header._num_packets = self._packet_num
            self._datagram_header._datagram_crc32 = datagram_crc
            self._datagram_header._datagram_id = datagram_id
            
            self._datagram_header.create_header_packet(datagram_crc)
        
    def calculate_datagram_crc32(self) -> int:
        payloads = bytearray()

        # don't include header packet
        for packet in self._packet_list[1:]:
            payloads += packet.payload

        return CRC32_INST.calculate(payloads)
