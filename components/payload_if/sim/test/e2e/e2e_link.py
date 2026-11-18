#!/usr/bin/env python3
"""BusOBC PAYLOAD_IF <-> simulated PayOBC integration test, no ground software.

Runs inside a container on the same Docker network as the flight software
(hostname nos-fsw) and the NOS3 sim command bus bridge (cmdbus-bridge).

- Commands enter the cFE Software Bus through CI_LAB (UDP 5012), exactly as a
  ground system would inject them.
- Telemetry leaves through TO_LAB (UDP 5013) inside the PAYLOAD_IF wrapper.
  The startup RTS points TO_LAB at active-gs, an alias for this container.
- Simulator faults are injected through the sim command bus bridge (TCP 12020).

The expected field layouts follow the Pi bench definitions in ../../README.md.
The runner also checks the simulator log for the unchanged command bytes.
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
PAYLOAD_IF_CMD_MID = 0x1850
PAYLOAD_IF_REQ_HK_MID = 0x1851
PAYLOAD_IF_HK_TLM_MID = 0x0860
PAYLOAD_IF_ENABLE_CC = 2
PAYLOAD_IF_DISABLE_CC = 3
PAYLOAD_IF_FORWARD_CC = 6
PAYOBC_WRAPPER_MID = 0x0862  # PAYLOAD_IF wrapper: telemetry header (BusOBC time) + PayOBC packet
PAYOBC_HEARTBEAT = 0x0011
PAYOBC_CMD_RESPONSE = 0x0012
WRAPPER_HEADER_LEN = 16

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


def payobc_command(seq, argument, opcode=0x20):
    """Pi command on APID 0x010, with five data bytes."""
    return struct.pack(">HHHBI", 0x1010, 0xC000 | seq, 4, opcode, argument)


def is_response(wrapper, opcode=0x20, status=0):
    packet = wrapper[WRAPPER_HEADER_LEN:]
    return (len(packet) == 8 and packet[:2] == b"\x00\x12"
            and packet[2] & 0xC0 == 0xC0 and packet[4:] == bytes([0, 1, opcode, status]))


def heartbeat(wrapper):
    packet = wrapper[WRAPPER_HEADER_LEN:]
    if len(packet) == 24 and packet[:2] == b"\x00\x11" and packet[4:6] == b"\x00\x11":
        return struct.unpack(">QBBfI", packet[6:])
    return None


def sequence(wrapper):
    return struct.unpack_from(">H", wrapper, WRAPPER_HEADER_LEN + 2)[0] & 0x3FFF


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

    def collect(self, mid, seconds, count=None, inner_mid=None):
        """Packets with the given message ID received within the window."""
        found = []
        deadline = time.monotonic() + seconds
        while time.monotonic() < deadline and (count is None or len(found) < count):
            self.tlm.settimeout(max(0.05, deadline - time.monotonic()))
            try:
                pkt = self.tlm.recv(4096)
            except socket.timeout:
                break
            if len(pkt) < 2 or struct.unpack_from(">H", pkt)[0] != mid:
                continue
            if inner_mid is not None:
                if len(pkt) < WRAPPER_HEADER_LEN + 6:
                    continue
                if struct.unpack_from(">H", pkt, WRAPPER_HEADER_LEN)[0] != inner_mid:
                    continue
            found.append(pkt)
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

    def exchange(self, seq, argument, opcode=0x20, seconds=5, count=1):
        self.drain()
        cmd = payobc_command(seq, argument, opcode)
        log(f"      uplink {hexs(cmd)}")
        self.uplink(cmd)
        pkts = self.collect(PAYOBC_WRAPPER_MID, seconds, count=count, inner_mid=PAYOBC_CMD_RESPONSE)
        for p in pkts:
            log(f"      downlink {hexs(p)}")
        return pkts

    def next_heartbeat(self):
        pkts = self.collect(PAYOBC_WRAPPER_MID, 5, count=1, inner_mid=PAYOBC_HEARTBEAT)
        return heartbeat(pkts[0]) if pkts else None


def main():
    link = Link()
    my_ip = socket.gethostbyname(socket.gethostname())

    log("== Setup: TO_LAB output to this container, enable PAYLOAD_IF, reset sim")
    link.uplink(cfs_command(TO_LAB_CMD_MID, TO_LAB_OUTPUT_ENABLE_CC, my_ip.encode().ljust(16, b"\0")))
    time.sleep(0.5)
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_ENABLE_CC))
    link.sim("RESET=0")
    time.sleep(1)
    hk = link.hk()
    if not check(hk is not None, "PAYLOAD_IF housekeeping is downlinked through TO_LAB"):
        return
    check(hk["enabled"] == 1, f"PAYLOAD_IF device enabled (hk {hk})")

    log("== 1. Periodic heartbeat and the separate 0x012 command response")
    beat = link.next_heartbeat()
    check(beat is not None and beat[1:] == (0, 0, 36.0, 32000), f"initial heartbeat is idle ({beat})")
    next_beat = link.next_heartbeat()
    check(beat is not None and next_beat is not None and next_beat[0] > beat[0], "heartbeat MET advances")
    pkts = link.exchange(0, 1)
    check(len(pkts) == 1 and is_response(pkts[0]), "experiment 1 replies with opcode 0x20 and ACK")

    log("== 2. Command argument changes the experiment and the mission clock")
    pkts = link.exchange(1, 2)
    check(len(pkts) == 1 and is_response(pkts[0]), "experiment 2 ACK")
    beat = link.next_heartbeat()
    check(beat is not None and beat[1:3] == (1, 2), f"heartbeat reports experiment 2 running ({beat})")
    pkts = link.exchange(2, 0x01234567, opcode=0x03)
    check(len(pkts) == 1 and is_response(pkts[0], 0x03), "time sync ACK")
    beat = link.next_heartbeat()
    check(beat is not None and 0x01234567 <= beat[0] <= 0x01234567 + 10000,
          f"heartbeat reflects all four time-sync argument bytes ({beat})")
    pkts = link.exchange(3, 0, opcode=0x7E)
    check(len(pkts) == 1 and is_response(pkts[0], 0x7E, status=1), "unknown opcode replies with NACK")

    log("== 3. Corrupt CRC is dropped and counted; next frame succeeds without restart")
    before = link.hk()
    link.sim("CORRUPT_CRC")
    pkts = link.exchange(4, 3, seconds=3)
    check(pkts == [], "no command response published from the corrupt frame")
    after = link.hk()
    check(after["dev_err"] == before["dev_err"] + 1,
          f"PAYLOAD_IF DeviceErrorCount +1 for the bad CRC ({before['dev_err']} -> {after['dev_err']})")
    pkts = link.exchange(5, 4)
    check(len(pkts) == 1 and is_response(pkts[0]), "next valid response passes")

    log("== 4. Frame split across UART writes is reassembled")
    link.sim("SPLIT=9")
    pkts = link.exchange(6, 5)
    check(len(pkts) == 1 and is_response(pkts[0]), "split response intact")

    log("== 5. Two frames back-to-back in one UART write are both published")
    link.sim("DOUBLE")
    pkts = link.exchange(7, 6, count=2)
    check(len(pkts) == 2 and all(is_response(p) for p in pkts)
          and sequence(pkts[1]) == (sequence(pkts[0]) + 1) & 0x3FFF,
          "both ACKs published with consecutive sequence counts")

    log("== 6. CCSDS length mismatch is dropped and counted")
    before = link.hk()
    link.sim("BAD_LENGTH")
    pkts = link.exchange(8, 7, seconds=3)
    check(pkts == [], "no command response published from the inconsistent packet")
    after = link.hk()
    check(after["dev_err"] == before["dev_err"] + 1,
          f"PAYLOAD_IF DeviceErrorCount +1 for the length mismatch ({before['dev_err']} -> {after['dev_err']})")

    log("== 7. Disallowed APID is dropped and counted")
    before = link.hk()
    link.sim("BAD_APID")
    pkts = link.exchange(9, 8, seconds=3)
    check(pkts == [], "no command response published from APID 0x010")
    after = link.hk()
    check(after["cmd_err"] == before["cmd_err"] + 1,
          f"PAYLOAD_IF CommandErrorCount +1 for the disallowed APID ({before['cmd_err']} -> {after['cmd_err']})")
    pkts = link.exchange(10, 9)
    check(len(pkts) == 1 and is_response(pkts[0]), "link recovers")

    log("== 8. Old type-0 message ID 0x0010 is not forwarded (ICD RevB D6)")
    link.drain()
    link.uplink(bytes([0x00, 0x10]) + payobc_command(11, 10)[2:])
    check(link.collect(PAYOBC_WRAPPER_MID, 3, inner_mid=PAYOBC_CMD_RESPONSE) == [], "no PayOBC response to MID 0x0010")
    pkts = link.exchange(12, 11)
    check(len(pkts) == 1 and is_response(pkts[0]), "correct command MID remains functional")

    log("== 9. Simulator reset to a known seed")
    link.sim("RESET=0x100")
    beat = link.next_heartbeat()
    check(beat is not None and beat[1:3] == (0, 0), f"reset returns the heartbeat to idle ({beat})")
    pkts = link.exchange(13, 12)
    check(len(pkts) == 1 and is_response(pkts[0]) and 0x100 <= sequence(pkts[0]) < 0x110,
          "response sequence is near the reset seed while heartbeats share the sequence")

    log("== 10. PAYLOAD_IF disable and re-enable restores communication")
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_DISABLE_CC))
    time.sleep(1)
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_ENABLE_CC))
    time.sleep(1)
    pkts = link.exchange(14, 13)
    check(len(pkts) == 1 and is_response(pkts[0]), "ACK after disable/enable")

    log("== 11. Ground forward command and BusOBC-timestamped wrapper (contract: payload ground interface)")
    before = link.hk()
    link.drain()
    inner = payobc_command(15, 14)
    log(f"      forward {hexs(inner)}")
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_FORWARD_CC, inner))
    wrapped = link.collect(PAYOBC_WRAPPER_MID, 3, inner_mid=PAYOBC_CMD_RESPONSE)
    if check(len(wrapped) == 1, "reply published once inside the 0x0862 wrapper"):
        w = wrapped[0]
        log(f"      wrapper {hexs(w[:16])} | {hexs(w[16:])}")
        check(is_response(w), "wrapper carries the Pi ACK after a 16-byte telemetry header")
        check(struct.unpack(">I", w[6:10])[0] > 0, f"wrapper stamped with BusOBC time ({struct.unpack('>I', w[6:10])[0]} s)")
    link.drain()
    link.uplink(cfs_command(PAYLOAD_IF_CMD_MID, PAYLOAD_IF_FORWARD_CC, bytes([0x00, 0x11]) + inner[2:]))
    check(link.collect(PAYOBC_WRAPPER_MID, 3, inner_mid=PAYOBC_CMD_RESPONSE) == [],
          "forward command with disallowed APID 0x011 is not sent")
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
