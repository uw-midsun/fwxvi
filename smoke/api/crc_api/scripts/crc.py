# @file     crc.py
#  @date    2025-03-01
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Example module for CRC manual calculation
#
#  @details References: https://github.com/Michaelangel007/crc32
#                       https://github.com/vedderb/FaultCheck_QuickCheck/blob/master/Examples/AUTOSAR_E2E_QuickCheck/E2E/src/Crc.c
#                       https://en.wikipedia.org/wiki/Cyclic_redundancy_check
#
#  @ingroup crc_sample_script

crc32_array = [0xDEADBEEF, 0xCAFEDEED, 0x12345678]
crc16_array = [0xDEAD, 0xBEEF, 0xCAFE, 0xDEED]
crc8_array = [1, 2, 3, 4, 5, 6]


class CRC32:
    """
    @class  CRC32
    @brief  CRC32 checksum calculation class

    @param  poly CRC32 polynomial. Defaults to 0x04C11DB7 (IEEE 802.3 standard polynomial)
    """

    def __init__(self, poly=0x04C11DB7):
        """
        @brief Initializes the CRC32 with a default polynomial

        @param poly CRC32 polynomial. Defaults to IEE 802.3 standard as per STM32 datasheet
        """
        self.poly = poly

    def calculate(self, buf):
        """
        @brief Calculates the CRC32 for a given buffer of data

        @param buf Buffer used for CRC32 calculation
        """
        crc = 0xFFFFFFFF

        for word in buf:
            crc ^= word
            for _ in range(32):
                if crc & 0x80000000:
                    crc = (crc << 1) ^ self.poly
                else:
                    crc = crc << 1
                crc &= 0xFFFFFFFF

        return crc


class CRC16:
    """
    @class  CRC16
    @brief  CRC16 checksum calculation class

    @param  poly CRC16 polynomial. Defaults to 0x1021 (CRC-16-CCITT)
    """

    def __init__(self):
        """
        @brief Initializes the CRC16 with the CRC-16-CCITT default polynomial
        """
        self.poly = 0x1021
        self.crc_table = {}
        for i in range(256):
            c = i << 8
            for _ in range(8):
                c = (c << 1) ^ self.poly if (c & 0x8000) else c << 1
            self.crc_table[i] = c & 0xFFFF

    def calculate(self, buf):
        """
        @brief Calculates the CRC16 for a given buffer of data

        @param buf Buffer used for CRC16 calculation
        """
        crc = 0xFFFF

        for val in buf:
            byte_hi = (val >> 8) & 0xFF
            byte_lo = val & 0xFF

            crc = ((crc << 8) & 0xFFFF) ^ self.crc_table[(crc >> 8) ^ byte_hi]
            crc = ((crc << 8) & 0xFFFF) ^ self.crc_table[(crc >> 8) ^ byte_lo]

        return crc


class CRC8:
    """
    @class  CRC8
    @brief  CRC8 checksum calculation class

    @param  poly CRC8 polynomial. Defaults to 0x07 (CRC-8)
    """

    def __init__(self):
        """
        @brief Initializes the CRC8 with the CRC-8 default polynomial
        """
        self.poly = 0x07
        self.crc_table = {}
        for i in range(256):
            c = i
            for _ in range(8):
                c = (c << 1) ^ self.poly if (c & 0x80) else c << 1
            self.crc_table[i] = c & 0xFF

    def calculate(self, buf):
        """
        @brief Calculates the CRC8 for a given buffer of data

        @param buf Buffer used for CRC8 calculation
        """
        crc = 0xFF

        for byte in buf:
            byte = byte & 0xFF
            crc = self.crc_table[crc ^ byte]

        return crc


crc32 = CRC32(0x04C11DB7)
result32 = crc32.calculate(crc32_array)
print(f"CRC32 Calculation: {result32}")

crc16 = CRC16()
result16 = crc16.calculate(crc16_array)
print(f"CRC16 Calculation: {result16}")

crc8 = CRC8()
result8 = crc8.calculate(crc8_array)
print(f"CRC8 Calculation: {result8}")
