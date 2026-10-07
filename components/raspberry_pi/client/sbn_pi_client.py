#!/usr/bin/env python3
"""Compatibility entry point for the retired SBN PayOBC prototype.

SPICEsat's PayOBC uses the point-to-point payload-link serial protocol. This
filename remains so old deployment scripts fail with a useful migration path.
"""

raise SystemExit(
    "The SBN PayOBC prototype is retired. Run payobc_link.py with the RS-422 "
    "serial device instead (for example: payobc_link.py /dev/ttyAMA0)."
)
