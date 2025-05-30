from can_datagram import Datagram, DatagramSender
import bootloader_id
from log_manager import update_log


class Jump_Application:
    def __init__(self, sender=None) -> None:
        if sender:
            self._sender = sender
        else:
            # Test channel
            self._sender = DatagramSender()

    def jump_application(self, **kwargs):
        node_ids = []
        for val in kwargs["node_ids"]:
            node_ids.append(val & 0xff)

        update_log(f"Starting jump application process for boards {node_ids}...")

        jump_datagram = Datagram(
            datagram_type_id=bootloader_id.JUMP_APP,
            node_ids=node_ids,
            data=bytearray(),
        )
        self._sender.send(jump_datagram)
        update_log(f"Jump application completed for boards {node_ids}")
