# @file    test_fota_rx.py
#  @date    2025-04-19
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Test module for FOTA RX
#
#  @details Designed and implemented for the XBee interface over a serial connection
#
#  @ingroup fota_validation

import time
import serial


def receive_data(ser):
    """
    @brief Receives data from the XBee
    """
    try:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            print(f"Receiving data: {data}")  # Not as hex
        else:
            print("No data available")
    except Exception as e:
        print(f"Error while receiving data: {e}")


def main():
    """
    @brief Main receiving loop
    """
    port = "/dev/ttyUSB0"  # CHANGE THIS BASED ON WHICH PORT XBEE IS CONNECTED TO
    baudrate = 115200
    timeout = 1

    try:
        ser = serial.Serial(port, baudrate, timeout=timeout)
        print(f"\nConnected to port {port} at {baudrate} baud!\n")

        while True:
            receive_data(ser)
            time.sleep(1)

    except Exception as e:
        print(f"Error: {e}")
    finally:
        if ser.is_open:
            ser.close()
            print(f"\nConnection to port {port} closed!\n")


if __name__ == "__main__":
    main()
