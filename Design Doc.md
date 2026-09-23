

SPICEsat: Sloshing Platform for In-Orbit Controller
## Experimentation: Software Design Document
## September 29th, 2025
Points of Contact:
## Dr. Laurent Burlion
## Assistant Professor, Rutgers University
laurent.burlion@rutgers.edu
## 848.445.2046
## Mike Fogel
## Graduate Student, Rutgers University
mikefogelny@gmail.com
## 518.852.0401
## Adil Hydari
## Flight Software Lead
## Undergraduate Student, Rutgers University
adil.hydari@rutgers.edu

STAR and Burlion Labs
## Contents
## 1    Introduction1
1.1Purpose   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .1
1.2Progress  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .1
## 2    Software Design2
2.1Planning  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .2
2.1.1Development Plan   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .2
2.1.2Quality Assurance   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .2
2.1.3Configuration Management    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .2
2.1.4Project Test  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .3
2.1.5Integration    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .3
2.2Requirements   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .3
2.2.1Functional Requirements  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .3
2.2.2Reliability Requirements  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .4
2.2.3Risk Assessment   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .4
2.2.4Fault Handling   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .5
2.3Preliminary Design   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .6
2.3.1Top Level Architecture  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .6
2.3.2Interfaces  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .6
2.3.3Development   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .7
2.4Detailed Design  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .8
2.4.1Memory Management   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .8
2.4.2RPI 3B+    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .8
2.5Core Flight Executable   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .9
2.5.1Hardware Abstraction    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .9
2.5.2Memory Management in cFE    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    11
2.5.3Memory Safety  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    11
2.5.4Software Bus  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    12
2.5.5Event Service  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    13
2.5.6Tabling Service  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    13
2.5.7File system   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    14
2.5.8Time service   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    14
2.6Middleware Considerations  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    15
2.6.1Middleware details and Design .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    16
2.7Component Details   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    17
2.7.1ADCS Details    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    17
2.7.2Payload Details  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    18
2.7.3Communications Details  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    21
2.7.4Thermal Details .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    22
2.7.5EPS Details  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    23
Page i of i

STAR and Burlion Labs
## 3    Unit Testing Harness/philosophy23
3.1Implementation  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    24
3.1.1Flight Software team member responsibilities  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    24
3.1.2Integration    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    24
3.1.3Validation  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    24
Page ii of i

STAR and Burlion Labs
## 1    Introduction
## 1.1    Purpose
The software design document serves as a guide for how development of flight software (FSW) and
ground software (GSW) should progress, and will contain the architectural design of the software.
The document closely follows the questions outlined in section 2.1.4.3.1 of the User’s Guide. The
primary audience are members of the software team as well as other subsystems interfacing with
software.
Key sections include:  planning, which includes project structure, software development life
cycle, and change management; requirements, which define what flight software should be able to
perform; preliminary design, which is a high level overview of all the components of the software;
detailed design, which goes into greater depth of the design; and finally, implementation, where
the development plan is executed.
## 1.2    Progress
The software design document is a living document. As more is learned about the software devel-
opment process and requirements become more well defined, this document will be continuously
updated to provide an up-to-date status of FSW. This document serves as a single source of truth
to reference for both newly joined and veteran team members. Several of the later sections may be
empty as we lack information on how to answer those particular questions.
Page 1 of 24

STAR and Burlion Labs
## 2    Software Design
## 2.1    Planning
## 2.1.1    Development Plan
The primary deliverables are a user guide for the flight software with documentation for indepen-
dent piece of the software, interface control documents, and testing and validation results.  Major
subsystems will have independent design documents to explain architectural choices and provide
context into the subsystem-specific code.  Those components include, but are not limited to, the
payload  control  software,  the  communications  software,  and  command  and  data  handling.   As
hardware acquisition begins and testing and validation commences, results will be documented.
The software team consists of around five people, and the team structure is nearly flat. There is
a single team lead who manages all of the members. The team lead is responsible for the software
roadmap, execution of it, the overall design and architecture, and maintaining the health of the
codebase. With such a small team, the team members do not specialize in one particular area, but
rather work on everything.
The software team follows an Agile development workflow, and uses Asana for tracking progress
on projects.  Asana makes tracking progress easy as we represent large tasks as Epics and break
them down into subtasks to visualize how work is progressing. We have weekly sprint planning to
determine what tasks should be prioritized and also assign tasks to team members.
To complete a task, the code change associated with it must be reviewed by one other developer.
In these code reviews,  the reviewer should verify that the code functions correctly,  is properly
tested, fits the style guide, and does not introduce new bugs. Once the code is merged, functionality
is verified by another developer. Review and verification may be done purely within the software,
but validation may not. In the cases where the code changes interacts with hardware components,
it is required to test the software on the on-board computer with the external component to validate
the change.
The primary tools we use are Github for version control, Asana for project tracking, GTest for
testing, and the framework to run software on.
## 2.1.2    Quality Assurance
Asana is used to track bugs and issues alongside the main project tasks.  Any bugs we find are
recorded in Asana to be reviewed in the next sprint planning, and the team decides on the highest
priority bugs to work on and who to assign it to. No single person is assigned to work on bug fixing
exclusively, rather, the entire team tackles bugs together.
## 2.1.3    Configuration Management
The primary unit of work is a task (also called a story in Agile terminology). Each task is identified
with the project prefix and a number.  For example, [FSW-10] would be the 10th task for flight
software.   Each  task  may  affect  code  in  multiple  files,  but  represents  one  cohesive  change  in
functionality  for  the  software.   Each  task  is  associated  with  a  single  commit  so  we  can  easily
link each task in Asana to all the code changes associated with it. We use a continuous deployment
model, where every commit checked into the Github repository is built with Jenkins and have tests
run against it to ensure new changes do not break existing code.  This allows us to have a robust
method to determine if the current build for the software is failing or passing.  When interfacing
with the hardware, the commit message states which pieces of hardware and what version is being
used.
For all documentation, we utilize a version log at the end of the document. Every change made
Page 2 of 24

STAR and Burlion Labs
to the document should add a new entry to the version log, specifying the date of the change and
the changes made to the documentation. Important documents such as component user guides, the
interface control document, and similar should adhere to this guideline.  In addition to this, cFS
has a verison logging protocol listed as [app]-version.h, this can help with version control when
commited to github or shared through email.
## 2.1.4    Project Test
As robustness is of utmost important when working with software in space, we prioritize testing
and aim for 100% test coverage.  Tests are written in step with code.  Every code change which
affects functionality should have an accompanying test written for every possible situation that
that particular functionality could create. Every member of the team is responsible for writing unit
tests, functional tests, and system tests for the software.
We  have  several  levels  of  code  verification.   First,  every  code  change  is  reviewed  by  an-
other team member.  Second, every code change must have tests for the appropriate functionality
changes.  Third, we have Jenkins performing continuous deployments of the code to ensure that
new commits to the main branch do not introduce new bugs to the code. Fourth, we aim to validate
our code on flight hardware every two weeks, and additionally after every integration of a major
subsystem or component. Finally, we use a code-linter to ensure consistency in our code style.
## 2.1.5    Integration
Due to the fact that we do not have the OBC currently, it is hard to foretell, how well or bumpy
integration will go.  Though, to give some idea, it would be simillar to how it works on the zynq
board.
-  First, a proper XSA file has to be either created from scratch or created using the reference
design that may or may not be supplied by GOMSPACE.
-  Next, using this XSA file, a custom build of petalinux will be created using petalinux tools.
Additionally, there may have to be some changes to the device tree (dtsi), in order to properly
incorporate the programmable logic into the Z7000’s ecosystem (including use of the FPGA
manager).
-  Once petalinux has been loaded into a .wic file for use on an SD card and booted on the OBC,
the first step is compiling a base build of cFS, to make sure that the base installation works
without flaws.  Additionally, from this base build, connecting build of some groundstation
(ie. cosmos) through an RF transceiver may be useful for initial testing.
-  From there, we will have to install a fresh version of our petalinux with any errors in the
software corrected.  From there, we will then incorporate our build of cFS (whether or not
what will be cFS or NOS3 is to be decided).
-  Finally,  testing  of  each  app  within  the  OBC,  as  well  as  connection  and  interfacing  with
components in real time.
## 2.2    Requirements
## 2.2.1    Functional Requirements
The flight software serves as the brain of the cubesat.  It must be able to process telemetry and
perform command and data handling.  It must be able to perform the experiments defined within
Page 3 of 24

