# Thermal Control Application - NOS3

## Overview
The thermal_control application implements closed-loop thermal control for spacecraft by monitoring the TMP100 temperature sensor and commanding the heater (via EPS switch 1) to maintain temperature within acceptable limits.

## Purpose
This cFS application demonstrates realistic spacecraft thermal control:
- Monitors spacecraft temperature via TMP100 sensor
- Implements hysteresis control algorithm to prevent chattering
- Commands EPS to power heater ON/OFF based on temperature thresholds
- Provides telemetry and ground control interface

## Architecture

### Software Bus Integration
**Subscriptions**:
- `TMP100_HK_TLM_MID` - Temperature sensor housekeeping telemetry
- `THERMAL_SEND_HK_MID` - Housekeeping request from scheduler
- `THERMAL_CMD_MID` - Ground commands

**Publications**:
- `THERMAL_HK_TLM_MID` - Thermal control housekeeping telemetry
- `GENERIC_EPS_CMD_MID` - Commands to EPS for heater control

### Message IDs
- Command MID: `0x1950`
- Send HK MID: `0x1951`
- Telemetry MID: `0x0950`

## Control Algorithm

### Hysteresis Control
The thermal control uses a simple but effective hysteresis algorithm:

```
Temperature
    ^
    |
 25°C |--------[HEATER OFF]--------
    |                            ↓
    |                         (cooling)
    |                            ↓
 20°C |--------[HEATER ON]---------
    |        ↑
    |     (heating)
    |        ↑
    +--------------------------------> Time
```

**States**:
- `THERMAL_STATE_DISABLED` - Control disabled by ground command
- `THERMAL_STATE_IDLE` - Initial state, waiting for first temperature reading
- `THERMAL_STATE_HEATING` - Heater ON, temperature rising
- `THERMAL_STATE_COOLING` - Heater OFF, temperature falling

**Transitions**:
1. **COOLING → HEATING**: When temperature drops below 20°C (low threshold)
2. **HEATING → COOLING**: When temperature rises above 25°C (high threshold)

**Benefits of Hysteresis**:
- Prevents rapid ON/OFF cycling (chattering)
- Reduces wear on power switching components
- More efficient power usage
- Extends heater lifetime

### Control Loop Frequency
- Temperature readings: Depends on TMP100 app schedule (typically 1 Hz)
- Control decisions: Made on each temperature update
- Heater switching: Event-driven (only on threshold crossings)

## Ground Commands

### Command Codes
| Code | Command | Description |
|------|---------|-------------|
| 0 | NOOP | No operation, increments command counter |
| 1 | RESET_COUNTERS | Reset all counters |
| 2 | ENABLE | Enable thermal control |
| 3 | DISABLE | Disable thermal control |
| 4 | SET_THRESHOLDS | Set temperature thresholds |
| 5 | HEATER_OVERRIDE_ON | Manually turn heater ON (bypasses control) |
| 6 | HEATER_OVERRIDE_OFF | Manually turn heater OFF (bypasses control) |

### Command Examples (COSMOS)

**Enable Thermal Control**:
```
THERMAL ENABLE
```

**Disable Thermal Control**:
```
THERMAL DISABLE
```

**Set Temperature Thresholds**:
```
THERMAL SET_THRESHOLDS with Low=22.0, High=28.0
```
Note: Low threshold must be less than high threshold, range is -40 to 125°C

**Manual Heater Override**:
```
THERMAL HEATER_OVERRIDE_ON    # Forces heater ON
THERMAL HEATER_OVERRIDE_OFF   # Forces heater OFF
```

**Reset Counters**:
```
THERMAL RESET_COUNTERS
```

## Telemetry

### Housekeeping Telemetry (`THERMAL_Hk_tlm_t`)

