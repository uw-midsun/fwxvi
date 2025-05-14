## @file    fota_packet_sender.py
#  @date    2025-05-14
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Packet sender class for fota_datagram_sender
#
#  @ingroup fota_validation

import serial

from fota_packet import FotaPacket

class FotaPacketSender():
    def create_fota_packet(datagram_id, packet_type, sequence_num, payload):
        pass

    def send_fota_packet(fota_packet: FotaPacket):
        pass