STAR and Burlion Labs
the  Mission  Design  Document.   It  is  responsible  for  controlling  when  the  cubesat  is  charging,
communicating, experimenting, or any other mode of operation, and ensures that the cubesat is
always in a stable state.  The specifics of the system may be found within the software section of
the Requirements Verification Matrix.
When the satellite is in space, the only way to interface with the flight software will be through
the ground station.  The ground station will be able to uplink telemetry to the cubesat to execute
commands.  The full list of possible commands is yet to be defined.  The ground station is also
responsible for downlinking data from the cubesat for analysis.
The flight software runs on the on-board computer.  The on-board computer interfaces with
all other subsystems: the ADCS, EPS, transceiver, solar panels, and payload sensors (camera and
pressure sensors). We interface with the ADCS, the Bluecanyon ADCS with RS422 using UART.
We  interface  with  the  2ndSpace  SOLO  EPS  with  I2C/UART.  We  interface  with  the  2ndSPace
CORE solar panels with I2C and by extension,  2ndSpace BM-4 batteries with I2C. As the HP
MK3 on the SDR MK3 is built as a Daughter/Mother board design, the TR600 transceiver com-
municates with the HP MK3 via the motherboard with an LVDS type connection.  The Thermal
Control System will be managed by the On-Board-Computer by interfacing via I2C with the Texas
Instruments TMP100MDBVREP temperature sensor as well as the on-board sensors for the EPS,
Batteries, Transciever, and the OBC. From there, the OBC will make a branching decision based
on stipulations from gathered temperature data and if heating is required, the OBC will interface
with the EPS in order to send a determined voltage to the on-board heaters; once the determined
time has elapsed, the OBC will communicate to the EPS in order to turn off the outgoing power to
the heater. We interface with both the daisy-chained pressure sensors and camera with SPI.
The flight software performs all calculations needed for nominal operation. This includes, but
is not limited by, determining which mode the cubesat should be in, where the cubesat should point,
and what the cubesat should be communicating.  There are also SPICEsat-specific computations
the software must be able to perform. First, we must be able to perform repeatable and reproducible
sloshing experiments. Second, we must be able to receive and run novel algorithms sent from the
ground station. Third, the software must be able to consume and store every set of experiment data
within long-term storage.  Finally, the software must be able to have an adjustable compression
rate to downlink data with.
## 2.2.2    Reliability Requirements
The flight software should be resilient against faults. We aim to create robust software that is able
to handle faults and perform self-recovery from them.  These faults may range from major faults
like power loss or component failure to minor faults like invalid commands and bogus data.  For
SPICEsat specifically, we must handle camera failure or pressure sensor failures, as those are key
to the science component of our mission.  As we progress through development of the software
and further integrate more components, we will explicitly define more faults and how we plan to
address them.  The primary drivers of our software system is the amount of data generated by the
experiment.  We must be able to process and compress the data before the next experiment, and
also need to manage storage carefully to ensure we are able to store all data not yet downlinked.
## 2.2.3    Risk Assessment
The aforementioned Agile methodology has been proven to be effective in developing software
over rapid iterations.   We know the majority of interfaces between subsystem components and
do not expect major issues to arise from commercial off-the-shelf products.  We will take special
Page 4 of 24

STAR and Burlion Labs
care to ensure our science equipment – the camera and pressure sensors – have their interfaces
thoroughly tested to ensure they function as expected.  The high level requirements for software
are well understood, and the expectation from stakeholders for this mission are well understood
and met.
The software team has no prior experience with satellite development and is and is fairly versed
in the tools and OBC utilized on the SPICESat mission, additionally we have allotted time to learn
and master the skills required. We believe our approach of continuous integration and continuous
testing will be effective and feasible to perform. This will be crucial when we integrate with other
subsystems to ensure that software functions correctly across interfaces on all components.  To
avoid gold-plated requirements, we will not define exactly how the software will be implemented
for certain functions, only what the end results of the functions must be.
## 2.2.4    Fault Handling
There are several primary methods for fault handling.  These range from the system level faults,
which may include major crashes that jeopardize the entire mission, to smaller issues such as bit
flips.
The first of the major Fault Handling protocols will be through both the onboard SoC with the
Real-Time-Clock (RTC) in conjunction with CFE’s included Timing Service.  In order to confirm
that timing is not a concern, it is critical (on the part of CFE) to periodically check the Mission-
Elapsed-Time (which is stored on a hardware register).  Mission-Elapsed-Time, otherwise known
as MET, which, in reality, is more a count of the seconds (nanoseconds may be used as seconds
-¿ nanoseconds is generally a resource heavy calculation) passed since the hardware register has
been initialized. Now, on to a more complicated topic: time correction services (CFS’ Developer’s
Documentation is the best source for understanding, at a face level, of how the time services both
work and how they perform their correction). Time correction services will be natively handled by
cFS, with of MET and STCF (Explanation), and the TAI will be handled via the pings and updates
from ground station.
To handle bit flips from radiation in space,  SPICESat will utilize checksums and error cor-
rection  for  critical  information  such  as  commands  and  telemetry.   Every  command  that  is  sent
will have a checksum attached for the OBC to validate that the command sent from the ground is
equivalent to the command it received.  If there is an error, the OBC will attempt to use forward
error correction to resolve the correct bit. This should alleviate almost all potential bit flips errors.
In the worst case where it cannot be resolved, the OBC will re-request the command.
WDT for disaster and reset clauseARM reference document for external reset interrupt This
documentation provides the registers and instructions that need to be written to that register in
order to perform 3 types of resets through external means.   Basic WDT reference.   More than
likely, for an external WDT, we will utilize a low-cost MCU that needs to be ”petted” (or pinged)
every x amount of seconds in order to reset the count down timer on the MCU. The internal WDT
will also be utilized in order to reset on a bit-flip or a latchup. Additionally, each of the two ARM
Cortex-A9 processors within the Zynq SoC has a private watchdog timer.  Zynq 7000 reference
internal WDT. Texas Instruments MSP430 MCU Programmable Watchdog Timer.
Page 5 of 24

