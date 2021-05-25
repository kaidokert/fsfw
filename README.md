![FSFW Logo](logo/FSFW_Logo_V3_bw.png)

# Flight Software Framework (FSFW)

The Flight Software Framework is a C++ Object Oriented Framework for unmanned,
automated systems like Satellites. 

The initial version of the Flight Software Framework was developed during
the Flying Laptop Project by the University of Stuttgart in cooperation
with Airbus Defence and Space GmbH.

## Quick facts

The framework is designed for systems, which communicate with external devices, perform control loops, receive telecommands and send telemetry, and need to maintain a high level of availability. Therefore, a mode and health system provides control over the states of the software and the controlled devices. In addition, a simple mechanism of event based fault detection, isolation and recovery is implemented as well. 

The FSFW provides abstraction layers for operating systems to provide a uniform operating system abstraction layer (OSAL). Some components of this OSAL are required internally by the FSFW but is also very useful for developers to implement the same application logic on different operating systems with a uniform interface.

Currently, the FSFW provides the following OSALs:

- Linux
- Host 
- FreeRTOS
- RTEMS

The recommended hardware is a microprocessor with more than 1 MB of RAM and 1 MB of non-volatile Memory. For reference, current applications use a Cobham Gaisler UT699 (LEON3FT), a ISISPACE IOBC or a Zynq-7020 SoC. The `fsfw` was also successfully run on the STM32H743ZI-Nucleo board and on a Raspberry Pi and is currently running on the active satellite mission Flying Laptop.

## Getting started

The [FSFW example](https://egit.irs.uni-stuttgart.de/fsfw/fsfw_example) provides a good starting point and a demo to see the FSFW capabilities and build it with the Make or the CMake build system. It is recommended to evaluate the FSFW by building and playing around with the demo application.

Generally, the FSFW is included in a project by compiling the FSFW sources and providing
a configuration folder and adding it to the include path. There are some functions like `printChar` which are different depending on the target architecture and need to be implemented by the mission developer.

A template configuration folder was provided and can be copied into the project root to have
a starting point. The [configuration section](doc/README-config.md#top) provides more specific information about the possible options.

## Index

[1. High-level overview](doc/README-highlevel.md#top) <br>
[2. Core components](doc/README-core.md#top) <br>
[3. Configuration](doc/README-config.md#top) <br>
[4. OSAL overview](doc/README-osal.md#top) <br>
[5. PUS services](doc/README-pus.md#top) <br>
[6. Device Handler overview](doc/README-devicehandlers.md#top) <br>
[7. Controller overview](doc/README-controllers.md#top) <br>
[8. Local Data Pools](doc/README-localpools.md#top) <br>



