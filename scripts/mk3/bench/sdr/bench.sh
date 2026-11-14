# Console helpers for the busOBC (SDR MK3) bench. Load them once per login with
#     . /data/star/bench.sh
# then type `bench` for the list. They wrap sbtool, which talks to cFS over loopback UDP
# (CI_LAB for commands, TO_LAB for telemetry). Written for the board's BusyBox shell.

# Newest by the UTC time in the package name: the board's clock resets at boot, so file times lie
STAR_PKG=${STAR_PKG:-$(ls -d /data/star/spicesat-mk3-*/ 2>/dev/null | sed 's/.*-\(.*\)$/\1 &/' | sort | tail -n 1 | cut -d' ' -f2)}
STAR_PKG=${STAR_PKG%/}
SBTOOL=$STAR_PKG/bin/sbtool
BENCH_LOGS=${BENCH_LOGS:-/data/star-test}
CFS_LOG=$BENCH_LOGS/cfs.log
TLM_LOG=$BENCH_LOGS/tlm.txt
mkdir -p "$BENCH_LOGS"

bench() {
    cat << EOF
cFS package: $STAR_PKG
  status          health check: J14, cFS, telemetry recorder, J14 byte counters
  cfs_start       start cFS, turn on its telemetry, enable payload_if on J14 (/dev/ttyS3)
  cfs_stop        stop cFS (closes J14)
  mon_start       start recording all telemetry to $TLM_LOG
  mon             watch that recording live (Ctrl-C stops watching, recording continues)
  mon_stop        stop recording
  hk              ask payload_if for one housekeeping packet and print it
  pingpi [MET]    send the Pi a time sync ping (MET in ms, default 42), show the replies
  picmd OP [ARG]  send the Pi any command: opcode in hex, 32-bit argument (default 0)
  events [N]      last N cFS event messages (default 10)
EOF
}

status() {
    echo "J14 (pl-rs422):   $(systemctl is-active pl-rs422)"
    if pidof core-cpu1 > /dev/null; then
        echo "cFS:              running, pid $(pidof core-cpu1)"
    else
        echo "cFS:              stopped"
    fi
    if _mon_pid > /dev/null; then
        echo "telemetry:        recording to $TLM_LOG"
    else
        echo "telemetry:        not recording (mon_start)"
    fi
    echo "J14 bytes:        $(grep '^3:' /proc/tty/driver/serial | sed 's/.*irq:[0-9]* //')"
}

cfs_start() {
    if pidof core-cpu1 > /dev/null; then
        echo "cFS is already running (pid $(pidof core-cpu1))"
        return 0
    fi
    if [ ! -e /dev/ttyS3 ] || [ "$(systemctl is-active pl-rs422)" != active ]; then
        echo "J14 is not available (pl-rs422 is not active). See the bench guide, J14 section."
        return 1
    fi
    (cd "$STAR_PKG/cpu1" && exec ./core-cpu1 -R PO > "$CFS_LOG" 2>&1) &
    local i=0
    while ! grep -q 'entering OPERATIONAL' "$CFS_LOG" 2> /dev/null; do
        i=$((i + 1))
        if [ $i -gt 20 ]; then
            echo "cFS did not start within 20 s. Its output is in $CFS_LOG"
            return 1
        fi
        sleep 1
    done
    "$SBTOOL" to-enable > /dev/null
    "$SBTOOL" cmd 0x1850 2 > /dev/null
    sleep 1
    echo "cFS running (pid $(pidof core-cpu1)); $(grep -o 'PAYLOAD_IF: Device enabled.*' "$CFS_LOG" | tail -n 1)"
}

cfs_stop() {
    local p
    p=$(pidof core-cpu1) || { echo "cFS is not running"; return 0; }
    kill $p
    local i=0
    while pidof core-cpu1 > /dev/null && [ $i -lt 10 ]; do
        sleep 1
        i=$((i + 1))
    done
    if pidof core-cpu1 > /dev/null; then echo "cFS is still running after 10 s (pid $p)"; else echo "cFS stopped"; fi
}

# The recorder's pid, if that process is still an sbtool
_mon_pid() {
    local p
    p=$(cat "$BENCH_LOGS/tlm.pid" 2> /dev/null) || return 1
    [ "$(cat /proc/$p/comm 2> /dev/null)" = sbtool ] && echo $p
}

_need_mon() {
    _mon_pid > /dev/null && return 0
    echo "Telemetry is not being recorded. Run mon_start first."
    return 1
}

mon_start() {
    if _mon_pid > /dev/null; then
        echo "Already recording to $TLM_LOG"
        return 0
    fi
    "$SBTOOL" tlm 86400 > "$TLM_LOG" 2>&1 &
    echo $! > "$BENCH_LOGS/tlm.pid"
    echo "Recording telemetry to $TLM_LOG (mon to watch, mon_stop to stop)"
}

mon() { tail -f "$TLM_LOG"; }

mon_stop() {
    local p
    p=$(_mon_pid) || { echo "Not recording"; return 0; }
    kill $p 2> /dev/null
    rm -f "$BENCH_LOGS/tlm.pid"
    echo "Stopped recording"
}

hk() {
    _need_mon || return 1
    "$SBTOOL" cmd 0x1851 0 > /dev/null
    sleep 1
    grep 'PAYLOAD_IF HK' "$TLM_LOG" | tail -n 1
}

picmd() {
    if [ -z "$1" ]; then
        echo "usage: picmd OPCODE_HEX [ARGUMENT]    e.g. picmd 03 42"
        return 2
    fi
    _need_mon || return 1
    # CCSDS telecommand to APID 0x010 with 5 data bytes: opcode, then a 32-bit argument
    local hex
    hex=$(printf '1810C0000004%02X%08X' "0x$1" "${2:-0}") || return 2
    "$SBTOOL" cmd 0x1850 6 "$hex" > /dev/null
    echo "sent $hex"
    sleep 2
    tail -n 4 "$TLM_LOG"
}

pingpi() { picmd 03 "${1:-42}"; }

events() { grep 'EVS Port' "$CFS_LOG" | tail -n "${1:-10}"; }

echo "Bench helpers loaded. Type bench for the list."
