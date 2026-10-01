#!/usr/bin/env python3
"""BusOBC PAYLOAD_IF <-> simulated PayOBC integration test, no ground software.

Runs inside a container on the same Docker network as the flight software
(hostname nos-fsw) and the NOS3 sim command bus bridge (cmdbus-bridge).

- Commands enter the cFE Software Bus through CI_LAB (UDP 5012), exactly as a
  ground system would inject them.
- Telemetry leaves through TO_LAB (UDP 5013), which this script points at
  itself and subscribes to the PayOBC telemetry message ID 0x0011. The
  startup RTS also points TO_LAB at host active-gs, so run_e2e.sh gives this
  container that network alias.
- Simulator faults are injected through the sim command bus bridge (TCP 12020).

Every expected PayOBC packet below is a literal byte string, derived by hand
from the status layout in ../../README.md, not produced by the code under test.
Launched by run_e2e.sh, which also checks the simulator log.
"""

import json
import socket
import struct
import sys
import time

FSW_HOST = "nos-fsw"
CI_LAB_PORT = 5012
TO_LAB_PORT = 5013
BRIDGE = ("cmdbus-bridge", 12020)
SIM_NODE = "payload_if-command"

TO_LAB_CMD_MID = 0x18E8
TO_LAB_OUTPUT_ENABLE_CC = 2
TO_LAB_ADD_PKT_CC = 6
PAYLOAD_IF_CMD_MID = 0x1850
PAYLOAD_IF_REQ_HK_MID = 0x1851
PAYLOAD_IF_HK_TLM_MID = 0x0860
PAYLOAD_IF_ENABLE_CC = 2
PAYLOAD_IF_DISABLE_CC = 3
PAYLOAD_IF_FORWARD_CC = 6
PAYOBC_WRAPPER_MID = 0x0862  # PAYLOAD_IF wrapper: telemetry header (BusOBC time) + PayOBC packet
PAYOBC_TLM_MID = 0x0011  # APID 0x011, telemetry, no secondary header (ICD RevB D6)

failures = []


def log(msg):
    print(msg, flush=True)


def check(condition, description):
    log(("PASS  " if condition else "FAIL  ") + description)
    if not condition:
        failures.append(description)
    return condition


def hexs(data):
    return data.hex(" ").upper()


def cfs_command(mid, cc, payload=b""):
    """cFE command: primary header, function code, checksum, payload."""
    length = 8 + len(payload)
    pkt = bytearray(struct.pack(">HHHBB", mid, 0xC000, length - 7, cc, 0) + payload)
    checksum = 0xFF
    for b in pkt:
        checksum ^= b
    pkt[7] = checksum
    return bytes(pkt)


def payobc_command(seq, counter):
    """ICD section 5.2 Run Experiment (opcode 0x20) on APID 0x010, argument 1.
    seq=0, counter=1 is the ICD RevB A1 frame test vector packet."""
    return bytes([0x10, 0x10, 0xC0 | (seq >> 8), seq & 0xFF, 0x00, 0x06,
                  0x20, counter >> 8, counter & 0xFF, 0x00, 0x00, 0x00, 0x01])


def payobc_status(seq, report, opcode, counter, accepted, bad_crc=0, resync=0, malformed=0, disallowed=0):
    """Expected PayOBC status packet (APID 0x011), see README.md."""
    return (bytes([0x00, 0x11, 0xC0 | (seq >> 8), seq & 0xFF, 0x00, 0x0D, report, opcode])
            + struct.pack(">HHHHHH", counter, accepted, bad_crc, resync, malformed, disallowed))


