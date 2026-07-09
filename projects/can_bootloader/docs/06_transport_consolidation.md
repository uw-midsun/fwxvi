# 06 — Transport Consolidation (FOTA + CAN Bootloader → one module)

**Decision:** adopt **FOTA as the base** and make the CAN bootloader a **transport**.
The shared currency is the **datagram**: a transport delivers complete datagrams
up and fragments datagrams down. Everything above the transport
(datagram verify → DFU → jump → config → CRC) is shared 100%; only
fragment/reassemble differs per medium.

This is the datagram seam from docs 04/05. **Refinement (doc 07):** the transports
themselves are **submodule** code — they own the wire framing and call *down* to the
host's raw-HAL `bl_port_*`. So "the transport is the port layer" means the *datagram
seam lives at the transport*, not that the host writes the transport. The host writes
only the raw HAL (`bl_port_can_*`, `bl_port_uart_*`, flash, time, jump).

---

## 1. Why this way around

FOTA is already layered the way we want (`network_buffer → packet_manager →
datagram → dfu`), already has an arm/x86 split, per-layer unit tests, a layered
Python client, and `target_node_id` multi-node routing. The CAN bootloader's logic
is the same workflow with HAL/protocol/state-machine interleaved. So: keep FOTA's
structure, plug CAN in beneath it. (And `fota_can.c` is already a copy of
`bootloader_can.c` — consolidating now stops that duplication from diverging.)

It's also not just dedup: UART is the **ingress** (laptop / RF gateway), CAN is the
**inter-board distribution**. A board can run one transport or both; a gateway
board receives over UART and forwards over CAN. That's the full-vehicle DFU story.

---

## 2. Consolidated layers

```
        ┌──────────────── transport/uart ─────────────┐   ┌──── transport/can ────┐
wire    │ UART ISR → network_buffer (bytes)           │   │ CAN RX (8B frames)    │
frame   │ packet_manager: SOF/EOF → FotaPacket (128B) │   │ CAN-ID frame scheme   │
re-asm  │ fragments → FotaDatagram                    │   │ fragments → FotaDatagram│
        └──────────────────────┬──────────────────────┘   └───────────┬───────────┘
                               │   complete, CRC-verified FotaDatagram │
                               ▼                                       ▼
                    ┌───────────────────────── SHARED CORE ─────────────────────────┐
                    │ fota_datagram_verify  (datagram CRC32)                         │
                    │ routing by target_node_id  (self → DFU, else → forward CAN)    │
                    │ fota_dfu  (datagram → flash, staging optional)                 │
                    │ jump (MSP/VTOR) · bootstrap · BootConfig (doc 05) · crc32      │
                    └────────────────────────────────────────────────────────────────┘
```

The dashed boxes are the **only** transport-specific code. Everything in the SHARED
CORE box is compiled once and reused by both.

---

## 3. The seam: a transport-neutral `FotaDatagram`

Today `FotaDatagram` is packet-centric (`num_packets`, `packet_received[]` bitmap) —
that's UART/`FotaPacket` framing leaking into the shared struct. To make it the
shared currency, slim it to transport-neutral fields and push fragment tracking
**into each transport**:

> Names/fields below are superseded by **doc 07 §1** (renamed `Bl*`, 64-bit
> `target_node_mask`, completeness tracked in the transport). Shown here for the
> rationale; doc 07 is authoritative.

```c
typedef struct {
  uint64_t target_node_mask; // bit i = node i; 0 = broadcast (doc 07 §3)
  uint8_t  type;             // BlDatagramType
  uint32_t datagram_id;
  uint32_t total_length;     // payload byte count
  uint32_t datagram_crc32;   // CRC over data[0..total_length]
} BlDatagramHeader;

typedef struct {
  BlDatagramHeader header;
  uint8_t  data[BL_MAX_DATAGRAM_SIZE];      // 2 KB
  uint32_t bytes_received;
  bool     complete;
} BlDatagram;
```

- **Completeness** is tracked by the *transport* (offset bitmap/watermark), then
  `bl_datagram_verify` checks `datagram_crc32`. The shared layer holds no per-fragment
  bitmap; the transport owns fragment bookkeeping so duplicates don't inflate progress.
- **Ordering / loss** is each transport's job (UART seq nums in `FotaPacket`; CAN
  seq nums in its frame scheme). A transport only ever writes a fragment into
  `data` at the correct offset and bumps `bytes_received`.

> This is a moderate refactor of `fota_datagram.c`: the packet bitmap + `num_packets`
> move into `transport/uart`; `fota_datagram_to_packets` / `process_*_packet`
> become UART-transport internals, not shared API.

---

## 4. The transport port contract

One small vtable. A board instantiates one or both.

```c
typedef void (*BlDatagramCb)(BlDatagram *dg, void *ctx);

typedef struct {
  // bring up the medium; cb fires on each complete, verified inbound datagram
  BlStatus (*init)(const void *settings, BlDatagramCb cb, void *ctx);
  // pump inbound wire data → reassemble → fire cb (called from the main loop)
  BlStatus (*poll)(void);
  // fragment dg onto the wire (TX / forwarding)
  BlStatus (*send_datagram)(const BlDatagram *dg);
} BlTransport;

extern const BlTransport bl_transport_uart;
extern const BlTransport bl_transport_can;
```

