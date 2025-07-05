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
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1) -> None:
        """
        @brief Initialize FotaPacketSender with a serial connection
        """
        self.ser = serial.Serial(port = port, baudrate = baudrate, timeout = timeout)

    def __del__(self):
        """
        @brief Close serial connection when object is deleted or program exits
        """
        self.close()

    @staticmethod
    def create(packet_type: int, datagram_id: int, sequence_num: int, payload: bytes) -> FotaPacket:
        """
        @brief Factory for FotaPacket objects
        """
        return FotaPacket(packet_type, datagram_id, sequence_num, payload)

    def send(self, fota_packet: FotaPacket) -> None:
        """
        @brief Transmit FotaPacket to XBee
        """
        try:
            packet_bytes = fota_packet.pack()
            self.ser.write(packet_bytes)
            self.ser.flush()  # Ensure immediate transmission

            print(f"Sent FOTA packet: {fota_packet}")

        except serial.SerialException as e:
            print(f"Serial error during transmission: {e}")
        
        except Exception as e:
            print(f"Unexpected error during transmission: {e}")

    def close(self) -> None:
        """
        @brief Close serial connection
        """
        if self.ser.is_open:
            self.ser.close()
            print(f"\nConnection to port {self.ser.port} closed!\n")
            