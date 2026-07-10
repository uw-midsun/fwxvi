# 11 - Announce Discovery Redesign

Status: implemented
Date: 2026-07-09
Author: Midnight Sun Team #24 - MSXVI

## 0. Revision (2026-07-09, v2: lightweight heartbeat)

The first cut of this redesign (sections below) broadcast the full 102 byte `BlIdentity` wrapped in a
21 byte datagram header, fragmented into 21 CAN frames per node per second. On hardware the app could
not sustain that: the app owns CAN and `bl_port_can_tx` dumps every fragment through only three TX
mailboxes back to back, so the burst truncated (and exposed the init bug below). Classic CAN has no
CAN FD, so the fix is to shrink the payload, not the framing.

Changes from the v1 plan:
- The announce is now a single fixed 8 byte heartbeat frame, not a datagram. See PROTOCOL.md §3
  (`BL_IDENTITY_HEARTBEAT_SIZE`). It carries only `flags` (mode bit, app_present bit), version and
  `app_size`. The node id still rides the arbitration id.
- The human friendly board and project names and the CRCs are dropped from the wire. The client
  resolves the board name from the node id (`client/src/names.rs`, mirrors `SystemCanDevice`).
- `BL_DG_ANNOUNCE` is removed from `BlDatagramType` (announce is no longer a datagram); ACK and NACK
  renumber to 3 and 4. The transport vtable slot is `send_heartbeat(const uint8_t *, uint8_t)`, the
  CAN transport sends one frame, UART is NULL.
- `bl_identity` now exposes `bl_identity_serialize_heartbeat(mode, buf, buf_size)` in place of the
  datagram announce builders; the `BlIdentity` struct is gone.

Separately, a latent init ordering bug in `can/src/arm/can_hw.c` was fixed: `HAL_CAN_ActivateNotification`
enabled the TX mailbox empty interrupt before the tx ready semaphore was created, and because all
three mailboxes start empty the ISR fired immediately and gave to a NULL handle (trap in
`configASSERT`). The semaphore and rx queue are now set up before any interrupt is enabled. This is
what the SIGINT backtrace at `xQueueGiveFromISR(xQueue=0x0)` was hitting.

Sections 4 through 9 below describe the v1 sizing and file plan; the wiring is otherwise as written.

## 1. Problem

Discovery today is solicited request/response. The host broadcasts a QUERY (or PING)
on the shared FRAGMENT id, and every node replies on a single shared arbitration id:

```
FRAGMENT = xfer_id_base + 0  = 0x100   host -> nodes
RESPONSE = xfer_id_base + 1  = 0x101   nodes -> host   (ALL nodes use this)
ACK      = xfer_id_base + 2  = 0x102   nodes -> host   (ALL nodes use this)
```

Nodes tag themselves only with a `src_node` byte inside the payload
(`bl_responder.c` / `bl_transport_can.c`), and the client demuxes by that byte
(`client/src/can.rs` `collect_discovery`).

This is physically impossible on classic CAN. Two nodes may share an arbitration id
only if they transmit identical bits. Here they win arbitration together (identical
id field), then drive different data bits (`src_node` differs). The first differing
bit makes one node read back a dominant bit while sending recessive, which is a bit
error. Both frames are destroyed and retried. The bus fills with error frames.

Observed symptoms, all explained by this:
- Scanning shows one device, flip flopping between rear and steering, sometimes none.
  Whichever node starts a few bit times ahead occasionally completes a clean frame
  before another starts. Which node wins is timing dependent.
- Ping and multi target flash "only work sometimes" (same flaw on the ACK id 0x102).
- Normal app traffic (steering to front) degrades during a scan, because the responder
  bit error storm drives the participating nodes toward error passive or bus off
  (`can_hw.c` has `AutoBusOff = ENABLE`, `AutoRetransmission = DISABLE`).

Root cause landed in commit `7cbcfaec "FOTA Rewrite"`, which wired the responder into
`can/src/arm/can_hw.c`.

## 2. Decision

Abolish solicited discovery entirely. Replace it with a periodic per node announce
(the CANopen heartbeat model, COB-ID `0x700 + node_id`). Each node emits its identity
on its own arbitration id on a timer. The client only listens. There is no host
request, so there is nothing to answer simultaneously, so there is no collision.

The per node arbitration id is mandatory and is NOT the thing being abolished. If every
node announced on one shared id, a 1 Hz timer would just realign and collide again.
"Per node request/response discovery" is removed. "Per node announce address" stays.

## 3. New arbitration id map

