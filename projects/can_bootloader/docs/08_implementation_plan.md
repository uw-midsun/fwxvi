# 08 — Implementation Plan

The phased path from today's code (old `projects/bootloader` + `projects/bootstrap` +
`projects/fota`) to the consolidated **`ms-bootloader`** submodule specified in 03–07.

Each phase ends at a **checkpoint** you can verify before moving on. Phases are ordered
by dependency: foundation (memory map, CRC, config) → shared currency (datagram) →
transports → app-facing flows → packaging. The doc-02 correctness bugs are fixed inside
the phase that naturally owns them (traceability table at the end).

> **Build-in-place, extract last.** Develop the module as a self-contained directory in
> this repo (e.g. `libraries/ms-bootloader/`) and get it working on real hardware +
> x86 first. Only **Phase 10** turns it into a git submodule — don't fight submodule
> mechanics while also debugging the protocol.
>
> **Keep the old path alive until its replacement works.** Don't delete
> `projects/bootloader`/`fota` files until the phase that supersedes them passes its
> checkpoint; then cut over and remove in the same phase.

---

## Milestones at a glance

```
P0  prerequisites ──▶ P1 memory map/autogen ──▶ P2 common: crc+jump ──▶ P3 common: config
                                                                              │
        ┌─────────────────────────────────────────────────────────────────┘
        ▼
P4 datagram core ──▶ P5 transport/can ──▶ P6 DFU + jump-to-app  ★ FIRST FLASH ★
                                                 │
                                                 ▼
                          P7 bootstrap + entry shim
                                                 │
                                                 ▼
                          P8 transport/uart + gateway
                                                 │
                                                 ▼
                          P9 discovery + unified Python client
                                                 │
                                                 ▼
                          P10 submodule extraction + reference port
```

★ **The de-risking milestone is the end of Phase 6:** CAN + DFU + jump is the first
point you can flash a real app onto a real board over the bus. Everything after that
(bootstrap, UART, gateway, discovery) is additive.

The **Python client can be built in parallel** from Phase 4 onward (it only needs the
datagram codec); it's listed under Phase 9 but doesn't have to wait.

---

## Phase 0 — Prerequisites & scaffolding

**Goal:** unblock everything; nothing protocol-level yet.

- **Pin the L4P5 flash facts** (the one open hardware unknown, doc 03 §2): real page
  size (4 KB vs 8 KB) and bank mode against RM0432 + the fuse/option-byte config we
  actually ship. Every later phase's geometry depends on this.
- Create the module skeleton: `common/ bootloader/ bootstrap/ transport/{can,uart}
  port/ linker/ tools/ client/` (doc 04 §2). Empty headers + a build stub.
- Add `bl_status.h` (the `BlStatus` codes).
- Stand up the **x86 test harness** + a **stub/mock port** (`port/x86/`) so every
  `common/` and `bootloader/` lib can unit-test without hardware.

**Checkpoint:** the skeleton builds (empty), x86 test target runs an empty suite, and
the L4P5 page size is written into doc 03's chip table as confirmed (drop the ⚠️).

---

## Phase 1 — Memory map & linker autogen

**Goal:** one source of truth for flash geometry; the 3 broken `.ld` files gone.
**Depends on:** P0.

- Write **`bootloader_user_config.h`** per chip (doc 05 §1) — geometry, page size, CAN
  ids, node-id source, bitrate. No timeouts (hardcoded in core).
- Write **`tools/generate_linkerscript.py`** (doc 05 §2): parse `BL_*` defines → compute
  layout (single-app, config = 1 page at top) → **validate** (page alignment, no
  overflow, app > 0, fail loudly) → emit `MEMORY{}` + the symbol contract
  (`_app_start`, `_config_start`, `_flash_page_size`, …).
- `sections_common.ld` + the `REGION_ALIAS` section templates (bootstrap/bootloader/app
  variants).
- Wire into SCons (`platform/arm.py get_link_flags`): run the generator pre-link, use
  the generated `-T`. Replaces the per-chip hand-edited maps.

