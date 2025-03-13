import can
import time
from bootloader_id import *
from can_datagram import Datagram, DatagramSender
from log_manager import update_log

class Ping_Application:
    def __init__(self, sender = None) -> None:
        if sender:
            self._sender = sender 
        else:
            self._sender = DatagramSender()
   
    #bytearray contains ascii value associated to each char
    def string_to_bytearray(self, string: str) -> bytearray:
        if not isinstance(string, str):
            raise TypeError(f"Expected a string, but got {type(string).__name__}")
        return bytearray(string, 'utf-8')

    def ping_application(self, **kwargs):
        start_time = time.time()

        node_ids = kwargs.get("node_ids")
        data = kwargs.get("data")
        message_extended_arbitration = kwargs.get("message_extended_arbitration", False)

        # Set state to PING
        meta_datagram = Datagram(
            datagram_type_id = PING_METADATA,
            node_ids = [],
            data = self.string_to_bytearray("PING"),
        )

        update_log("Starting ping application process for boards {}...".format(node_ids))

        self._sender.send(meta_datagram)

        #Below is an altered send_data() from can_datagram.py

        # Start chunking the data (8 chars per chunk)

        
        chunk_messages = list(DatagramSender._chunkify(self.string_to_bytearray(data), 8))
        sequence_number = 0

        while chunk_messages:
            # Prepare up to 1024 bytes (128 chunks of 8 bytes each)
            current_chunk = chunk_messages[:128]
            chunk_messages = chunk_messages[128:]

            # Send data chunks (up to 1024 bytes)
            for chunk in current_chunk:
                try:
                    data_msg = can.Message(
                        arbitration_id = PING_DATA,
                        data = chunk,
                        is_extended_id = message_extended_arbitration,
                    )
                    self._sender.bus.send(data_msg)
                except BaseException:
                    time.sleep(0.01)
                    self._sender.bus.send(data_msg)

            update_log("Sent {} bytes".format(len(current_chunk) * 8))

            if chunk_messages:
                ack_received = False
                retry_count = 0
                max_retries = 3

                while not ack_received and retry_count < max_retries:
                    try:
                        ack_msg = self._sender.bus.recv(timeout=5.0)

                        if ack_msg and ack_msg.arbitration_id == ACK:
                            if ack_msg.data[0] == 0x01:
                                ack_received = True
                                update_log("Received ACK for sequence\n".format({sequence_number}))
                            elif ack_msg.data[0] == 0x00:
                                update_log("Received NACK for sequence {}, retrying...".format(sequence_number))
                                retry_count += 1
                                break
                            else:
                                update_log("Received unknown response for sequence {}, retrying...".format(sequence_number))
                                retry_count += 1
                        else:
                            update_log("No ACK/NACK received for sequence {}, retrying...".format(sequence_number))
                            retry_count += 1

                    except can.CanError:
                        update_log("Error waiting for ACK/NACK for sequence {}, retrying...".format(sequence_number))
                        retry_count += 1
                    

                if not ack_received:
                    raise Exception(
                        f"Failed to receive ACK for sequence {sequence_number} after {max_retries} attempts"
                    )
                
            sequence_number += 1
        end_time = time.time()

        update_log("--------------------------------- COMPLETED ---------------------------------")
        update_log(f"Time Elapsed: {end_time - start_time}")
        update_log(f"All data sent successfully. Total sequences: {sequence_number}\n")

        ### End
        # Separate enum that each ECU will read and process accordingly
        # datagram = Datagram(datagram_type_id=PING_DATA, data=string_to_bytearray(data))
        # self._sender.send(datagram)
        update_log("Ping application completed for boards {node_ids}")