The main loop becomes transport-agnostic:

```c
for (;;) {
  if (uart_enabled) bl_transport_uart.poll();
  if (can_enabled)  bl_transport_can.poll();
  if (bl_is_timed_out()) jump_to_app_if_valid();
}
```

Routing (shared callback) — 64-bit mask, see doc 07 §4c for the multi-target form:

```c
void on_datagram(BlDatagram *dg, void *ctx) {
  if (dg->header.target_node_mask == 0 || (dg->header.target_node_mask & (1ull<<MY_NODE_ID)))
    bl_dfu_process(dg);                  // flash myself (incl. broadcast)
  if (dg->header.target_node_mask & ~(1ull<<MY_NODE_ID))
    bl_transport_can.send_datagram(dg);  // gateway: forward to other targets
}
```

---

## 5. Per-transport responsibilities

| | `transport/uart` | `transport/can` |
|---|---|---|
| Wire unit | byte stream | 8-byte CAN frame |
| Framing | `BlPacket` (SOF/EOF, 128B payload, per-packet CRC) — **renamed FOTA code** | role-only arbitration ID + 2B fragment index (doc 07 §11) |
| Reassembly | byte FSM → packet → datagram offset | frame → `data[frag_index*6]` |
| Loss handling | packet seq num + ACK/NACK | fragment index + whole-datagram CRC → NACK/retransmit |
| Source of bytes | `bl_port_uart_rx` → `network_buffer` | `bl_port_can_rx` |

Both end at the same place: a filled `BlDatagram` handed to `on_datagram`.

---

## 6. File migration map

Target dirs follow doc 04 §2: `common/` (shared lib), `bootloader/` (DFU engine).

| Becomes | From |
|---|---|
| `bootloader/bl_datagram.{c,h}` (slimmed, §3) | `fota/fota_datagram.*` |
| `bootloader/bl_dfu.{c,h}` (direct-to-app, §7) | `fota/fota_dfu.*` |
| `common/bl_jump.*` (+ `bootstrap/`) | bootstrap's correct `jump_to`; FOTA `fota_jump_handler` |
| `common/bootloader_config.*` (doc 05) | new — replaces `crc.txt`/midFS |
| `common/bootloader_crc32.*` (one algorithm) | merge `fota` + `bootloader` CRCs |
| `linker/` autogen (doc 05) | replaces `fota_memory_map.h` **and** the three `memory_map.ld` |

| Becomes `transport/uart` | From |
|---|---|
| network buffer, packet FSM, `BlPacket` (de)serialize | `fota/network*.c`, `packet_manager.c`, `fota_packet.c` (renamed `bl_*`) |

| Becomes `transport/can` | From |
|---|---|
| CAN init/tx/rx + CAN frame ↔ datagram | `bootloader_can.c` (+ `fota_can.c`, dedup), `bootloader_can_datagram.c` |

| Deleted / collapsed | |
|---|---|
| `fota_can.c` vs `bootloader_can.c` duplication → one `transport/can` | |
| `fota_memory_map.h` (dup of broken bootloader map) | |
| bootloader's bespoke per-page DFU state machine → FOTA's datagram DFU | |

---

## 7. What we fix during the move (don't inherit FOTA's baggage)

- **Memory map**: drop `fota_memory_map.h` (it's the broken 80K/staging copy);
  adopt the parametric autogen + **single-app** layout (docs 03/05).
- **Staging optional**: FOTA's DFU copies staging→app. Steering (421 KB) has no room
  for a second slot, so the DFU must support **direct-to-app** writes when no staging
  region is configured. Make staging a config knob, not an assumption.
- **One CRC**: collapse the FOTA C CRC, `fota/scripts/crc32.py`, the bootloader CRC,
  and `bootloader/scripts/crc32.py` into the single spec'd algorithm + a host↔target
  round-trip test.
- **Encryption**: move the hook to the **datagram level** (encrypt the serialized blob
  before fragmentation) so CAN and UART get it uniformly — not per-UART-packet as
  today. Optional, off by default (doc 07 §0).
- **Apply the doc-02 bootloader bug fixes** to the new `transport/can` (RX filter +
  bounds, no loopback-on-by-default, real MSP/VTOR jump, node targeting).

---

## 8. Open questions — RESOLVED (doc 07)

- **CAN frame format**: ✅ defined fresh in doc 07 §11 (role-only arbitration ID +
  2-byte fragment index), not the old per-page CAN-ID layout.
- **Datagram buffers on small-RAM parts**: ✅ node side = **1** active inbound
  datagram (+1 outbound response). The 64-way concurrency lives only on the client.
- **Single Python client** with `--transport {uart,can}`: ✅ planned (doc 07 §10).
- **Module name**: ✅ **`ms-bootloader`**, one module, everything renamed `bl_*`.
