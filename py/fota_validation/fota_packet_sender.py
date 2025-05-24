## @file    fota_packet_sender.py
#  @date    2025-05-14
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Packet sender class for fota_datagram_sender
#
#  @ingroup fota_validation

import serial

from fota_packet import FotaPacket

class FotaPacketSender():
    """
    @brief Accessed by DatagramSender to transmit FotaPackets via XBees
    """

    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1):
        """
        @brief Initialize FotaPacketSender with a serial connection
        """

        self.serial_port = serial.Serial(port = port, baudrate = baudrate, timeout = timeout)

    def create_fota_packet(packet_type: int, datagram_id: int, sequence_num: int, payload: bytes) -> FotaPacket:
        """
        @brief Factory for FotaPacket objects
        """

        return FotaPacket(packet_type, datagram_id, sequence_num, payload)

    def send_fota_packet(fota_packet: FotaPacket):
        """
        @brief Transmit FotaPacket to XBee
        """

        pass