| Field | Type | Description |
|-------|------|-------------|
| CommandErrorCount | uint8 | Count of command errors |
| CommandCount | uint8 | Count of valid commands |
| ControlEnabled | uint8 | 1=enabled, 0=disabled |
| ControlState | uint8 | Current state (0=DISABLED, 1=IDLE, 2=HEATING, 3=COOLING) |
| HeaterState | uint8 | Current heater power state (1=ON, 0=OFF) |
| HeaterEpsSwitch | uint8 | EPS switch number controlling heater (default=1) |
| CurrentTemperature | float | Most recent temperature reading (°C) |
| TempLowThreshold | float | Turn heater ON below this (°C) |
| TempHighThreshold | float | Turn heater OFF above this (°C) |
| HeaterOnCount | uint32 | Number of times heater turned ON |
| HeaterOffCount | uint32 | Number of times heater turned OFF |

### Telemetry Monitoring (COSMOS)

**Key Fields to Monitor**:
- `ControlState`: Shows current control state
- `HeaterState`: Shows if heater is currently ON or OFF
- `CurrentTemperature`: Real-time temperature
- `HeaterOnCount` / `HeaterOffCount`: Tracks cycling behavior

**Normal Operation Indicators**:
- ControlState oscillates between HEATING (2) and COOLING (3)
- Temperature oscillates between low and high thresholds
- HeaterOnCount and HeaterOffCount increment slowly (minutes per cycle)

## Events

### Information Events
- `THERMAL_STARTUP_INF_EID (1)` - App initialized
- `THERMAL_COMMANDNOP_INF_EID (3)` - NOOP command received
- `THERMAL_COMMANDRST_INF_EID (4)` - Counters reset
- `THERMAL_ENABLED_INF_EID (10)` - Control enabled
- `THERMAL_DISABLED_INF_EID (11)` - Control disabled
- `THERMAL_HEATER_ON_INF_EID (12)` - Heater commanded ON
- `THERMAL_HEATER_OFF_INF_EID (13)` - Heater commanded OFF
- `THERMAL_THRESHOLD_SET_INF_EID (14)` - Thresholds updated

### Error Events
- `THERMAL_COMMAND_ERR_EID (2)` - Invalid command received
- `THERMAL_INVALID_MSGID_ERR_EID (5)` - Invalid message ID
- `THERMAL_LEN_ERR_EID (6)` - Invalid message length
- `THERMAL_PIPE_ERR_EID (7)` - Software Bus pipe error
- `THERMAL_THRESHOLD_ERR_EID (15)` - Invalid threshold values
- `THERMAL_EPS_CMD_ERR_EID (17)` - Failed to send EPS command

## Configuration

### Default Parameters
```c
#define THERMAL_TEMP_LOW_THRESHOLD   20.0  // Turn heater ON below this (°C)
#define THERMAL_TEMP_HIGH_THRESHOLD  25.0  // Turn heater OFF above this (°C)
#define THERMAL_HEATER_EPS_SWITCH    1     // EPS switch controlling heater
```

### Startup Configuration
Defined in `cpu1_cfe_es_startup.scr`:
```
CFE_APP, thermal_control, THERMAL_AppMain, THERMAL, 84, 16384, 0x0, 0;
```
- Priority: 84 (runs after TMP100 at 82)
- Stack Size: 16384 bytes
- Exception Action: 0 (restart app on exception)

## Integration

### Required Components
- **TMP100 App**: Provides temperature telemetry
- **EPS App**: Controls heater power via switch
- **Heater Simulator**: Models heater thermal dynamics
- **SCH App**: Triggers housekeeping telemetry

### Software Bus Dependencies
The app requires these message IDs to be defined:
- `TMP100_HK_TLM_MID` (from tmp100_msgids.h)
- `GENERIC_EPS_CMD_MID` (from generic_eps_msgids.h)

### EPS Integration
The thermal_control app sends commands to EPS:
```c
GENERIC_EPS_Switch_cmd_t cmd;
cmd.SwitchNumber = 1;           // Heater on switch 1
cmd.State = 0xAA;               // 0xAA = ON, 0x00 = OFF
```

EPS then sends ENABLE/DISABLE commands to the heater simulator via NOS Engine.

## Operation

### Normal Startup Sequence
1. cFS launches thermal_control app (priority 84)
2. App initializes, subscribes to TMP100_HK_TLM_MID
3. Control starts in ENABLED state with default thresholds
4. Waits for first TMP100 temperature reading
5. Begins thermal control based on temperature

