#!/bin/bash -i
#
# Convenience script for NOS3 development
# Use with the Dockerfile in the deployment repository
# https://github.com/nasa-itc/deployment
#

# Note this is copied to ./cfg/build as part of `make config`
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $SCRIPT_DIR/../../scripts/env.sh

# Launch options, usually passed through make:
#   HEADLESS=1               Run every container detached instead of in a gnome-terminal tab.
#                            Follow one with `docker logs -f <name>`, or `docker attach <name>`
#                            for an interactive one (detach with Ctrl-P Ctrl-Q, not Ctrl-C).
#   COMPONENTS="eps rw ..."  Start only these simulated components (space or comma separated).
#                            Default: all. The NOS Engine server, time driver, flight software,
#                            and ground utilities (NOS terminals, sim command bus bridge) always
#                            run. "42" is the 42 dynamics and truth sims; "radio" includes the
#                            CryptoLib ground link.
SELECTABLE_COMPONENTS="42 onair cam css eps fss gps imu mag rw radio raspberry_pi sample st thruster tmp100 heater torquer payload_if"
COMPONENTS=${COMPONENTS//,/ }
for component in $COMPONENTS; do
    if [[ " $SELECTABLE_COMPONENTS " != *" $component "* ]]; then
        echo ""
        echo "    Unknown simulated component '$component' in COMPONENTS."
        echo "    Valid names: $SELECTABLE_COMPONENTS"
        echo ""
        exit 1
    fi
done

# True when the named component should be started
component_selected() {
    [ -z "$COMPONENTS" ] || [[ " $COMPONENTS " == *" $1 "* ]]
}

# Start one component container. The first argument is its terminal title (or
# --window and the title for its own window); the rest is the "docker run ..."
# command built from $DFLAGS.
launch_container() {
    local window=0
    if [ "$1" = "--window" ]; then
        window=1
        shift
    fi
    local title=$1
    shift
    if [ "$HEADLESS" = "1" ]; then
        # $1 $2 is "<docker|podman> run": -d detaches, and $DFLAGS' -it keeps stdin and a TTY for attach
        "$1" "$2" -d "${@:3}" > /dev/null
    elif [ $window -eq 1 ]; then
        gnome-terminal --title="$title" -- "$@" &
    else
        gnome-terminal --tab --title="$title" -- "$@"
    fi
}

# Check that local NOS3 directory exists
if [ ! -d $USER_NOS3_DIR ]; then
    echo ""
    echo "    Need to run make prep first!"
    echo ""
    exit 1
fi

# Check that configure build directory exists
if [ ! -d $BASE_DIR/cfg/build ]; then
    echo ""
    echo "    Need to run make config first!"
    echo ""
    exit 1
fi

echo "Make data folders..."
# FSW Side
mkdir $FSW_DIR/data 2> /dev/null
mkdir $FSW_DIR/data/cam 2> /dev/null
mkdir $FSW_DIR/data/evs 2> /dev/null
mkdir $FSW_DIR/data/hk 2> /dev/null
mkdir $FSW_DIR/data/inst 2> /dev/null
touch $FSW_DIR/data/dummy.txt
echo "1234567890" > $FSW_DIR/data/dummy.txt
truncate -s 1M $FSW_DIR/data/dummy.txt
# GSW Side
mkdir /tmp/nos3 2> /dev/null
mkdir /tmp/nos3/data 2> /dev/null
mkdir /tmp/nos3/data/cam 2> /dev/null
mkdir /tmp/nos3/data/evs 2> /dev/null
mkdir /tmp/nos3/data/hk 2> /dev/null
mkdir /tmp/nos3/data/inst 2> /dev/null
mkdir /tmp/nos3/uplink 2> /dev/null
cp $BASE_DIR/fsw/build/exe/cpu1/cf/cfe_es_startup.scr /tmp/nos3/uplink/tmp0.so 2> /dev/null
cp $BASE_DIR/fsw/build/exe/cpu1/cf/sample.so /tmp/nos3/uplink/tmp1.so 2> /dev/null

echo "Create ground networks..."
$DNETWORK create \
    --driver=bridge \
    --subnet=192.168.41.0/24 \
    --gateway=192.168.41.1 \
    nos3-core
echo ""

echo "Launch GSW..."
echo ""
source $BASE_DIR/cfg/build/gsw_launch.sh

echo "Create NOS interfaces..."
export GND_CFG_FILE="-f nos3-simulator.xml"
launch_container "NOS Terminal"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name "nos-terminal"        --network=nos3-core -w $SIM_BIN $DBOX ./nos3-single-simulator $GND_CFG_FILE stdio-terminal
launch_container "NOS UDP Terminal"  $DFLAGS -v $SIM_DIR:$SIM_DIR --name "nos-udp-terminal"    --network=nos3-core -w $SIM_BIN $DBOX ./nos3-single-simulator $GND_CFG_FILE udp-terminal
launch_container "NOS CmdBus Bridge"  $DFLAGS -v $SIM_DIR:$SIM_DIR --name "nos-sim-bridge"      --network=nos3-core -w $SIM_BIN $DBOX ./nos3-sim-cmdbus-bridge $GND_CFG_FILE

echo ""

# Note only currently working with a single spacecraft
export SATNUM=1

#
# Spacecraft Loop
#
for (( i=1; i<=$SATNUM; i++ ))
do
    export SC_NUM="sc0"$i
    export SC_NETNAME="nos3-"$SC_NUM
    export SC_CFG_FILE="-f nos3-simulator.xml" #"-f sc_"$i"_nos3_simulator.xml"

    # Debugging
    #echo "Spacecraft number        = " $SC_NUM
    #echo "Spacecraft network       = " $SC_NETNAME
    #echo "Spacecraft configuration = " $SC_CFG_FILE
    
    echo $SC_NUM " - Create spacecraft network..."
    $DNETWORK create $SC_NETNAME 2> /dev/null
    echo ""

    echo $SC_NUM " - Connect GSW " "${GSW:-cosmos-openc3-operator-1}" " to spacecraft network..."
    $DNETWORK connect  $SC_NETNAME "${GSW:-cosmos-openc3-operator-1}" --alias cosmos --alias active-gs
    echo ""

    echo $SC_NUM " - 42..."
    rm -rf $USER_NOS3_DIR/42/NOS3InOut
    cp -r $BASE_DIR/cfg/build/InOut $USER_NOS3_DIR/42/NOS3InOut
    if [ "$HEADLESS" = "1" ]; then
        # No display is used when headless: run 42 without its graphics front end
        sed -i 's/^TRUE\( *!  Graphics Front End?\)/FALSE\1/' $USER_NOS3_DIR/42/NOS3InOut/Inp_Sim.txt
    elif component_selected 42; then
        xhost +local:*
    fi
    component_selected 42 && launch_container $SC_NUM" - 42" $DFLAGS -e DISPLAY=$DISPLAY -v $USER_NOS3_DIR:$USER_NOS3_DIR -v /tmp/.X11-unix:/tmp/.X11-unix:ro --name $SC_NUM"-fortytwo" -h fortytwo --network=$SC_NETNAME -w $USER_NOS3_DIR/42 -t $DBOX $USER_NOS3_DIR/42/42 NOS3InOut
    echo ""

    echo $SC_NUM " - OnAIR..."
    component_selected onair && launch_container $SC_NUM" - OnAIR" $DFLAGS -v $BASE_DIR:$BASE_DIR --name $SC_NUM"-onair" --network=$SC_NETNAME -w $FSW_DIR -t $DBOX $SCRIPT_DIR/fsw/onair_launch.sh
    echo ""

    echo $SC_NUM " - Flight Software..."
    cd $FSW_DIR
    # Debugging
    # Replace `--tab` with `--window-with-profile=KeepOpen` once you've created this gnome-terminal profile manually
    launch_container --window $SC_NUM" - NOS3 Flight Software" $DFLAGS -v $BASE_DIR:$BASE_DIR --name $SC_NUM"-nos-fsw" -h nos-fsw --network=$SC_NETNAME -w $FSW_DIR --sysctl fs.mqueue.msg_max=10000 --ulimit rtprio=99 --cap-add=sys_nice $DBOX $SCRIPT_DIR/fsw/fsw_respawn.sh
    #gnome-terminal --window-with-profile=KeepOpen --title=$SC_NUM" - NOS3 Flight Software" -- $DFLAGS -v $BASE_DIR:$BASE_DIR --name $SC_NUM"-nos-fsw" -h nos-fsw --network=$SC_NETNAME -w $FSW_DIR --sysctl fs.mqueue.msg_max=10000 --ulimit rtprio=99 --cap-add=sys_nice $DBOX $FSW_DIR/core-cpu1 -R PO &
    echo ""

    echo $SC_NUM " - Simulators..."
    cd $SIM_BIN
    launch_container $SC_NUM" - NOS Engine Server" $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-nos-engine-server"  -h nos-engine-server --network=$SC_NETNAME -w $SIM_BIN $DBOX /usr/bin/nos_engine_server_standalone -f $SIM_BIN/nos_engine_server_config.json
    component_selected 42         && launch_container $SC_NUM" - 42 Truth Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-truth42sim"          -h truth42sim --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE  truth42sim
    
    $DNETWORK connect $SC_NETNAME nos-terminal
    $DNETWORK connect $SC_NETNAME nos-udp-terminal
    $DNETWORK connect $SC_NETNAME nos-sim-bridge

    # Component simulators
    component_selected cam        && launch_container $SC_NUM" - CAM Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-cam-sim"      --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE camsim
    component_selected css        && launch_container $SC_NUM" - CSS Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-css-sim"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-css-sim
    component_selected eps        && launch_container $SC_NUM" - EPS Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-eps-sim"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-eps-sim
    component_selected fss        && launch_container $SC_NUM" - FSS Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-fss-sim"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-fss-sim
    component_selected gps        && launch_container $SC_NUM" - GPS Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-gps-sim"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE gps
    component_selected imu        && launch_container $SC_NUM" - IMU Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-imu-sim"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-imu-sim
    component_selected mag        && launch_container $SC_NUM" - MAG Sim"      $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-mag-sim"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-mag-sim
    component_selected rw         && launch_container $SC_NUM" - RW 0 Sim"     $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-rw-sim0"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-reactionwheel-sim0
    component_selected rw         && launch_container $SC_NUM" - RW 1 Sim"     $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-rw-sim1"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-reactionwheel-sim1
    component_selected rw         && launch_container $SC_NUM" - RW 2 Sim"     $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-rw-sim2"      -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-reactionwheel-sim2
    component_selected radio      && launch_container $SC_NUM" - Radio Sim"    $DFLAGS -e "TCP_GROUND=1" -e "MULTI_GDS=0" -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-radio-sim"    -h radio-sim --network=$SC_NETNAME --network-alias=radio-sim -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-radio-sim
    component_selected raspberry_pi && launch_container $SC_NUM" - Raspberry Pi Sim" $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-raspberry-pi-sim" -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE raspberry-pi-sim
    component_selected sample     && launch_container $SC_NUM" - Sample Sim"   $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-sample-sim"   -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE sample-sim
    component_selected st         && launch_container $SC_NUM" - StarTrk Sim"  $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-startrk-sim"  -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-star-tracker-sim
    component_selected thruster   && launch_container $SC_NUM" - Thruster Sim" $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-thruster-sim" --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-thruster-sim
    component_selected tmp100     && launch_container $SC_NUM" - TMP100 Sim"   $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-tmp100-sim"   -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE tmp100-sim
    component_selected heater     && launch_container $SC_NUM" - Heater Sim"   $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-heater-sim"   -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE heater-sim
    component_selected torquer    && launch_container $SC_NUM" - Torquer Sim"  $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-torquer-sim"  -h trq-sim --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE generic-torquer-sim
    component_selected payload_if && launch_container $SC_NUM" - Payload IF Sim"   $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-payload_if-sim"   -v /dev/shm:/dev/shm --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE payload_if-sim

    # gnome-terminal --tab --title=$SC_NUM" - Blackboard Sim"  -- $DFLAGS -v $SIM_DIR:$SIM_DIR --name $SC_NUM"-blackboard-sim" -v /dev/shm:/dev/shm -h blackboard-sim --network=$SC_NETNAME -w $SIM_BIN $DBOX ./nos3-single-simulator $SC_CFG_FILE blackboard-sim
    # cp cfg/InOut/Inp_IPC.shmem.txt cfg/InOut/Inp_IPC.txt
    # cp cfg/sims/nos3-simulator.shmem.xml cfg/sims/nos3-simulator.xml
    
    echo ""
    echo $SC_NUM " - CryptoLib..."
    component_selected radio      && launch_container $SC_NUM" - CryptoLib GSW" $DFLAGS -e "STANDALONE_TCP=1" -e "GSWALIAS=cosmos" -e "CRYPTO_HOST=cryptolib" -v $BASE_DIR:$BASE_DIR --name $SC_NUM"-cryptolib-gsw"  -h cryptolib --network=$SC_NETNAME --network-alias=cryptolib -w $BASE_DIR/gsw/build $DBOX ./support/standalone
    echo ""
done

echo "NOS Time Driver..."
sleep 8
launch_container "NOS Time Driver"   $DFLAGS -v $SIM_DIR:$SIM_DIR --name nos-time-driver --network=nos3-core -w $SIM_BIN $DBOX ./nos3-single-simulator $GND_CFG_FILE time
sleep 1
for (( i=1; i<=$SATNUM; i++ ))
do
    export SC_NUM="sc0"$i
    export SC_NETNAME="nos3-"$SC_NUM
    export TIMENAME=$SC_NUM"-nos-time-driver"
    $DNETWORK connect --alias nos-time-driver $SC_NETNAME nos-time-driver
done
echo ""

echo "Docker launch script completed!"
if [ "$HEADLESS" = "1" ]; then
    echo ""
    echo "Running headless. Containers: $DCALL ps --format '{{.Names}}'"
    echo "  Follow output:   $DCALL logs -f sc01-nos-fsw"
    echo "  Interact:        $DCALL attach nos-time-driver   (detach with Ctrl-P Ctrl-Q)"
fi