**Checkpoint:** a trivial firmware links at the correct addresses on **all three** chips;
`arm-none-eabi-size` / `objdump -h` confirms region origins/lengths; feeding bad geometry
fails the build with a clear message.
**Deletes:** the 3 `platform/hardware/*/memory_map.ld`, `fota_memory_map.h`.
**Bugs fixed:** doc-02 memory-map overlaps (all chips), wrong L4P5 page size.

---

## Phase 2 — `common/`: CRC + jump + status

**Goal:** the shared, dependency-light primitives both stages need.
**Depends on:** P0.

- **`bootloader_crc32.c`** — ONE software CRC-32 (zlib-reflected) used everywhere
  (image, chunk, config). Matches Python `zlib.crc32`. Drop the STM32 hardware-CRC path.
- **`bl_jump.c`** — `bl_jump(vector_addr)`: set `MSP` from vector[0], `SCB->VTOR`,
  `DSB`/`ISB`, disable IRQs, branch to vector[1]; plus `bl_vector_table_valid()` (SP in
  RAM, reset vector in the target region). Port from the **bootstrap's correct**
  `jump_to`.
- **`bl_align_up8()`** tail-padding helper (replaces the inverted `boot_align_*`).
- Finalize **`bootloader_port.h`** prototypes (CAN/UART/flash/time/watchdog/jump/reset).

**Checkpoint:** x86 unit tests: CRC host↔target parity round-trip (a known blob's CRC ==
`zlib.crc32`); vector-validation accepts a good table and rejects garbage; align helper
covers the non-8-multiple tail.
**Bugs fixed:** 3 disagreeing CRC32s → one; `boot_align_to_32bit_words` inversion;
jump-without-MSP/VTOR (the correct jump now lives in `common/`).

---

## Phase 3 — `common/`: BootConfig

**Goal:** flash-resident identity + CRCs, no filesystem.
**Depends on:** P1 (`_config_start`), P2 (CRC).

- **`bootloader_config.{c,h}`** — the forward-compatible struct (doc 05 §3): fixed
  prologue + append-only body + size-delimited trailing CRC; `bl_config_read()`
  (app + bootloader) and `bl_config_write()` (bootloader only); single page; address
  from `_config_start`.
- **`tools/provision_config.py`** — computes the bootloader `.bin` CRC and writes
  `bl_size`/`bl_crc32` + `board_id`/`board_name` into the config page at SWD-flash time
  (OpenOCD step or a `.bin` patch).

**Checkpoint:** x86 tests: write→read→verify; a truncated/garbage page reads back as
"blank → no app"; **forward-compat test** — a v2-shaped blob (extra trailing field,
larger `size`) is validated and its schema-1 fields read correctly by the v1 reader.
**Deletes:** the `crc.txt`-in-midFS provisioning path.
**Bugs fixed:** the `crc.txt`/midFS chicken-and-egg.

---

## Phase 4 — Datagram core (the shared currency)

**Goal:** the transport-neutral `BlDatagram` + codec.
**Depends on:** P2 (CRC).

- **`bl_datagram.{c,h}`** — `BlDatagramHeader` (21 B, 64-bit `target_node_mask`),
  little-endian serialize/deserialize of the blob (header ++ payload), `bl_datagram_verify`
  (CRC32 over `total_length`), the `BlDatagramType` enum, NACK status codes. **No**
  packet bitmap in the shared struct (completeness lives in transports).
- Slim down from `fota_datagram` (rename `bl_*`, drop `num_packets`/`packet_received[]`).

**Checkpoint:** x86 unit tests: serialize→deserialize round-trips for each type;
`verify` passes good / rejects corrupted; over-long/short `total_length` rejected. No
transport involved yet.

---

## Phase 5 — Transport vtable + `transport/can`

**Goal:** datagrams move over CAN frames.
**Depends on:** P4, plus `bl_port_can_*`.

- **`bl_transport.h`** — the `BlTransport` vtable (`init`/`poll`/`send_datagram`).
- **`transport/can`** — frame ↔ datagram per doc 07 §11: role-only arbitration IDs
  (`BL_ENTER_ID`, `BL_XFER_ID_BASE+{0,1,2}`), `[frag_index:2][data:6]` fragments,
  per-transport **completeness bitmap/watermark**, `BL_FRAGMENT`/`BL_RESPONSE`/`BL_ACK`.
  Calls `bl_port_can_rx/tx`.

