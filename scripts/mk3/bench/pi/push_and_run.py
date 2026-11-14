#!/usr/bin/env python3
"""Copy a file to the SDR MK3 over its debug serial console with base64, then run commands.

Usage: push_and_run.py PORT LOCAL_FILE REMOTE_PATH LOG OUT CMD_TSV
  CMD_TSV lines are "name<TAB>timeout_seconds<TAB>command" (THREE columns, unlike probe.py)

The file is decoded on the board by `base64 -d`, then its sha256 is compared on both
ends. Throughput is about 2.5 KB/s (a 200 KB package takes about 80 s), so strip
binaries first.
"""
import base64
import hashlib
import os
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
        deadline = time.time() + seconds
        while True:
            left = deadline - time.time()
            if left <= 0:
                return buf
            ready, _, _ = select.select([self.fd], [], [], min(left, 0.05))
            if ready:
                chunk = os.read(self.fd, 4096)
                self.log.write(chunk)
                buf += chunk

    def read_until(self, needle, timeout):
        buf = b""
        deadline = time.time() + timeout
        while time.time() < deadline:
            buf += self.drain(0.2)
            if needle in buf:
                buf += self.drain(0.3)
                return buf, True
        return buf, False

    def send(self, data):
        for i in range(0, len(data), 32):
            os.write(self.fd, data[i:i + 32])
            time.sleep(0.004)

    def run(self, idx, cmd, timeout):
        start, end = "__S_%d__" % idx, "__E_%d__" % idx
        self.send(('echo __S""_%d__; %s; echo __E""_%d__\r' % (idx, cmd, idx)).encode())
        buf, ok = self.read_until(end.encode(), timeout)
        text = buf.decode(errors="replace").replace("\r", "")
        if ok and start in text:
            return text.split(start, 1)[1].split(end, 1)[0].strip("\n"), True
        return "!! TIMEOUT or markers missing; raw:\n" + text, False


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

        body, ok = con.run(0, "mkdir -p %s && echo dir-ok" % os.path.dirname(REMOTE), 15)
        out.write("=== mkdir ===\n%s\n" % body)
        if "dir-ok" not in body:
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

        body, ok = con.run(1, "sha256sum %s" % REMOTE, 30)
        remote_sha = body.split()[0] if body.split() else ""
        match = remote_sha == local_sha
        out.write("=== sha256 ===\nlocal  %s\nremote %s\nmatch: %s\n\n" % (local_sha, body, match))
        print("sha256 match:", match, flush=True)
        if not match:
            return 4

        with open(CMD_FILE) as f:
            cmds = [tuple(l.rstrip("\n").split("\t", 2)) for l in f if l.strip()]
        for i, (name, timeout, cmd) in enumerate(cmds, start=2):
            body, ok = con.run(i, cmd, float(timeout))
            out.write("=== %s ===\n$ %s\n%s\n\n" % (name, cmd, body))
            out.flush()
            print("%-16s %s" % (name, "ok" if ok else "TIMEOUT"), flush=True)
    os.close(fd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
