# @file    test_fota_tx.py
#  @date    2025-04-19
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Test module for FOTA TX
#
#  @details Designed and implemented for the XBee interface over a serial connection
#
#  @ingroup fota_validation

import serial


def send_data(ser, data):
    """
    @brief Transmits data with the XBee
    """
    try:
        data_in_bytes = bytes.fromhex(data)
        print(f"Sending user data: {data_in_bytes}")
        ser.write(data_in_bytes)
    except ValueError:
        print("Invalid input. Please try entering a proper hexidecimal value")


def main():
    """
    @brief Main transmit loop
    """
    port = "/dev/ttyUSB2"  # CHANGE THIS BASED ON WHICH PORT XBEE IS CONNECTED TO
    baudrate = 115200
    timeout = 1

    try:
        ser = serial.Serial(port, baudrate, timeout=timeout)
        print(f"\nConnected to port {port} at {baudrate} baud!\n")

        # Test transmission during startup
        sample_data = b'\xAA\x00\x00\x00\x01\x07\x01\x01\x01\x01\x01\x01\x01\xBB'

        print(f"Sending sample data: {sample_data}")
        ser.write(sample_data)

        while True:
            user_input = input("\nEnter hexidecimal data (or 'exit' to quit): ")

            if user_input.lower() == "exit":
                break

            send_data(ser, user_input)

    except Exception as e:
        print(f"Error: {e}")

    finally:
        if ser.is_open:
            ser.close()
            print(f"\nConnection to port {port} closed!\n")


if __name__ == "__main__":
    main()