STAR and Burlion Labs
## 2.3    Preliminary Design
After  extensive  deliberation,  NASA  cFS  was  deemed  the  ideal  selection  for  the  fundamental
software framework for the SPICESat mission. Numerous critical factors informed this conclusion,
including the update to the on-board computer (OBC), along with the framework’s adaptability,
ease of use, software backup, and the extensive community supporting the cFS initiative.  Having
assessed the computational and data allocations for the SPICESat mission, we determined that the
NANOMIND SDR (MK3) platform was the most appropriate selection for our venture. Equipped
with dual ARM CORTEX A9 and a Xilinx Artix-7 FPGA, the Z7000 is a System on Chip (SoC)
that  allows  advanced  processing  capabilities  such  as  the  use  of  a  Linux  kernel,  specifically  a
Yocto based Kernel and distro, enabling the integration of Linux and cFS. Furthermore, the cFS’s
malleability and redundancy played a large role in the decision, allowing all software developers
on the FSW team to create cFS-based applications and simulations in NOS3, with which all cFS
based apps could first be tested with NOS3 and later incorporated into the final model running on
the HP MK3.
AddendumAdditional considerations: The Z7000willbe used, but it seems as though it makes
more sense (a case test must occur before a final decison), to have a dedicated payload computer in
addition to the OBC system. However, it is still to be decided whether a Pi or an addtional Z7000
entirely for payload computations.   However,  this adds additional computational complexity to
the OBC/Payload architecture, as it introduces a need for checksums for comms between the two
computers as well as serial comm delay between the two computers.  As well as this,  if the Pi
is decided to be the Single-Board-Computer (SBC) to go with, this introduces the extra issue of
interfacing the Z7000 (ARM A9 32 bit) and a chip running on a 64 bit architecture.  Further, as
an argument for the RPi, it does include an onboard GPU which would allow for GPU hardware
acceleration for video encoding and decoding;  meaning this would offload video computational
complexity/resources from the CPU onto the GPU.
## 2.3.1    Top Level Architecture
With the use of cFS comes the crucial application of the core Flight Executive (cFE). This al-
lows for high-level control of cFS applications, Tabling, Software Bus, Event Servicing, Thread
Safety/Management, and Data/Filesystem management.
## 2.3.2    Interfaces
The onboard computer will interface with several physical components within SPICESat.  Shown
below is a diagram for the high level architecture.
Page 6 of 24

STAR and Burlion Labs
-  Pressure sensors: SPI
-  ArduCAM: SPI
## •  ADCS: RS-422
-  Solar panels: I2C
-  TCS: I2C (Texas instruments)
## •  EPS: I2C/UART
## •  Transceiver: Daughter Board
## 2.3.3    Development
Flight software will be run within a Linux virtual machine for development.  The virtual machine
will be configured to emulate the conditions of the onboard computer as closely as possible.  It
will be a 32-bit Yocto based kernel with the meta-gomspace/meta-xilinx pack for abstraction with
limited processing power and RAM. Once the onboard computer is acquired, there will be regular
tests of the software on the onboard computer.
Prior to obtaining the onboard computer, which is expected to arrive
inQ12023 end of August
- start of September,  we will focus heavily on the design of each component’s application and
verify with the associated subteam that we are supporting all requirements and use-cases. We will
also begin preliminary implementation work of the flight software and follow the aforementioned
development process mentioned above.
Page 7 of 24

STAR and Burlion Labs
## 2.4    Detailed Design
Currently the detailed design consists of the interfaces or API calls that would have to exist between
the application manager and application driver code.  It does not contain the implementation for
either the manager or the driver; it is simply what we would need in between the two.
## 2.4.1    Memory Management
For memory, we will have a separate, radiation hardened flash memory connected to the onboard
computer to record and store data.  We will consider two types of data:  experimental data, which
generates from conducting experiments and includes data by the pressure sensors, camera, ADCS,
and more.  There will also be health data, generated periodically from various other components
such as the battery voltage, satellite temperature, orientation, and more. We will divide the memory
into two sections: one for experimental data and one for health data. When writing to the memory,
we will track two memory addresses for each area:  the beginning of the data and the end of the
data. Whenever we write data, we update the end of the data to ensure we know that is valid data.
Whenever we delete or downlink data such that we no longer need to store it, we update the data
pointers in mempool or the file service.
## 2.4.2    RPI 3B+
As mentioned when analyzing our choice in OBC in the preliminary design, an RPI was highly
considered for our project.  The RPI 3B+ has some flight heritage, in the PISat, and has had quite
a number of case study papers have been been dedicated to the RPI 3B+ in order to analyze its
flight capabilities. Aside from this, after deliberating and budgeting the amount of data processing
needed (video encoding, pressure sensor data) to complete the experiment loop; an RPI 3B+ was
chosen for the Payload team to offload data processing to a dedicated payload unit.  The RPI was
chosen in particular for the GPU functionality & to greatly speed up the rate at which the payload
team can iterate their software development.
Design considerationsThe added complexity of this design is apparent.  It adds more points
of failure, and less chance for redundancy and fault-proof systems as a separate platform will be
developed along side the OBC, making full integration harder.  However, after deliberation and
considering options for redundancy, basic USB-2.0 and Spacewire-to-Ethernet are two interfaces
that  are  leading  in  consideration.   Firstly,  payload  should  not  be  considered  data  as  critical  to
the mission when compared against telemetry readouts from the ADCS or state of health data on
the EPS. Secondly, payload data often has high through-put in the form of tarred data files sent
to the OBC for down-linking.  Having high-speed interfaces like Ethernet or USB-2.0 help with
elimination of buffer overflow errors and frees space on the RPI for additional data collection to be
performed.
The  case  for  Spacewire-to-Ethernet  or  USB-2.0  can  go  both  ways.   Spacewire-to-Ethernet
would  add  a  fair  amount  of  complexity  to  the  project  in  concern  to  development,  but  would
much simpler to implement. Spacewire and Ethernet both have link/network layers implemented,
as  such,  Spacewire  and  Ethernet  can  both  assign  IP  addresses  with  the  help  of  a  router  and
switch.   This  means  that  a  simple  SSH  can  be  used  in  order  to  use  SFTP  over  LAN  via  the
Spacewire/Ethernet  Bus  to  send  payload  data  to  the  OBC.  However,  as  mentioned,  Spacewire
does add quite a bit more complexity in development,  as a board and software will have to be
Page 8 of 24

STAR and Burlion Labs
develop a router/switch for a Spacewire-Ethernet interface.  In many cases, an FPGA board will
be formulated from an existing frame work, and an IP core will have to be created in Vitis HDL
in  order  to  provide  switching  and  routing  cabalities  between  Spacewire  and  Ethernet.   On  the
other  hand,  USB-2.0  provides  much  less  fault-tolerance  then  what  Spacewire  will  provide,  as
Spacewire is fundamentally an LVDS type data interface. This means that instead of serial data in
transport over copper, it is a simple voltage difference that signals a switch in high and low; greatly
increasing fault-tolerance. However, USB-2.0 is quite a bit easier to implement than the Spacewire-
to-Ethernet interface, as it is plug and play as USB is standardized across all platforms (including
the SDR MK3). However, USB-2.0 is about 6 times slower in max data throughput when compared
to the Ethernet jack on the RPI. Both are quite robust options, but as the development window
tightens, it seems as though USB-2.0 is the correct direction simply due to its simplicity.
## 2.5    Core Flight Executable
The cFE (Core Flight Executive) offers an operational environment for Flight Software (FSW).
Within this framework, a set of daemons serves as the foundational components for creating and
hosting FSW Applications. cFE is comprised of five core services:
-  Executive Service (ES)
-  Software Bus Service (SB)
-  Event Service (EVS)
-  Table Service (TBL)
-  Time Service (TIME)
Each of these cFE services comes with its own executable task and defines both high and low level
APIs for direct control over variables (i.e. Mission Elapsed Time) as well as runtime access to the
cFE daemons.
## 2.5.1    Hardware Abstraction
Abstraction layersAs part of NASA’s cFS, NASA includes a core daemon/kernel for handling
all core processes of cFS accordingly named the Core Flight Executable.  cFE is considered to
be a strictly ”software only” Application, meaning that it requires direct no hardware interaction
on the part of the Applications and daemons contained within it.  As such, cFE, and by extension
cFS, are theoretically portable across all platforms (i.e.  x86, arm7hl, RISC-V) in part due to the
hardware abstraction provided by the Operating System Abstraction Layer (OSAL) as well as the
”software only” nature of cFE applications.  At the lowest level,  the API for cFE facilitates all
and any hardware interaction (such as driver or device initialization) and provides a low level API
for hardware interrupts for real-time processing.  By extension of this hardware abstraction, cFE
also manages all thread creation at runtime, this is done to further push abstraction into the cFS
userspace rather than direct kernel/hardware thread management; this ensures a Service-oriented
architecture in which porting cFS from one environment to the next is as seamless as possible.
Page 9 of 24

