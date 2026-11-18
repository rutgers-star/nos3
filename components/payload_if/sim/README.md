# PayOBC link simulator

`libpayload_if_sim.so` simulates the PayOBC at the far end of the BusOBC
`PAYLOAD_IF` UART. It is a deterministic link-integration model: it proves that
complete CCSDS packets cross the payload-link framing in both directions. It
does not simulate the camera, sensors, or the final payload application.

It replaces the legacy nine-byte `0xDEAD`/`0xBEEF` transaction.

## Structure

| File | Role |
|---|---|
| `inc/payobc_link_model.hpp`, `src/payobc_link_model.cpp` | Protocol behavior: decoding, validation, responses, fault injection, transmit queue. No NOS Engine dependency. |
| `inc/payload_if_hardware_model.hpp`, `src/payload_if_hardware_model.cpp` | NOS Engine glue: UART callback in, paced UART writes out on each time tick, sim-control commands. |
| `test/test_payobc_link_model.cpp` | GoogleTest unit tests for the model. |
| `test/e2e/` | Integration test against the real cFS `PAYLOAD_IF` app, without ground software. |

## Dependencies

The simulator builds against the **same submodules as the BusOBC FSW**:

- `../fsw/payload-link`: framing, CRC, and the streaming decoder. The simulator
  does not reimplement any framing logic.
- `../fsw/payload-apids`: APID constants and the BusOBC-to-PayOBC allowlist
  (`STAR::payload_apids`).

The pinned commits are recorded by the superproject (`git submodule status`).
payload-link must be at least `ca5053b`: LENGTH+BODY CRC coverage and C++
linkage guards.

## Link behavior

Frame profile, from payload-link `frame.h`:
`SYNC 1ACFFC1D | LENGTH (2, BE) | BODY (7..518) | CRC-16/CCITT-FALSE over LENGTH+BODY (2, BE)`.

Input:

- Arbitrary UART chunks go to one persistent decoder, so a frame may be split
  across chunks and a chunk may hold several frames.
- A frame with a bad CRC or an invalid length is dropped and counted, and the
  decoder hunts for the next sync.
- A decoded body is accepted only when all of these hold:
  - CCSDS version is 0.
  - The CCSDS length field agrees with the body length.
  - Sequence flags are `11` (unsegmented).
  - The APID passes `star_payload_apid_allowed_bus_to_pay()`.
- APID `0x010` must also be a telecommand with no secondary header (ICD RevB D6)
  and have exactly five data bytes: an opcode and a big-endian uint32 argument.
- Other allowed APIDs are counted as unhandled and get no response.

Output:

- Every well-formed `0x010` command produces one response on APID `0x012`.
- While enabled, the simulator emits a heartbeat on APID `0x011` once per
  simulated second.
- Output is queued and written on NOS time ticks at no more than the UART line
  rate: `baud / 10` bytes per second (8N1), 115200 baud by default. A response
  larger than one tick's budget leaves in several writes.

## Pi command response and heartbeat

The simulator uses the wire format in the Pi bench OpenC3 definitions. Command
responses use primary header packet ID `00 12` and two data bytes: the opcode
and result (`00` ACK, `01` NACK). The model handles DCE authority grant/revoke
(ACK), time sync, run/abort experiment, and emergency halt. An unknown opcode is
NACKed. The authority commands acknowledge receipt without modeling attitude control.

The heartbeat uses packet ID `00 11`. Both packets use telemetry type, no
secondary header, and unsegmented sequence flags. One 14-bit sequence count
starts at the reset seed and increments across both packet types. Heartbeat data
is 18 bytes, big-endian:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 8 | Mission elapsed time in milliseconds |
| 8 | 1 | Operating state: idle `00`, running `01`, fault `FF` |
| 9 | 1 | Active experiment ID, zero if none |
| 10 | 4 | CPU temperature as IEEE-754 binary32 (simulated 36 °C) |
| 14 | 4 | Storage in MB (simulated 32000) |

Use the `COUNTERS` simulator command for decoder and packet statistics.

## Packet vectors

With sequence seed 0 and no intervening heartbeat, Run Experiment with ID 1
and its ACK:

```text
command  1A CF FC 1D 00 0B 10 10 C0 00 00 04 20 00 00 00 01 09 CF
response 1A CF FC 1D 00 08 00 12 C0 00 00 01 20 00 10 B8
```

The next command (sequence 1, experiment ID 2):

```text
command  1A CF FC 1D 00 0B 10 10 C0 01 00 04 20 00 00 00 02 7E 7F
response 1A CF FC 1D 00 08 00 12 C0 01 00 01 20 00 BA E9
```

The CRCs were computed independently with Python `binascii.crc_hqx`.

## Simulator control

Commands go to command node `payload_if-command`, for example through the sim
command bus bridge. The definitions are in `../gsw/PAYLOAD_IF_SIM_CMD.txt`. Fault
controls are one-shot and apply to the next command response or requested
heartbeat. Periodic heartbeats leave these faults pending.

