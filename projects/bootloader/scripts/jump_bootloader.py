from can_datagram import Datagram, DatagramSender
import bootloader_id
from log_manager import update_log

class Jump_Bootloader:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            # Test channel
            self._sender = DatagramSender()

    def jump_bootloader(self, **kwargs):
        node_ids = []
        for val in kwargs["node_ids"]:
            node_ids.append(val & 0xff)

        update_log(f"Starting jump application process for boards {node_ids}...")

        jump_datagram = Datagram(
            datagram_type_id=bootloader_id.JUMP_BOOTLOADER,
            node_ids=node_ids,
            data=bytearray(),
        )
        self._sender.send(jump_datagram)
        update_log(f"Jump bootloader completed for boards {node_ids}")
