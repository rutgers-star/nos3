#!/bin/bash
#
# Build the bench link plugin and load it into OpenC3 on this laptop. Starts OpenC3 if needed.
# The NOS3 simulator plugin, if installed, keeps its own interfaces; it is not changed.
#
HERE=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
OPENC3_DIR=${OPENC3_DIR:-$HOME/.nos3/openc3}
# The CLI container wants a terminal; script gives it one when this runs without one
openc3_cli() {
    if [ -t 0 ]; then
        "$OPENC3_DIR/openc3.sh" cli "$@"
    else
        script -qefc "$OPENC3_DIR/openc3.sh cli $*" /dev/null
    fi
}
PLUGIN=openc3-cosmos-star-bench
VERSION=1.0.$(date +%Y%m%d%H%M%S)

if ! curl --fail --silent --output /dev/null http://localhost:2900; then
    (cd "$OPENC3_DIR" && ./openc3.sh run) || exit 1
    for _ in $(seq 1 60); do
        curl --fail --silent --output /dev/null http://localhost:2900 && break
        sleep 2
    done
fi

# Build from a copy: the OpenC3 CLI runs in a container that mounts the working directory
cd "$OPENC3_DIR" || exit 1
rm -rf "$PLUGIN"
cp -r "$HERE/$PLUGIN" . || exit 1
cd "$PLUGIN" || exit 1
openc3_cli rake build VERSION=$VERSION || exit 1
openc3_cli validate ./$PLUGIN-$VERSION.gem DEFAULT || exit 1
openc3_cli load ./$PLUGIN-$VERSION.gem DEFAULT || exit 1
echo "Loaded $PLUGIN $VERSION. Open http://localhost:2900, Telemetry Viewer: PAYOBC LINK, PAYOBC DATA, GROUND FAULTS, BUSOBC HEALTH."

# OpenC3 keeps a packet count per telemetry packet in Redis, and its interface crashes on any
# count for a packet that no longer exists. Drop counts left by packets renamed or removed here.
REDIS_CONTAINER=$(cd "$OPENC3_DIR" && docker compose ps -q openc3-redis)
REDIS_USER=$(sed -n 's/^OPENC3_REDIS_USERNAME=//p' "$OPENC3_DIR/.env")
REDIS_PASS=$(sed -n 's/^OPENC3_REDIS_PASSWORD=//p' "$OPENC3_DIR/.env")
redis() { docker exec "$REDIS_CONTAINER" redis-cli --no-auth-warning --user "$REDIS_USER" --pass "$REDIS_PASS" "$@"; }
for TARGET in $(ls "$HERE/$PLUGIN/targets"); do
    KEY="DEFAULT__TELEMETRYCNTS__{$TARGET}"
    for PACKET in $(redis hkeys "$KEY"); do
        grep -qs "^TELEMETRY $TARGET $PACKET " "$HERE/$PLUGIN/targets/$TARGET"/cmd_tlm/* || redis hdel "$KEY" "$PACKET" > /dev/null
    done
done

# The NOS3 plugin's interfaces look for simulators that aren't running on the bench. Stop them
# retrying, so the bench link is the only thing active. Reconnect them in CmdTlmServer to go back.
KEY=$(sed -n 's/^OPENC3_SERVICE_PASSWORD=//p' "$OPENC3_DIR/.env")
for INTERFACE in DEBUG RADIO SIM_42_TRUTH_INT SIM_CMDBUS_BRIDGE_INT; do
    curl --silent --output /dev/null -H "Authorization: $KEY" -H 'Content-Type: application/json-rpc' \
        -d "{\"jsonrpc\":\"2.0\",\"method\":\"disconnect_interface\",\"params\":[\"$INTERFACE\"],\"id\":1,\"keyword_params\":{\"scope\":\"DEFAULT\"}}" \
        http://localhost:2900/openc3-api/api
done