| Command | Effect |
|---|---|
| `RESET[=SEED]` | Reset the clock and experiment state. Clear counters, decoder, transmit queue, and pending faults. Set the sequence count to `SEED` (decimal or `0x` hex, 0 to 0x3FFF). Without a seed, reuse the last one. |
| `SEND_STATUS` | Queue one heartbeat immediately. |
| `CORRUPT_CRC` | Invert the last CRC byte of the next frame. |
| `SPLIT[=BYTES]` | Split the next frame across UART writes after `BYTES` bytes (default: half). |
| `DOUBLE` | Send the next telemetry as two valid frames (consecutive sequence counts) in one write. Combined with `CORRUPT_CRC`, it gives a bad frame followed by a good one. |
| `BAD_LENGTH` | Send the next packet with a CCSDS length one too large (the frame itself is valid). |
| `BAD_APID` | Send the next packet on APID `0x010`, which is not allowed PayOBC to BusOBC. |
| `ENABLE`, `DISABLE` | While disabled, UART input is ignored and output is held. |
| `COUNTERS` | Reply with the internal counters. |
| `HELP`, `STOP` | As for other NOS3 simulators. |

Optional `nos3-simulator.xml` settings: `<baud>` on the `usart` connection
(default 115200), and `<initial-sequence-count>` under `<hardware-model>`
(default 0).

## Tests

Unit tests need only the NOS3 build image. They use no NOS Engine and no ground
software:

```sh
docker run --rm -u $(id -u):$(id -g) -v $PWD:$PWD -w $PWD ivvitc/nos3-64:20260619 bash -c \
  "cmake -S components/payload_if/sim/test -B /tmp/payobc-test && cmake --build /tmp/payobc-test && ctest --test-dir /tmp/payobc-test --output-on-failure"
```

They cover:

- Literal command and response vectors, heartbeat fields, and deterministic sequence counts.
- Rejection of wrong command sizes and invalid payload-link lengths.
- Frames split at every byte boundary, and combined frames.
- Garbage before sync, and a bad CRC followed by a valid frame.
- CCSDS length and header errors, and allowed and disallowed APIDs.
- Partial output delivery, every fault control, and reset to a seed.

The integration test runs the real flight software and simulator:

```sh
make config && make fsw && make sim
components/payload_if/sim/test/e2e/run_e2e.sh
```

`run_e2e.sh` starts only the NOS Engine server, the time driver, cFS, this
simulator, and the sim command bus bridge, on a private Docker network. Then
`e2e_link.py`:

- Injects commands through CI_LAB (UDP 5012).
- Receives telemetry through TO_LAB (UDP 5013).
- Checks wrapped replies, heartbeat fields, state changes, bad CRC
  recovery, split and doubled frames, length and APID rejection, the old `0x0010`
  message ID, reset, and PAYLOAD_IF disable and re-enable.

The script also checks the simulator log for the unchanged command bytes.

`make fsw` builds the NOS3 target only when `fsw/build` was not configured with
`ENABLE_UNIT_TESTS=true`. After `make build-test` into the same directory, the
flight binary uses the Linux hardware UART driver and every UART open fails.

## Through OpenC3

```sh
make config && make fsw && make sim && make gsw
make launch            # or: make launch HEADLESS=1 COMPONENTS="payload_if radio"
```

In the OpenC3 UI (http://localhost:2900):

1. **Command Sender**: send `PAYLOAD_IF_DEBUG PAYLOAD_IF_ENABLE_CC`.
2. **Command Sender**: send `PAYLOAD_IF_DEBUG PAYOBC_CMD`. The defaults run
   experiment 1 (opcode `0x20`, argument 1).
3. **Packet Viewer**: `PAYLOAD_IF_DEBUG PAYOBC_CMD_RESPONSE` shows the echoed
   opcode and ACK/NACK. `PAYLOAD_IF_DEBUG PAYOBC_HEARTBEAT` shows the elapsed
   time and active experiment. Both show the BusOBC receive timestamp in
   `CCSDS_SECONDS` and `CCSDS_SUBSECS`.
4. **Command Sender**, target `SIM_CMDBUS_BRIDGE`: the `PAYLOAD_IF_SIM_*`
   commands reset the simulator and inject faults (see Simulator control).

`PAYOBC_CMD` carries the complete PayOBC packet in a PAYLOAD_IF forward command
(code 6). The heartbeat and response arrive inside PAYLOAD_IF wrapper message
`0x0862`, with the inner bytes unchanged.

The `_DEBUG` targets use CI_LAB and TO_LAB directly. The `_RADIO` targets go
through CryptoLib and the radio simulator, and their downlink starts only
after `CFS_RADIO TO_ENABLE_OUTPUT`.

## Known limitations

- Only APID `0x010` has behavior. Other BusOBC-to-PayOBC APIDs are validated and
  counted, not answered.
- `payload-apids` has no packet-type field yet, so the D6 telecommand check for
  `0x010` is local to this simulator.
- payload-link does not rescan bytes consumed by a rejected frame. A real sync
  word inside a dropped frame's body is therefore missed.
- The CryptoLib standalone ground tool forwards only packets whose first byte is
  `0x08` or `0x09`, plus idle packets and CFDP. It discards any other packet and
  the rest of its TM frame. PayOBC packets are downlinked inside the `0x0862`
  wrapper, so they are not affected. `make build-cryptolib` also applies
  `scripts/gsw/patches/cryptolib/`, which makes the tool forward every valid
  space packet.
- The mission ground definition is OpenC3 under `../gsw/PAYLOAD_IF`. The YAMCS
  XTCE file is retained only for NOS3 compatibility and is not the SPICEsat
  command-and-telemetry authority.
