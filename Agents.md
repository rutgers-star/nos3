# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

NOS3 (NASA Operational Simulator for Small Satellites) is a comprehensive suite of tools for satellite software development, testing, and simulation. The system consists of:
- Flight Software (FSW) - using cFS (Core Flight System) or F Prime
- Ground Software (GSW) - using COSMOS or YAMCS  
- Hardware simulators and models
- Mission configuration system

## Installation and Setup

### Prerequisites for Native Linux (without Vagrant/VirtualBox)

If running NOS3 directly on Linux (not in a Vagrant VM), you must have:
- **Docker or Podman** installed and configured
- **Python 3** with pip and venv (`sudo apt install python3-pip python3-venv python3-dev`)
- User added to docker group: `sudo usermod -aG docker $USER` (then reboot)

### First-Time Setup

**IMPORTANT**: Before building NOS3 for the first time, you MUST run:

```bash
# 1. Clone submodules (if not already done)
git submodule update --init --recursive

# 2. Set up Python virtual environment (recommended for Linux)
python3 -m venv .venv
source .venv/bin/activate

# 3. Prepare the environment (downloads Docker images, sets up ~/.nos3)
make prep

# 4. Configure the mission
make config

# 5. Now you can build
make all
```

The `make prep` step is critical - it downloads the required Docker image and sets up the build environment. **All builds run inside Docker containers**, not directly on your host system.

### Docker vs Podman

NOS3 build scripts use Docker by default. If using Podman as a Docker replacement (common on Arch Linux):
- **The `scripts/env.sh` has been modified to detect rootless Podman** and adjust flags accordingly
- Rootless Podman requires running containers without the `-u` (user) flag to avoid permission issues
- Ensure `/etc/containers/nodocker` exists to suppress Podman emulation warnings
- All Docker commands in scripts will use Podman transparently

### Known Issues on Arch Linux / Podman

1. **sbn_tcp module**: Create symlink: `ln -s fsw/apps/sbn/modules/protocol/sbn_tcp fsw/apps/sbn_tcp`

2. **to_lab_sub.c table error**: The generated table in `cfg/build/nos3_defs/tables/to_lab_sub.c` may use `CFE_SB_DEFAULT_QOS` which causes a pedantic error. Replace with `{0,0}` if needed.

3. **Bash RC errors**: If you see errors like `bash: /home/user/.cargo/env: No such file or directory`, these are harmless - your `.bashrc` references files that don't exist. The build continues normally.

## Build Commands

### Primary Build Commands
```bash
# Full build (config + fsw + sim + gsw)
make all

# Individual component builds
make config                    # Configure mission (use SC1_CFG=path/to/config.xml for custom configs)
make fsw                       # Build flight software
make sim                       # Build simulators
make gsw                       # Build ground software

# Clean commands
make clean                     # Clean all build artifacts
make clean-fsw                 # Clean flight software only
make clean-sim                 # Clean simulators only
make clean-gsw                 # Clean ground software only
```

### Custom Configuration
You can use alternative spacecraft configurations with the `SC1_CFG` variable:
```bash
# Use minimal configuration
make config SC1_CFG=spacecraft/sc-minimal-config.xml

# Use F Prime configuration
make config SC1_CFG=spacecraft/sc-fprime-config.xml

# Use research configuration
make config SC1_CFG=spacecraft/sc-research-config.xml
```

### Testing Commands
```bash
make build-test               # Build unit tests
make test-fsw                 # Run flight software tests
make code-coverage            # Generate code coverage report
make gcov                     # Generate gcov coverage results
make system-tests             # Run system tests with GUI
make ci-launch                # Headless system testing
```

### Launch and Operation
```bash
make launch                   # Launch full NOS3 system
make stop                     # Stop entire system
make debug                    # Launch debug terminal
make checkout                 # Run checkout application
make cosmos-operator          # Launch with COSMOS GUI
make yamcs-operator          # Launch with YAMCS GUI
make log                      # View system logs
```

