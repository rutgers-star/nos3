# SPICEsat PayOBC integration checklist

The former SBN integration checklist is retired. The approved software boundary
is the `payload_if` RS-422 payload link.

- [x] Disable the legacy `raspberry_pi` cFS app/simulator in the mission config.
- [x] Enable the busOBC `payload_if` cFS app and PayOBC link simulator.
- [x] Use the shared payload-link sync/length/CRC-16 framing contract.
- [x] Use the generated payload APID registry for direction checks.
- [x] Replace the Python SBN prototype with `client/payobc_link.py`.
- [x] Test the Python encoder against the shared literal frame vector.
- [ ] Connect the Python endpoint to the flight RS-422 adapter and record the
  port/device configuration.
- [ ] Implement and verify mission opcode/APID dispatch on the PayOBC.
- [ ] Run hardware-in-the-loop link, restart, timeout, and throughput tests.

The remaining unchecked work requires the flight PayOBC image and RS-422
hardware; it is verification work, not permission to reactivate the SBN path.
