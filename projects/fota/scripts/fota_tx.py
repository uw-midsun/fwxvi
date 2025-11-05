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
from fota_ack import FotaAck
from fota_rx import FotaRx


ACK_TIMEOUT = 5.0


class FotaTx():
    """ @brief Fota transmit class to send datagrams over XBee """
    def __init__(self, packet_sender: FotaPacketSender, serial_receive: FotaRx):
        self._packet_sender = packet_sender
        self._tx_queue = []
        self._serial_receive = serial_receive


    def send_packet(self, packet: FotaPacket) -> bool:
        """@brief send packet through serial """
        self._packet_sender.send(packet)

        ack = self.ack_wait()

        if ack.ack_status == 0:
            return True
        
        return False
    
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
                status = self._packet_sender.send(packet)

                if not status:
                    print("ERROR: could not send packet, or did not receive ack")
                    return False

            
            print("Finished transmitting Datagram")
            return True

        except Exception as error:
            print(f"Unexpected error during transmission: {error}")
            return False

    def ack_wait(self) -> FotaAck:
        buffer = bytearray()
        received_bytes = 0
        expected_len = 1 + 1 + 4 + 256 # from ACK struct, see fota_ack.py

        while received_bytes < expected_len:
            buffer += self._serial_receive.recieve()

        if received_bytes == expected_len:
            return FotaAck(buffer)
        
        print(f"ERROR: could not parse ack packet")
        return None
