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

# pylint: disable=R0902
# pylint: disable=R0917
class FotaDatagramHeader():
    """
    @brief Defined format for the datagram header
    """
    # pylint: disable=too-many-arguments

    def __init__(self, target_node, length, num_packets, datagram_crc32, datagram_id):
        """@brief FotaDatagramHeader initiali"""
        self._target_node = target_node
        self._type = 0
        self._length = length
        self._num_packets = num_packets
        self._datagram_crc32 = datagram_crc32
        self._datagram_id = datagram_id
        self._header_packet = None

    @property
    def target_node(self):
        """@brief GETTER for target node """
        return self._target_node

    @target_node.setter
    def target_node(self, new_val):
        """@brief SETTER for the target node ID of the datagram"""
        self._target_node = new_val

    @property
    def type(self):
        """@brief GETTER for the datagram type"""
        return self._type

    @type.setter
    def type(self, new_val):
        """@brief SETTER for the datagram type"""
        self._type = new_val

    @property
    def length(self):
        """@brief GETTER for the total length (in bytes) of the datagram"""
        return self._length

    @length.setter
    def length(self, new_val):
        """@brief SETTER for the total datagram length"""
        self._length = new_val

    @property
    def num_packets(self):
        """@brief GETTER for the number of packets in the datagram"""
        return self._num_packets

    @num_packets.setter
    def num_packets(self, new_val):
        """@brief SETTER for the number of packets in the datagram"""
        self._num_packets = new_val

    @property
    def datagram_crc32(self):
        """@brief GETTER for the CRC32 checksum of the datagram"""
        return self._datagram_crc32

    @datagram_crc32.setter
    def datagram_crc32(self, new_val):
        """@brief SETTER for the datagram CRC32 checksum"""
        self._datagram_crc32 = new_val

    @property
    def datagram_id(self):
        """@brief GETTER for the datagram ID"""
        return self._datagram_id

    @datagram_id.setter
    def datagram_id(self, new_val):
        """@brief SETTER for the datagram ID"""
        self._datagram_id = new_val

    @property
    def header_packet(self):
        """@brief GETTER for the FotaPacket that represents the datagram header"""
        return self._header_packet

    @header_packet.setter
    def header_packet(self, new_val):
        """@brief SETTER for the datagram header FotaPacket"""
        self._header_packet = new_val

    def create_header_packet(self, datagram_crc32):
        """@brief creates the header packet given the datagramcrc32 """
        # Placeholder payload with 0 for packet_crc32
        header_payload = (
            self.target_node.to_bytes(1, BYTE_ORDER) +          # target_node_id (1 byte)
            self.type.to_bytes(1, BYTE_ORDER) +                 # type (1 byte)
            self.num_packets.to_bytes(2, BYTE_ORDER) +          # num_packets (2 bytes)
            self.datagram_id.to_bytes(4, BYTE_ORDER) +          # datagram_id (4 bytes)
            self.length.to_bytes(4, BYTE_ORDER) +               # total_length (4 bytes)
            datagram_crc32.to_bytes(4, BYTE_ORDER) +             # datagram_crc32 (4 bytes)
            (0).to_bytes(4, BYTE_ORDER)                          # packet_crc32 placeholder (4 bytes)
        )

        # Compute CRC32 over the payload excluding the last 4 bytes (packet_crc32)
        packet_crc32 = CRC32_INST.calculate(header_payload[:-4])

        # Rebuild the header payload with actual packet_crc32
        header_payload = (
            self.target_node.to_bytes(1, BYTE_ORDER) +
            self.type.to_bytes(1, BYTE_ORDER) +
            self.num_packets.to_bytes(2, BYTE_ORDER) +
            self.datagram_id.to_bytes(4, BYTE_ORDER) +
            self.length.to_bytes(4, BYTE_ORDER) +
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

    def __init__(self, datagram_header=None):
        """@brief initialize the fota datagram class object"""

        self._datagram_header = datagram_header
        self._packet_list = []
        self._packet_num = 0
        self._complete_send = False

    @property
    def datagram_header(self):
        """@brief GETTER for datagram header """
        return self._datagram_header

    @property
    def packet_list(self):
        """@brief GETTER for the list of packets that make up the datagram"""
        return self._packet_list

    @property
    def packet_num(self):
        """@brief GETTER for the total number of packets in the datagram"""
        return self._packet_num

    @property
    def complete_send(self):
        """@brief GETTER to check if the datagram has been fully transmitted"""
        return self._complete_send

    def bin_to_datagram(self, path, datagram_id, node_id) -> None:
        """@brief given a path, turn the binary into the datagram"""
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

        if self.datagram_header is None:
            self.datagram_header = FotaDatagramHeader(node_id, total_len, self._packet_num, datagram_crc, datagram_id)
        else:
            self.datagram_header.target_node = node_id
            self.datagram_header.type = 0
            self.datagram_header.length = total_len
            self.datagram_header.num_packets = self._packet_num
            self.datagram_header.datagram_crc32 = datagram_crc
            self.datagram_header.datagram_id = datagram_id

            self.datagram_header.create_header_packet(datagram_crc)

    def calculate_datagram_crc32(self) -> int:
        """@brief create the crc32 from datagram"""
        payloads = bytearray()

        # don't include header packet
        for packet in self._packet_list[1:]:
            payloads += packet.payload

        return CRC32_INST.calculate(payloads)
