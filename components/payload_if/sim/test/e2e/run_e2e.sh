#!/bin/bash
#
# BusOBC PAYLOAD_IF <-> simulated PayOBC integration test without ground software.
#
# Starts only what the link needs on an isolated Docker network: NOS Engine
# server, time driver, cFS flight software, the payload_if simulator, and the
# sim command bus bridge. Runs e2e_link.py, checks the simulator log, and
# removes everything it started.
#
# Prerequisites: make prep, make config, make fsw, make sim.
# Usage: components/payload_if/sim/test/e2e/run_e2e.sh [log directory]
#
set -u

HERE=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
BASE_DIR=$(cd "$HERE/../../../../.." && pwd)
source "$BASE_DIR/scripts/env.sh" > /dev/null

LOG_DIR=${1:-$BASE_DIR/sims/build/payobc-e2e}
NET=payobc-e2e
PREFIX=payobc-e2e
CONTAINERS="$PREFIX-engine $PREFIX-time $PREFIX-fsw $PREFIX-sim $PREFIX-bridge $PREFIX-driver"
USER_FLAGS="-u $(id -u):$(id -g) -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro"

cleanup() {
    for c in $CONTAINERS; do
        $DCALL logs "$c" > "$LOG_DIR/$c.log" 2>&1 || true
        $DCALL rm -f "$c" > /dev/null 2>&1 || true
    done
    $DNETWORK rm "$NET" > /dev/null 2>&1 || true
}

wait_for_log() {
    local container=$1 pattern=$2 seconds=$3
    for _ in $(seq "$seconds"); do
        if $DCALL logs "$container" 2>&1 | grep -q "$pattern"; then
            return 0
        fi
        sleep 1
    done
    echo "Timed out waiting for '$pattern' in $container"
    return 1
}

for f in "$FSW_DIR/core-cpu1" "$SIM_BIN/nos3-single-simulator" "$SIM_BIN/nos3-sim-cmdbus-bridge" "$SIM_BIN/../lib/libpayload_if_sim.so"; do
    if [ ! -e "$f" ]; then
        echo "Missing $f; run make config, make fsw, and make sim first."
        exit 1
    fi
done

mkdir -p "$LOG_DIR"
cleanup
trap cleanup EXIT
$DNETWORK create "$NET" > /dev/null

echo "Starting NOS Engine server, time driver, flight software, payload_if-sim, and cmdbus bridge..."
$DCALL run -dit --name $PREFIX-engine --network $NET \
    --network-alias nos-engine-server --network-alias sc01-nos-engine-server $USER_FLAGS \
    -v "$SIM_DIR:$SIM_DIR" -w "$SIM_BIN" $DBOX \
    /usr/bin/nos_engine_server_standalone -f "$SIM_BIN/nos_engine_server_config.json" > /dev/null
sleep 2

$DCALL run -dit --name $PREFIX-time --network $NET $USER_FLAGS \
    -v "$SIM_DIR:$SIM_DIR" -w "$SIM_BIN" $DBOX \
    ./nos3-single-simulator -f nos3-simulator.xml time > /dev/null

$DCALL run -dit --name $PREFIX-sim --network $NET $USER_FLAGS \
    -v "$SIM_DIR:$SIM_DIR" -w "$SIM_BIN" $DBOX \
    ./nos3-single-simulator -f nos3-simulator.xml payload_if-sim > /dev/null

$DCALL run -dit --name $PREFIX-bridge --network $NET --network-alias cmdbus-bridge $USER_FLAGS \
    -v "$SIM_DIR:$SIM_DIR" -w "$SIM_BIN" $DBOX \
    ./nos3-sim-cmdbus-bridge -f nos3-simulator.xml > /dev/null

# Flight software runs as the invoking user, as make launch runs it. Running it as
# root leaves root-owned files (CryptoLib log.txt, EEPROM.DAT, ...) in $FSW_DIR,
# and CryptoLib then fails to initialize under make launch.
$DCALL run -dit --name $PREFIX-fsw -h nos-fsw --network $NET $USER_FLAGS \
    -v "$BASE_DIR:$BASE_DIR" -e LD_LIBRARY_PATH="$FSW_DIR:/usr/lib:/usr/local/lib" \
    -w "$FSW_DIR" --sysctl fs.mqueue.msg_max=10000 --ulimit rtprio=99 --cap-add=sys_nice \
    $DBOX bash -c "exec ./core-cpu1 -R PO" > /dev/null

wait_for_log $PREFIX-sim "Construction complete" 30 || exit 1
wait_for_log $PREFIX-fsw "PAYLOAD_IF App Initialized" 60 || exit 1
wait_for_log $PREFIX-fsw "CFE_ES_Main entering OPERATIONAL state" 60 || exit 1

echo "Running e2e_link.py..."
# SC RTS 001 points TO_LAB output at host active-gs during startup; answer to that name too
$DCALL run --name $PREFIX-driver -h payobc-e2e-drv --network $NET --network-alias active-gs $USER_FLAGS \
    -v "$HERE:$HERE:ro" $DBOX python3 "$HERE/e2e_link.py"
DRIVER_STATUS=$?

# The simulator must have received the uplinked command packets unchanged
SIM_LOG="$LOG_DIR/$PREFIX-sim.log"
$DCALL logs $PREFIX-sim > "$SIM_LOG" 2>&1
SIM_STATUS=0
for packet in 1010C000000620000100000001 1010C001000620000200000001; do
    if grep -q "accepted command packet $packet" "$SIM_LOG"; then
        echo "PASS  simulator received unchanged command packet $packet"
    else
        echo "FAIL  simulator did not log command packet $packet"
        SIM_STATUS=1
    fi
done
if grep -q "dropped" "$SIM_LOG"; then
    echo "FAIL  simulator dropped frames or packets from PAYLOAD_IF:"
    grep "dropped" "$SIM_LOG"
    SIM_STATUS=1
else
    echo "PASS  simulator dropped nothing sent by PAYLOAD_IF"
fi

echo "Logs: $LOG_DIR"
if [ $DRIVER_STATUS -ne 0 ] || [ $SIM_STATUS -ne 0 ]; then
    echo "E2E RESULT: FAIL"
    exit 1
fi
echo "E2E RESULT: PASS"