**Apply the doc-02 CAN bug fixes here:**
- RX hardware filter to the bootloader ID block (not accept-all `0x0000`).
- Bounds-check the role→handler lookup (kills the out-of-bounds `arbitration_id_handle[]`
  dispatch on live traffic).
- Loopback **off** by default.
- `send_ack` high byte `>> 8` (was `<< 8`).

**Checkpoint:** x86 with the mock CAN port: a 2 KB datagram fragments → reassembles →
verifies; a dropped fragment fails completeness → retransmit recovers; a duplicate frame
doesn't corrupt progress. On real HW: a loopback/2-board bring-up frames one datagram
end-to-end.
**Bugs fixed:** accept-all RX filter, OOB dispatch, loopback-on, ack byte order.

---

## Phase 6 — DFU + jump-to-app  ★ first flash ★

**Goal:** actually flash an app over CAN and boot it.
**Depends on:** P3 (config), P5 (CAN datagrams), P2 (jump).

- **`bl_dfu.{c,h}`** — `METADATA` → `CHUNK ×N` → end-CRC → write BootConfig → `JUMP_APP`
  (doc 07 §5): direct-to-app, **lazy page erase**, 8-byte tail padding, whole-image CRC
  over `app_size`, write `app_*`/`project`/`git`/`version` to config on success.
- **Router** `on_datagram` (doc 07 §4c) — mask check (self / forward), single-node ACK.
- **`bootloader.c`** — state machine + main loop: `poll()` the CAN transport, 5 s entry
  window, `jump-if-valid` on timeout/`JUMP_APP`; IWDG via `bl_port_watchdog_*`.

**Checkpoint (the big one):** from the Python client (CAN), flash a real app (e.g.
blinky) onto a board; it CRC-verifies, writes config, jumps, and the app runs. A
corrupted image → NACK, `app_present=0`, board stays in the bootloader (no brick).
**Bugs fixed:** jump without MSP/VTOR (now uses `common/bl_jump`), `bootloader_ping`
`sizeof(ptr)` memset + bytes_written, `bootloader_init` masking sub-init errors, no IWDG.

---

## Phase 7 — Bootstrap + app entry shim

**Goal:** the permanent first stage and the app→BL re-entry path.
**Depends on:** P2 (jump), P3 (config), P6 (a bootloader to jump to).

- **`bootstrap.c`** — validate the bootloader (vector sanity always + CRC vs
  `bl_crc32` when present) → `bl_jump(bootloader)`. Links **`common/` only**; no midFS,
  no CAN. Target ~8 KB.
- **`bl_entry_shim.{c,h}`** — app-side: CAN RX filter on `BL_ENTER_ID` (+ mask check) →
  write **no-init RAM magic** → `bl_port_reset()`; UART byte-pattern variant. Reserve
  the `.noinit` magic word in the linker; bootloader checks/clears it on boot and stays
  in DFU when set.

**Checkpoint:** full chain `reset → bootstrap → bootloader → app`; the 5 s window jumps
to a valid app; a bad bootloader CRC keeps bootstrap from jumping; sending `BL_ENTER`
to a running app drops it back into the bootloader (and it *stays*).

---

## Phase 8 — `transport/uart` + gateway

**Goal:** UART ingress and full-vehicle (UART→CAN) DFU.
**Depends on:** P5 (vtable), plus `bl_port_uart_*`.

- **`transport/uart`** — `network_buffer` (byte ring) + `BlPacket` SOF/EOF FSM (128 B,
  per-packet CRC) → datagram offset; completeness bitmap here. Renamed/refactored FOTA
  code. Calls `bl_port_uart_rx/tx`.