### Typical Operating Cycle
1. **Cold Start** (temp = 18°C):
   - Detects temp < 20°C (low threshold)
   - Commands EPS: "Turn switch 1 ON"
   - State → HEATING
   - Temperature rises at ~2°C/min

2. **Warm Phase** (temp reaches 26°C):
   - Detects temp > 25°C (high threshold)
   - Commands EPS: "Turn switch 1 OFF"
   - State → COOLING
   - Temperature falls at ~0.5°C/min

3. **Cycle Repeats**:
   - Temperature oscillates between 20-25°C
   - Heater cycles ON/OFF every ~10-15 minutes

### Manual Override
Ground operators can override automatic control:
1. Send `THERMAL_HEATER_OVERRIDE_ON` to force heater ON
2. Automatic control is bypassed
3. Send `THERMAL_DISABLE` to stop automatic control
4. Send `THERMAL_ENABLE` to resume automatic control

## Troubleshooting

### Heater Not Turning ON
**Symptoms**: Temperature drops but heater stays OFF

**Checks**:
1. Verify control is enabled: `ControlEnabled = 1`
2. Check current temperature vs low threshold
3. Verify EPS command was sent (check event log)
4. Check EPS switch 1 configuration
5. Verify heater simulator is receiving ENABLE commands

### Temperature Not Changing
**Symptoms**: Temperature readings constant, heater toggles correctly

**Checks**:
1. Verify TMP100 is reading updated temperatures
2. Check heater simulator is updating temperature
3. Verify TMP100 is receiving temperature updates from heater
4. Check NOS Engine command bus connectivity

### Rapid Cycling
**Symptoms**: Heater turns ON/OFF very frequently (< 1 minute)

**Possible Causes**:
1. Thresholds too close together (increase hysteresis)
2. Heating/cooling rates mismatched
3. Temperature sensor noise

**Solution**: Increase threshold separation
```
THERMAL SET_THRESHOLDS with Low=18.0, High=28.0
```

### Control Not Responding
**Symptoms**: Temperature changes but control state doesn't change

**Checks**:
1. Verify thermal_control app is running
2. Check Software Bus subscriptions are active
3. Verify TMP100 telemetry is being published
4. Review event messages for errors
5. Check command/error counters in telemetry

## Performance

### CPU Usage
- **Idle**: Negligible (event-driven)
- **Active**: < 0.1% (simple threshold comparisons)
- **Peak**: Occurs on state transitions when sending EPS commands

### Memory
- Stack: 16 KB allocated
- Actual usage: ~4-6 KB
- Data segment: ~2 KB (global app data structure)

### Timing
- Response time: < 1 ms (from TMP100 telemetry to EPS command)
- Control loop: Driven by TMP100 telemetry rate (typically 1 Hz)
- State transitions: Event-driven, no periodic processing

## Testing

### Unit Testing
(To be implemented)
- Test control state transitions
- Test threshold validation
- Test command handling
- Test telemetry generation

### Integration Testing
1. Launch NOS3 with thermal control enabled
2. Monitor temperature and heater state in COSMOS
3. Verify closed-loop operation over 30+ minutes
4. Test manual override commands
5. Test threshold adjustment
6. Verify event generation

## Future Enhancements

1. **Advanced Control Algorithms**:
   - PID controller for tighter temperature regulation
   - Adaptive thresholds based on environmental conditions
   - Predictive control using thermal models

2. **Multi-Zone Control**:
   - Support multiple heaters and temperature zones
   - Coordinated control strategies
   - Zone priority management

3. **Safety Features**:
   - Over-temperature shutdown
   - Under-temperature alarms
   - Heater failure detection
   - Power consumption limits

4. **Telemetry Enhancements**:
   - Temperature history buffer
   - Min/max temperature tracking
   - Average power consumption
   - Control performance metrics

5. **Autonomy**:
   - Automatic threshold adjustment
   - Self-tuning control parameters
   - Fault detection and recovery

## References
- Thermal Control System Implementation Guide
- Heater Component README
- TMP100 Component README
- Generic EPS Component README
- cFS Application Developer's Guide

## Version History
- v1.0.0 - Initial implementation with hysteresis control
