# 10 — Gateway (UART → CAN bridging)

How one board relays a firmware update from a host on UART onto the CAN bus so other
nodes can be flashed through it (`laptop → UART → gateway → CAN → leaf`). This is the
"full vehicle" DFU path. It lands in **Phase 8** alongside the UART transport; nothing
here changes the wire format or the DFU flow, it only adds a forward path in the router.

> Authoritative wire details live in [07_protocol_spec.md](07_protocol_spec.md); this
> doc is the design rationale and the resolved decisions for the bridge.

---

## 1. The core idea: relay, do not transcode

The datagram is the universal unit and it is **transport neutral**. Its 21-byte header
(including `datagram_crc32`) is computed over the payload, never over the transport
framing. So a datagram that arrived over UART is **already** a valid CAN datagram.

```
        +------------------------- one datagram --------------------------+
        | [21B header: mask,type,id,total_len,crc32] [payload ... <=2KB]  |
        +----------------------------------------------------------------+
                  |                                       |
        UART transport frames it                CAN transport frames it
        as [idx:2][len:1][data:128][crc:4]      as [frag_idx:2][data:6] x N
        x N fragments                           fragments

        The header + payload bytes are IDENTICAL on both wires.
```

The gateway reassembles a whole datagram on the UART side, then hands that exact
`BlDatagram` to the CAN transport's `send_datagram`, which re-fragments it onto CAN. No
unwrap, no re-CRC, no per-chunk wrapper. A downstream node cannot tell a relayed flash
from a direct one.

```
   host (PC) ──UART──▶ Gateway (node 5) ──CAN──▶ target (node 8)

   UART:  type=FIRMWARE_CHUNK   mask=bit8   [binary]
                 │  gateway: "bit8 is not me" -> forward verbatim
                 ▼
   CAN :  type=FIRMWARE_CHUNK   mask=bit8   [binary]   (same bytes)
                 │
          node 8 bootloader: "bit8 is me" -> flash
```

The UART stream is **literally the normal DFU flow** (METADATA, CHUNK x N, JUMP), with
the `target_node_mask` pointed at the downstream node instead of at the gateway.

### 1a. Why not a REPACKAGE datagram type

An earlier idea was a dedicated `REPACKAGE` type wrapping each chunk. It was dropped: a
wrapper forces the gateway to parse + unwrap + re-emit every chunk, which **is**
transcoding, reintroduces a second header and CRC, and (because the inner target is a
single node) throws away multicast. Relay-by-mask keeps the byte-identical invariant and
keeps multicast for free (§5).

---

## 2. Selection: a compile flag, not a runtime mode

"Which board can bridge" is a **build-time** decision, gated by `BL_FEATURE_GATEWAY` in
`bootloader_user_config.h`. Only the board physically wired to the host UART is built
with the forward path, so only it can ever relay. That **is** the selection of the
bridge node, decided when you flash it.

There is **no runtime arm/disarm**. A compiled gateway forwards automatically on a
three-lock rule (the third lock is the direction guard from §3):

```
   forward this datagram?  =  BL_FEATURE_GATEWAY   (compiled in, this is the bridge)
                           AND arrived on UART      (downstream direction only, no echo)
                           AND addressed elsewhere  (mask is broadcast, or has a
                                                      bit other than my own node)
```

Arming would add nothing: a non-gateway board has no forward code and never sees host
UART traffic, and a gateway only ever forwards datagrams addressed to **someone else**
(the mask says so), which is exactly the host's intent every time. The host driving the
UART session is the trigger; the mask is the downstream selector.

---

## 3. Direction and the reverse path

Routing is directional and **asymmetric**. UART is the upstream (host) side, CAN is the
downstream (bus) side.

```
   datagrams  (METADATA / CHUNK / JUMP / QUERY / PING)   UART ──▶ CAN   (downstream only)
   acks       (ACK / NACK)                                CAN ──▶ UART   (upstream only)
   self addressed datagrams                               consumed locally, ack back
                                                          out the port they arrived on
```

