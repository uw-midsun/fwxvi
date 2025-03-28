# pylint: skip-file
"""This Module Tests methods in ping_application.py"""
import unittest
from unittest.mock import MagicMock
from ping_application import Ping_Application
from bootloader_id import PING_METADATA, PING_DATA, STANDARD_CRC32_POLY
from can_datagram import DatagramSender
from crc32 import CRC32

TEST_DATA = "TestingData"
TEST_NODE_IDS = [1, 2, 3]
TEST_CMD = 0

class TestPingApplication(unittest.TestCase):
    def setUp(self):
        self.mock_sender = MagicMock(DatagramSender)
        self.mock_sender.bus = MagicMock()
        self.app = Ping_Application(sender=self.mock_sender)

    def test_ping_application_metadata(self):
        self.app.ping_application(node_ids=TEST_NODE_IDS, data=TEST_DATA, ping_cmd=TEST_CMD)

        self.mock_sender.send.assert_called_once()

        #extract sent datagram 
        actual_datagram = self.mock_sender.send.call_args[0][0]

        #4 bit request type, 12 bit length, 32 bit crc32
        expected_metadata = ((TEST_CMD & 0xF) << 44) | ((len(TEST_DATA.encode()) & 0xFFF) << 32) | (CRC32(STANDARD_CRC32_POLY).calculate(TEST_DATA.encode()) & 0xFFFFFFFF)

        #compare attributes to expected values
        self.assertEqual(actual_datagram.datagram_type_id, PING_METADATA)
        self.assertEqual(actual_datagram.node_ids, TEST_NODE_IDS)
        self.assertEqual(actual_datagram.data, bytearray(expected_metadata.to_bytes(6, byteorder="little")))

    def test_ping_application_data(self):
        self.app.ping_application(node_ids=TEST_NODE_IDS, data=TEST_DATA, ping_cmd=TEST_CMD)

        self.mock_sender.send_data.assert_called_once()

        #extract sent datagram 
        actual_datagram = self.mock_sender.send_data.call_args[0][0]

        #compare attributes to expected values
        self.assertEqual(actual_datagram.datagram_type_id, PING_DATA)
        self.assertEqual(actual_datagram.node_ids, TEST_NODE_IDS)
        self.assertEqual(actual_datagram.data, bytearray(TEST_DATA.encode()))
    
if __name__ == "__main__":
    unittest.main()