STAR and Burlion Labs
OSAL APIThe OSAL API was created to offer a universal interface for all Applications, regard-
less of the RTOS/OS used.  Additionally, it was designed to have a minimal footprint to function
on a wide range of processors. The cFE has been engineered to leverage this OS Abstraction Layer
to enhance its portability across different RTOS systems. It serves as an intermediary between the
OS API and the cFS Applications, as it provides extra Executive Services not commonly found in
a standard RTOS. Since duplicating the OSAL API within cFE is redundant (in a bad way), cFE
Applications also have access to the OS API.
Semaphores  and  MutexescFE  provides  3  different  types  of  semaphores  for  thread  safety:
Binary,  Counting,  and  Mutexes.   A  binary  semaphore  is  like  a  switch  that  can  be  either  on  or
off.   When  an  Application  uses  the  BinSemTake(through  the  OSAL  API)  function  to  grab  a
binary semaphore, what happens next depends on whether the semaphore is currently on or off.
Counting semaphores very simillar to binary semaphores, with one key difference they don’t just
show a binary on or off value. Instead, counting semaphores enable multiple takes, counting each
”flag” take, before they become unavailable.  Mutex semaphores serve a crucial role by providing
”mutual exclusion” for shared resources.  They prevent multiple Applications from accessing the
same  resource  at  the  same  time,  thus  avoiding  a  data  race  between  applications  and  insuring
thread  safety.   Priority  inversion  is  a  significant  concern  when  sharing  resources.   This  often
slows down applications as data race conditions often incur high kernel-space callbacks (which
are often expensive when compared to user-space) as any thread can unlock a binary semaphore.
Mutexes  semaphores  offer  a  simple  solution  to  this  problem.   Mutexes,  and  by  extension  the
operating system (OSAL API), ensure that only one Application can possess the mutex at any
given moment.  If an Application attempts to acquire a mutex that is not currently in use, it gains
access immediately. However, if the mutex is already held by another Application, the requesting
Application goes into a waiting state until the current holder releases it.
On the topic of Thread SafetyThere are various forms of parallel programming, including
shared memory and distributed memory models. Shared memory allows multiple processing units
to access the same memory space, while distributed memory involves separate memory stores with
data passed between them, often used in clusters of networked computers.
Parallel tasks can be broadly categorized into ”embarrassingly parallel problems” and ”serial
problems.” Embarrassingly parallel problems are easy to split into independent tasks, suitable for
parallelization. In contrast, serial problems have dependencies between tasks, making paralleliza-
tion more challenging.   However,  even serial problems can be parallelized at a higher level by
distributing independent sub-tasks.
cFE strives to achieve multi-threaded parallelism by developing their own platform for creating,
managing,  and  destroying  threads.   Although  cFS’  implementation  of  concurrency  is  slightly
archaic.  Both reasons in the fact that Mutexes are not near as advanced as spinlocks (in reducing
syscalls to the kernel-space) or provide the extreme usability of a hybrid spinlock.  Nevertheless,
even in its simplest form, parallelism provides massive advantages in multi-core systems (such as
the Zynq 7030 on board the SDR MK3). This allows multiple data points and functions to be run
synchronously, offering a substantial decrease in processing times.
The  main  functions  described  in  the  cFE  handbook  for  thread  management  mostly  revolve
around semaphores for thread safety, as mentioned above, as well as the concept of parent/child
processes.  At its core, a parent/child thread manager only allows the ”parent” (the main applica-
Page 10 of 24

STAR and Burlion Labs
tion) to spawn a ”child” thread (any computational tasks that are part of the parent). As such, any
cFS developer has full control over basic aspects of thread management by using high-level API
to spawn child threads in a parent application.  Furthermore, this high-level API requires that the
child be destroyed before the parent can exit out of runtime, providing thread safety.
2.5.2    Memory Management in cFE
cFE provides four core concepts for data storage:  the memory pool, Critical Data Store, Tabling,
and the cFE file-system.
Mem-poolThe memory pool library in cFE provides a high level API for memory management,
similar to malloc or alloc, providing a way in which applications can allocate memory blocks of
varying sizes which can later be accessed by functions within the Application without the concern
of memory corruption.
Critical Data StoreWhen an Applications need to retain a small amount of data even after a
cFE Reset, the cFE offers a library function known as a Critical Data Store (CDS). This memory
area remains unaltered by the cFE during a reset, depending on mission parameters and the chosen
platform.  This could be memory located externally, such as in a bulk memory device, or it might
be a reserved section of memory that the cFE doesn’t modify such as the flash or ROM.
TablingA  table  is  essentially  a  connected  collection  of  data  values,  akin  to  a  C  structure  or
an array.  As such, a table is considered a shared memory resource.  These tables can be loaded
and unloaded as a single unit from the ground control.  In the context of flight software, tables
serve  a  crucial  role  by  allowing  ground  operators  to  modify  static  variables  used  in  the  flight
software while the spacecraft is operating normally, eliminating the need for software patching.
Additionally, certain tables are utilized to transmit occasionally required status information to the
ground upon request.  To manage tables, cFE includes a tabling service which is able to manage
the sharing of tables and perform updates/modifications without the Application being involved.
File-systemThe  OSAL  API  provides  a  POSIX  standard  interface  for  performing  file  system
activities.  This ensures portability across all major filesystems that also file a POSIX format for
file system capabilities.
## 2.5.3    Memory Safety
Numerous applications require validation of received data or data stored in memory.  This valida-
tion process is employed by a Cyclic Redundancy Check (CRC). To ensure consistent calculation
across a mission, the Executive Services offers an API for CRC calculation that all mission appli-
cations can utilize.  The core CRC API is also used in cFE’s software bus in order to ensure the
correct transmission of data across pipes.
Page 11 of 24

STAR and Burlion Labs
## 2.5.4    Software Bus
Arguably one of the most important core daemons/services offered by cFE is the software bus. The
software bus acts as a sort of high-way between apps, allowing data to transfer across applications
at  run-time,  sanctioning  apps  to  access  or  send  data  packets  necessary  for  housekeeping  and
telemetry.
Core conceptThe primary purpose of the Software Bus (SB) is to offer a mechanism that enables
applications to transmit data packets(i.e.  HK and TLM). Applications can further send packets
without needing to know where the packet should be sent and authorizing applications to receive
packets without requiring knowledge of the packet’s source.   The SB operates by employing a
publish/subscribe based model to establish these communication pathways. The SB is responsible
for forwarding the SB Message to all applications that have subscribed and opened a pipe to receive
that specific SB Message.
## Technicalities
-  Software Bus Messages:  Messages are data collections treated as a single entity, identified
and routed using a Message ID. Applications create SB Messages by allocating memory,
initializing contents, and adding user data.  Message header:  The Message header includes
details such as time stamps and IDs; the most crucial aspect is the Message ID. The Message
ID ensures that packets are sent to where they need to go, and also can be traced back to the
packet sender to debug issues with data in transit.
-  Message Module: Provides two implementations for Message ID manipulation (MISSION-
MSGV1 and MISSIONMSGV2),  each with different header interpreters to find Message
IDs.  The Message ID should be defined externally and should only be managed with the
Message Module API.
-  Pipes: Intial destinations for SB Messages, acting as queues/buffers for packets that are yet
to be copied over due to lack of computational overhead. Each pipe can be opened and used
by one application, applications can create and access pipes using the SB API for sending
and receiving packets.
-  Message  Limits:   SB  software  sets  limits  on  pipe  depth  and  message  limit.   Pipe  depth
restricts the number of SB Messages at a pipe, preventing overflow.  Message limit restricts
the number of specific Message IDs in a pipe.
-  Routing  of  SB  Messages:  The  cFE  efficiently  routes  SB  Messages  between  applications
using a Routing Table. Routing can be implemented as a message map or hash, with memory
considerations.
-  Sending  Packets:  Any  software  application  can  send  SB  Messages,  except  interrupt  and
exception handlers.
-  Receiving Packets: Any software application can receive SB Messages, but not interrupt and
exception handlers.  SB Messages are processed in the order received, with old messages
discarded when new ones arrive.
Page 12 of 24