Datagrams are forwarded **only** UART to CAN. The only thing that travels CAN to UART is
a relayed ack (§4). A datagram that arrives on the CAN side addressed elsewhere is **not**
forwarded onto UART, this asymmetry both matches the host and bus roles and removes any
chance of an echo loop (a gateway hearing its own CAN transmission would otherwise relay
it back upstream).

Because traffic is directional, the bridge is **stateless** for anything not addressed
to itself: it copies datagrams downstream and acks upstream.

---

## 4. ACK model: transparent relay

The gateway passes each downstream ACK/NACK **straight up** to the host and does **not**
aggregate or generate its own ACK for a forwarded datagram. The **host is the
sender-of-record**: it runs the §2 multi-target wait and retransmits to laggards (the
retransmit just flows back down through the gateway like any other request).

An ACK/NACK is a single frame, not a datagram, so it does **not** arrive through the
datagram callback. The transport vtable gains an inbound ack hook:

```
   on_ack(code, datagram_id, src_node, ctx)   downstream CAN ack -> relay up the UART
```

The ack frame carries a `src_node_id` so the host can tell **which** node acked
`datagram_id` (required for the multicast laggard retransmit in §5). `send_ack` therefore
takes the source node too. A leaf node never receives acks, so `on_ack` only does work on
a board built `BL_FEATURE_GATEWAY`.

This is the key to flow control. The host already gates its send loop on ACKs (it will
not send chunk N+1 until it has the ACK for chunk N). When that ACK is the downstream
node's **real** flash-and-verify ACK relayed back, the host's existing loop becomes the
end-to-end backpressure for free, and it naturally throttles the fast UART link down to
the slower CAN bus.

```
   host ─CHUNK(id=N)─▶ gateway ─fragments─▶ node 8 ──flash+verify──┐
   host ◀──────── ACK(id=N, src=8) relayed up ◀── BL_ACK ──────────┘
   host ─CHUNK(id=N+1)─▶ ...    (only after seeing the real ACK for N)
```

---

## 5. Multicast comes for free

`target_node_mask` is 64-bit, so one UART upload fans out:

```
   mask = bit8 | bit9 | bit12   ->  one upload, three boards flash at once
   mask = 0 (broadcast)         ->  every board on the bus flashes
```

The gateway forwards the datagram with that same mask onto CAN, and **each** downstream
bootloader independently honors its own bit. The host collects the per-`src_node` ACKs
relayed up and narrows the mask to laggards on retransmit (§2). A `REPACKAGE`-style
single inner target would lose this.

---

## 6. Bounded memory: per-datagram store-and-forward

The gateway relays **one datagram at a time**: reassemble a single datagram on the UART
side (<= 2 KB), re-fragment it onto CAN, let the host's ACK gate the next one. It never
buffers the whole ~421 KB image. Worst-case gateway RAM is one datagram buffer, the same
as any leaf node.

---

## 7. Not interrupting an in-flight relay

A gateway must not jump to its own application mid-campaign. A gateway **stays in the
bootloader for the whole host session** and only jumps to its own app on an explicit JUMP
datagram addressed to its own node. This avoids dropping the bridge during a lull between
boards (the host may pause between targets, and an idle-timeout auto-jump would strand the
rest of the campaign).

```
   suppress my own auto-jump  =  is a gateway     (an upstream transport is wired in)
   jump anyway                =  explicit JUMP addressed to my own node
```

Because being a gateway suppresses the auto-jump for the entire session, no relay-activity
timer is needed (the explicit-jump-only rule fully subsumes it). A leaf node is unchanged,
it still auto-jumps to a valid app once the 5 s boot window expires and no update is in
progress. The gateway is a normal ECU that also bridges (it has its own app), it just does
not auto-run that app while it is acting as the bridge.

---

