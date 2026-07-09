# 07 — Protocol & Decisions Spec

The agreed, implementable spec. Consolidates the decisions from the Q&A rounds.
Where this doc and 01–06 disagree, **this doc wins** (it's newer).

Naming: everything is `bl_` / `Bl*` / `BL_`; the module is **`ms-bootloader`** (one
module — the old `Fota*` names are renamed to `Bl*`).

---

## 0. Decision ledger

| # | Decision | Choice |
|---|----------|--------|
| Architecture | base | **FOTA layering**, renamed to `bl_*`; CAN bootloader becomes a transport (doc 06) |
| Naming | prefix / module | **`bl_` / `Bl*` / `BL_`**, single module **`ms-bootloader`** |
| Seam | shared currency | **datagram**; transports differ only in fragment/reassemble |
| Addressing | node targeting | **64-bit `target_node_mask`** in datagram header; `0x0` = broadcast-all; `bit = 1<<node_id`, node_id 0..63 |
| Addressing | identity source | node_id from **`BootConfig.board_id`**; client builds a board→project/rev map via **discovery**, no system_can dependency |
| Reliability | ACK model | **stop-and-wait per 2KB datagram** |
| Reliability | multi-node | wait for **all `popcount(mask)` ACKs**; retransmit with mask **narrowed to laggards** |
| CAN | arbitration ID | **role-only** (ID = frame role; node targeting in payload) |
| CAN | ID allocation | **`BL_ENTER_ID`** low/high-prio (app-facing) + **`BL_XFER_ID_BASE`** block for FRAGMENT/RESPONSE/ACK |
| CAN | fragment format | **2-byte index + data** (`offset = index*stride`); position-independent, loss-tolerant |
| CAN | ID width | **standard 11-bit** |
| Crypto | encryption | **optional hook at the datagram level**, off by default |
| Flash | write target | **direct-to-app always; no staging region** |
| Config | size | **exactly 1 page, no A/B**; forward-compatible schema (prologue + size-delimited CRC, append-only) |
| Locking | MPU/WRP | **none** — bootstrap (permanent, SWD-reflashable) is the safety net |
| Bootstrap | BL validation | **both**: vector-table sanity always + CRC vs `bl_crc32` when present |
| BL self-update | scope | **deferred** (SWD-only for the bootloader in v1) |
| Transports | v1 | **CAN + UART + gateway** all in v1 |
| App→BL entry | mechanism | **always boot to BL for 5s**, + app re-enters on a `BL_ENTER` msg via **RAM-magic → reset** |
| Chunk | datagram payload | **fixed 2 KB**, decoupled from page size via **lazy page erase** |
| CAN | bitrate | **`bootloader_user_config.h` knob** |
| Discovery | query | **full QUERY + PING**; full identity in broadcast response (client-side reassembly) |
| Identity | BootConfig fields | **full set** (id, name, project, git, version, sizes, CRCs) |
| Header | version byte | **no** |
| Timeouts | boot window / frame / IWDG | **hardcoded in core** (5 s / ~25 ms / ~2 s) — not user-config |

---

## 1. Datagram (shared, transport-neutral)

```c
typedef struct {
  uint64_t target_node_mask;  // bit i = node i; 0x0 = broadcast-all (64 nodes)
  uint8_t  type;              // BlDatagramType
  uint32_t datagram_id;       // monotonic; doubles as sequence for stop-and-wait
  uint32_t total_length;      // payload bytes (<= 2048)
  uint32_t datagram_crc32;    // CRC32 over data[0..total_length]
} BlDatagramHeader;            // 21 bytes on the wire; no protocol-version byte

typedef struct {
  BlDatagramHeader header;
  uint8_t  data[2048];        // BL_MAX_DATAGRAM_SIZE
  uint32_t bytes_received;
  bool     complete;
} BlDatagram;
```

- **Endianness:** little-endian everywhere (STM32 native).
- **Completeness** is tracked in each **transport**, not by a raw byte counter: a
  fragment is written at `index * stride`, and a small per-transport **received
  bitmap / highest-contiguous watermark** decides done-ness. (A duplicate frame must
  not inflate progress.) Then `bl_datagram_verify` checks `datagram_crc32`.
- **Max payload = 2 KB**, matching the fixed chunk (§5).
- Only **1 active inbound datagram** per node (+1 outbound response). The 32/64-way
  concurrency at discovery lives only on the Python client (it has the RAM).

### Datagram types

```c
BL_DG_FIRMWARE_METADATA  // size, crc, version, project id, git hash
BL_DG_FIRMWARE_CHUNK     // up to 2 KB of image
BL_DG_JUMP_TO_APP        // validated jump request
BL_DG_QUERY              // request identity (discovery)
BL_DG_QUERY_RESPONSE     // node identity reply
BL_DG_PING               // liveness request
BL_DG_PONG               // liveness reply (node id)
BL_DG_ACK                // status + echo of datagram_id
BL_DG_NACK               // status/error code + datagram_id
```

---

## 2. Reliability — stop-and-wait per datagram

**Single target:**
1. Client sends one complete datagram (metadata, or one ≤2 KB chunk).
2. Node reassembles, verifies `datagram_crc32`, acts (e.g. writes the page).
3. Node replies **ACK** (echoing `datagram_id`) or **NACK** (with a status code).
4. Client sends the next datagram only after the ACK. On NACK/timeout it retries the
   same `datagram_id`.

**Multi-target (mask hits N nodes):**
- The sender knows `N = popcount(target_node_mask)` and collects ACKs keyed by
  `src_node`.
- It advances to the next datagram only when **all N** have ACKed the current
  `datagram_id`.
- On timeout/NACK, it retransmits the **same datagram with the mask narrowed to the
  nodes that didn't ACK** (laggards only) — nodes that already wrote it aren't
  re-flashed. Idempotent either way (same `datagram_id`).

`datagram_id` is the sequence number: a node rejects an out-of-order id with a NACK.
Inter-frame timeout ~25 ms (a stalled datagram is discarded); boot window 5 s.

---

## 3. Node addressing

- 64-bit `target_node_mask`; bit *i* = node *i* (0..63). `0x0` = broadcast-all.
- A node acts/ACKs only if `(mask == 0) || (mask & (1ull << my_node_id))`.
- `my_node_id` comes from **`BootConfig.board_id`** (the bootloader's own identity),
  **not** from any system-CAN device table. The module carries no dependency on the
  host repo's CAN map.
- The **client discovers the bus**: on startup it PINGs/QUERYs, builds a
  `node_id → {project, revision, git, app_present}` map (from each node's
  `QUERY_RESPONSE`), and targets by that map. This is how "flash all `drive_inverter`
  rev-B nodes" works without hardcoding ids.
- One binary → many identical nodes in a single stream. **Out of scope v1:**
  interleaving two *different* binaries on the bus at once (needs concurrent
  multi-datagram reassembly *per node*).

---

## 4. Transports

`transport/can` and `transport/uart` are **submodule** code implementing the vtable
from doc 06 (`init` / `poll` / `send_datagram`). They own the wire framing (§11) and
call **down** to the host-implemented raw HAL (`bl_port_can_*`, `bl_port_uart_*`).
A host repo never re-implements a transport — it only writes the port (doc 04 §4).

Both hand a complete, verified `BlDatagram` to the shared router.

### 4a. CAN (`transport/can`)

- **Arbitration ID denotes frame role only** (standard 11-bit). Node targeting is in
  the payload (`target_node_mask`), **never** in the ID — every bootloader node
  filters in the same small ID block, reassembles, *then* checks the mask. The
  datagram CRC32 is the integrity authority. See §11 for the byte-exact frame map.
- IDs (from `bootloader_user_config.h`):
  - `BL_ENTER_ID` — **low ID / high priority**, app-facing jump-to-bootloader (§7).
  - `BL_XFER_ID_BASE + {0,1,2}` — `BL_FRAGMENT` (client→node), `BL_RESPONSE`
    (node→client, src-tagged), `BL_ACK` (node→client, single frame). DFU runs on a
    quiet bus, so these sit higher (lower priority) than `BL_ENTER`.
- Bitrate from user config.
- Apply the doc-02 fixes here: filter to the bootloader ID block, bounds-check the
  role→handler lookup, no loopback-by-default, real MSP/VTOR jump.

- Byte stream → **indexed 128 B fragments** → datagram, the same scheme as the CAN transport
  so both share one shape (wire frames carry fragments, fragments reassemble by index via a
  bitmap into a datagram). A fragment frame is `0x7E, index:2, len:1, payload:len, crc32:4`;
  an ack is `0x7D, status:1, datagram_id:4, src_node:1, crc32:4`.
- **Per-fragment CRC32 uses `bl_crc32`** (the one project CRC, not a separate FOTA CRC), so a
  corrupt fragment is dropped and resent without failing the whole datagram. The reassembled
  datagram's own CRC stays the end-to-end check. The **received bitmap lives in the
  transport**, the shared datagram stays framing neutral.
- This indexed-fragment shape leaves the seam for a future per-fragment NAK/retransmit (resend
  only missing fragments) without touching the datagram layer.

> **Vocabulary (both transports):** a **datagram** is fragmented into indexed **fragments**;
> each fragment rides in one wire **frame** (a CAN frame, or a UART SOF-delimited frame). The
> per-transport code frames bytes; reassembly of fragments → datagram is the shared layer.

### 4c. Gateway routing (shared)

```c
void on_datagram(BlDatagram *dg, const BlTransport *from) {
  // Always act on a datagram addressed to this node (or broadcast)
  if (is_targeted(dg->header.target_node_mask))
    bl_dfu_process(dg);
#if BL_FEATURE_GATEWAY
  // A bridge board also forwards anything addressed elsewhere, byte for byte
  if (dg->header.target_node_mask == 0 ||
      (dg->header.target_node_mask & ~(1ull << MY_NODE_ID)))
    bl_gateway_forward(from, dg);       // out the OTHER transport, verbatim
#endif
}
```

A gateway board (UART + CAN) self-flashes if targeted **and** forwards anything
addressed elsewhere onto the other transport, **byte for byte** (no transcode, no
re-CRC, no per-chunk wrapper). The forwarded datagram is identical on both wires, so a
downstream node cannot tell it was relayed. See **doc 10** for the full design. The
short version:

- **Capability is a compile flag** (`BL_FEATURE_GATEWAY` in `bootloader_user_config.h`):
  only the board wired to the host UART builds the forward path, so only it can ever
  bridge. There is **no runtime arm/disarm** — a compiled gateway forwards on the
  two-lock rule above (compiled in **and** addressed elsewhere); the host driving the
  UART session is the trigger, the mask is the downstream selector.
- **Transparent ACK relay:** the gateway passes each downstream ACK/NACK straight back
  upstream and does **not** aggregate. The **host** is the sender-of-record and runs the
  §2 multi-target wait + laggard retransmit; the gateway just relays, staying near
  stateless. An ack is a single frame, so the transport vtable gains an `on_ack` inbound
  hook, and `send_ack`/the ack frame carry a `src_node_id` so the host can tell which node
  acked (multicast laggard tracking).
- **Direction (asymmetric):** datagrams flow downstream only (UART → CAN); acks flow
  upstream only (CAN → UART). A CAN-side datagram addressed elsewhere is not forwarded
  onto UART (matches host/bus roles, removes echo loops).
- **Two transports:** a gateway build holds both an upstream (UART) and downstream (CAN)
  transport; the router learns the arrival side from the per-transport `ctx`. A leaf build
  has CAN only.
- **Jump policy:** a gateway stays in the bootloader for the whole host session and jumps
  to its own app only on an explicit JUMP addressed to it (being a gateway suppresses the
  auto-jump outright, so no relay-idle timer is needed). It is a normal ECU that also bridges.

Leaf boards run CAN only.

---

## 5. DFU flow (direct-to-app, no staging)

```
METADATA  → record size/crc/version/project/git; reset write ptr to _app_start;
            (lazy erase armed)
CHUNK ×N  → for each ≤2KB datagram:
              if the page this write lands in isn't erased yet → erase it;
              write the chunk straight into the app region (double-word);
              ACK
   …       repeat until bytes_written == binary_size
(end)     → pad the final write up to an 8-byte boundary with 0xFF;
            CRC32 the written app over binary_size (not the padding);
            if mismatch → NACK, mark app_present=0 in BootConfig, stay in BL
            if match    → write app_size/app_crc32/app_present/project/git/version
                          into BootConfig (read-modify-write, 1 page)
JUMP_APP  → validate, jump (MSP/VTOR) only if BootConfig app CRC re-checks OK
```

- **Lazy erase:** erase a flash page the first time a write enters it; subsequent
  2 KB chunks in the same page write without re-erase. Decouples the fixed 2 KB wire
  chunk from 2K/4K/8K page sizes.
- Page size from the `_flash_page_size` linker symbol (doc 05) — never hardcoded.
- **Tail alignment:** binaries whose size isn't a multiple of 8 get the last
  double-word padded with `0xFF`; the integrity CRC covers only `binary_size`.

---

## 6. Discovery

- **QUERY:** client broadcasts (or masks) a `BL_DG_QUERY`; each matched node replies
  `BL_DG_QUERY_RESPONSE` with its **full identity** (from BootConfig, §8). The client
  assembles a `node_id → {project, revision, git, app_present}` map for `boot-ls` and
  pattern-matched targeting (§3).
  - **Concurrent responders:** many nodes may answer one broadcast. Their responses
    stream over `BL_RESPONSE` frames (`[src_node:1][frag_index:2][data:5]`); the
    **client** demuxes fragments into per-`src_node` reassembly buffers (it has RAM
    for ≤64 in-flight). **Each node still handles only one outbound datagram** — the
    concurrency lives on the client, so this does *not* reintroduce constrained-node
    concurrent reassembly (that stays v2, for interleaving different binaries).
  - **Response backoff:** a node waits `node_id × slot_ms` before transmitting its
    `QUERY_RESPONSE`, so up to 64 responders don't all hit the bus on the same tick.
- **PING/PONG:** lightweight liveness — `PONG` is a single self-contained `BL_RESPONSE`
  frame carrying the node id (no reassembly). Quick "who's in the bootloader" checks.

**Implemented (P9 firmware side):**
- `bl_dfu_process` handles `BL_DG_QUERY` → `BL_DG_QUERY_RESPONSE` (the `BlIdentity` payload:
  board_id + version + app/bl size+crc + app_present + board/project/git names, from BootConfig;
  board_id falls back to the running `node_id` so an unprovisioned board still answers) and
  `BL_DG_PING` → `BL_DG_PONG` (node id). The DFU layer stays pure: it fills
  `BlDfuResult.send_response` + `.response`, and the router (`bootloader.c`) sends it via the
  arriving transport's new `send_response(dg)` vtable entry.
- CAN `send_response` rides `BL_RESPONSE` (`xfer_id_base+1`), each frame `[src_node:1][index:2]
  [data:5]` (src = `settings.node_id`). UART aliases `send_response` to a normal datagram send
  (point-to-point, no demux needed).
- **Deferred seams:** the `slot_ms` response backoff and the client-side per-`src_node` demux of
  concurrent responders are host/bus concerns (the firmware replies immediately); gateway relay
  of replies CAN→UART upstream is not built (v1 discovery is host-on-CAN per the P9 checkpoint).

---

## 7. Boot & app→bootloader entry

- **Every boot:** bootstrap validates the bootloader (vector sanity always; + CRC vs
  `bl_crc32` when present) and jumps to it. The bootloader runs a **5 s** window
  listening for a client, then jumps to the app **iff** the app CRC matches config.
- **Forced re-entry from the app:** the app links a tiny **bootloader-entry shim**
  (ships in the submodule, ~50 lines, *not* the full datagram stack) that listens for
  one self-contained control message, writes a **magic value to a no-init RAM word**,
  and resets. The bootloader checks that word on boot; if set, it clears it and stays
  in DFU (does not auto-jump after 5 s).
  - **CAN:** one extra RX filter for `BL_ENTER_ID` (`[target_mask:8][magic:?]` — fits a
    single classic frame only if trimmed; see §11 note); act if
    `mask == 0 || mask & (1ull<<MY_NODE_ID)`.
  - **UART (FOTA app):** a small byte-pattern matcher for one fixed framed control
    packet (`SOF + ENTER + mask + crc + EOF`), independent of `packet_manager`.
  - The enter message is deliberately **not a datagram** so the shim stays tiny and
    transport-local on both media.
- A no-init RAM word (linker `(NOLOAD)` / `.noinit`) must be reserved for the magic.

**Implemented (P7):**
- **Boot flag port contract** — `bl_port_boot_flag_get()` / `bl_port_boot_flag_set(value)` back
  the no-init word; the shim writes `BL_BOOT_ENTER_MAGIC` (`0xB00710AD`) and resets, the
  bootloader reads it in `bl_bootloader_init`, clears it, and sets `force_stay` (suppresses the
  5 s auto-jump exactly like the gateway hold; an explicit `JUMP_TO_APP` still jumps).
- **Enter frame format** (the §11 "left to impl" choice): magic is 4 bytes (strong stray-frame
  guard); the mask is the **low 32 bits** of the node mask — act if `mask == 0` (broadcast) or
  `mask & (1<<node_id)` for `node_id < 32`. Nodes 32-63 are reached by a broadcast enter, then
  DFU-targeted precisely (the enter path needs no 64-bit precision).
  - **CAN:** on `enter_id`, `[magic:4 LE][mask:4 LE]` (one classic frame).
  - **UART:** `[0x7C SOF][magic:4 LE][mask:4 LE][crc32:4 LE]` (crc32 via `bl_crc32` over the 8
    body bytes). `0x7C` is distinct from the transport's `0x7E` datagram / `0x7D` ack.
- **Bootstrap** — `bl_bootstrap_select()` / `bl_bootstrap_run()`: vector-table sanity always,
  plus a `bl_crc32` check over `bl_size` bytes when the config provisions `bl_size`/`bl_crc32`
  (blank config falls back to the vector check). Links `common/` only.

---

## 8. BootConfig (1 page, top of flash)

Full identity set, **one page, no A/B**, read-modify-write (preserves `bl_*` when the
bootloader writes `app_*`). Designed to **grow**: fixed prologue + append-only body +
size-delimited trailing CRC, so v2 can add fields without breaking a v1 reader or
moving the CRC. Full struct + read/write rules in **doc 05 §3**.

```c
typedef struct {
  /* fixed prologue — never reorder/resize across schema versions */
  uint32_t magic;             // BOOTCONFIG_MAGIC
  uint16_t schema_version;
  uint16_t size;              // total valid bytes; CRC lives at base + size - 4
  /* schema-1 body — append-only (v2+ adds fields after, before the CRC) */
  uint16_t board_id;          // 0..63 — IS the bootloader node_id (§3)
  char     board_name[32];
  char     project_name[32];
  char     git_hash[16];
  uint8_t  fw_version_major;
  uint8_t  fw_version_minor;
  uint32_t bl_size;
  uint32_t bl_crc32;
  uint32_t app_size;
  uint32_t app_crc32;
  uint8_t  app_present;
  /* v2+ fields appended here, BOOTCONFIG_SCHEMA bumped */
  uint32_t config_crc32;      // CRC32 over first (size - 4) bytes
} BootConfig;
```

- Fits a single page (4 KB on L4P5, 2 KB on L433/L496) with room to grow.
- `bl_size`/`bl_crc32` written by the SWD provision step; `app_*`/`project`/`git`/
  `version` written by the bootloader after a verified DFU; `board_id`/`board_name`
  at provision time. App links the read path only.
- A torn write (no A/B) just fails the CRC → config treated blank → "no app, stay in
  DFU." Safe fallback.

---

## 9. Out of scope for v1

- Bootloader self-update over CAN (bootstrap apply path) — SWD-only for now.
- Interleaving two different binaries on the bus simultaneously.
- Encryption enabled (hook present, off). **Implemented:** `bl_datagram_set_crypto(encrypt,
  decrypt)` registers in-place payload transforms at the datagram level (shared by every
  transport); default is NULL/passthrough. encrypt runs on serialize over the on-wire payload,
  decrypt on deserialize before the CRC check, so a wrong key fails the (plaintext) CRC. Wiring
  a real cipher + key management is what stays out of scope.
- A/B staging / rollback (no staging region at all).

## 10. Left to implementation (not user-facing)

- The no-init RAM magic word address + linker placement.
- Lazy-erase bookkeeping (which page is currently erased).
- Per-transport completeness bitmap/watermark representation.
- `slot_ms` for the QUERY-response backoff; concrete reserved CAN ID values.
- Single Python client unifying the two script trees behind the datagram codec,
  with `--transport {uart,can}`.

---

## 11. CAN wire format

The unifying idea: **both transports move the same serialized datagram blob; they
differ only in how each fragment is tagged with its position.** Above the transport,
the reassembled `BlDatagram` is identical.

```
serialized datagram blob = HEADER(21B) ++ PAYLOAD(total_length ≤ 2048B)
  HEADER = mask:8  type:1  datagram_id:4  total_length:4  crc32:4   (little-endian)
  blob length = 21 + total_length  (≤ 2069B → ≤ 345 fragments at 6B each)
```

CAN tags each fragment in the arbitration ID (role) + 2 in-band index bytes; UART
tags it in `BlPacket` header fields. Same information, different place.

### Reserved IDs (standard 11-bit, set in `bootloader_user_config.h`)

| ID | Direction | Payload | Purpose |
|----|-----------|---------|---------|
| `BL_ENTER_ID` (low/high-prio) | host → **app** | `[target_mask…][magic…]` | jump-to-bootloader; handled by the app shim (§7), not the BL |
| `BL_XFER_ID_BASE+0 = BL_FRAGMENT` | client → node | `[frag_index:2][data:6]` | every datagram, streamed; `offset = frag_index*6` |
| `BL_XFER_ID_BASE+1 = BL_RESPONSE` | node → client | `[src_node:1][frag_index:2][data:5]` | QUERY_RESPONSE / PONG; src-tagged for client demux of concurrent responders |
| `BL_XFER_ID_BASE+2 = BL_ACK` | node → client | `[status:1][datagram_id:4][src_node:1]` | single frame, no reassembly (ACK/NACK) |

> **`BL_ENTER_ID` payload note:** a classic CAN frame is 8 bytes; a full 8-byte mask
> leaves no room for a magic. Options (left to impl): trim the enter-mask to the low
> 32 bits (boards >31 use broadcast or a 2nd frame), use CAN-FD if available, or send
> mask+magic as a 2-frame mini-sequence. The enter path doesn't need 64-bit precision
> as urgently as the datagram path does.

### Reassembly (receiver)

1. Write each fragment's `data` at `frag_index * stride` (`stride` = 6 inbound, 5 on
   `BL_RESPONSE`); record the fragment in the per-transport completeness map.
2. Once the first 21 bytes are present, parse `total_length` → know the final index.
3. When all fragments [0..last] are present → verify `datagram_crc32`.
4. CRC OK → act + `BL_ACK`; CRC bad / gap → `NACK`, client retransmits the **whole**
   datagram (cheap under stop-and-wait).

A dropped frame is self-healing: the completeness map never marks done, the inter-frame
timeout fires, and the client retransmits. Duplicates are idempotent (same offset).

### Notes

- `BL_FRAGMENT` carries *all* client→node datagrams (metadata, chunk, query). The role
  ID says "bootloader fragment"; the datagram `type` (in the header) says what it is.
- Priority order (ascending ID = ascending priority): `BL_ENTER_ID` highest, then the
  `BL_XFER_ID_BASE` block — control/acks win arbitration over bulk transfer, and the
  app catches `BL_ENTER` against live traffic.
- `BL_ENTER_ID` lives outside system_can so the submodule has no host-repo CAN-map
  dependency; the app adds one raw RX filter for it (§7 shim).