STAR and Burlion Labs
## 2.5.5    Event Service
Event messages are notifications used in applications to signal significant events, alerting Ground
Station  operators  or  aiding  in  debugging.   These  messages  are  sent  via  software  bus  and  but
optionally  can  be  sent  as  telemetry  data  to  Ground  Station.   Event  servicing  is  a  fairly  simple
concept, however the API functions have granularity for specific event servicing tasks. It is highly
encouraged to review the cFE User’s guide in order to find specific APIs for the developers needs
with event servicing.
-  CFE-EVS-SendEvent can be used in order to signal the event service that a major event has
occured.  In most cases, CFE-EVS-SendEvent is used to signal to EVS that a process has
completed successfully, or errored out.
-  CFE-ES-WriteToSysLog should only be used in the case that CFE-EVS-SendEvent is not
able to record an event. CFE-ES-WriteToSysLog is akin to a printf function in C, it does not
actually write to syslog, instead prints to an ASCII buffer than is preserved during reset.
-  CFE-EVS-ERROR events are notifications of abnormal behavior caused by error conditions,
of which the developer should expect during development as well as after unit testing. Exam-
ples include erroneous commands, illegal mode change attempts, and switching to redundant
hardware. This API function is often used in conjunction with CFE-EVS-SendEvent in order
to signal to cFE that some fatal error in an event has occurred.
## 2.5.6    Tabling Service
As explained above, with regards to memory, tabling is important for redundant data storage, as
well as debugging problems encountered while the Cubesat is in flight.
Table types and Validation
-  there are two broad types for any table: Active and Inactive. The Active Table is accessible
by applications by obtaining pointers, while the Inactive Table is a copy that can be modified
and  swapped  with  the  Active  Table.   Inactive  tables  are  generally  used  when  a  Ground
Software operator wants to change static variables defined within tables. Inactive tables are
be changed during operation of cFS from ground station, and as mentioned, can be swapped
with an active table.
-  Tables can be Single Buffered or Double Buffered. Single Buffered Tables save memory as
many tables can access the buffer but will briefly delay applications during updates. Double
Buffered Tables are swift and suitable for time-critical tasks but demand more memory as
each table requires a dedicated inactive table.
-  Loading  and  activating  a  table  involve  both  Operators  and  Applications.   They  have  the
capability to load data into the Inactive Table Image using values specified in a file or a block
of memory. For Operators, this process involves several steps. They first upload a specified
file to the onboard filesystem and then issue a Table Load command,  which transfers the
contents of the uploaded file into the Inactive Table Image of the designated table. Operators
can subsequently conduct validation checks  on the contents of the Inactive Table Image.
Page 13 of 24

STAR and Burlion Labs
Once they are satisfied that the table is correctly configured, they can activate it, causing the
contents of the Active Table Image to be replaced by the Inactive Table Image.
-  Operators can command Table Services to generate a Table Dump File.  This feature serves
as a mechanism for Operators to obtain the current settings of Application parameters. The
dump file is stored in the same format as a Table Load file and may be used later as a Load
Image. It’s worth noting that Applications have the option to define a table as a ”dump only”
table during table registration. In such cases, no table buffers are allocated.
-  Operators have the ability to validate the contents of a table, Active or Inactive.  The val-
idation process involves two key steps.  Firstly, Table Services calculate the current ”Data
Integrity Value” for the contents of the table.  Secondly, if the parent task has a validation
function, it receives a notification indicating that a Validation request has been initiated. As
such, the parent task is responsible for conducting a Validation on the table.  This typically
involves checking specific values within the table to ensure they fall within specified bounds
defined by the developer and are logically coherent.  The result of this validation check is
then combined with the previously calculated Data Integrity Check Value.  The combined
result is reported to the ground in the form of a Table Services Housekeeping Telemetry
Packet, providing feedback on the status and integrity of the table.
2.5.7    File system
The File System manged by cFE is like any other POSIX compliant file system in it’s functionality,
however, it does miss out on a terminal interface or graphical interface common in many operating
systems.  Instead, cFE provides API functions for manipulating file and the metadata (header) for
files stored in the file system, zipping and unzipping folders, and dumping data into the File System
using API calls.
2.5.8    Time service
Epoch  DefinitionThe  Epoch  is  a  fixed  time  reference  and  should  ideally  remain  unchanged
for the duration of the mission.  Each mission has to define its own Epoch, which serves as the
mission’s reference point in time.  This is crucial as it forms the basis for determining absolute
time throughout the mission’s lifecycle.
Mission Elapsed Time (MET)MET is a metric that is maintained within a hardware register
and is a running count of clock ticks since the hardware was initialized.  One thing to note is that
the MET
isn’t the elapsed time since the spacecraft’s launch.  Instead, it reflects the time elapsed
since the hardware register’s initialization. The Time Service allows operators to update the MET
from the ground, provided that the register can be modified at runtime.
Spacecraft Time Correlation Factor (STCF)To account for variations in time due to factors
like onboard oscillator drift caused by temperature conditions and aging, Time Services applies
Spacecraft Time Correlation Factor (STCF). This factor is multiplied against the MET to relate
it to the mission’s Epoch and the current time.   The Time Service’s server provides commands
accessible from ground station that enable operators to update the STCF. The update can either be
Page 14 of 24

STAR and Burlion Labs
a one-time change or a continuous adjustment applied every second.  Continuous adjustments are
favoured over the former as it compensates for known spacecraft oscillator drift, and updates the
STCF accordingly.
Time Correlation to International Atomic Time (TAI)The cFE Time Service establishes a
link between spacecraft time (MET and STCF) and International Atomic Time (TAI), a precise
time measurment standard.   The TAI is expressed through the equation:  TAI = MET + STCF.
However, it is worth noting that the TAI in cFE’s Time Service is only equivalent to the actual TAI
when the epoch chosen by the operator is the same as the TAI’s epoch(00:00:00 January 1, 1958).
Basic Time FunctionsThe cFE Time Service offers a set of fundamental time-related functions.
The primary function, CFE-TIME-GetTime, is commonly used by developers to obtain the current
spacecraft time relative to the mission’s specific Epoch.  This time can be expressed in either TAI
or UTC, providing flexibility for different mission requirements.
Clock StateThe cFE Time Service employs a Time Server/Time Client system to ensure con-
sistent timekeeping.  When the CFE-TIME-GetClockState function returns CFE-TIME-VALID,
it indicates that the time obtained through cFE Time functions is synchronized with the primary
onboard time base. In contrast, CFE-TIME-INVALID suggests that the time is local and has never
been synchronized with the primary onboard time base.
Time Conversion FunctionsTo simplify time measurements, the cFE Time Services provides
two conversion functions.  CFE-TIME-Sub2MicroSecs converts subseconds value into microsec-
onds. Conversely, CFE-TIME-Micro2SubSecs reverses this process, converting an integer into the
corresponding subseconds value.
Time Manipulation FunctionsThe cFE Time Service offers functions like CFE-TIME-Subtract,
which calculates the delta between two given times. CFE-TIME-Add handles time addition, taking
into account subseconds and rollovers.  Other combinations of subtracted time types will either
produce an absolute time, a delta time or garbage as shown below:
AbsoluteTime – AbsoluteTime = DeltaTime
AbsoluteTime – DeltaTime = AbsoluteTime
DeltaTime – DeltaTime = DeltaTime
DeltaTime – AbsoluteTime = garbage
## 2.6    Middleware Considerations
Middleware is a crucial part of the development process for the onboard flight-software and sim-
ulations; to both ensure that final integration with the hardware is both accurate as well as swift.
NOS3 was developed for this specific purpose, as a Segway between preliminary design and final
integration into actual hardware. NOS3 is built on top of NASA’s cFS and by extension, Core Flight
Executive.  NOS3 does add quite a few features that I believe to be crucial to the development
Page 15 of 24

