# Heater Component - NOS3

## Overview
This component simulates the Turbo Flex heater 220362 used for spacecraft thermal control. The heater is controlled by the EPS (Electrical Power System) via a switch and provides realistic thermal dynamics for testing thermal control algorithms.

## Hardware Specifications
Based on thermal team documentation:
- **Model**: Turbo Flex heater 220362
- **Size**: 2 inches diameter
- **Voltage**: 12V (nominal 11.1V in testing)
- **Current**: 2.62A
- **Power**: 31.4W
- **Resistance**: 4.59Ω
- **Max Temperature**: 125°C (257°F)

## Thermal Model
The simulator implements a simple linear thermal model:
- **Heating Rate**: 2°C/minute when powered ON
- **Cooling Rate**: 0.5°C/minute when powered OFF
- **Ambient Temperature**: 20°C (default, can be driven by 42 dynamics simulator)
- **Temperature Range**: Ambient to 125°C maximum

## Component Architecture

### Simulator (`sim/`)
The heater simulator models the physical behavior of the heating element:

1. **Hardware Model** (`heater_hardware_model.cpp`):
   - Registers as I2C slave at address `0x49` on bus `i2c_3`
   - Monitors power state controlled by EPS switch
   - Updates temperature every simulation tick based on power state
   - Sends temperature updates to TMP100 sensor via NOS Engine command bus
   - Provides telemetry via I2C registers

2. **Data Provider** (`heater_data_provider.cpp`):
   - Simple provider: returns fixed ambient temperature (20°C)
   - 42 provider: can integrate with 42 dynamics for spacecraft body temperature

3. **Data Point** (`heater_data_point.cpp`):
   - Wraps ambient temperature data
   - Lazy parsing from 42 data stream if available

### I2C Interface
The heater responds to standard I2C read/write commands:

**Registers:**
- `0x00` - STATUS: Bit 0 = power state (1=ON, 0=OFF)
- `0x01` - POWER: Write 0xAA to turn ON, 0x00 to turn OFF
- `0x02` - TEMPERATURE: Current heater surface temperature (16-bit, centi-degrees C)

**Usage Example:**
```c
// Turn heater ON
uint8_t cmd[2] = {0x01, 0xAA};
i2c_write(HEATER_ADDR, cmd, 2);

// Read temperature
uint8_t reg = 0x02;
i2c_write(HEATER_ADDR, &reg, 1);
uint8_t temp_data[2];
i2c_read(HEATER_ADDR, temp_data, 2);
int16_t temp_centi = (temp_data[0] << 8) | temp_data[1];
float temp_celsius = temp_centi / 100.0;
```

## Integration with NOS3 System

### Power Control Flow
```
Ground Station → Thermal Control App → EPS App → EPS Simulator
                                                        ↓
                                               Switch ON/OFF
                                                        ↓
                                        (NOS Engine command bus)
                                                        ↓
                                             Heater Simulator
```

### Thermal Data Flow
```
42 Dynamics → Heater Data Provider → Heater Simulator
                                            ↓
                                    (thermal model)
                                            ↓
                          (NOS Engine temperature update)
                                            ↓
                                     TMP100 Simulator
                                            ↓
                                      TMP100 App
                                            ↓
                                 Thermal Control App
```

## Configuration

### XML Configuration (`cfg/nos3-heater-simulator.xml`)
```xml
<simulator>
    <name>heater_sim</name>
    <active>true</active>
    <library>libheater_sim.so</library>
    <hardware-model>
        <type>HEATER</type>
        <connections>
            <connection type="command" bus-name="command" node-name="heater-sim-command-node"/>
            <connection type="i2c" bus-name="i2c_3" bus-address="0x49"/>
        </connections>
        <data-provider>
            <type>HEATER_PROVIDER</type>
        </data-provider>
        <physical>
            <power-watts>31.4</power-watts>
            <voltage>12.0</voltage>
            <current>2.62</current>
            <max-temperature>125.0</max-temperature>
            <heating-rate-celsius-per-sec>0.0333</heating-rate-celsius-per-sec>
            <cooling-rate-celsius-per-sec>0.00833</cooling-rate-celsius-per-sec>
        </physical>
    </hardware-model>
</simulator>
```

### EPS Configuration
The heater should be connected to EPS switch 1 (12V rail):
```xml
<switch-1>
    <node-name>heater-sim-command-node</node-name>
    <voltage>12.00</voltage>
    <current>2.62</current>
    <hex-status>0000</hex-status> <!-- Initially OFF -->
</switch-1>
```

## Command Interface
The heater simulator accepts backdoor commands via SimTerminal:

- `HELP` - Display available commands
- `ENABLE` - Enable heater (sent by EPS when switch turns ON)
- `DISABLE` - Disable heater (sent by EPS when switch turns OFF)
- `POWER_ON` - Manually turn heater ON
- `POWER_OFF` - Manually turn heater OFF
- `TEMPERATURE=X` - Set heater temperature to X°C
- `STOP` - Stop simulator

## Testing

### Manual Testing via SimTerminal
```bash
# Connect to heater simulator
SET SIMNODE heater-sim-command-node

# Check current state
HELP

# Manually turn heater ON
POWER_ON

# Monitor temperature (will rise at 2°C/min)
# Wait ~30 seconds, then check again

# Turn heater OFF
POWER_OFF
```

### Integration Testing
1. Start NOS3 system with heater simulator enabled
2. Use thermal_control app to command heater via EPS
3. Monitor TMP100 telemetry to see temperature changes
4. Verify closed-loop control: temperature oscillates between thresholds

## Related Components
- **TMP100**: Temperature sensor that reads environment temperature (influenced by heater)
- **EPS (generic_eps)**: Controls heater power via switch 1
- **Thermal Control App**: Implements closed-loop thermal control algorithm

## Future Enhancements
- [ ] More realistic thermal model with exponential heating/cooling curves
- [ ] Integration with 42 spacecraft thermal simulation
- [ ] Heat transfer to specific spacecraft components
- [ ] Multiple heater zones
- [ ] Power consumption tracking and reporting to EPS
- [ ] Failure modes (short circuit, open circuit, degraded performance)

## References
- Thermal Team Documentation
- TMP100 Component README
- Generic EPS Component README
- NOS3 Simulator Architecture Documentation