```
ENTER     = enter_id                       = 0x010          host -> nodes   (unchanged)
FRAGMENT  = xfer_id_base + 0x00            = 0x100          host -> nodes   (unchanged, host sole sender)
ANNOUNCE  = xfer_id_base + 0x40 + node_id  = 0x140..0x17F   node -> bus     (NEW, one id per node)
ACK       = xfer_id_base + 0x80 + node_id  = 0x180..0x1BF   node -> host    (per node, was shared 0x102)
```

`node_id` is 0..63 (`bootloader_config.h`, `board_id`). Each 64 wide block fits the 11
bit standard id space and clears the current system CAN map:
- Real non priority system ids in use: `0x13, 0x23, 0x33, 0x43, 0x64`.
- Real priority system ids carry the `0x400` bit, so they are `>= 0x400`.
- `0x140..0x1BF` is free of both.

Lower id equals higher CAN priority, so critical system messages (`0x13`, `0x23`, ...)
preempt announces. Announces yield to real traffic.

## 4. Bus load

Announce datagram = header + identity:
- header `BlDatagramHeader` = 21 bytes
- payload `BlIdentity` = 102 bytes
- total = 123 bytes

Fragmented at 6 payload bytes per frame ([index:2][data:6]):
- `ceil(123 / 6)` = 21 CAN frames per node per announce

At 1 Hz:
- per node: 21 frames/s
- 3 nodes on the bus now (steering, front, rear): 63 frames/s
- 6 nodes (full car): 126 frames/s

Frame cost at 500 kbps, 8 data byte classic frame, worst case with stuffing ~135 bits:
- 6 nodes: 126 * 135 = ~17 kbps = ~3.4% of 500 kbps
- 3 nodes: ~1.7%

Conclusion: negligible at 1 Hz. If we ever want it lower, the identity is static per
boot, so we can send the full identity every N seconds and nothing in between. Not
needed now. Keep 1 Hz in the slow cycle.

## 5. Datagram type changes (wire format)

Remove the four discovery types, add one announce type. This renumbers ACK and NACK,
which is fine because we control and reflash both ends. Both C and Rust must match.

Before (`bl_datagram.h`):
```
FIRMWARE_METADATA=0 FIRMWARE_CHUNK=1 JUMP_TO_APP=2
QUERY=3 QUERY_RESPONSE=4 PING=5 PONG=6 ACK=7 NACK=8
```

After:
```
FIRMWARE_METADATA=0 FIRMWARE_CHUNK=1 JUMP_TO_APP=2
ANNOUNCE=3 ACK=4 NACK=5
```

`ANNOUNCE` carries the existing `BlIdentity` payload verbatim, so the client parser is
reused. The announce still uses the datagram header (CRC32 + total_length), so
reassembly and integrity checks are unchanged, only keyed by arbitration id.

## 6. Emit side (two places)

Both places already have a poll loop. Neither builds a `BlDatagram` on a task stack
(see the stack constraint note, `BlDatagram` is ~2 KB). Both serialize the identity
into a ~123 byte buffer with `bl_identity_serialize_announce` and fragment that.

### 6a. Application mode (profile B, app owns CAN)

Replace `bl_responder` with `bl_announce`:
- delete header sniffing, `targeted()`, `pending`, `on_header_complete`, the QUERY/PING
  latch, and `bl_responder_feed_can`
- add `bl_announce_emit(void)`: serialize identity (mode = APPLICATION), fragment onto
  `ANNOUNCE(node_id)` via `bl_port_can_tx`
- `bl_announce_init(const BlCanSettings *)` keeps reading `board_id` from the config page

Wire into the 1 Hz slow cycle. In this repo that is `can/src/can.c` `run_can_tx_slow`,
which calls a new `can_tx_board_info()` that forwards to `bl_announce_emit()`. Remove
`bl_responder_poll()` from `run_can_rx_all`.

`can/src/arm/can_hw.c`: swap `bl_responder_init` for `bl_announce_init`, and drop the
`bl_responder_feed_can` call from `s_process_rx_fifo` (announce is TX only, nothing to
feed). The ENTER shim feed stays.

### 6b. Bootloader mode (profile A, core owns CAN)

The core drives its transport vtable. Replace the `send_response` vtable slot with
`send_announce(const BlDatagram *dg)`. For CAN it fragments onto `ANNOUNCE(node_id)`.
For UART it is NULL (a UART host does not announce).

`bootloader.c` `bl_bootloader_poll`: emit on a timer using `bl_port_now_ms`, every
`BL_ANNOUNCE_PERIOD_MS` (1000). Build the announce datagram from `BlIdentity`
(mode = BOOTLOADER) and call `downstream->send_announce`. A board sitting in the
bootloader (never jumped, or failed app) still shows up.