STAR and Burlion Labs
workflow working with the NASA cFS ecosystem.   Namely,  the addition of the NOS3 engine,
NASA’s 42 engine, as well as changes to the core functions of cFS with the addition of apps such
as:  hardware-library (hwlib),  io-library (iolib),  and the addition of a custom workflow for cFS
driver development.
2.6.1    Middleware details and Design
NOS  EngineOne  of  the  most  important  features  of  NASA’s  NOS3  middleware  is  the  NOS
engine. The NOS engine provides an extensible feature set for the sake of simulation between the
acquirement of actual hardware and the software architecture design.  NOS Engine is a modular
engine for simulation, offering time synchronization, data manipulation, and fault injection.  It’s
based on nodes and buses, where any node in the system must belong to a group, referred to as
a bus.  In NOS3, it simulates hardware buses and supports various protocols like MIL-STD-1553
and SpaceWire.
HWLIB and IOLIBFundamentally, cFS is meant to be an abstracted layer that resides on top
of the operating system and kernel. Due to this, it means that it is fundamentally harder to provide
hardware based communications protocols (i.e.  I2C, SPI, UART) on the cFS abstraction layer.
As such, NOS3 provides the HWLIB and IOLIB, these libraries are add-on applications which
provide bus and hardware integration directly from the cFS abstraction layer.   This serves as a
way to grant full portability across developer instances, and leads easy integration with the final
software release.
Software EnvironmentsAs part of the NOS3 packaging, NOS3 provides a way to provision
virtual machines for redundant environments across all developer instances.  The provisioning is
provided by Vagrant,  which provides the image for the development environment,  and actually
virtualized in either Docker, VirtualBox, or via an Ansible playbook.  To me, I believe this to be
one of the most important features. Environment redundancy across all developers provides a way
in which any member of FSW can debug and troubleshoot issues in applications without worrying
about development environment factors that differ between two development instances.
COSMOS ground-stationThe COSMOS ground-station software is a built-in part of NOS3, but
its use spans beyond middleware, COSMOS will also be used as our GSW interface for telemetry,
SOH, and Housekeeping data requests and commands.  As standard, cFS does provide a guide as
to how to link base cFS to the COSMOS interface, however, NOS3 provides that environment by
default by use of the virtual machine provisioning by vagrant. COSMOS serves 3 main purposes,
to monitor OBC output, request and record telemetry/SOH/Housekeeping data, and to provide an
interface for command input. COSMOS is extremely versatile in its programming, it is QT based
application  and  provides  a  Application  Layer  ssh-like  client-server  system  and  UDP  protocol;
albeit  misses  out  on  the  security  protocols  implemented  by  ssh.   Additionally,  COSMOS  also
supports nodes/agents between hardware for testing, this means that testing command inputs from
the OBC to any linked hardware can be unit tested and brought to a fault-tolerant state. As well as
this, COSMOS supports adding any command input to its interface due to its customizability via
the COSMOS header declaration files.
Page 16 of 24

STAR and Burlion Labs
## 2.7    Component Details
Component overview; API requirements; App overview
2.7.1    ADCS Details
Required basic API
-  Get current orientation
-  Set new orientation
-  Desaturate reaction wheels
-  Report attitude data
Required complex API
## 1.  Detumble
ADCS  APPThe  ADCS  Application  will  strictly  be  developed  within  the  framework  of  cFS
using the Middleware NOS3.  As such,  the NOS3 engine will be utilized in order to develop a
generic simulated model of the BlueCanyon ADCS based on the documentation provided.  The
ADCS  communicates  with  the  UART  protocol  via  an  RS422/485  interface.   In  many  aspects,
the ADCS is designed to be fully self sufficient,  containing its own microprocessor as well as
proprietary firmware and algorithms for commanding reaction wheels, star-trackers, sun-sensors,
and  magnetorquer.   All  that  is  provided  to  the  OBC  in  terms  of  abstraction  is  the  outcoming
telemetry and housekeeping data that the XACT provides. Including this, the XACT also provides
an interface document for all virtual memory addresses corresponding to where telemetry data
exists.  This data comes in a few forms according to this document, first is the level-0 hardware
telemetry  reads,  which  provide  boot  status,  watchdog  status,  ADC  word  status  and  so  on;  all
of which are related to the hardware on-board the ADCS. Associated with these are command
OP-codes, which can be written to the command buffer via the RS422, these OP-codes are also
hardware related, used for resetting watchdogs or setting boot priority on ROMs.  However, this
same concept of telemetry/housekeeping data also applies for the software on the XACT, each
software command is associated with an OP-code which can be sent in combination with an AppID
in order to send data to a specific function/task on the ADCS. These include functions such as
setting the torque on the reaction wheels, reading and writing to a flash table, and performing CRC
calculations.  Additionally, BlueCanyon directly provides a COSMOS interface for testing with
the ADCS, which can later be pipe lined into a final ground station model that integrates with the
OBC’s GSW telemetry and command list.
Page 17 of 24

STAR and Burlion Labs
## 2.7.2    Payload Details
Required basic API
-  Initialize camera / pressure sensors
-  Record camera / pressure sensors
-  Stop camera / pressure sensors
-  Save recorded data
Required complex API
-  Record experiment
Payload ProcessingPayload processing should be designated into a special category in con-
sideration with  Flight Software,  as  it is  developed analog  to the  FSW built  for the  main CDH
unit. As such, final integration of the payload processor into the OBC’s ecosystem will have to be
performed by the FSW team after the software of payload has been completed to a functional state.
## Nonetheless,
it is crucial for any Flight Software member to understand the development process
and software considerations undertaken by the Payload team.
Payload driver code that controls the Pressure sensors and Camera for video output and image
output is all managed by Payload, meaning I won’t cover that.  Instead, Flight Software’s focus
should  be  on  OBC  integration  with  the  Payload  data  from  the  Raspberry  Pi.   Considering  the
environment utilized by the Raspberry Pi, Linux, reading and writing data should be fairly simple
on both sides as it is a simple file transaction using /dev/tty profiles. The core issue with this data
integration is passing data from the kernel level, to the layer above it, the cFE abstraction layer. To
move a file on linux in C, you typically use the rename() syscall, but it can sometimes fail. If you
can’t use rename() because the source and target are on different filesystems (which is true in our
case), you need to follow a more complex process involving stat(), read(), write(), open(), close(),
chmod(), chown(), utime() for attribute, metadata, and data copying.  Potentially you may want
to use the getxattr(), setxattr(), or listxattr() syscalls, these functions will give extended metadata
relating to the file location and the name.  sendfile() may also be used as a syscall over read() and
write() as it more efficent than that combination.  Additional design considerations can be done
with regard to payload prior to the completion of the payload driver software.   Meaning that a
framework application for Payload within cFS should be laid out based on software architecture
details.
ADCSAlthough  attitude  determination  and  control  (ADCS)  is  primarily  managed  by  the  On-
Board Computer (OBC), certain payload experiments on the Raspberry Pi require awareness of—or
limited control over—the spacecraft’s pointing.
-  Requesting Orientation or Attitude Updates:
The OBC always retains “master” authority over the ADCS. This is essential for safety and
fault-recovery: if the Pi experiences a software or hardware anomaly, the spacecraft can still
detumble, maintain safe orientation, and properly manage critical maneuvers.
The payload (running on the Pi) only has “secondary” or “conditional” control.  When the
mission  timeline  or  science  plan  allows,  the  Pi  may  request  specific  orientations,  spins,
Page 18 of 24