## 8. The forward gate, end to end

```
   datagram arrives (reassembled + CRC verified)
        │
        ├─ addressed to me (mask == 0 or my bit set) ─▶ bl_dfu_process, ack back on
        │                                                the arriving transport
        │
        └─ BL_FEATURE_GATEWAY and arrived on UART and addressed elsewhere
                                                       ─▶ re-fragment onto CAN, verbatim
                                                          (downstream ack relays back up
                                                           via on_ack, §4)

   ack arrives on CAN (BL_FEATURE_GATEWAY)            ─▶ relay up the UART, src_node intact
```

A board has to be **built** to bridge, the datagram has to arrive **on the UART side**,
and it has to be **for someone else**. Nothing relays by accident, and nothing loops.

If re-fragmenting onto CAN fails (bus off, arbitration lost past retries), the gateway
synthesizes a **NACK upstream** for that `datagram_id` so the host retries immediately
rather than waiting out its timeout. `send_datagram` is allowed to block until all
fragments are out, the host is ack gated and waiting on that datagram anyway.

---

## 9. Constraints and notes

- **One gateway per bus segment.** If two boards on the same CAN bus were both built as
  gateways and both received the host stream, they would double-forward. This is an
  operator/wiring constraint (only one board is wired to the host UART), not enforced in
  firmware.
- **Translation is out of scope.** A dedicated relay type would only earn its keep if the
  UART and CAN sides used different addressing schemes or datagram formats. We use one
  unified node space and one datagram, so the bridge is a pure verbatim relay.
- **Encryption hook** (when added) sits at the datagram level, so a relayed datagram
  carries its encryption end to end and the gateway never needs the key.

---

## 10. x86 testability

The router and forward path are tested on the simulator without real UART: instantiate
**two simulated transports** (a loopback/in-memory transport plus the existing CAN sim
harness), wire the gateway between them, and assert that a datagram injected on the
upstream side appears, byte-identical, on the downstream side and that a downstream ACK
appears upstream. This mirrors the CAN inject/capture harness already used in P5/P6.

**Checkpoint (P8):** UART-only DFU of a directly attached board works; and
`host → UART → gateway → CAN → leaf` flashes a downstream board end to end, including a
multi-target fan-out where one node is dropped and the host retransmits to the laggard
only.

---

## 11. Interface changes this requires (P8)

The current single-transport router (`bootloader.c on_datagram`, one `s_transport`)
cannot express the bridge. P8 makes these concrete changes:

- **Transport vtable (`bl_transport.h`)**
  - add `on_ack` inbound hook so a gateway can receive a downstream ack:
    `void (*BlAckCb)(BlNackCode code, uint32_t datagram_id, uint16_t src_node, void *ctx)`
    passed alongside `BlDatagramCb` at `init`.
  - `send_ack` gains a source node: `send_ack(BlNackCode code, uint32_t datagram_id,
    uint16_t src_node)`, and the CAN ack frame carries `src_node_id` (§4).
- **Config (`bootloader.h`)**
  - `BlBootloaderConfig` grows from one `transport` to an upstream (UART) plus downstream
    (CAN) pair. A leaf build leaves upstream NULL, a gateway build sets both. The router
    registers `on_datagram`/`on_ack` with `ctx` = the owning transport so it knows the
    arrival side (which selects "the other" transport and enforces the §3 direction).
- **Router (`bootloader.c`)**
  - fill the existing non-self no-op with the §8 forward gate, add the `on_ack` relay,
    and the gateway jump-suppression (§7). The core keys off whether an upstream transport
    is present (`is_gateway()`), so the forward path needs no `#ifdef`, which keeps it x86
    testable.
- **Build**
  - `BL_FEATURE_GATEWAY` in `bootloader_user_config.h` is the host-side selector for
    whether the upstream (UART) transport is linked and wired into the config. A leaf build
    leaves `upstream` NULL and the relay code is simply never reached.
