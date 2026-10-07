# Superseded design export

This file previously contained an unversioned Markdown extraction of an early
SPICEsat design document. It is retained only as a stable pointer so old links
do not silently resolve to obsolete architecture.

Do not use the former contents as a design authority. Current mission intent
and interfaces are controlled by:

1. the current Mission Design Document (MDD);
2. `SPICEsat_Software_Design_Document_CDR_Rev1.tex` (SDD);
3. `SPICEsat_Command_and_Telemetry_List_CDR_Rev1.tex` (CTL); and
4. the applicable subsystem interface-control documents.

For implemented behavior, the repository's current `main` branch and pinned
dependencies are authoritative. Known document/code differences must be called
out explicitly in the SDD, CTL, or tracked work rather than copied into this
placeholder.

As of 2026-10-06, the implemented busOBC–PayOBC link uses the pinned
`payload-link` profile documented in `components/payload_if/README.md`:
4-byte sync, 2-byte big-endian body length, one complete CCSDS packet, and a
2-byte CRC-16/CCITT-FALSE over length plus body. It does not implement the old
SPICEnet COP-1/CRC-8/SNACK framing description.
