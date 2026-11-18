#!/usr/bin/env python3
"""Copy a file to the SDR MK3 over its debug serial console with base64, then run commands.

Usage: push_and_run.py PORT LOCAL_FILE REMOTE_PATH LOG OUT CMD_TSV
  CMD_TSV lines are "name<TAB>timeout_seconds<TAB>command" (THREE columns, unlike probe.py)

The file is decoded on the board by `base64 -d`, then its sha256 is compared on both
ends. A failed or timed-out remote command aborts the deployment. Throughput is
about 2.5 KB/s (a 200 KB package takes about 80 s), so strip binaries first.
"""
import base64
import hashlib
import os
import re
import select
import sys
import termios
import time

PORT, LOCAL, REMOTE, LOG, OUT, CMD_FILE = sys.argv[1:7]


def configure(fd):
    attrs = termios.tcgetattr(fd)
    cc = attrs[6]
    cc[termios.VMIN] = 0
    cc[termios.VTIME] = 0
    termios.tcsetattr(fd, termios.TCSANOW,
                      [0, 0, termios.CS8 | termios.CREAD | termios.CLOCAL, 0,
                       termios.B115200, termios.B115200, cc])
    termios.tcflush(fd, termios.TCIFLUSH)


class Console:
    def __init__(self, fd, log):
        self.fd, self.log = fd, log

    def drain(self, seconds):
        buf = b""
        deadline = time.monotonic() + seconds
        while True:
            left = deadline - time.monotonic()
            if left <= 0:
                return buf
            ready, _, _ = select.select([self.fd], [], [], min(left, 0.05))
            if ready:
                chunk = os.read(self.fd, 4096)
                self.log.write(chunk)
                buf += chunk

    def send(self, data):
        for i in range(0, len(data), 32):
            os.write(self.fd, data[i:i + 32])
            time.sleep(0.004)

    def run(self, idx, cmd, timeout):
        # Split each marker in the shell input so an echo of the typed command
        # cannot be mistaken for a completion marker.
        self.send(
            (f"printf '\\n%s\\n' __S''_{idx}__; {cmd}; "
             f"printf '\\n__E''_{idx}__=%s\\n' \"$?\"\r").encode()
        )
        result = re.compile(
            rf"^__S_{idx}__\r?\n(.*?)^__E_{idx}__=(\d+)\r?\n".encode(),
            re.MULTILINE | re.DOTALL,
        )
        buf = b""
        deadline = time.monotonic() + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            buf += self.drain(min(0.2, remaining))
            match = result.search(buf)
            if match:
                self.drain(0.3)
                output = match[1].decode(errors="replace").replace("\r", "").strip("\n")
                return output, int(match[2])
        text = buf.decode(errors="replace").replace("\r", "")
        return "!! TIMEOUT or markers missing; raw:\n" + text, None


def main():
    data = open(LOCAL, "rb").read()
    local_sha = hashlib.sha256(data).hexdigest()
    lines = base64.encodebytes(data).splitlines()

    fd = os.open(PORT, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    configure(fd)
    with open(LOG, "ab") as log, open(OUT, "w") as out:
        con = Console(fd, log)
        con.send(b"\r")
        prompt = con.drain(2.5).decode(errors="replace").replace("\r", "").strip()
        last = prompt.splitlines()[-1] if prompt else ""
        out.write("=== PROMPT CHECK ===\n%s\n" % last)
        if not (last.endswith("#") or last.endswith("$")):
            print("ABORT: no shell prompt. Last line: %r" % last)
            return 2

        body, status = con.run(0, "mkdir -p %s" % os.path.dirname(REMOTE), 15)
        out.write("=== mkdir ===\n%s\n" % body)
        if status != 0:
            print("ABORT: could not create remote directory")
            return 3

        # base64 -d reads until Ctrl-D at the start of a line
        con.send(("base64 -d > %s\r" % REMOTE).encode())
        con.drain(0.5)
        t0 = time.time()
        for n, line in enumerate(lines):
            con.send(line + b"\n")
            # ~8 KB/s, below the 11.5 KB/s line rate, and keep reading the echo
            con.drain(len(line) / 8000.0)
            if n % 500 == 0:
                print("sent %d/%d lines" % (n, len(lines)), flush=True)
        con.send(b"\x04")
        con.drain(2.0)
        print("transfer took %.1f s" % (time.time() - t0), flush=True)

        body, status = con.run(1, "sha256sum %s" % REMOTE, 30)
        remote_sha = body.split()[0] if body.split() else ""
        match = status == 0 and remote_sha == local_sha
        out.write("=== sha256 ===\nlocal  %s\nremote %s\nmatch: %s\n\n" % (local_sha, body, match))
        print("sha256 match:", match, flush=True)
        if not match:
            return 4

        with open(CMD_FILE) as f:
            cmds = [tuple(l.rstrip("\n").split("\t", 2)) for l in f if l.strip()]
        for i, (name, timeout, cmd) in enumerate(cmds, start=2):
            body, status = con.run(i, cmd, float(timeout))
            if status is None:
                outcome = "TIMEOUT"
            elif status != 0:
                outcome = f"FAILED (remote exit {status})"
            else:
                outcome = "ok"
            out.write(f"=== {name} ===\n$ {cmd}\n{body}\n{outcome}\n\n")
            out.flush()
            print("%-16s %s" % (name, outcome), flush=True)
            if status != 0:
                return 5
    os.close(fd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
