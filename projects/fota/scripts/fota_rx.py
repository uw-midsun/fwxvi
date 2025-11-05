# @file    fota_rx.py
#  @date    2025-11-05
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Core module for fota_python
#
#  @details
#
#  @ingroup FOTA_Python

import serial


class FotaRx:
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1) -> None:
        """
        @brief Initialize FotaPacketSender with a serial connection
        """
        self.ser = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)

    
    def __del__(self):
        """
        @brief Close serial connection when object is deleted or program exits
        """
        self.close()


    def recieve(self):
        if self.ser.in_waiting > 0:
            return self.ser.read(self.ser.in_waiting)
        
        return None
    

    def close(self):
        if self.ser.is_open:
            self.ser.close()

