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
- APID `0x010` must also be a telecommand with no secondary header (ICD RevB D6).
- Other allowed APIDs are counted as unhandled and get no response.

Output:

- Every accepted `0x010` command produces one status packet on APID `0x011`.
- Output is queued and written on NOS time ticks at no more than the UART line
  rate: `baud / 10` bytes per second (8N1), 115200 baud by default. A response
  larger than one tick's budget leaves in several writes.

## Status telemetry (APID 0x011)

Primary header: `00 11`, with telemetry type and no secondary header (ICD RevB
D6). Sequence flags are `11`. The 14-bit sequence count starts at the reset seed
and increments for every packet sent. User data is 14 bytes, big-endian:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | Report type: `01` command response, `02` requested through sim control |
| 1 | 1 | Opcode of the command (first user-data byte), 0 if none |
| 2 | 2 | Command counter echoed from user-data bytes 1-2 (ICD section 5.2), 0 if absent |
| 4 | 2 | Commands accepted since reset |
| 6 | 2 | Frames dropped for bad CRC |
| 8 | 2 | Frames dropped for invalid payload-link length |
| 10 | 2 | Packets dropped as malformed CCSDS |
| 12 | 2 | Packets dropped by the APID allowlist |

Counters are reported modulo 65536. The commands-accepted count and the
sequence count prove the simulator processed a command rather than echoing it.

## Packet vectors

After `RESET=0`, the ICD RevB A1 frame test vector:

```text
command  1A CF FC 1D 00 0D 10 10 C0 00 00 06 20 00 01 00 00 00 01 9B BA
response 1A CF FC 1D 00 14 00 11 C0 00 00 0D 01 20 00 01 00 01 00 00 00 00 00 00 00 00 F3 68
```

The next command (sequence 1, command counter 2):

```text
command  1A CF FC 1D 00 0D 10 10 C0 01 00 06 20 00 02 00 00 00 01 1A 2D
response 1A CF FC 1D 00 14 00 11 C0 01 00 0D 01 20 00 02 00 02 00 00 00 00 00 00 00 00 B6 34
```

These were computed independently with Python `binascii.crc_hqx`, not with the
code under test. The first command matches the frame in the ICD.

## Simulator control

Commands go to command node `payload_if-command`, for example through the sim
command bus bridge. The definitions are in `../gsw/PAYLOAD_IF_SIM_CMD.txt`. Fault
controls are one-shot and apply to the next outgoing frame or packet.

| Command | Effect |
|---|---|
| `RESET[=SEED]` | Clear counters, decoder, transmit queue, and pending faults. Set the sequence count to `SEED` (decimal or `0x` hex, 0 to 0x3FFF). Without a seed, reuse the last one. |
| `SEND_STATUS` | Queue one status packet with report type `02`. |
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

- Literal command and response vectors, and deterministic sequence counts.
- Minimum and maximum body sizes, and invalid payload-link lengths.
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
- Checks literal packets for the normal exchange, state change, bad CRC
  recovery, split and doubled frames, length and APID rejection, the old `0x0010`
  message ID, reset, and PAYLOAD_IF disable and re-enable.

The script also checks the simulator log for the unchanged command bytes.

`make fsw` builds the NOS3 target only when `fsw/build` was not configured with
`ENABLE_UNIT_TESTS=true`. After `make build-test` into the same directory, the
flight binary uses the Linux hardware UART driver and every UART open fails.

## Known limitations

- Only APID `0x010` has behavior. Other BusOBC-to-PayOBC APIDs are validated and
  counted, not answered.
- `payload-apids` has no packet-type field yet, so the D6 telecommand check for
  `0x010` is local to this simulator.
- payload-link does not rescan bytes consumed by a rejected frame. A real sync
  word inside a dropped frame's body is therefore missed.
- The YAMCS definition `../gsw/payload_if.xtce` still carries the component
  template's message IDs (0x18FA/0x18FB, 0x08FA/0x08FB) and does not match
  PAYLOAD_IF. The mission uses OpenC3, whose definitions are in `../gsw/PAYLOAD_IF`.