- **Gateway routing** (doc 10) — a UART+CAN board self-flashes if targeted **and**
  forwards anything addressed elsewhere onto CAN **byte for byte** (no transcode, no
  per-chunk wrapper). Capability is the `BL_FEATURE_GATEWAY` compile flag (only the
  host-UART board builds it); **no runtime arm/disarm** — forward on the two-lock rule
  (compiled in **and** addressed elsewhere). **Transparent ACK relay**: downstream ACKs
  pass straight upstream; the **host** is sender-of-record and retransmits to laggards
  (doc 07 §2). Implicit jump hold-off while relay traffic is recent.
- **Encryption hook** moved to the **datagram level** (off by default), so CAN and UART
  share it.

**Checkpoint:** UART-only DFU of a directly-attached board works; laptop → UART →
gateway → CAN → leaf flashes a downstream board end-to-end.
**Deletes:** `fota_can.c` (dup of the CAN transport), the old FOTA packet files (now
renamed under `transport/uart`).

---

## Phase 9 — Discovery + unified Python client

**Goal:** see the bus, target by identity, one client for both transports.
**Depends on:** P5/P8 (transports), P3 (identity).

- **Discovery** — `QUERY`/`QUERY_RESPONSE` (full identity, src-tagged `BL_RESPONSE`,
  `node_id × slot_ms` backoff) + `PING`/`PONG` (doc 07 §6).
- **Unified Python client** (`client/`) — datagram codec, `--transport {uart,can}`,
  `boot-ls` (builds the `node → {project, rev, git, app_present}` map from discovery),
  `flash`, `jump`, `enter`. Unify `fota/scripts` + `bootloader/scripts`; collapse the
  **third (Python) CRC** onto `zlib.crc32`.

**Checkpoint:** `boot-ls` enumerates the bus into the identity map; flash-by-pattern
("all `drive_inverter` rev-B") works; a multi-node flash with one node dropped retransmits
to the **laggard only** and completes.
**Bugs fixed:** Python-client CRC divergence (now matches the single algorithm).

---

## Phase 10 — Submodule extraction + reference port

**Goal:** the "drop-in" goal, proven.
**Depends on:** all prior.

- Extract `libraries/ms-bootloader/` → its own repo; re-add to this repo as a **git
  submodule**; this repo keeps only `bootloader_user_config.h` + `port/stm32l4/`.
- Ship a clean **reference `port/stm32l4/`** and the SCons + Make/CMake build snippets.
- **Integration test:** bring the module up in a second, clean context following the doc
  04 §6 checklist (one header + a port) and confirm a flash works.

**Checkpoint:** a fresh repo adds the submodule, edits one header, provides/symlinks a
port, and can flash a board — no submodule files touched. North-star met.

---

## Doc-02 bug → phase traceability

| Bug (doc 02 / known-bugs memory) | Fixed in |
|---|---|
| Memory maps overlap / copy-pasted across chips | P1 |
| Wrong L4P5 page size assumption | P0 / P1 |
| Three disagreeing CRC32s (target ×2 + Python) | P2 (target), P9 (Python) |
| `boot_align_to_32bit_words` inverted | P2 |
| Jump without MSP/VTOR | P2 (impl) / P6 (use) |
| CAN RX filter accepts all ids | P5 |
| OOB `arbitration_id_handle[msg->id]` dispatch | P5 |
| Loopback on by default | P5 |
| `send_ack` high byte `<<8` vs `>>8` | P5 |
| `bootloader_ping` `sizeof(ptr)` + bytes_written | P6 |
| `bootloader_init` masks sub-init errors | P6 |
| No IWDG watchdog | P6 |
| MPU dead code / wrong region size | removed (no MPU, doc 07 §0) |
| `crc.txt` in midFS provisioning | P3 |
| Per-node targeting commented out | P4 (mask) / P5 (CAN) / P9 (client) |

---

## Testing strategy (every phase)

- **x86 unit tests** against the stub port for everything in `common/`, `bootloader/`,
  and each transport's reassembly logic (P2–P6, P8).
- **Host↔target parity** tests for the CRC (P2) and the datagram codec (P4) so the
  Python client and firmware can't diverge.
- **Hardware bring-up** at the end of each transport/flow phase (P5 loopback, P6 real
  flash, P7 full boot chain, P8 gateway).
- A phase isn't done until its checkpoint passes **and** the code it supersedes is
  deleted (no parallel stale copies).
