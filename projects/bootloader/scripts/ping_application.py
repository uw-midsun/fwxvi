from bootloader_id import *
from can_datagram import Datagram, DatagramSender
from log_manager import update_log
from crc32 import CRC32

class Ping_Application:
    def __init__(self, sender = None) -> None:
        if sender:
            self._sender = sender 
        else:
            self._sender = DatagramSender()

    def ping_application(self, **kwargs):
        node_ids = kwargs.get("node_ids")
        data = kwargs.get("data")
        ping_req = kwargs.get("ping_cmd")
        message_extended_arbitration = kwargs.get("message_extended_arbitration", False)

        update_log("Starting ping application process for boards {}...".format(node_ids))
        
        # 4-bit request type
        ping_req = (ping_req & 0xF) << 44 

        # 12-bit data length
        data_len = (len(data.encode()) & 0xFFF) << 32

        # 32-bit CRC32
        crc32 = CRC32(STANDARD_CRC32_POLY)
        crc32_value = crc32.calculate(data.encode()) & 0xFFFFFFFF

        # combine all components to a 48-bit int
        metadata = ping_req | data_len | crc32_value

        # convert metadata to a byte array
        metadata_bytes = metadata.to_bytes(6, byteorder='little')

        # set state to PING
        meta_datagram = Datagram(
            datagram_type_id=PING_METADATA,
            node_ids=node_ids,
            data=bytearray(metadata_bytes)  
        )

        # start chunking the data (8 chars per chunk)
        ping_datagram = Datagram(
            datagram_type_id = PING_DATA,
            node_ids = node_ids,
            data = bytearray(data.encode()),
            message_extended_arbitration = message_extended_arbitration
        )
        
        self._sender.send(meta_datagram)
        self._sender.send_data(ping_datagram)

        update_log("Ping application completed for boards {node_ids}")
