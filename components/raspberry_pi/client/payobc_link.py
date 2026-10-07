#!/usr/bin/env python3
"""SPICEsat PayOBC endpoint for the payload-link serial protocol.

The pure framing helpers have no external dependencies. Running the module as a
serial endpoint requires pyserial and dispatches validated CCSDS packets to the
supplied handler.
"""

from __future__ import annotations

import argparse
import binascii
import logging
import struct
from collections.abc import Callable, Iterable

SYNC = bytes.fromhex("1a cf fc 1d")
MIN_BODY = 7
MAX_BODY = 518
MAX_FRAME = len(SYNC) + 2 + MAX_BODY + 2

BUS_TO_PAY_APIDS = frozenset({0x010, 0x020, 0x022, 0x033, 0x034, 0x035})
PAY_TO_BUS_APIDS = frozenset({0x011, 0x012, 0x021, 0x030, 0x031, 0x032, 0x035})


def crc16_ccitt_false(data: bytes) -> int:
    return binascii.crc_hqx(data, 0xFFFF)


def encode_frame(packet: bytes) -> bytes:
    """Frame one complete CCSDS Space Packet for the RS-422 UART."""
    validate_ccsds(packet, BUS_TO_PAY_APIDS | PAY_TO_BUS_APIDS)
    if not MIN_BODY <= len(packet) <= MAX_BODY:
        raise ValueError(f"payload-link body must be {MIN_BODY}..{MAX_BODY} bytes")
    length = struct.pack(">H", len(packet))
    protected = length + packet
    return SYNC + protected + struct.pack(">H", crc16_ccitt_false(protected))


def validate_ccsds(packet: bytes, allowed_apids: Iterable[int]) -> int:
    """Validate the common CCSDS/header rules and return the packet APID."""
    if len(packet) < MIN_BODY:
        raise ValueError("CCSDS packet is shorter than the primary header plus one data byte")
    stream_id, sequence, data_length = struct.unpack(">HHH", packet[:6])
    if stream_id >> 13:
        raise ValueError("only CCSDS version 0 is supported")
    if sequence >> 14 != 0b11:
        raise ValueError("payload-link packets must be unsegmented")
    if data_length + 7 != len(packet):
        raise ValueError("CCSDS length field does not match the packet size")
    apid = stream_id & 0x07FF
    if apid not in allowed_apids:
        raise ValueError(f"APID 0x{apid:03X} is not allowed in this direction")
    return apid


class FrameDecoder:
    """Incremental payload-link decoder for arbitrary serial read boundaries."""

    def __init__(self) -> None:
        self._buffer = bytearray()

    def feed(self, data: bytes) -> list[bytes]:
        self._buffer.extend(data)
        packets: list[bytes] = []
        while True:
            sync_at = self._buffer.find(SYNC)
            if sync_at < 0:
                del self._buffer[:-3]
                break
            if sync_at:
                del self._buffer[:sync_at]
            if len(self._buffer) < 6:
                break
            body_length = struct.unpack(">H", self._buffer[4:6])[0]
            if not MIN_BODY <= body_length <= MAX_BODY:
                del self._buffer[0]
                continue
            frame_length = 4 + 2 + body_length + 2
            if len(self._buffer) < frame_length:
                break
            protected = bytes(self._buffer[4 : 6 + body_length])
            received_crc = struct.unpack(">H", self._buffer[6 + body_length : frame_length])[0]
            del self._buffer[:frame_length]
            if crc16_ccitt_false(protected) != received_crc:
                continue
            packet = protected[2:]
            try:
                validate_ccsds(packet, BUS_TO_PAY_APIDS)
            except ValueError:
                continue
            packets.append(packet)
        return packets


def run_serial(port: str, handler: Callable[[bytes], bytes | None], baud: int = 115200) -> None:
    """Run a blocking PayOBC endpoint; handler responses are framed back to BusOBC."""
    try:
        import serial
    except ImportError as exc:
        raise SystemExit("pyserial is required for serial operation") from exc

    decoder = FrameDecoder()
    with serial.Serial(port, baudrate=baud, bytesize=8, parity="N", stopbits=1, timeout=0.1) as uart:
        while True:
            for packet in decoder.feed(uart.read(4096)):
                response = handler(packet)
                if response is not None:
                    validate_ccsds(response, PAY_TO_BUS_APIDS)
                    uart.write(encode_frame(response))


def main() -> None:
    parser = argparse.ArgumentParser(description="Run the SPICEsat PayOBC payload-link endpoint")
    parser.add_argument("port", help="serial device, for example /dev/ttyAMA0")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO)

    def log_packet(packet: bytes) -> None:
        apid = validate_ccsds(packet, BUS_TO_PAY_APIDS)
        logging.info("received APID 0x%03X (%d bytes)", apid, len(packet))
        return None

    run_serial(args.port, log_packet, args.baud)


if __name__ == "__main__":
    main()
