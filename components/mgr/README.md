# MGR - NOS Component
This repository contains the NOS Manager (MGR) component.
This includes flight software (FSW) and ground software (GSW) directories.
As this is not a physical component no simulator or standalone checkout application is necessary.

## Overview
The MGR component is to be used by the design reference mission included in NOS.
MGR will store mission critical telemetry with the capability to store and load those from file.
Other mission specific functionality to ease operational use may be included as it is identified.
The available FSW is for use in the core Flight System (cFS) while the GSW supports COSMOS.

One example use not yet demonstrated of the MGR app is to handle deployable devices such as solar arrays or antennas.


# Configuration
The various configuration parameters available for each portion of the component are captured below.

## FSW
Refer to the file [fsw/platform_inc/mgr_platform_cfg.h](fsw/platform_inc/mgr_platform_cfg.h) for the default
configuration settings, as well as a summary on overriding parameters in mission-specific repositories.


# Documentation
If this mgr application had an ICD and/or test procedure, they would be linked here.
