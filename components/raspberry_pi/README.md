# Raspberry Pi compatibility component

This directory contains the earlier generic Raspberry Pi cFS/NOS3 component.
It is retained for reference and generic NOS3 exercises, but it is **not** the
SPICEsat PayOBC flight interface.

## SPICEsat architecture

- cFS and `payload_if` run on the busOBC.
- The Raspberry Pi 4B is the separate PayOBC and runs Linux payload services,
  not a duplicate cFS `raspberry_pi` device app on the busOBC.
- BusOBC and PayOBC communicate over a point-to-point full-duplex RS-422 UART
  using the pinned payload-link frame around complete CCSDS Space Packets.
- APID ownership and direction are defined by `payload_if/fsw/payload-apids`.
- The old SBN/TCP prototype and the generic `0xDEAD`/`0xBEEF` UART protocol are
  not part of the mission architecture.

The mission spacecraft configuration therefore disables this component and
enables `payload_if`. Its cFS source, simulator, and ground definitions remain
available only to avoid removing generic NOS3 material before a separate
repository cleanup decision.

## PayOBC Python endpoint

`client/payobc_link.py` is the replacement Python prototype. Its framing and
decoder helpers have no third-party dependencies; live serial operation needs
`pyserial`.

```sh
cd components/raspberry_pi/client
python3 -m unittest -v test_payobc_link.py
python3 payobc_link.py /dev/ttyAMA0
```

The handler in `payobc_link.py` is deliberately small: it validates and logs
incoming packets and provides the integration point for payload command
dispatch. Mission payload services should register opcode/APID handlers there
without reimplementing framing, CRC, or stream recovery.