### Help and Documentation
```bash
make help                     # Display basic help
make help-all                 # Display comprehensive help with all targets
```

The Makefile includes inline documentation for all targets. Use `make help-all` to see all available options including advanced setup targets like `prep-gsw`, `prep-sat`, `start-gsw`, and `start-sat`.

### Configuration GUI (Igniter)
NOS3 includes a graphical configuration tool called Igniter:
```bash
make igniter                  # Launch configuration GUI
```

Igniter provides a GUI for editing mission and spacecraft configurations. It requires Python 3 with PySide6 and xmltodict packages (installed by `make prep`).

## Architecture

### Directory Structure
- **cfg/** - Mission and spacecraft configuration files
  - `nos3-mission.xml` - Main mission configuration
  - `spacecraft/` - Spacecraft-specific configurations
  - `nos3_defs/` - cFS definitions and tables
  - `sims/` - Simulator configurations

- **components/** - Hardware component models and interfaces
  - Each component contains FSW interfaces and simulation models
  - Components include: arducam, generic_adcs, generic_eps, generic_css, etc.

- **fsw/** - Flight Software
  - `cfe/` - Core Flight Executive
  - `osal/` - Operating System Abstraction Layer
  - `psp/` - Platform Support Package
  - `apps/` - cFS applications
  - `fprime/` - F Prime flight software (alternative to cFS)

- **gsw/** - Ground Software
  - `cosmos/` - COSMOS ground station
  - `yamcs/` - YAMCS mission control
  - `ait/` - AMMOS Instrument Toolkit

- **sims/** - Simulators
  - `truth_42_sim/` - 42 dynamics simulator integration
  - `sim_common/` - Common simulation libraries
  - `nos_time_driver/` - Time synchronization

- **scripts/** - Build and operational scripts
  - `cfg/` - Configuration scripts
  - `fsw/` - Flight software scripts
  - `gsw/` - Ground software scripts

### Configuration System

The mission is configured through XML files in the `cfg/` directory. The main configuration file (`nos3-mission.xml`) specifies:
- Flight software type (cFS or F Prime)
- Ground software type (COSMOS or YAMCS)
- Number of spacecraft
- Spacecraft configuration files

Configuration is applied via `make config` which generates build artifacts in `cfg/build/`.

**Available Spacecraft Configurations:**
- `sc-mission-config.xml` - Full mission configuration (default)
- `sc-minimal-config.xml` - Minimal set of components
- `sc-fprime-config.xml` - F Prime flight software configuration
- `sc-research-config.xml` - Research-oriented configuration

The spacecraft configuration XML controls which cFS applications and hardware components are enabled for the mission.

### Build System

NOS3 uses a multi-stage build system that runs inside Docker containers:
1. **Configuration** - Processes XML configs and generates build files
2. **FSW Build** - CMake-based build for cFS or fprime-util for F Prime
3. **Sim Build** - CMake build for simulators
4. **GSW Build** - Builds CryptoLib and ground software components

Build directories:
- `fsw/build/` - Flight software build artifacts
- `sims/build/` - Simulator build artifacts
- `gsw/build/` - Ground software build artifacts
- `cfg/build/` - Configuration build artifacts

**Important Build Notes:**
- All builds run in Docker containers using the `ivvitc/nos3-64:20250514` image
- Build scripts are in `scripts/` directory and are invoked by Makefile targets
- FSW builds use CMake with custom `ComponentSettings.cmake` for compiler flags
- F Prime builds use `fprime-util` instead of CMake

### Component Architecture

Each hardware component follows a standard structure:
- **FSW interface** - cFS app or F Prime component for flight software
- **Simulator** - NOS Engine-based hardware model
- **Configuration** - XML configuration for component parameters

Components communicate via:
- **cFS Software Bus** - Message-based communication for cFS
- **NOS Engine** - Time-synchronized simulation framework
- **UDP/TCP sockets** - Network communication between FSW and simulators

## Key Development Tasks

### Adding a New Component
1. Create component directory in `components/`
2. Implement FSW interface (cFS app or F Prime component)
3. Create simulator model
4. Add component to spacecraft configuration XML
5. Update `ComponentSettings.cmake`

### Modifying Tables (cFS)
Tables are defined in `cfg/nos3_defs/tables/`. After modifications:
1. Edit the table source file
2. Rebuild with `make fsw`
3. Tables are compiled and installed to the build directory

### Running Tests
1. Build test configuration: `make build-test`
2. Run tests: `make test-fsw`
3. View results in `fsw/build/amd64-posix/default_cpu1/`

### Debugging
- Use `make debug` to launch debug container (opens interactive Docker shell)
- GDB can attach to running FSW processes
- Simulator logs are in `sims/build/`
- FSW logs depend on configuration (cFS uses syslog by default)

### Working with Individual Components
Each component in `components/` contains:
- `fsw/cfs/` - cFS application implementation
- `fsw/fprime/` - F Prime component implementation (if available)
- `sim/` - Hardware simulator implementation
- `gsw/` - Ground software integration (if applicable)

Component structure example (`components/generic_adcs/`):
```
generic_adcs/
├── fsw/
│   ├── cfs/          # cFS app for ADCS
│   └── fprime/       # F Prime component for ADCS
├── gsw/              # Ground software files
└── sim/              # ADCS hardware simulator
```

## Creating New Components

This section documents best practices learned from implementing the thermal control system (heater simulator + thermal_control cFS app).

### Component Directory Structure

A complete hardware component should have:
```
components/my_component/
├── sim/                          # Simulator (hardware model)
│   ├── inc/                      # Header files
│   │   ├── my_component_hardware_model.hpp
│   │   ├── my_component_data_point.hpp
│   │   ├── my_component_data_provider.hpp
│   │   └── my_component_42_data_provider.hpp
│   ├── src/                      # Implementation files
│   │   ├── my_component_hardware_model.cpp
│   │   ├── my_component_data_point.cpp
│   │   ├── my_component_data_provider.cpp
│   │   └── my_component_42_data_provider.cpp
│   ├── cfg/                      # Simulator configuration
│   │   └── nos3-my_component-simulator.xml
│   └── CMakeLists.txt            # Build configuration
├── fsw/cfs/                      # Flight software (cFS app)
│   ├── src/                      # Source files
│   │   ├── my_component_app.c
│   │   ├── my_component_app.h
│   │   ├── my_component_msg.h
│   │   ├── my_component_events.h
│   │   ├── my_component_version.h
│   │   └── my_component_perfids.h
│   ├── platform_inc/             # Platform-specific headers
│   │   └── my_component_msgids.h
│   └── CMakeLists.txt            # Build configuration
├── gsw/                          # Ground software files
│   └── MY_COMPONENT_SIM_CMD.txt  # COSMOS simulator commands
└── README.md                     # Component documentation
```

### Step-by-Step Component Creation

#### 1. Create Simulator (Hardware Model)

**CMakeLists.txt Pattern:**
```cmake
project(my_component_sim)

find_package(ITC_Common REQUIRED QUIET COMPONENTS itc_logger)
find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client i2c)

# CRITICAL: Include directories must include 'inc' and sim_common
include_directories(inc
                    ${sim_common_SOURCE_DIR}/inc
                    ${ITC_Common_INCLUDE_DIRS}
                    ${NOSENGINE_INCLUDE_DIRS})

set(my_component_sim_src
    src/my_component_hardware_model.cpp
    src/my_component_data_point.cpp
    src/my_component_data_provider.cpp
    src/my_component_42_data_provider.cpp
)

file(GLOB my_component_sim_inc inc/*.hpp)

set(my_component_sim_libs
    sim_common
    ${ITC_Common_LIBRARIES}
    ${NOSENGINE_LIBRARIES}
)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib")

add_library(my_component_sim SHARED ${my_component_sim_src} ${my_component_sim_inc})
target_link_libraries(my_component_sim ${my_component_sim_libs})
install(TARGETS my_component_sim LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
```

**Common Mistakes:**
- Forgetting to include `inc` directory in `include_directories()`
- Not including `${sim_common_SOURCE_DIR}/inc`
- Using `SYSTEM` modifier which can hide include errors

**Data Point Constructor:**
```cpp
// MUST use the 'count' parameter to avoid unused parameter warnings
MyComponentDataPoint::MyComponentDataPoint(double count) : _not_parsed(false)
{
    _data_is_valid = true;
    _my_value = 20.0 + 2.0 * sin(count * 0.05);  // Time-varying value
}
```

**Data Provider Base Classes:**
- Simple provider: inherit from `SimIDataProvider`
- 42 provider: inherit from `SimData42SocketProvider`
- Always pass `config` to base class constructor: `SimIDataProvider(config)`

#### 2. Create cFS Application

**Modern cFS API (2020+):**
```c
// OLD API (DO NOT USE):
CFE_SB_MsgPtr_t MsgPtr;
void ProcessTelemetry(CFE_SB_MsgPtr_t MsgPtr);

// NEW API (CORRECT):
CFE_SB_Buffer_t *MsgPtr;
void ProcessTelemetry(CFE_SB_Buffer_t *BufPtr);
bool VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, uint16 expected_length);
```

**Message Access Pattern:**
```c
// In AppData structure:
CFE_SB_Buffer_t *MsgPtr;

// Receiving messages:
CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&AppData.MsgPtr,
                     AppData.CmdPipe, CFE_SB_PEND_FOREVER);

// Accessing message fields:
CFE_MSG_GetMsgId(&AppData.MsgPtr->Msg, &MsgId);
CFE_MSG_GetFcnCode(&AppData.MsgPtr->Msg, &CommandCode);

// Passing to verification function:
VerifyCmdLength(&AppData.MsgPtr->Msg, sizeof(MyCommand_t));

// Casting to specific message type:
MyCommand_t *cmd = (MyCommand_t *)AppData.MsgPtr;
```

**CMakeLists.txt for cFS App:**
```cmake
project(MY_COMPONENT_APP C)

# If you need headers from other apps:
set(APPLICATION_PLATFORM_INC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/platform_inc
    ${other_component_MISSION_DIR}/fsw/cfs/platform_inc
)

# Create the app
add_cfe_app(my_component_app
    src/my_component_app.c
)

target_include_directories(my_component_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${APPLICATION_PLATFORM_INC_LIST}
)
```

#### 3. Create Ground Software Files

**COSMOS Simulator Commands (gsw/MY_COMPONENT_SIM_CMD.txt):**
```
COMMAND SIM_CMDBUS_BRIDGE MY_COMPONENT_SIM_ENABLE BIG_ENDIAN "Enable My Component Sim"
  APPEND_PARAMETER CMD_TEMPLATE 0 STRING '{"node":"my-component-sim-command-node","cmd":"ENABLE"}'

COMMAND SIM_CMDBUS_BRIDGE MY_COMPONENT_SIM_DISABLE BIG_ENDIAN "Disable My Component Sim"
  APPEND_PARAMETER CMD_TEMPLATE 0 STRING '{"node":"my-component-sim-command-node","cmd":"DISABLE"}'

COMMAND SIM_CMDBUS_BRIDGE MY_COMPONENT_SIM_SET_VALUE BIG_ENDIAN "Set My Component Value"
  APPEND_PARAMETER VALUE 32 FLOAT -100.0 100.0 0.0
  APPEND_PARAMETER CMD_TEMPLATE 0 STRING '{"node":"my-component-sim-command-node","cmd":"VALUE=<VALUE>"}'
```

**CRITICAL COSMOS Rules:**
- NEVER use `MIN_FLOAT`, `MAX_FLOAT`, `MIN_INT32`, etc. - Use actual numeric values
- Node name in JSON must match the `<node-name>` in simulator XML configuration
- Parameter names in `<VALUE>` must match the APPEND_PARAMETER name exactly

#### 4. Integration Steps

**Add to Spacecraft Configuration (cfg/spacecraft/sc-mission-config.xml):**
```xml
<fsw-app>
  <name>my_component</name>
  <directory>my_component/fsw/cfs</directory>
</fsw-app>
```

**Add to cFS Build (cfg/nos3_defs/targets.cmake):**
```cmake
list(APPEND MISSION_GLOBAL_APPLIST
    # ... existing apps ...
    my_component/fsw/cfs
)
```

**Add to Startup Script (cfg/nos3_defs/cpu1_cfe_es_startup.scr):**
```
CFE_APP, my_component, MY_COMPONENT_AppMain, MY_COMPONENT, 80, 16384, 0x0, 0;
```

**Add Simulator to nos3-simulator.xml (cfg/sims/nos3-simulator.xml):**
```xml
<simulator>
    <name>my-component-sim</name>
    <active>true</active>
    <library>libmy_component_sim.so</library>
    <hardware-model>
        <type>MyComponentHardwareModel</type>
        <connections>
            <connection>
                <type>command</type>
                <bus-name>command</bus-name>
                <node-name>my-component-sim-command-node</node-name>
            </connection>
            <connection>
                <type>time</type>
                <bus-name>command</bus-name>
                <node-name>my-component-time-node</node-name>
            </connection>
        </connections>
        <simulator>
            <config-file>nos3-my-component-simulator.xml</config-file>
        </simulator>
    </hardware-model>
</simulator>
```

### Common Build Errors and Solutions

#### Error: "unknown type name 'CFE_SB_MsgPtr_t'"
**Cause:** Using deprecated cFS API
**Solution:** Replace with modern API:
- `CFE_SB_MsgPtr_t` → `CFE_SB_Buffer_t *`
- Function parameters expecting messages → `CFE_MSG_Message_t *`
- Access message: `MsgPtr` → `&MsgPtr->Msg`

#### Error: "heater_hardware_model.hpp: No such file or directory"
**Cause:** Missing include directory in CMakeLists.txt
**Solution:** Add to simulator CMakeLists.txt:
```cmake
include_directories(inc
                    ${sim_common_SOURCE_DIR}/inc
                    ...)
```

#### Error: "unused parameter 'count' [-Werror=unused-parameter]"
**Cause:** Data point constructor parameter not used
**Solution:** Use the count parameter for time-varying data:
```cpp
MyDataPoint::MyDataPoint(double count) : _not_parsed(false)
{
    _my_value = base_value + variation * sin(count * 0.1);
}
```

#### Error: "class 'MyProvider' does not have any field named 'SimDataProvider'"
**Cause:** Header uses `SimIDataProvider` but source uses `SimDataProvider`
**Solution:** Use consistent base class `SimIDataProvider` in both files

#### COSMOS Error: "Could not convert constant: MIN_FLOAT"
**Cause:** COSMOS doesn't recognize MIN_FLOAT/MAX_FLOAT constants
**Solution:** Use explicit numeric ranges:
```
APPEND_PARAMETER TEMP 32 FLOAT -50.0 150.0 25.0
```

#### Error: "class NosEngine::Message has no member named 'is_reply_expected'"
**Cause:** Method doesn't exist in NOS Engine Message class
**Solution:** Don't try to distinguish message sources - just process all messages

### Inter-Component Communication

**Sending Commands Between Simulators (NOS Engine):**
```cpp
// In hardware model, send temperature update to another simulator
std::string command = "TEMPERATURE=" + std::to_string(_heater_temperature);
send_non_confirmed_message("tmp100-sim-command-node", command);
```

**Commanding Hardware via cFS (Software Bus):**
```c
// Send command to EPS to control switch
GENERIC_EPS_Switch_cmd_t eps_cmd;
CFE_MSG_Init(&eps_cmd.CmdHeader.Msg,
             CFE_SB_ValueToMsgId(GENERIC_EPS_CMD_MID),
             sizeof(GENERIC_EPS_Switch_cmd_t));
CFE_MSG_SetFcnCode(&eps_cmd.CmdHeader.Msg, GENERIC_EPS_SWITCH_CC);
eps_cmd.SwitchNumber = 1;
eps_cmd.State = 0xAA;  // ON
CFE_SB_TransmitMsg(&eps_cmd.CmdHeader.Msg, true);
```

**Subscribing to Telemetry:**
```c
// Subscribe to another app's telemetry in AppInit()
status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TMP100_HK_TLM_MID),
                         AppData.CmdPipe);

// Process in ProcessCommandPacket()
case TMP100_HK_TLM_MID:
    ProcessTelemetry(AppData.MsgPtr);
    break;
```

### Rebuild Requirements After Changes

**Simulator changes → `make clean-sim && make sim`**
**cFS app changes → `make clean-fsw && make fsw`**
**Configuration changes → `make config && make all`**
**GSW changes → `make clean-gsw && make gsw`**

If in doubt, use `make clean && make all` to rebuild everything.

## Important Environment Details

### Build Environment Variables
Key environment variables set in `scripts/env.sh`:
- `DBOX` - Docker image name (`ivvitc/nos3-64:20250514`)
- `BASE_DIR` - Repository root directory
- `FSW_DIR` - Flight software build directory
- `SIM_DIR` - Simulator build directory
- `USER_NOS3_DIR` - User's `~/.nos3` directory for 42 simulator and other tools

### Directory Ownership and Permissions
- The `~/.nos3` directory is created by `make prep` and stores:
  - 42 dynamics simulator (`~/.nos3/42/`)
  - COSMOS configuration (`~/.nos3/cosmos/`)
- Build artifacts in `fsw/build/`, `sims/build/`, and `gsw/build/` are created by Docker containers
- With rootless Podman, files are owned by your user; with Docker, may be owned by root

### Configuration Workflow
When you run `make config`:
1. Copies baseline configs to `cfg/build/`
2. Processes `nos3-mission.xml` and spacecraft config XML
3. Generates cFS definitions in `cfg/build/nos3_defs/`
4. Creates launch scripts in `cfg/build/`
5. Saves the config path to `cfg/build/current_config_path.txt`

## Troubleshooting

### "Permission denied" or "No such file or directory" during CMake build

**Symptoms**: CMake fails with errors like:
- `file failed to open for writing (No such file or directory)`
- `Could not open file for write`
- `Permission denied` when accessing build directories

**Cause**: You likely skipped `make prep` or are running builds outside the Docker container.

**Solution**:
1. Run `make clean` to remove corrupted build artifacts
2. Run `make prep` to set up the Docker environment
3. Run `make config` to configure the mission
4. Run `make all` to build

### Build scripts cannot find Docker image

**Symptoms**: Build fails with "image not found" or similar Docker errors

**Solution**: Run `make prep` to download the required Docker image (`ivvitc/nos3-64:20250514`)

### Submodule errors or missing files

**Symptoms**: Missing directories in `fsw/`, `components/`, or other subdirectories

**Solution**:
```bash
git submodule update --init --recursive
```

### When to reconfigure and rebuild

After making these changes, you MUST run `make config` (and possibly rebuild):

**Requires `make config`:**
- Modifying `nos3-mission.xml` (FSW/GSW type, spacecraft count)
- Modifying spacecraft configuration XML (enabling/disabling components)
- Switching between configuration profiles (using `SC1_CFG`)
- Changing cFS definitions in `cfg/nos3_defs/`

**Requires `make clean-fsw && make config && make fsw`:**
- Modifying cFS table definitions in `cfg/nos3_defs/tables/`
- Adding/removing cFS applications
- Changing component configurations

**Requires full rebuild (`make clean && make all`):**
- Major configuration changes
- Docker image updates
- Switching between cFS and F Prime