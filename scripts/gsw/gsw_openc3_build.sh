#!/bin/bash
#
# Convenience script for NOS3 development
#

CFG_BUILD_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SCRIPT_DIR=$CFG_BUILD_DIR/../../scripts
source $SCRIPT_DIR/env.sh
export GSW="openc3-openc3-operator-1"

# Keep the OpenC3 integration and container release paired with this NOS3 tree.
# Do not replace these with a branch name: a branch would make fresh builds vary
# over time.
OPENC3_NOS3_REPOSITORY="https://github.com/nasa-itc/openc3-nos3.git"
OPENC3_NOS3_REVISION="b65559fa9e7267fadcce34ab0e7bf719cabb3979"
OPENC3_IMAGE_TAG="6.3.0"

verify_openc3_image() {
    local image_name="$1"
    local expected_digest="$2"
    local actual_digests

    actual_digests=$($DCALL image inspect --format '{{range .RepoDigests}}{{println .}}{{end}}' "${image_name}:${OPENC3_IMAGE_TAG}")
    if ! grep -Fxq "${image_name}@${expected_digest}" <<< "$actual_digests"; then
        echo "Expected ${image_name}@${expected_digest}, but a different image was pulled."
        exit 1
    fi
}

# Check that local NOS3 directory exists
if [ ! -d $USER_NOS3_DIR ]; then
    echo ""
    echo "    Need to run make prep first!"
    echo ""
    exit 1
fi

echo "Prepare OpenC3 docker containers..."
if [ ! -d "$OPENC3_DIR/.git" ]; then
    git clone "$OPENC3_NOS3_REPOSITORY" "$OPENC3_DIR"
fi

if [ ! -d "$OPENC3_DIR/.git" ]; then
    echo ""
    echo "    OpenC3 Cloning Failed!"
    echo ""
    exit 1
fi

if ! git -C "$OPENC3_DIR" diff --quiet || ! git -C "$OPENC3_DIR" diff --cached --quiet; then
    echo ""
    echo "    OpenC3 checkout has tracked local changes; refusing to change its pinned revision."
    echo ""
    exit 1
fi

if ! git -C "$OPENC3_DIR" cat-file -e "${OPENC3_NOS3_REVISION}^{commit}" 2>/dev/null; then
    git -C "$OPENC3_DIR" fetch --no-tags origin "$OPENC3_NOS3_REVISION"
fi
git -C "$OPENC3_DIR" checkout --detach "$OPENC3_NOS3_REVISION"

if ! grep -qx "OPENC3_TAG=${OPENC3_IMAGE_TAG}" "$OPENC3_DIR/.env"; then
    echo ""
    echo "    Expected OpenC3 image tag ${OPENC3_IMAGE_TAG} was not found in the pinned checkout."
    echo ""
    exit 1
fi

$DOCKER_COMPOSE_COMMAND -f "$OPENC3_DIR/compose.yaml" pull
verify_openc3_image openc3inc/openc3-operator sha256:84bfd997268b9319722d6ce5de95c757e3d891261483a587eed8f9cc55803633
verify_openc3_image openc3inc/openc3-redis sha256:a0e6c008477e180c25ee991c9753d8961e5a306462935348fa135a7a0ca1eb81
verify_openc3_image openc3inc/openc3-cosmos-script-runner-api sha256:b4c44de4fe7191908150a4f3cea55071066b7652a83333c1e13cb68bd4173d68
verify_openc3_image openc3inc/openc3-cosmos-cmd-tlm-api sha256:a07ac21127495fd34fc6da347418dd88e46ede5f525d6d12e75d37adaaef5480
verify_openc3_image openc3inc/openc3-traefik sha256:92388c190e46c27be59ac755efe7a8c99f296e33a0f71376ebc0974a90e19f54
verify_openc3_image openc3inc/openc3-cosmos-init sha256:963b3beda3d9f99d51fa85ddc3dfbc817ad90366574d16def84c88f503814acf
verify_openc3_image openc3inc/openc3-minio sha256:8e59cd6ce26fda7721c2b2fafda371d1139a23f7f7c9886a0168e03b7a54cc3c
echo ""

echo "Launch openc3 containers..."
cd $OPENC3_DIR
$OPENC3_PATH run
echo ""

#echo "Set a password in openc3 via firefox..."
#echo "  Refresh webpage if error page shown."
#echo ""
#sleep 5
#firefox localhost:2900 &

# Start by changing to a known location
cd $OPENC3_DIR

# Delete generated state so a removed mission target cannot persist from a
# previous build.
rm -rf build openc3-cosmos-nos3
if [ -d "build" ]
then
    echo ""
    echo "ERROR: Failed to delete build directory!"
    echo ""
    exit 1
fi

# Start generating the plugin
mkdir build
# cd build
$OPENC3_CLI generate plugin nos3 --ruby
if [ ! -d "openc3-cosmos-nos3" ]
then
    echo ""
    echo "ERROR: cli generate plugin nos3 failed!"
    echo ""
    exit 1
fi

