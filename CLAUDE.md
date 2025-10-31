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