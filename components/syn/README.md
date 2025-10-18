# Syn - NOS3 Component
This repository contains the NOS3 Syn Component.
This includes flight software (FSW), ground software (GSW), simulation, and support directories.
This is a component example for the SYNOPSIS library created by JPL.

The SYNOPSIS library and this component, need to be integrated with specific science missions and their data.
Canned data from the OWLS project is being utilized to establish this example.
There are eight sets of data within this canned directory that are used within this SYN component example.

Because this is an example, it does not currently utilize the radio for any type of downlink procedures.
It will be the responsibility of the users of the component to establish connection to, and the location of experiment data, as well as how synopsis will interact with this data.
Including how it is added to the SYNOPSIS database, prioritization procedures, as well as any downlink functionality - as SYNOPSIS needs to know when the files that are part of its prioritization database have been successfully downlinked.
This process can be configured to be manual, or automatic at the preference of users, missions, and types of data.

## Component Configurations
This [syn component](https://aetd-git.gsfc.nasa.gov/itc/jpl/synopsis/syn_component/syn_nos3) has several configuration options:
* Data Paths - Configured within the synopsis_bridge
  * Data Path
  * Database Path
  * Data Bundle Paths
  * Similarity File Path
  * Rules Path
These paths are defined at the top of the bridge file.
Because this is an example, where and how these are passed to the SYNOPSIS library is dependant upon the project.
However, SYNOPSIS must be aware of the location of these files.

Additionally, the component needs to be made aware of two of these paths as well:
* Database Path
* Clean Database Path

These may also be reconfigured, or passed in elsewhere.
How this is accomplished may also be changed per mission SYNOPSIS needs to be integrated into.
In this example, these two paths are utilized for resetting the database, and the component.
It is up to the user to determine the persistance of the database file, file locations, and how components would need to be reset.

## Component Usage
The SYN component is initialized as with other components within the SYN_AppInit function.
For this example, memory is allocated within this function, as well as the initialization of the SYNOPSIS Library.


# Default Command Codes
* SYN_NOOP_CC
* SYN_RESET_COUNTERS_CC

# SYN Component Example Command Codes
* SYN_ADD_DATA_CC - Add individual data components to SYNOPSIS (8 total)
* SYN_CONFIG_DL_CC - SYNOPSIS can prioritize data based on download constraints.  This command code allows for this modification
* SYN_CONFIG_ALPHA_CC - SYNOPSIS can prioritize data based on their variance from the type of data desired.  This value allows for this manipulation.
* SYN_RESET_CC - Resets the SYN component and SYNOPSIS library application back to a 'clean slate' - including the prioritization database.
* SYN_PRIO_CC - Prioritizes the data based on the downlink, sigma, similarities, and rules files.  This does not currently return any frames of data, but a display to FSW.  How this is handled is up to the user.
* SYN_GET_PDATA_CC - This command code can be configured multiple ways. It can be part of the downlink process, or however users prefer to get their prioritized data.  In this case, as this is an example, this function simply pops off the most prioritized file from the stack, and sets its downlink flag within the SYNOPSIS database as having been downlinked.  This allows it to no longer be part of the prioritization process.
This function REPRIORITIZES the data on each use.
* SYN_DISP_PDATA_CC - Displays the current prioritized list to the FSW terminal.

# SYN Component Usage
To utilize the SYNOPSIS library within the SYN component, users need to follow some basic steps:
* Adjust Downlink - If there are downlink constraints that users would like to consider within the collected data, or even apply to data anyway, this command code should be utilized.
* Adjust Sigma - If users would like to modify the variance of data that is prioritized, users should utilize this command code.

Following setup, users must add the data to be analyzed to the SYNOPSIS database.
* Add Data - Data is added individually within this example.  However this can be modified to be automated, or implemented in the manner that is necessary for the mission being analyzed.
* Priori Data - Use the PRIO command code to prioritize the data once it has been added to the database.
* Get Data - Determine how the data will be passed to ground. This process currently flags the highest priority data as having been downlinked, to highlight SYNOPSIS capabilities (but does not move the data in any way). This is up to mission as to how this will actually be implemented. However, SYNOPSIS needs to know when data is no longer available.
* Reset and continue to experiment

## Overview
The syn component provides an executable SYNOPSIS example for the user.
The available FSW is for use in the core Flight System (cFS) while the GSW supports COSMOS.