## 7. Listen side (client, Rust)

- `config.rs`: add `announce_id_base` (= `xfer_id_base + 0x40`) and `ack_id_base`
  (= `xfer_id_base + 0x80`). Drop the single `response_id` / `ack_id` scalars.
- `can.rs` `collect_discovery`: stop sending QUERY. Listen for `window` (~1200 ms, one
  announce period plus margin). Accept frames with id in `[announce_base,
  announce_base + 63]`, `node = id - announce_base`, buffer fragments per node, run the
  existing `try_assemble` (reads header `total_length`, verifies CRC), parse `BlIdentity`.
- `can.rs` `recv_ack`: listen on the ACK block `[ack_base, ack_base + 63]`, demux node by
  id. Payload no longer needs the `src_node` byte, but keeping it is harmless.
- `ping`: no PING sent. Alive = that node's announce heard within the timeout.
- `protocol.rs`: drop `Query/QueryResponse/Ping/Pong`, add `Announce = 3`, renumber
  `Ack`/`Nack`.
- `client.rs`, `worker.rs`: discovery and ping call sites updated to the passive flow.
  Node list building from `BlIdentity` is unchanged.

## 8. Flash path

Flashing keeps the datagram + ACK path. Moving ACK to `xfer_id_base + 0x80 + node_id`
makes multi target flash collision free too, same root cause as discovery. Single node
flash was already fine. Included in this pass.

## 9. Full file change list

Bootloader submodule (`libraries/ms-bootloader/`):
- `bootloader/inc/bl_datagram.h` - datagram type enum
- `transport/can/inc/bl_transport_can.h` - remove RESPONSE role/stride, add ANNOUNCE
  and ACK block offsets and `BL_CAN_MAX_NODES`
- `transport/can/src/bl_transport_can.c` - remove `can_send_response`, add
  `can_send_announce`, make `can_send_ack` and `handle_ack` per node
- `entry/inc/bl_responder.h` + `entry/src/bl_responder.c` -> `bl_announce.{h,c}`
- `bootloader/src/bl_dfu.c` - delete QUERY and PING cases, remove `send_response` results
- `bootloader/src/bootloader.c` - announce timer in `bl_bootloader_poll`, use
  `send_announce`
- `bootloader/inc/bl_transport.h` - replace `send_response` slot with `send_announce`
- `common/inc/bl_identity.h` + `common/src/bl_identity.c` - keep `BlIdentity`, replace
  the query response / pong builders with `bl_identity_serialize_announce`
- `client/src/{protocol,config,can,client,worker}.rs` - passive listen, id blocks
- tests under each `test/` dir - update to the new types and flow

This repo:
- `can/src/arm/can_hw.c` - swap responder for announcer, drop the responder RX feed
- `can/src/can.c` - `can_tx_board_info()` in `run_can_tx_slow`, drop `bl_responder_poll`
- `can/inc/can.h` - declare `can_tx_board_info`

Docs:
- `projects/can_bootloader/docs/07_protocol_spec.md`, `libraries/ms-bootloader/PROTOCOL.md`

## 10. Test plan

Unit (x86, Unity):
- announce serialize round trips to a valid `BlIdentity` with the right mode
- transport fragments a 123 byte announce into 21 frames on `ANNOUNCE(node_id)`
- client reassembles two nodes announcing concurrently (interleaved frames on two ids)
  into two correct identities
- ACK per node round trip, two nodes acking, client demuxes both

Bench (hardware, 3 boards):
- passive scan lists steering, front, rear every time, no flip flop, no "no devices"
- pull one board, it drops from the list within ~2 s
- steering to front app traffic is unaffected during a scan (no bus off)
- flash one node, then flash two nodes, both ACK streams demux cleanly

## 11. Risks and notes

- Wire format renumber: every board and the client must be rebuilt and reflashed together.
  There is no mixed fleet compatibility, which is acceptable for a bench bring up.
- Discovery latency is now up to one announce period (~1 s) instead of a round trip.
  Acceptable for a flashing tool. If snappier is ever needed, move the emit to the 10 Hz
  medium cycle (10x traffic, still ~34% of nothing).
- A board being flashed should not spend bus time announcing mid transfer. Optional:
  suppress announce while a DFU transfer is active on that node. Minor, can follow later.
- `announce_base`, `ack_base`, and `enter_id` remain per project config values sourced
  from `board.toml`, same as `xfer_id_base` today. The 0x140 and 0x180 blocks assume the
  system CAN map above; if a project adds messages in that range the config must move.
