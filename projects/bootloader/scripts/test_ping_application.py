# pylint: skip-file
"""This Module Tests methods in ping_application.py"""
import unittest
import can
from unittest.mock import MagicMock, call
from ping_application import Ping_Application
from bootloader_id import ACK, PING_METADATA, PING_DATA
from can_datagram import DatagramSender

TEST_DATA = "TestingData"
TEST_NODE_IDS = [1, 2, 3]

class TestPingApplication(unittest.TestCase):
    def setUp(self):
        self.mock_sender = MagicMock(DatagramSender)
        self.mock_sender.bus = MagicMock()
        self.app = Ping_Application(sender = self.mock_sender)

    def test_ping_application_metadata(self):
        self.app.ping_application(node_ids = TEST_NODE_IDS, data = TEST_DATA)

        self.mock_sender.send.assert_called_once()

        #extract sent datagram 
        actual_datagram = self.mock_sender.send.call_args[0][0]

        #compare attributes to expected values
        self.assertEqual(actual_datagram.datagram_type_id, PING_METADATA)
        self.assertEqual(actual_datagram.node_ids, [])
        self.assertEqual(actual_datagram.data, bytearray(b'PING'))

    def test_ping_application_data(self):
        expected_chunks = list(DatagramSender._chunkify(self.app.string_to_bytearray(TEST_DATA), 8))
        expected_calls = [
            call(can.Message(arbitration_id=PING_DATA, data=chunk, is_extended_id=False))
            for chunk in expected_chunks
        ]

        #mock ack response
        ack_msg = can.Message(arbitration_id=ACK, data=[0x01], is_extended_id=False)
        self.mock_sender.bus.recv.return_value = ack_msg

        self.app.ping_application(node_ids=[0], data=TEST_DATA)
        
        self.assertEqual(repr(expected_calls), repr(self.mock_sender.bus.send.call_args_list).replace("\n","")) #normalize formatting for comparison
        self.assertEqual(self.mock_sender.bus.send.call_count, len(expected_chunks))

if __name__ == "__main__":
    unittest.main()
