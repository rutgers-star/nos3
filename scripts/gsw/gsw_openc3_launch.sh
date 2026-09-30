#!/bin/bash -i
#
# Convenience script for NOS3 development
#

CFG_BUILD_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SCRIPT_DIR=$CFG_BUILD_DIR/../../scripts
source $SCRIPT_DIR/env.sh
export GSW="openc3-openc3-operator-1"

# Debugging
#echo "Script directory = " $SCRIPT_DIR
#echo "Base directory   = " $BASE_DIR
#exit

#echo "Make /tmp folders..."
#mkdir /tmp/data 2> /dev/null
#mkdir /tmp/data/hk 2> /dev/null
#mkdir /tmp/uplink 2> /dev/null

echo "Prepare openc3 containers..."
cd $OPENC3_DIR
$OPENC3_PATH run

OPENC3_VERSION=$(sed -n 's/^OPENC3_TAG=//p' "$OPENC3_DIR/.env" | head -n 1)
OPENC3_VERSION=${OPENC3_VERSION:-unknown}
OPENC3_PORT=2900
OPENC3_URL="http://localhost:${OPENC3_PORT}"

# `openc3.sh run` starts services asynchronously.  Wait until the web UI can
# answer before reporting that OpenC3 is ready; do not open a host browser.
for _ in $(seq 1 30); do
    if curl --fail --silent --output /dev/null "$OPENC3_URL"; then
        echo "OpenC3 COSMOS ${OPENC3_VERSION} is up and running at ${OPENC3_URL}"
        # launch.sh sources this script; exit would end the whole launch
        return 0 2>/dev/null || exit 0
    fi
    sleep 1
done

echo "OpenC3 COSMOS ${OPENC3_VERSION} was started, but is not reachable at ${OPENC3_URL} yet."
exit 1