STAR and Burlion Labs
or angle hold times for an experiment (e.g., fluid-sloshing trials).  However, the OBC can
override or veto these requests as needed.
-  Command Interface and Telemetry:
Requesting Orientation:  When a payload experiment requires the spacecraft to point in a
certain orientation, the Pi sends a high-level orientation request to the OBC. These requests
might include target angles, rotation rates, or hold durations.
The Pi can request (almost) real-time telemetry from the ADCS (e.g., attitude quaternions,
angular velocity, reaction wheel speeds). The OBC broadcasts or logs this data, enabling the
Pi’s software to correlate experiment readings with the vehicle’s actual motion or pointing.
It is important to note here that the ADCS outputs data at   4 Hz, meaning that in OBC to
RASPI connection isfarfrom the weakest link in the chain from the ADCS to the RASPI.
There should be no appreciable difference between a direct connection between the RASPI
to the ADCS and the passthrough between the OBC and the RASPI.
In many cases, the OBC writes ADCS telemetry to a file or software bus queue that the Pi can
access.  This prevents the Pi from hitting ADCS registers directly, minimizing concurrency
issues and preserving safe isolation of flight-critical code.
-  Safe-Mode and Fault Tolerance:
If the OBC’s watchdog (or any safety monitor) detects that the Pi has hung, gone into an
undefined state, or is requesting unsafe orientations (e.g., orientation changes that exceed
torque or momentum limits), the OBC will ignore or override the Pi’s commands.  If the
Pi is powered down (for thermal or power constraints) or becomes unresponsive, the OBC
automatically reverts to nominal control loops for attitude stabilization, including detumble
or safe-mode pointing. Since the Pi might not always be aware of ADCS-specific faults, the
OBC is responsible for diagnosing and responding to anomalies (e.g., star-tracker failure,
reaction wheel saturation).  The Pi’s requests are deprioritized until the system is back in a
healthy state.
-  Use Cases for Payload Integration:
If fluid-sloshing experiments demand specific spin rates or stable orientation, the Pi can send
discrete instructions (e.g., “Rotate about the X-axis by 10°/s for 30 seconds”). The OBC then
executes these requests using the BlueCanyon firmware.  The Payload team might combine
ADCS telemetry (like angular velocity) with its own sensor readings (e.g., pressure data from
a fluid chamber) to analyze how orientation influences fluid behavior. Thus, it is critical that
we can receive and store ADCS telemetry & payload experiment data.
During a longer experiment, the Pi may periodically poll the OBC for updated orientation
data to track how the actual orientation deviates from the target, adjusting camera exposures
or sensor timing in real time.
Communications Between OBC and RASPIAlthough the payload software runs on a sepa-
rate processor (the Raspberry Pi), it must seamlessly exchange data with the On-Board Computer
(OBC) to support experiments, diagnostics, and mission operations.
-  Physical Link and Network Setup:
Point-to-Point Protocol (PPP): To simplify higher-layer communications, the OBC and Pi
connect using PPP over USB or RS485.  This creates a lightweight but robust IP network
Page 19 of 24

STAR and Burlion Labs
between the two boards.
Network Abstraction: Once PPP is established, both sides can use standard TCP/IP sockets,
using well-known protocols like FTP, SFTP, HTTP, or custom protocols.  This spares the
flight software from writing a custom serial or CAN-based protocol.
-  Command and Control Interface:
Daemon on the Pi: A small background process (e.g., a systemd service) listens on a desig-
nated port or socket for incoming commands from the OBC. These commands might start or
stop the camera, request sensor data, or retrieve experiment logs.
OBC Messaging:  On the OBC side,  a cFS-based application (or an equivalent manager)
formats and sends commands to the Pi over the PPP link.  The same or a separate app can
parse the Pi’s responses and route them into cFS data structures (e.g., software bus messages
or logs).
Bidirectional Flow:  Although the Pi primarily receives commands and pushes data, there
are moments when the Pi might proactively send triggers to the OBC (e.g., “experiment data
ready” or “payload hardware status change”).
-  File Transfers and Experiment Data:
Large Data Sets:  High-volume experiment data (e.g., videos) are stored locally on the Pi’s
SD card.  The Pi then transfers these files (in a compressed format) to the OBC’s storage
for eventual downlink, either in chunks or full batches, using a reliable file-transfer protocol
(e.g., SFTP over PPP).
Low-Volume Telemetry:  Simple housekeeping messages (e.g.,  camera status,  experiment
temperature) can be sent in near real time.  This might be done via a straightforward UDP
socket or by encapsulating data into cFS-style packets.
Automated Scheduling:  The OBC can schedule data pulls during communication windows
or power-available modes. Conversely, the Pi can also push critical notifications if near-real-
time updates are needed (e.g., when an experiment completes or a sensor reading crosses a
threshold).
-  Error Handling and Logging:
Checksums and Retransmissions:  All critical command packets and large file transfers use
standard TCP checksums or additional integrity checks.  If errors arise (e.g., partial corrup-
tion over RS485), the Pi and OBC will request retransmission.
Communication Loss:  If the link drops or the Pi is power-cycled, the OBC automatically
detects the session failure.  On reestablishing PPP, the OBC can resume file transfers from
the last confirmed offset or reissue any pending commands.
Connection Watchdog:  A software watchdog on the OBC (or Pi) can send periodic health
pings to confirm the link is alive.  If the Pi fails to respond within a timeout, the OBC may
log an anomaly, attempt a graceful reset, or power-cycle the Pi.
## •  Resource Management:
Power  Constraints:   If  the  spacecraft  enters  low-power  mode,  the  OBC  can  suspend  Pi
operations (turn off the Pi’s power rail) until battery levels recover.  The Pi’s daemon must
be stateless enough to resume after a cold boot, reestablish PPP, and restore operations.
Thermal Considerations: The Pi’s usage may be throttled if the spacecraft becomes thermally
Page 20 of 24