# Copy targets
mkdir openc3-cosmos-nos3/targets
cd openc3-cosmos-nos3/targets
targets=""
for i in $(find $BASE_DIR/components -name target.txt) 
do 
    j=$(dirname $i)
    cp -r $j .
    targets="$targets $(basename $j)"
done
for i in $(find $GSW_DIR/config/targets -name target.txt) 
do 
    j=$(dirname $i)
    cp -r $j .
    k=$(basename $j)
    targets="$targets $(basename $j)"
done
# Simulator control commands for the sim command bus bridge, one file per component
cp $BASE_DIR/components/*/gsw/*_SIM_CMD.txt SIM_CMDBUS_BRIDGE/cmd_tlm/
for i in $(find . -name *.txt)
do 
    sed -i -e 's/<%= CosmosCfsConfig::PROCESSOR_ENDIAN %>/LITTLE_ENDIAN/; s/<%=CF_INCOMING_PDU_MID%>/0x1800/; s/<%=CF_SPACE_TO_GND_PDU_MID%>/0x0800/;' $i
done
cd ..

# Copy lib
cp -r $GSW_DIR/lib .

# Create plugin.txt
echo "Create plugin..."
rm plugin.txt
if [ -f "plugin.txt" ]
then
    echo ""
    echo "ERROR: Failed to remove plugin.txt file!"
    echo ""
    exit 1
fi

for i in $targets
do
    if [ "$i" != "SIM_42_TRUTH" -a "$i" != "SYSTEM" -a "$i" != "TO_DEBUG" -a "$i" != "SIM_CMDBUS_BRIDGE" ]
    then
        debug=$i"_DEBUG"
        radio=$i"_RADIO"
        echo TARGET $i $debug >> plugin.txt
        echo TARGET $i $radio >> plugin.txt
    else
        echo TARGET $i $i >> plugin.txt
    fi
done
echo "" >> plugin.txt
echo "INTERFACE DEBUG udp_interface.rb nos-fsw 5012 5013 nil nil 128 10.0 nil" >> plugin.txt
for i in $targets
do
    if [ "$i" != "SIM_42_TRUTH" -a "$i" != "SYSTEM" -a "$i" != "TO_DEBUG" -a "$i" != "SIM_CMDBUS_BRIDGE" ]
    then
        debug=$i"_DEBUG"
        echo "   MAP_TARGET $debug" >> plugin.txt
    fi
done
echo "   MAP_TARGET TO_DEBUG" >> plugin.txt
echo "" >> plugin.txt

echo "INTERFACE RADIO udp_interface.rb cryptolib 6010 6011 nil nil 128 10.0 nil" >> plugin.txt
for i in $targets
do
    if [ "$i" != "SIM_42_TRUTH" -a "$i" != "SYSTEM" -a "$i" != "TO_DEBUG" -a "$i" != "SIM_CMDBUS_BRIDGE" ]
    then
        radio=$i"_RADIO"
        echo "   MAP_TARGET $radio" >> plugin.txt
    fi
done
echo "" >> plugin.txt

echo "INTERFACE SIM_42_TRUTH_INT udp_interface.rb truth42sim 5110 5111 nil nil 128 10.0 nil" >> plugin.txt
echo "   MAP_TARGET SIM_42_TRUTH" >> plugin.txt
echo "" >> plugin.txt

# Simulator control: newline-terminated JSON to the NOS3 sim command bus bridge
echo "INTERFACE SIM_CMDBUS_BRIDGE_INT tcpip_client_interface.rb nos-sim-bridge 12020 12020 10.0 nil TEMPLATE 0x0A 0x0A" >> plugin.txt
echo "   MAP_TARGET SIM_CMDBUS_BRIDGE" >> plugin.txt

# Capture date created
echo "" >> plugin.txt
echo "# Created on " $DATE >> plugin.txt
echo ""

# Build plugin
echo "Build plugin..."
$OPENC3_CLI rake build VERSION=1.0.$DATE
if [ ! -f "openc3-cosmos-nos3-1.0.$DATE.gem" ]
then
    echo ""
    echo "ERROR: cli rake build failed!"
    echo ""
    exit 1
fi
echo ""

## Install plugin
echo "Install plugin..."
cd $OPENC3_DIR/openc3-cosmos-nos3
# "cli load" installs (or upgrades) the plugin, creating its targets and interfaces.
# "geminstall" only stored the gem, so OpenC3 never had the NOS3 targets.
$OPENC3_CLI load ./openc3-cosmos-nos3-1.0.$DATE.gem DEFAULT
INSTALL_STATUS=$?

if [ $INSTALL_STATUS -eq 0 ]; then
    echo "Plugin installation successful"
else
    echo "Plugin installation failed with exit code: $INSTALL_STATUS"
    exit 1
fi
echo ""


echo "OpenC3 build script complete."
echo "Note that while this script is complete, OpenC3 is likely still be processing behind the scenes!"
sleep 15
echo "Done sleeping, but check cpu use prior to proceeding!"
echo ""
