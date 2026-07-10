# Bootloader Interview Prep — `ms-bootloader`

> Personal study sheet to speak fluently about the CAN/UART bootloader I designed and built for
> Midnight Sun XVI (Car #24, UWaterloo solar car, STM32L4 ECUs). Target role: Tesla bootloader /
> firmware. Everything below maps to real code in `libraries/ms-bootloader/`.

---

## 0. The 30-second pitch (memorize this)

> "I built a reusable, chip-agnostic bootloader for our solar car — a fleet of ~10 STM32L4 boards
> on a CAN bus. It's a permanent SWD-only **bootstrap** plus a field-updatable **bootloader**, and
> it does firmware update over **CAN or UART** through one shared transport-neutral **datagram**
> layer. The whole thing is structured as a drop-in git submodule: all logic is portable C with a
> thin `bl_port_*` HAL seam, so it compiles and runs fully on x86 — I have 60-odd unit tests
> covering the protocol, the DFU state machine, and the flash flow without touching hardware.
> Integrity is a whole-datagram CRC32 plus a vector-table sanity check before every jump, and a
> failed flash can never brick a board — it just stays in the bootloader."

If they want one more sentence: *"The design intentionally leaves seams for the things a production
automotive bootloader needs next — signed images, A/B rollback, secure boot — without paying for
them in v1."*

---

## 1. The problem & constraints (why it looks like this)

- **Fleet of STM32L4 ECUs on one CAN bus.** Front/rear controller, BMS, MPPT, etc. Reflashing each
  by SWD/JTAG with a probe is painful; we want to push firmware to any node (or all of them) over
  the bus the car already has.
- **Three chips, one codebase.** L433CCU6 (256 KB flash / 2 KB page), L4P5VET6 (512 KB / 4–8 KB),
  L496RGT6 (1 MB / 2 KB). The bootloader must not be rewritten per chip.
- **Bare-metal superloop, no RTOS.** The bootloader is the simplest thing that can run — no
  scheduler, no dynamic allocation. (The *apps* run FreeRTOS; the bootloader does not.)
- **Must be unbrickable in the field.** A solar car at a race — a bad flash mid-update cannot leave
  a board dead with no recovery path short of pulling it and probing it.
- **The steering/biggest app is ~421 KB on a 512 KB part** → it physically cannot fit twice, so
  classic A/B dual-slot is off the table for v1. That single fact drives a lot of the design.

**Talking point:** I let the hardware reality (one app can't fit twice) pick the architecture
(single app slot + a tiny permanent bootstrap as the safety net) rather than cargo-culting A/B.

---

## 2. The three-stage boot chain (draw this on the whiteboard)

```
power-on / reset
      │
      ▼
┌─────────────┐   vector sanity + (optional) CRC over bootloader
│  BOOTSTRAP  │   permanent, SWD-flash-only, ~8 KB, links common/ ONLY
│  @0x0800_0000│  ── if BL bad ──► halt for SWD (safety net)
└─────┬───────┘
      │ jump (set MSP + VTOR)
      ▼
┌─────────────┐   5 s window: listen on CAN/UART for an update
│ BOOTLOADER  │   ── update arrives ──► DFU session (flash the app)
│ @0x0800_2000│   ── window expires + valid app ──► jump to app
│  ~32 KB     │   ── app forced re-entry ──► stay in DFU
└─────┬───────┘
      │ jump (CRC + vector check first)
      ▼
┌─────────────┐   normal firmware (FreeRTOS). Carries a ~50-line
│    APP      │   "entry shim": on a CAN/UART ENTER command it sets a
│ @0x0800_A000│   no-init RAM magic word and resets → back to bootloader
└─────────────┘
```

**Why three stages, not two?** The bootstrap exists *only* so the bootloader itself can be updated
over CAN with a fallback. If a bootloader self-update were interrupted, the bootstrap still boots
and can re-validate. The bootstrap is the one thing that's *never* updated over the wire — SWD only
— so it's the immovable root of recovery. Keeping it tiny (~8 KB, links `common/` only) keeps that
trusted base small. (`bootstrap/src/bl_bootstrap.c`)

**Memory map (L433, 256 KB):** bootstrap 8 KB @ `0x08000000` → bootloader 32 KB @ `0x08002000` →
app @ `0x0800A000` → **BootConfig** = last 2 KB page @ `0x0803F800`. One source of truth: a Python
generator (`tools/generate_linkerscript.py`) reads `bootloader_user_config.h` and emits the linker
`MEMORY{}` block + symbols, so the map can't drift between code and linker.

---

## 3. Layered architecture (the part interviewers probe)

The submodule is layered so the *only* chip-specific code is one small port:

```
port/<chip>/   ← the ONLY hardware code. Implements bl_port_* (CAN/UART/flash/time/wdt/jump/reset)
   ▲   (x86 port exists today: fake-flash array + FIFOs → everything unit-testable)
   │
common/        ← portable, links into BOTH bootstrap & bootloader:
   │              crc32 (streaming), bl_jump vector-validate, BootConfig, bl_status
   ▲
bootloader/    ← bl_datagram (wire currency), bl_dfu (update FSM), bl_reassembler, bootloader.c (superloop/router)
   ▲
transport/     ← bl_transport.h vtable + can/ + uart/  (fragmentation ↔ reassembly per medium)
```

**The key idea — a transport vtable + a transport-neutral datagram.** (`bootloader/inc/bl_transport.h`)

```c
typedef struct BlTransport {
  BlStatus (*init)(const void *settings, BlDatagramCb dg_cb, BlAckCb ack_cb, void *ctx);
  BlStatus (*poll)(void);                              // pump wire bytes/frames, fire callbacks
  BlStatus (*send_datagram)(const BlDatagram *dg);     // fragment a datagram onto the wire
  BlStatus (*send_ack)(BlNackCode, uint32_t id, uint16_t src_node);
  BlStatus (*send_response)(const BlDatagram *dg);     // QUERY_RESPONSE / PONG channel
} BlTransport;
```

Everything above the transport speaks **`BlDatagram`** and never knows whether bytes came from CAN
or UART. CRC is over the datagram payload, not the framing — so a datagram reassembled on UART *is*
a byte-identical valid CAN datagram. That's what makes the **gateway** a verbatim relay instead of
a transcode (see §7).

**No `#ifdef` in the core.** Gateway-vs-leaf is a *runtime* fact (`is_gateway() == upstream != NULL`),
not a compile flag inside the logic — that's what keeps the whole thing x86-testable in one build.
(`bootloader/src/bootloader.c:43`)

**Talking point — "the transport IS the port layer."** I deliberately put the medium abstraction at
the datagram boundary: transports deliver *complete* datagrams up and fragment them down; DFU,
jump, config, and CRC are all shared. Adding a new medium = one new file implementing 5 vtable fns.

---

## 4. The wire protocol (be able to recite the header)

**Datagram = the shared currency.** 21-byte little-endian header + payload ≤ 2048 B.
(`bootloader/inc/bl_datagram.h`)

```
 offset size field
   0     8   target_node_mask   bit i = node i; 0 = broadcast (64 nodes addressable)
   8     1   type               METADATA/CHUNK/JUMP/QUERY/QUERY_RESPONSE/PING/PONG/ACK/NACK
   9     4   datagram_id        monotonic; doubles as the stop-and-wait sequence number
  13     4   total_length       payload byte count ≤ 2048
  17     4   datagram_crc32     CRC32 over the payload
  21    ...  payload
```

**Two-level addressing decision (good story):** targeting is *always* the payload `target_node_mask`,
**never** the CAN arbitration ID. The arbitration ID only encodes a **role** (fragment / ack /
response), not a destination. Why: it makes multicast free (one upload with many mask bits fans out
to many boards) and keeps the bus reserving only a handful of IDs regardless of fleet size.

**Fragmentation.** A 2 KB datagram doesn't fit in an 8-byte CAN frame, so each transport slices the
serialized blob into indexed **fragments**:
- **CAN fragment frame:** `[index:2][data:6]`, payload offset = `index * 6`. (stride 6)
  (`transport/can/src/bl_transport_can.c:95`)
- **CAN response frame:** `[src_node:1][index:2][data:5]` — the source byte lets the host **demux
  concurrent responders** during discovery (many boards answer a broadcast QUERY at once). (stride 5)
- **CAN ack frame:** single frame `[status:1][datagram_id:4][src_node:1]`.
- **UART fragment frame:** SOF-delimited, `[0x7E][index:2][len:1][payload≤128][crc32:4]`. (stride 128)

**Reassembly is shared** (`bootloader/src/bl_reassembler.c`): blob buffer + a received bitmap +
`try_complete`. It reads `total_length` once the header fragments are present, then waits until every
fragment index is seen, then deserializes → verifies CRC → fires the callback. It's **dup-safe**
(bitmap), **drop-safe** (a gap just holds delivery), and **resync-safe**. Both transports feed
`bl_reasm_push()` — I extracted it from the two transports once I saw the code was identical.

**Reliability = stop-and-wait per datagram.** Whole-datagram CRC32 is the integrity authority. Any
dropped/corrupt fragment → CRC fails → NACK → host retransmits the *whole* datagram. Simple,
correct, and the node only ever holds **one** in-flight inbound datagram — all the concurrency
burden lives in the host Python client, not on the MCU.

---

## 5. The DFU state machine & flash flow (the meat)

Two layers, mirrored deliberately (`bootloader/src/bl_dfu.c`):

- **Layer 1 — superloop** (`bootloader.c`): *not* an FSM. Just `poll → poll → kick watchdog`, plus
  the 5 s auto-jump window. The transport owns reassembly; the loop only sees complete, verified
  datagrams via the `on_datagram` router.
- **Layer 2 — DFU session FSM** (`bl_dfu.c`): acts on complete datagrams keyed by **type**, never
  on fragments. States: `UNINITIALIZED → IDLE → RECEIVING`. (COMPLETE/JUMP are transient — act and
  return.)

**The flash sequence:**
1. **METADATA** datagram → validate `image_size ≤ app_region` (else `NACK_OVERSIZED`), arm a session,
   set `write_cursor = erase_cursor = app_start`, `expected_seq = id + 1`, go `RECEIVING`. Entering
   `RECEIVING` disarms the 5 s auto-jump so an in-progress flash isn't interrupted.
2. **CHUNK** datagrams → check state + sequence (`NACK_SEQUENCE` on a gap), **lazily erase** pages as
   the write cursor advances into them, **pad the final write up to an 8-byte boundary with 0xFF**
   (STM32L4 programs in 64-bit double-words), write via `bl_port_flash_write`.
3. On the last byte → **finalize**: stream a CRC32 over the *written* flash, compare to the metadata
   CRC. Match → write BootConfig (`app_present=1`, size, crc, version, project, git — read-modify-write
   so the bootloader's own `bl_size`/`bl_crc32` are preserved). Mismatch → set `app_present=0` +
   `NACK_CRC`.
4. **JUMP** datagram → `bl_dfu_jump_if_valid`.

**Design points to land:**
- **Direct-to-app write, no staging.** Because the big app can't fit twice. The trade-off: during a
  flash the old app is already erased, so an interrupted flash leaves no runnable app — *but* that's
  exactly why a failed flash sets `app_present=0` and the board safely **stays in the bootloader**
  instead of jumping to garbage. No brick, just "re-flash me."
- **Lazy page erase** decouples the protocol from page size (2 KB vs 4 KB vs 8 KB across the three
  chips) — I erase a page the first time the cursor crosses into it, driven by `bl_port_flash_page_size()`.
- **Fixed 2 KB chunk** matches the datagram max so framing math is uniform.

---

## 6. Integrity & fail-safety (Tesla will push hard here)

Three independent gates, and the honest distinction between **integrity** and **security**:

1. **Whole-datagram CRC32** (`bl_datagram_verify`) — catches wire corruption / dropped fragments.
2. **Whole-image CRC32 re-read from flash** after writing, and **again before every jump**
   (`crc_over_flash` in `bl_dfu.c`) — catches a bad write or flash bit-rot, not just bad transport.
3. **Vector-table sanity** before any jump (`common/src/bl_jump.c`), used by *both* bootstrap→BL and
   BL→app:
   ```
   initial_sp must land inside RAM
   reset_handler must have the Thumb bit set (Cortex-M)
   reset_handler (Thumb bit masked) must lie inside the target image region
   ```
   This is a pure function over two already-read words + region bounds — so it's x86-testable with no
   raw dereference of `0x08000000`.

**The jump itself** (set MSP, set `SCB->VTOR`, `DSB`/`ISB`, branch) is the *one* Cortex-M-specific
thing that can't live in `common/` — it lives in the port (`bl_port_jump`). On x86 the stub just
records the jump target so a test can assert on it.

**BootConfig — the flash-resident record** (`common/inc/bootloader_config.h`): one page, magic
`"BOOT"`, schema version, and CRC32s + sizes for **both** the bootloader and the app, plus identity
(board id, name, project, git hash). It's **forward-compatible by construction**: a fixed prologue
that never moves + append-only body + a *size-delimited* trailing CRC (found via `size`, not a fixed
offset) — so a v1 reader correctly validates and reads a page a v2 writer wrote. A **torn write
fails CRC → treated as blank → stay-in-bootloader** (the safe fallback). Exactly one page, no A/B —
the torn-write-is-blank rule is what makes a single page safe without redundancy.

**"How is this NOT bricking?"** — be ready: the bootstrap is SWD-reflashable and never updated over
the wire; the bootloader only jumps to an app that passes CRC + vector check; a failed app flash
clears `app_present`; a torn config reads blank. Every failure path lands back in the bootloader
listening for a re-flash.

---

## 7. The gateway (nice "systems thinking" story)

Most boards are **leaf** nodes (CAN only). One board is the **gateway**: it also has a UART to a
laptop and bridges host↔bus. Because the datagram is transport-neutral, the gateway is a **verbatim
relay**, not a transcode — it reassembles one datagram on UART and hands the byte-identical blob to
CAN `send_datagram`. (`bootloader/src/bootloader.c:74`)

Decisions worth quoting:
- **Asymmetric forwarding, three locks.** Datagrams forward **UART→CAN only**; the only thing going
  CAN→UART is a relayed **ACK**. Forward iff: it's a gateway **and** the datagram arrived on UART
  **and** it's addressed elsewhere (broadcast or a foreign node bit). That kills echo loops and
  matches host/bus roles.
- **Transparent ACK relay, not aggregation.** The gateway passes each downstream ACK straight up; the
  **host is sender-of-record** and does the multi-target wait + "retransmit with the mask narrowed
  to the laggards." The host's ACK-gated send loop *is* the backpressure that throttles fast UART to
  slow CAN — free flow control.
- **A gateway stays in the bootloader for the whole host session** (won't drop the bridge
  mid-campaign); it only jumps to its own app on an explicit JUMP addressed to it.

I originally let myself consider a `REPACKAGE` wrapper type to "select the node" and **rejected it** —
it would double the header/CRC and lose multicast. The mask already selects nodes; the relay stays dumb.

---

## 8. App re-entry (no-init RAM magic)

A running app needs a way to drop back into the bootloader on command (e.g. host says "everyone into
DFU"). Mechanism (`entry/src/bl_entry_shim.c`, ~50 lines linked into the *app*):

1. App's shim listens for an **ENTER** command — a primitive control frame, *not* a datagram, so the
   shim stays tiny. CAN: `[magic:4][mask:4]` on the enter ID. UART: `[0x7C][magic:4][mask:4][crc32:4]`.
2. On a match (broadcast or my node's bit), the shim writes `BL_BOOT_ENTER_MAGIC` (`0xB00710AD`) to a
   **no-init RAM word** and triggers a reset.
3. After reset the bootloader reads the magic, clears it, and sets `force_stay` — it suppresses the
   5 s auto-jump and holds in DFU until an explicit JUMP. (`bootloader.c:122`)

**Why a no-init RAM word and not flash?** It survives the warm reset but costs no flash wear and no
write latency, and it auto-clears on a cold boot (power cycle = normal boot). It lives in a
`.noinit (NOLOAD)` linker section the startup code doesn't zero. This is the classic
"reboot-into-bootloader flag" pattern (same idea as STM32 system-bootloader entry, or a magic value
in a backup register).

---

## 9. Portability & testing (your strongest differentiator — lead with it if they're a test-minded team)

- **One HAL seam, ~15 functions** (`common/inc/bootloader_port.h`): `bl_port_can_*`, `bl_port_uart_*`,
  `bl_port_flash_{erase,write,read}`, `bl_port_*_region`, `bl_port_now_ms`, `bl_port_watchdog_*`,
  `bl_port_jump`, `bl_port_reset`, `bl_port_boot_flag_get/set`. The host repo writes **only** the raw
  HAL and `bootloader_user_config.h` — it never edits a submodule file.
- **x86 port = full simulator.** `port/x86` backs flash with a `s_fake_flash[]` array (sized from the
  *same* `bootloader_user_config.h`, so there's no second geometry source), CAN/UART with FIFO rings,
  a mock millisecond clock, and a recorded "last jump address" + reset counter. So the **entire**
  protocol/DFU/gateway/discovery path runs and is asserted on a laptop.
- **~60 unit tests across 14 files** (Unity): datagram round-trip + 64-bit mask + corruption; CAN
  2 KB / 345-fragment round-trip, drop-recovery, dup-safety, foreign-id ignore; UART split-across-polls
  + resync-after-junk; full DFU flash+jump, bad-CRC-no-brick, oversized, out-of-sequence; gateway
  forward byte-identical + ack relay; bootstrap select; entry shim; reassembler; encryption seam.
- **Host/target CRC parity by construction:** the CRC is reflected poly `0xEDB88320` verified against
  `zlib.crc32` known answers, so the Python provisioner and the MCU agree.

**Talking point:** "I treated the bootloader as portable software with a hardware driver, not as a
hardware project. That's why it's chip-agnostic *and* why I could unit-test the protocol before a
board existed."

---

## 10. Design decisions & trade-offs (rapid-fire — have the "why" ready)

| Decision | Why | Trade-off I accepted |
|---|---|---|
| Single app slot, no A/B | 421 KB app can't fit twice in 512 KB | No seamless rollback; mitigated by stay-in-BL on failure |
| Tiny permanent bootstrap | Lets the *bootloader* be field-updated with a fallback | One more stage, ~8 KB flash |
| Stop-and-wait per 2 KB datagram | Simplicity + node holds 1 inbound; CRC is the authority | Lower throughput than windowed; fine for our image sizes |
| Targeting in payload mask, role in arb ID | Free multicast, few reserved CAN IDs, fleet-size independent | A node must parse the header to know if it's addressed |
| Whole-datagram CRC (not per-fragment only) | Drop/corrupt → one clean retransmit unit | Re-send the whole 2 KB on any loss |
| Direct-to-app write | No room to stage | Old app gone during flash → rely on no-brick fallback |
| Lazy page erase | Decouples protocol from 2K/4K/8K page size | Slightly more bookkeeping (erase cursor) |
| No `#ifdef` in core; runtime gateway flag | Keeps one x86-testable build | A leaf still links the (unused) forward path |
| Encryption hook off by default | Ship a seam, not a half-baked cipher | No confidentiality in v1 (integrity only) |

---

## 11. Tesla-relevant gaps & how I'd extend it (be honest, show the roadmap)

This is **v1 for a student race team** — integrity-focused, not a production secure boot. If asked
"what would you add for production / automotive," hit these in order:

1. **Authenticity / secure boot (the big one).** Today integrity = CRC32, which is *not* security —
   anyone on the bus can forge a valid-CRC image. Production needs a **signature**: sign the image
   (and the metadata) with an asymmetric key (ECDSA P-256 / Ed25519), bootloader verifies with a
   public key fused/locked in. Chain of trust: an immutable ROM/bootstrap verifies the bootloader's
   signature, the bootloader verifies the app's. I deliberately put an **encryption/transform seam at
   the datagram level** (`bl_datagram_set_crypto`) so a real crypto layer drops in there — but
   signing belongs at the *image* level (over the whole image + version), which is the right place to
   add it.
2. **Anti-rollback / version monotonicity.** Store a minimum-version counter; refuse images older
   than what's installed so a signed-but-old vulnerable image can't be replayed.
3. **A/B partitions + rollback** where flash allows (it doesn't on our 512 KB part, but Tesla's MCUs
   do). Boot the new slot, require an app "health confirm," else auto-revert. My single-slot
   `app_present` flag is the degenerate version of this.
4. **Read/write protection & lock bits.** I explicitly *didn't* set WRP/MPU (bootstrap is the
   SWD-reflashable safety net). Production locks the bootstrap region (WRP) and may use the option
   bytes / RDP. Worth saying I know the option-byte story and chose not to lock a student board.
5. **UDS / ISO 14229 + ISO-TP (15765-2).** Automotive standard for diagnostics & flashing — my
   protocol is a simpler bespoke equivalent. I'd map onto UDS services (RequestDownload `0x34`,
   TransferData `0x36`, RequestTransferExit `0x37`, RoutineControl for erase/checksum, SecurityAccess
   `0x27`) and ISO-TP for segmentation, which is essentially my fragment/reassembly layer
   standardized. Knowing the mapping = knowing my design is "the right shape."
6. **Windowed/streaming transfer** for throughput, and **power-loss-atomic config** (I have
   torn-write-is-blank; A/B config pages would make it atomic-update).

**Framing line:** "I optimized v1 for *correctness, portability, and unbrickability* under a hard
flash-size constraint. The seams I left — datagram crypto hook, image-level metadata, single-source
geometry, transport vtable — are exactly where signing, A/B, and UDS would slot in."

---

## 12. Likely interview questions → crisp answers

- **"Walk me through what happens from power-on to running app."** → §2 chain: reset → bootstrap
  validates BL (vector + CRC) → jumps → bootloader 5 s window listening on CAN → no update + valid
  app (CRC + vector) → jump to app.
- **"How do you jump from bootloader to app on Cortex-M?"** → Validate the app's vector table
  (SP in RAM, reset handler Thumb-bit set & in-region), then set MSP from `vectors[0]`, set
  `SCB->VTOR` to the app base, `DSB`/`ISB`, branch to `vectors[1]`. Disable/clear interrupts &
  peripherals first so the app starts clean.
- **"Why CRC and not a hash/signature?"** → CRC is *integrity*, catches corruption, cheap on an
  MCU. It's **not authenticity** — that needs a signature. v1 is integrity-only by scope; the seam
  for crypto is at the datagram level and signing would go at the image level. (Don't oversell CRC.)
- **"What if power dies mid-flash?"** → The app is being written directly, so the old app is gone,
  but `app_present` is only set after a full-image CRC passes — so on reboot the bootloader sees
  no valid app and stays in DFU waiting for a re-flash. No brick. The bootstrap is untouched.
- **"What if the config page write is torn?"** → It fails its size-delimited CRC → read as blank →
  treated as "no valid app/config" → safe stay-in-bootloader. Single page, no A/B, but safe.
- **"How do you address a specific board / all boards?"** → 64-bit `target_node_mask` in the
  datagram header; bit i = node i, 0 = broadcast. Multicast is free; the CAN arbitration ID only
  carries the *role* (fragment/ack/response).
- **"How do you reassemble a 2 KB image over 8-byte CAN frames?"** → Each frame is `[index:2][data:6]`;
  the reassembler places bytes at `index*6` in a blob, tracks a received bitmap, reads `total_length`
  from the header once those fragments arrive, completes when all indices are seen, then
  deserialize + verify CRC. Dup-safe and drop-safe.
- **"How is this testable without hardware?"** → §9: one `bl_port_*` seam, an x86 port with fake
  flash + FIFOs + mock clock, ~60 Unity tests covering the full protocol/DFU/gateway.
- **"How would you make this secure / production-automotive?"** → §11, lead with signed images +
  chain of trust, anti-rollback, then A/B, lock bits, UDS/ISO-TP.
- **"What was the hardest bug / a design mistake you corrected?"** → §13.

---

## 13. War stories (interviewers love a real correction)

- **CAN response framing.** My first cut of the discovery-response framing reused the fragment
  format (`[index:2][data:6]`, source only in the payload). I caught that concurrent responders to a
  broadcast QUERY would be indistinguishable until fully reassembled — so I changed the response
  frame to `[src_node:1][index:2][data:5]`, putting the source in **every** frame so the host can
  demux per-responder *before* reassembling. Lesson: think about the multi-talker case at the frame
  level, not just the happy path.
- **UART transport — I over-simplified, then fixed it.** I first shipped a length-delimited UART byte
  stream (different model from CAN). It worked but it was a one-off; I reworked it to the **same
  indexed-fragment model** as CAN so both transports share one mental model and one reassembler. That
  refactor is what let me extract `bl_reassembler` and delete the duplicate blob/bitmap/`try_complete`
  from both transports. Lesson: unify the abstraction even when the quick version passes tests.
- **Unified vocabulary.** I had "packet" (UART) vs "fragment" (CAN) for the same concept. Picked one —
  **fragment** (matches IP fragmentation and the CAN code) — across the whole codebase. Small thing,
  but it's the kind of consistency that makes a shared protocol layer legible.

---

## 14. Numbers & names cheat-sheet (so you don't fumble facts)

- Datagram header **21 bytes**; payload ≤ **2048 B**; max serialized **2069 B**.
- CAN strides: fragment **6**, response **5**; UART fragment stride **128**.
- A full 2 KB datagram = **345** CAN fragments (the stress test).
- Boot window **5 s**; watchdog **2 s**.
- Boot magic `BL_BOOT_ENTER_MAGIC = 0xB00710AD`; BootConfig magic `"BOOT" = 0x424F4F54`.
- CRC32: reflected, poly `0xEDB88320`, init/finalxor `0xFFFFFFFF` (zlib-compatible).
- L433 map: bootstrap 8 KB @ `0x08000000`, bootloader 32 KB @ `0x08002000`, app @ `0x0800A000`,
  config @ `0x0803F800`. CAN 500 kbps. Enter ID `0x010`, xfer base `0x100`.
- Datagram types: METADATA, CHUNK, JUMP_TO_APP, QUERY, QUERY_RESPONSE, PING, PONG, ACK, NACK.
- NACK codes: OK, CRC, SEQUENCE, INCOMPLETE, FLASH, OVERSIZED, BAD_STATE, NO_APP, INTERNAL.
  (CRC/SEQUENCE/INCOMPLETE = transient/retry; the rest = fatal/abort.)
- Status: `BlStatus` (internal, `BL_OK = 0`) vs `BlNackCode` (1-byte wire status).

---

## 15. Files to skim the night before

- `bootloader/inc/bl_datagram.h` — the wire header (recite it)
- `bootloader/inc/bl_transport.h` — the vtable (the architecture in one struct)
- `bootloader/src/bl_dfu.c` — the flash state machine
- `bootloader/src/bootloader.c` — superloop + router + gateway
- `common/src/bl_jump.c` — vector-table validation
- `common/inc/bootloader_config.h` — the forward-compatible BootConfig
- `transport/can/src/bl_transport_can.c` — fragmentation in practice
- `bootstrap/src/bl_bootstrap.c` — the safety-net first stage

**Honesty note for yourself:** the firmware is feature-complete and x86-unit-tested; the
`port/stm32l4` HAL, the linker `.noinit` wiring, and the Python host client are the remaining
hardware bring-up. If asked "did it flash a real board," say exactly that — full protocol/DFU proven
in simulation, hardware port is the next step — rather than overclaiming.
```