STAR and Burlion Labs
stressed.  The OBC can command the Pi to pause video processing or large data transfers,
avoiding risk to the entire vehicle.
Priority of Flight-Critical vs.  Payload Data:  In critical phases (deployment, detumble, safe
mode), the OBC might block or queue nonessential Pi commands to ensure stable pointing,
communications, or power.  Normal science data flow can resume once safe conditions are
restored.
## •  Example Workflow
1) Boot and Link-Up:  The Pi powers on, the PPP daemon starts, and the OBC negotiates a
PPP session. Both sides confirm link readiness with a quick handshake or ping.
2) Experiment Command:  The OBC sends a “Start Camera Capture” command.   The Pi
acknowledges, begins recording, and logs progress.
3) Data Generation:  The Pi captures images or video, storing them locally.  It also streams
small status messages (like current frame count) to the OBC.
4) File Upload:  Once recording ends, the Pi notifies the OBC it has new files.  The OBC
either requests or automatically pulls them via SFTP.
5) Completion and Cleanup: The Pi sends a completion status, and the OBC logs the result.
The link either stays open for further commands or times out if idle.
## 2.7.3    Communications Details
Required basic API
-  Listen for data packets
-  Send data packets
Required complex API
-  Check for health pings
-  Responds to health pings
-  Downlink health data
-  Downlink experiment data
CommunicationsFlight Software’s job with Communications deals less so with the link layer
of communications (i.e.   S-band with a QPSK modulation/de-modulation scheme) and more to
do with the networking layer of communication (i.e.  mac addressing, internet protocol, transport
layer).   In general,  the way the OSI model has always been handled is providing levels of ab-
straction at each different layer.   The first 2 layers will mostly be handled by Communications
team with the physical layer,  and the FSW lead will help out with the data link layer with the
communications team. Broadly speaking, the FSW team will develop layers 3-7, which I consider
to  be  mostly  high-level  abstractions  from  bare  metal  (i.e.   PHY  chips)  as  well  as  Data-linking
(i.e.  802.11a/b/g/n/ac/ax).  Instead, FSW will focus on developing a TCP/IP-like model for the
Network and Transport layer, as well as developing a model for transport layer security in the form
Page 21 of 24

STAR and Burlion Labs
of a secure protocol like TTLS. However, TCP/IP and network programming from the ground up
is quite a demanding task, often times it takes an experienced coder to first understand networking
then implement it in a low-level language like C. As such, only the lead should work on the early
framework for the TCP/IP model and transport security, and then pass his/her work down as they
see fit. Explaining networking and the inner-workings of my entire networking model is out of the
scope of this paragraph; instead, at a later date, I will work on a full write-up on the development.
Aside from this, there should be a few basic requirements that should be fulfilled in the course
of developing this application.  First, this application will live outside of the cFS/cFE ecosystem,
meaning that it will have to integrate well with both the ground station’s communications protocols,
as well as align with the requirements of the data being down linked.   Next,  the model should
and  always
allow  full-duplex  communications,  meaning  it  cannot  be  a  primitive  10Base2-like
system that only supports transmission one-way at a time.  The final product should and always
be abstracted from the operating system, this ensures that every developer can work in different
hardware environments and still integrate their development into the final product. Now pertaining
to the technical aspects, there needs to be a loop-back interface as a means of testing and debugging
network related issues on the client (CubeSat). Additionally, there needs to be an ICMP request and
ping-back to signal that a server is open and ready for data transmission. Next, all communications
on the transport and network layer should both be thread safe as well as support secure transport
layer protocols such as TTLS or RSA. Lastly, Promiscuous mode should be available between the
server and the client, meaning that the server or client can sniff packets in transit between the two.
## 2.7.4    Thermal Details
Required basic API
-  Read temperature sensor
-  Command heater
Required complex API
-  Maintain homeostasis
Thermal Controls SystemThermal control system programming within the scope of the cFS
ecosystem might very well be the easiest application to code, but must be extensively unit tested to
ensure failure does not occur. Any software fault in the thermal control system can often be fatal, as
temperatures can quickly drop to below operating temperatures for hardware on-board the Cubesat,
resulting in mission failure. As such, thermal programming should and will be taken seriously by
all FSW members.  Thermal control relies on three core components, temperature readouts from
on-board thermal sensors on the SDR MK3, external nodes in the form of the TI TMP100, and
lastly the Heater control via the EPS. The TMP100 is a digital-based IC for thermal readouts, as
such, the TMP100 broadcasts data to the OBC in the form of I2C reads on a location, and changes
configurations based on I2C writes to a location.  As such, by using the NOS3 hardware library,
a developer can easily implement the abstracted functions for I2C slave and master registration
and  quite  easily  perform  the  reads  on  memory  addresses  pre-defined  in  the  Texas  Instruments
documentation.  After this read of temperature from the TMP100, the OBC reads the temperature
Page 22 of 24

STAR and Burlion Labs
sensor on-board the TR-600 and makes a decision, based on a defined logic tree, on whether the
heater needs to be powered on or not.
Another design consideration to keep in mind, is of automation of thermal data being requested
to the OBC and allow the cascading logic to make a decision on the heaters.   Again,  this sort
of  process  would  be  much  easier  in  plain  C,  using  Linux  header  files  and  low  level  APIs  for
I/O; but this would take away from reusability,  portability,  and create issues with a centralized
scheduling platform that will issue commands upon a watchdog timeout.  As such, the developers
must keep in mind that when working on the TCS application and device, that the SCH app from
NASA must be utilized in order to issue commands into the Software-Bus from directly within
the satellite.  There are other-ways to go about this automation, such as a Linux based software
watchdog that will countdown and send an interrupt that the application will pick up execute the
logic diagram for enabling the heaters.  Another method is to use the ZYNQ’s IRQ subsystem to
enable software interrupts that are more reliable than that of a normal software interrupts as it is a
defined subsystem that has been independently tested by AMD (which is an important factor when
considering something like thermal which is crucial for the CubeSat’s serviceability and ability to
functionat all.
2.7.5    EPS Details
Required basic API
-  Read battery info
Electrical Power SystemAgain, the EPS is one of the easier applications to code in the cFS
ecosystem,  but this does not mean it should be taken lightly.Anyfatal error in the Electrical
Power System’s application controlling software can result in quick and complete mission failure.
As such, this application will be rigorously unit-tested to provide a fault-tolerant environment. The
EPS works in a similar fashion to the TMP100, it can be controlled through a serial port: either I2C
or UART. As such, the NOS3 hardware library can be utilized once again in order to either initiate
a UART serial read, or register and read serial data from I2C. Once we read those values, this data
can be transformed into data structs that can be represented as State of Health data or Telemetry
data.  From there, another logic tree will be employed based on current data such as charge and
voltage, and will make a decision on whether to start charging or not. As well as this, there should
be a command interface available through the COSMOS ground station software that allows SOH
and Telemetry data to be dumped to ground in order to periodically check the health of the EPS
system.
## 3    Unit Testing Harness/philosophy
Unit testing is an important part of the final deliverable for pre-flight review, as such, the unit testing
architecture must be well defined in order to ensure a coherent and comprehensive unit testing
harness for all FSW applications. Concerning Middleware and cFS specifically, cFS does include a
unit testing harness right out of the box, for testing applications. Conversely, as NOS3 implements
a slightly different approach to cFS flight software, the unit testing harness also includes a way
in which to test the component part of NOS3 which can later be ported to cFS. Partial to this,
Page 23 of 24

STAR and Burlion Labs
the OBCs operating system must also be extensively unit tested in order to ensure a fault tolerant
as well as SEU and hard reset friendly environment for worst case faults.   Yocto’s project test
environment is to be utilized for testing all bit-bake recipes as well as support layers that may be
developed to add new packages to the OBCs Linux build.  Test Environment.  Linux testing is to
be considered later on, when all parts of the CubeSat are acquired and connected to the OBC; in
order to test all recipes and meta layers in the Yocto package.
## 3.1    Implementation
There are currently two ongoing developments of FSW, one is a personal fork of NOS3 for the
sake of integration as well as personal development:https://github.com/AdilHydari/
nos3_spicesat.git.  As well as an FSW repository for ongoing development for the FSW
team:https://github.com/rutgers-star/FSW.  Both are private repositories, but can
be made public upon request.
3.1.1    Flight Software team member responsibilities
Adil:  Team management, FSW member on-boarding, Operating system manager, TCS Applica-
tion, EPS App. Nila: Communications data link layer, Payload-Flight software integration
## 3.1.2    Integration
We have a majority of components and iterating with NOS3 to integrate it.
## 3.1.3    Validation
We are not yet at this step.
Page 24 of 24