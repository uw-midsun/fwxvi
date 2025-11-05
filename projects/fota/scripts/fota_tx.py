# @file    fota_tx.py
#  @date    2025-03-12
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Core module for fota_python
#
#  @details
#
#  @ingroup FOTA_Python

from fota_datagram import FotaDatagram
from fota_packet import FotaPacket
from fota_packet_sender import FotaPacketSender


class FotaTx():
    """ @brief Fota transmit class to send datagrams over XBee """
    def __init__(self, transport, packet_sender: FotaPacketSender):
        self._transport = transport
        self._packet_sender = packet_sender
        self._tx_queue = []
        self._active = False


    def send_packet(self, packet: FotaPacket) -> bool:
        """@brief send packet through serial """
        return self._packet_sender.send(packet)
    
    def queue_datagram(self, datagram: FotaDatagram):
        """@brief pack a datagram into a queue to send over transmit layer """
        transmit_queue = []

        transmit_queue.append(datagram._datagram_header)

        for packet in datagram._packet_list:
            transmit_queue.append(packet)

    def transmit(self) -> bool:
        """@brief start tranmission of anything in tx_queue"""
        
        if not self._tx_queue:
            print("Nothing in queue to transmit")
            return False

        print("Transmitting datagram")

        try:
            for packet in self._tx_queue:
                self._packet_sender.send(packet)

            
            print("Finished transmitting Datagram")
            return True

        except Exception as error:
            print(f"Unexpected error during transmission: {error}")
            return False

