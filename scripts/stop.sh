#!/bin/bash -i
#
# Convenience script for NOS3 development
# Use with the Dockerfile in the deployment repository
# https://github.com/nasa-itc/deployment
#

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/env.sh

# NOS3 GPIO
rm -rf /tmp/gpio_fake

# NOS3 Stored HK
rm -rf $BASE_DIR/fsw/build/exe/cpu1/scratch/*

# Docker stop
cd $SCRIPT_DIR; $DFLAG compose down > /dev/null 2>&1
# Only NOS3's own containers: those from the NOS3 and COSMOS images, and names that
# start with a NOS3 prefix. Name filters are regular expressions matched anywhere in
# the name, so every pattern is anchored; "sc_*" alone would match any name
# containing "sc".
NOS3_CONTAINERS=$(
    {
        $DCALL ps -aq --filter ancestor="$DBOX"
        $DCALL ps -aq --filter ancestor="ballaerospace/cosmos:4.5.0"
        for pattern in '^sc[0-9]+[-_]' '^nos[-_]' '^ait' '^influxdb' '^ttc-command' '^openc3-' '^cosmos-openc3-operator-1$'; do
            $DCALL ps -aq --filter name="$pattern"
        done
    } | sort -u
)

# Docker cleanup: stop and remove only those containers. A system-wide
# "container prune" would also delete unrelated stopped containers.
if [ -n "$NOS3_CONTAINERS" ]; then
    $DCALL stop $NOS3_CONTAINERS > /dev/null 2>&1
    $DCALL rm $NOS3_CONTAINERS > /dev/null 2>&1
fi
$DNETWORK ls --format '{{.Name}}' | grep -E '^(nos3-.+|cosmos-openc3-operator-1)$' | xargs -r $DNETWORK rm > /dev/null 2>&1
rm /dev/shm/Blackboard 2> /dev/null

# 42
rm -rf $USER_NOS3_DIR/42/NOS3InOut
rm -rf /tmp/gpio*

# COSMOS
yes | rm $GSW_DIR/Gemfile > /dev/null 2>&1
yes | rm $GSW_DIR/Gemfile.lock > /dev/null 2>&1

exit 0
