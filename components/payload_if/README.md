# BusOBC–PayOBC payload interface

`payload_if` is the BusOBC cFS endpoint for SPICEsat's dedicated PayOBC serial
link. It is not a NOS3 template device and it does not use the historical
`0xDEAD`/`0xBEEF` request/response protocol.

## Flight contract

- Physical interface: point-to-point full-duplex RS-422, exposed to software as
  a 115200-baud, 8N1 UART with no flow control.
- Frame: `1A CF FC 1D | body length (uint16, big-endian) | body | CRC-16`.
- CRC: CRC-16/CCITT-FALSE over the encoded length and body. The sync and CRC
  fields are excluded.
- Body: one complete CCSDS Space Packet, 7–518 bytes. CCSDS multi-byte fields
  are big-endian and packets must be unsegmented.
- Routing: APID ownership and direction come from the pinned `payload-apids`
  registry. The app rejects malformed packets and packets traveling in a
  disallowed direction.
- Recovery: the streaming decoder tolerates partial UART reads, multiple frames
  in one read, garbage before sync, and resumes sync search after an invalid
  length or CRC. This baseline has no COP-1 byte, SNACK, or automatic retry.

The framing implementation is the pinned `fsw/payload-link` library. Both the
flight app and simulator link the same implementation; neither maintains a
private framing copy.

## cFS and ground boundary

BusOBC-to-PayOBC packets are published on their internal payload MID and are
framed unchanged for UART. PayOBC-to-BusOBC packets are validated and published
unchanged on the cFS software bus. For ground transport, payload APIDs are
carried inside PAYLOAD_IF messages so internal payload APIDs never collide with
the spacecraft's S-band packet dictionary:

- uplink: `PAYLOAD_IF_CMD_MID`, function code `PAYLOAD_IF_FORWARD_CC`, followed
  by the complete PayOBC CCSDS packet;
- downlink: `PAYLOAD_IF_PAYOBC_TLM_MID`, BusOBC receive timestamp, followed by
  the complete PayOBC CCSDS packet.

OpenC3 command and telemetry definitions live under `gsw/PAYLOAD_IF`. The
`PAYOBC_CMD` and `PAYOBC_STATUS` definitions exercise this encapsulation. The
legacy YAMCS XTCE file is retained for NOS3 compatibility but is not the
SPICEsat ground authority.

## Implementation map

- `fsw/cfs`: cFS app, asynchronous UART receive task, APID validation, and
  ground wrappers.
- `fsw/payload-link`: framing, CRC, and streaming decoder.
- `fsw/payload-apids`: generated APID registry and direction allowlists.
- `sim`: deterministic PayOBC link model and fault injection.
- `gsw/PAYLOAD_IF`: OpenC3 command, telemetry, and procedures.

The older generic device housekeeping/data structures remain only for NOS3
component ABI and test compatibility. They do not define the PayOBC wire
protocol. Function code 4 (`PAYLOAD_IF_CONFIG_CC`) is reserved and rejected
until a configuration packet is assigned in the payload APID registry.

## Verification

The cFS unit suite covers framing, UART chunking, APID direction, partial
writes, restart, periodic housekeeping, and ground wrapping. The simulator
suite and its end-to-end runner are documented in [sim/README.md](sim/README.md).

```sh
make build-test
make test-fsw

cmake -S components/payload_if/sim/test -B /tmp/payobc-test
cmake --build /tmp/payobc-test
ctest --test-dir /tmp/payobc-test --output-on-failure
```