class Link:
    def __init__(self):
        self.tlm = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.tlm.bind(("0.0.0.0", TO_LAB_PORT))
        self.cmd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.bridge = socket.create_connection(BRIDGE, timeout=10)

    def uplink(self, packet):
        self.cmd.sendto(packet, (FSW_HOST, CI_LAB_PORT))

    def sim(self, command):
        log(f"      sim control: {command}")
        self.bridge.sendall((json.dumps({"node": SIM_NODE, "cmd": command}) + "\n").encode())
        time.sleep(0.3)

    def drain(self):
        self.tlm.setblocking(False)
        try:
            while True:
                self.tlm.recv(4096)
        except BlockingIOError:
            pass
        self.tlm.setblocking(True)

    def collect(self, mid, seconds, count=None):
        """Packets with the given message ID received within the window."""
        found = []
        deadline = time.time() + seconds
        while time.time() < deadline and (count is None or len(found) < count):
            self.tlm.settimeout(max(0.05, deadline - time.time()))
            try:
                pkt = self.tlm.recv(4096)
            except socket.timeout:
                break
            if len(pkt) >= 2 and struct.unpack(">H", pkt[:2])[0] == mid:
                found.append(pkt)
        return found

    def collect_any(self, seconds):
        """All packets received within the window."""
        found = []
        deadline = time.time() + seconds
        while time.time() < deadline:
            self.tlm.settimeout(max(0.05, deadline - time.time()))
            try:
                found.append(self.tlm.recv(4096))
            except socket.timeout:
                break
        return found

    def hk(self):
        """Current PAYLOAD_IF housekeeping counters.

        PAYLOAD_IF also reports HK periodically, so older reports can still be
        queued in TO_LAB when the requested one is sent. Request one and use
        the newest report received shortly afterwards. Counters are read
        relative to the end of the packet (HK_PERIOD is the last 2 bytes)."""
        self.drain()
        for _ in range(3):
            self.uplink(cfs_command(PAYLOAD_IF_REQ_HK_MID, 0))
            pkts = self.collect(PAYLOAD_IF_HK_TLM_MID, 2.5)
            if pkts:
                p = pkts[-1]
                return {"cmd_err": p[-19], "cmd": p[-18], "dev_err": p[-17], "dev": p[-16], "enabled": p[-15]}
        return None

    def exchange(self, seq, counter, seconds=5, count=1):
        self.drain()
        cmd = payobc_command(seq, counter)
        log(f"      uplink {hexs(cmd)}")
        self.uplink(cmd)
        pkts = self.collect(PAYOBC_TLM_MID, seconds, count=count)
        for p in pkts:
            log(f"      downlink {hexs(p)}")
        return pkts


