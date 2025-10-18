# Raspberry Pi Component - NOS3 Integration Checklist

This checklist follows the NOS3 component integration process from `docs/wiki/NOS3_Generating_Component.md`.

## ✅ Prerequisites Completed
- [x] Component generated using `generate_template.sh raspberry_pi`
- [x] Unique PerfID configured (127)
- [x] Unique Message IDs configured (0x1900-0x1902, 0x0900-0x0902)
- [x] UART interface configured (usart_20)
- [x] SBN protocol implementation updated for compatibility
- [x] COSMOS command/telemetry definitions updated with correct opcodes
- [x] Python SBN client created for Raspberry Pi

## 🔧 Integration Steps (Following NOS3 Documentation)

### 1. **Add Component to NOS3 Simulator Configuration**
**File**: `cfg/sims/nos3-simulator.xml`

Add the following simulator block:
```xml
<simulator>
    <name>raspberry-pi-sim</name>
    <active>true</active>
    <library>libraspberry_pi_sim.so</library>
    <hardware-model>
        <type>RASPBERRY_PI</type>
        <connections>
            <connection><type>command</type>
                <bus-name>command</bus-name>
                <node-name>raspberry-pi-command</node-name>
            </connection>
            <connection><type>usart</type>
                <bus-name>usart_20</bus-name>
                <node-port>20</node-port>
            </connection>
        </connections>
        <data-provider>
            <type>RASPBERRY_PI_PROVIDER</type>
        </data-provider>
    </hardware-model>
</simulator>
```

### 2. **Update TO Configuration Tables**
**File**: `cfg/nos3_defs/tables/to_config.c`

Add include:
```c
#include "raspberry_pi_msgids.h"
```

Add to TO_ConfigTable:
```c
{CFE_SB_ValueToMsgId(RASPBERRY_PI_HK_TLM_MID),     {0, 0}, 4},
{CFE_SB_ValueToMsgId(RASPBERRY_PI_DEVICE_TLM_MID), {0, 0}, 4},
```

**File**: `cfg/nos3_defs/tables/to_lab_sub.c`

Add include:
```c
#include "raspberry_pi_msgids.h"
```

Add to TO_LAB_Subs:
```c
{CFE_SB_ValueToMsgId(RASPBERRY_PI_HK_TLM_MID),     CFE_SB_DEFAULT_QOS, 4},
{CFE_SB_ValueToMsgId(RASPBERRY_PI_DEVICE_TLM_MID), CFE_SB_DEFAULT_QOS, 4},
```

### 3. **Add to cFS Startup Script**
**File**: `cfg/nos3_defs/cpu1_cfe_es_startup.scr`

Add to CFE_APP table:
```
CFE_APP, /cf/raspberry_pi.so,     RASPBERRY_PI_AppMain,    RASPBERRY_PI,   81,   16384, 0x0, 0;
```

### 4. **Add to Build Targets**
**File**: `cfg/nos3_defs/targets.cmake`

Add line:
```cmake
add_cfe_app(raspberry_pi ${CFS_APP_PATH}/raspberry_pi/fsw/cfs)
```

### 5. **Add to Spacecraft Configuration**
**Files**: `cfg/spacecraft/sc-*.xml` (all relevant spacecraft configs)

Add to components section:
```xml
<raspberry_pi>
    <enable>true</enable>
</raspberry_pi>
```

### 6. **Configure SBN for External Pi**
**File**: `fsw/apps/sbn/tables/sbn_conf_tbl.c`

Add TCP protocol (if not already present):
```c
{
    .LibFileName = "sbn_tcp.so",
    .LibSymbol = "SBN_TCP_Ops",
    .ProtocolId = SBN_TCP_PROTOCOL_ID
}
```

Add Raspberry Pi peer to Networks array:
```c
{
    .ProcessorID = 100,
    .SpacecraftID = 0,
    .NetNum = 0,
    .ProtocolConfig = {
        .BufNum = 3,
        .BufSize = 16384,
        .ConnectOut = 0,  /* Accept connection from Pi */
        .ValidCheck = 1,
        .PeerAddr = "",
        .PeerPort = 2234,
    }
}
```

### 7. **Configure COSMOS Targets**
**File**: `gsw/cosmos/config/system.txt`

Add target declarations:
```ruby
DECLARE_TARGET RASPBERRY_PI
DECLARE_TARGET RASPBERRY_PI_INT
```

**File**: `gsw/cosmos/config/tools/cmd_tlm_server/cmd_tlm_server.txt`