def main():
    link = Link()
    my_ip = socket.gethostbyname(socket.gethostname())

    log("== Setup: TO_LAB output to this container, subscribe 0x0011, enable PAYLOAD_IF, reset sim")
    link.uplink(cfs_command(TO_LAB_CMD_MID, TO_LAB_OUTPUT_ENABLE_CC, my_ip.encode().ljust(16, b"\0")))
    time.sleep(0.5)
    # TO_LAB_AddPacket_Payload_t: MsgId (uint32, host order), QoS (2 x uint8), BufLimit (uint8), pad
    link.uplink(cfs_command(TO_LAB_CMD_MID, TO_LAB_ADD_PKT_CC, struct.pack("<IBBBx", PAYOBC_TLM_MID, 0, 0, 32)))
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_ENABLE_CC))
    link.sim("RESET=0")
    time.sleep(1)
    hk = link.hk()
    if not check(hk is not None, "PAYLOAD_IF housekeeping is downlinked through TO_LAB"):
        return
    check(hk["enabled"] == 1, f"PAYLOAD_IF device enabled (hk {hk})")

    log("== 1. ICD RevB A1 command reaches the sim; deterministic 0x011 telemetry is published")
    pkts = link.exchange(0, 1)
    check(pkts == [payobc_status(0, 1, 0x20, 1, 1)], "literal response: seq 0, counter 1, accepted 1")

    log("== 2. Second command changes simulator state")
    pkts = link.exchange(1, 2)
    check(pkts == [payobc_status(1, 1, 0x20, 2, 2)], "literal response: seq 1, counter 2, accepted 2")

    log("== 3. Corrupt CRC is dropped and counted; next frame succeeds without restart")
    before = link.hk()
    link.sim("CORRUPT_CRC")
    pkts = link.exchange(2, 3, seconds=3)
    check(pkts == [], "no telemetry published from the corrupt frame")
    after = link.hk()
    check(after["dev_err"] == before["dev_err"] + 1,
          f"PAYLOAD_IF DeviceErrorCount +1 for the bad CRC ({before['dev_err']} -> {after['dev_err']})")
    pkts = link.exchange(3, 4)
    check(pkts == [payobc_status(3, 1, 0x20, 4, 4)], "next valid frame accepted: seq 3, accepted 4")

    log("== 4. Frame split across UART writes is reassembled")
    link.sim("SPLIT=9")
    pkts = link.exchange(4, 5)
    check(pkts == [payobc_status(4, 1, 0x20, 5, 5)], "split response intact: seq 4, accepted 5")

    log("== 5. Two frames back-to-back in one UART write are both published")
    link.sim("DOUBLE")
    pkts = link.exchange(5, 6, count=2)
    check(pkts == [payobc_status(5, 1, 0x20, 6, 6), payobc_status(6, 1, 0x20, 6, 6)],
          "both packets published: seq 5 and 6")

    log("== 6. CCSDS length mismatch is dropped and counted")
    before = link.hk()
    link.sim("BAD_LENGTH")
    pkts = link.exchange(7, 7, seconds=3)
    check(pkts == [], "no telemetry published from the inconsistent packet")
    after = link.hk()
    check(after["dev_err"] == before["dev_err"] + 1,
          f"PAYLOAD_IF DeviceErrorCount +1 for the length mismatch ({before['dev_err']} -> {after['dev_err']})")

    log("== 7. Disallowed APID is dropped and counted")
    before = link.hk()
    link.sim("BAD_APID")
    pkts = link.exchange(8, 8, seconds=3)
    check(pkts == [], "no telemetry published from APID 0x010")
    after = link.hk()
    check(after["cmd_err"] == before["cmd_err"] + 1,
          f"PAYLOAD_IF CommandErrorCount +1 for the disallowed APID ({before['cmd_err']} -> {after['cmd_err']})")
    pkts = link.exchange(9, 9)
    check(pkts == [payobc_status(9, 1, 0x20, 9, 9)], "link recovers: seq 9, accepted 9")

    log("== 8. Old type-0 message ID 0x0010 is not forwarded (ICD RevB D6)")
    link.drain()
    link.uplink(bytes([0x00, 0x10]) + payobc_command(10, 10)[2:])
    check(link.collect(PAYOBC_TLM_MID, 3) == [], "no PayOBC response to MID 0x0010")
    pkts = link.exchange(10, 11)
    check(pkts == [payobc_status(10, 1, 0x20, 11, 10)], "0x0010 never reached the sim: accepted stays 10")

    log("== 9. Simulator reset to a known seed")
    link.sim("RESET=0x100")
    pkts = link.exchange(11, 12)
    check(pkts == [payobc_status(0x100, 1, 0x20, 12, 1)], "after RESET=0x100: seq 0x100, accepted 1")

    log("== 10. PAYLOAD_IF disable and re-enable restores communication")
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_DISABLE_CC))
    time.sleep(1)
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_ENABLE_CC))
    time.sleep(1)
    pkts = link.exchange(12, 13)
    check(pkts == [payobc_status(0x101, 1, 0x20, 13, 2)], "response after disable/enable: seq 0x101, accepted 2")

    log("== 11. Ground forward command and BusOBC-timestamped wrapper (contract: payload ground interface)")
    before = link.hk()
    link.drain()
    inner = payobc_command(13, 14)
    log(f"      forward {hexs(inner)}")
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_FORWARD_CC, inner))
    pkts = link.collect_any(5)
    raw = [p for p in pkts if p[:2] == bytes([0x00, 0x11])]
    wrapped = [p for p in pkts if struct.unpack(">H", p[:2])[0] == PAYOBC_WRAPPER_MID]
    expected = payobc_status(0x102, 1, 0x20, 14, 3)
    check(raw == [expected], "forwarded command answered: seq 0x102, counter 14, accepted 3")
    if check(len(wrapped) == 1, "reply also published once inside the 0x0862 wrapper"):
        w = wrapped[0]
        log(f"      wrapper {hexs(w[:16])} | {hexs(w[16:])}")
        check(w[16:] == expected, "wrapper carries the PayOBC packet unchanged after a 16-byte telemetry header")
        check(struct.unpack(">I", w[6:10])[0] > 0, f"wrapper stamped with BusOBC time ({struct.unpack('>I', w[6:10])[0]} s)")
    link.drain()
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_FORWARD_CC, bytes([0x00, 0x11]) + inner[2:]))
    check(link.collect(PAYOBC_TLM_MID, 3) == [], "forward command with disallowed APID 0x011 is not sent")
    after = link.hk()
    check(after["cmd_err"] == before["cmd_err"] + 1,
          f"PAYLOAD_IF CommandErrorCount +1 for the refused forward ({before['cmd_err']} -> {after['cmd_err']})")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:  # report, then fail the run
        failures.append(f"exception: {exc!r}")
        log(f"FAIL  exception: {exc!r}")
    log("")
    log(f"RESULT: {'FAIL' if failures else 'PASS'} ({len(failures)} failure(s))")
    for f in failures:
        log(f"  - {f}")
    sys.exit(1 if failures else 0)