Add interfaces:
```ruby
INTERFACE RASPBERRY_PI_INT tcpip_client_interface.rb host.docker.internal 1235 1235 10.0 nil BURST
  TARGET RASPBERRY_PI

INTERFACE RASPBERRY_PI_DEBUG_INT tcpip_client_interface.rb host.docker.internal 1236 1236 10.0 nil BURST
  TARGET RASPBERRY_PI
```

### 8. **Update Launch Scripts**
**File**: `scripts/checkout.sh`

Add checkout target:
```bash
# Launch Raspberry Pi standalone checkout
if [[ $* == *RASPBERRY_PI* ]]; then
    echo "Starting Raspberry Pi standalone checkout..."
    cd $BASE_DIR && make -C components/raspberry_pi checkout
fi
```

**File**: `scripts/fsw/fsw_cfs_launch.sh`

Add simulator launch:
```bash
# Raspberry Pi Simulator
if [ "$RASPBERRY_PI_EN" == "true" ]; then
    echo "Starting Raspberry Pi Simulator..."
    $SIM_BIN_DIR/nos3-single-simulator $RASPBERRY_PI_SIM_CONFIG &
    sleep 1
fi
```

### 9. **Update Configuration Script**
**File**: `scripts/cfg/configure.py`

Add configuration variables (around line 120):
```python
sc_raspberry_pi_en = sc_root.find('components/raspberry_pi/enable').text
```

Add to conditional processing (around line 494):
```python
if (sc_raspberry_pi_en != 'true'):
    lines[raspberry_pi_index] = sim_disabled
```

Add index finding (around line 483):
```python
if line.find('raspberry-pi-sim</name>') != -1:
    if (lines.index(line)) < raspberry_pi_index:
        raspberry_pi_index = lines.index(line) + 1
```

## 🧪 Testing Steps

### 1. **Build and Test**
```bash
# Configure and build
make config
make all

# Test standalone checkout (optional)
make checkout RASPBERRY_PI

# Launch full system
make launch
```

### 2. **Verify SBN Connection**
- Check SBN status in COSMOS
- Monitor for Raspberry Pi ProcessorID 100 connection
- Verify SBN telemetry shows peer connected

### 3. **Test with Physical Raspberry Pi**
```bash
# On Raspberry Pi
cd components/raspberry_pi/client
python3 sbn_pi_client.py <NOS3_HOST_IP>
```

### 4. **Verify COSMOS Integration**
- Send commands via COSMOS: `cmd("RASPBERRY_PI RASPBERRY_PI_NOOP_CC")`
- Monitor telemetry: `tlm("RASPBERRY_PI RASPBERRY_PI_HK_TLM COMMAND_COUNT")`
- Check device telemetry for sensor data

### 5. **End-to-End Test**
- Send command from COSMOS → FSW → SBN → Raspberry Pi
- Verify data flow: Raspberry Pi → SBN → FSW → TO → COSMOS

## 🔍 Troubleshooting

### No SBN Connection
- Verify port 2234 is accessible
- Check ProcessorID matches (100)
- Ensure SBN configuration is correct
- Review firewall settings

### No Telemetry in COSMOS
- Verify TO tables include Raspberry Pi MIDs
- Check COSMOS target configuration
- Ensure component is enabled in spacecraft config

### Commands Not Working
- Verify command MIDs match
- Check FSW app is loaded and running
- Review SBN subscription configuration

## 📋 Component Information

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Component Name** | raspberry_pi | |
| **PerfID** | 127 | Performance monitoring |
| **ProcessorID** | 100 | SBN peer identification |
| **UART Interface** | usart_20 | For simulator |
| **SBN Port** | 2234 | TCP connection |
| **Command MIDs** | 0x1900, 0x1901 | |
| **Telemetry MIDs** | 0x0900, 0x0901, 0x0902 | |
| **SBN Data MIDs** | 0x0902 (in), 0x1902 (out) | |

## ✅ Final Verification

- [ ] Component builds successfully
- [ ] Simulator launches without errors
- [ ] COSMOS shows component in system
- [ ] SBN connection established (if external Pi connected)
- [ ] Commands can be sent from COSMOS
- [ ] Telemetry appears in COSMOS
- [ ] End-to-end data flow verified

## 📚 References

- NOS3 Component Generation Guide: `docs/wiki/NOS3_Generating_Component.md`
- SBN Documentation: `fsw/apps/sbn/doc/cfs_sbn.md`
- SBN Client Documentation: `fsw/apps/sbn_client/README.md`