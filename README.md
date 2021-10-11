![FSFW Logo](misc/logo/FSFW_Logo_V3_bw.png)

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

The recommended hardware is a microprocessor with more than 1 MB of RAM and 1 MB of non-volatile
memory. For reference, current applications use a Cobham Gaisler UT699 (LEON3FT), a
ISISPACE IOBC or a Zynq-7020 SoC. The `fsfw` was also successfully run on the
STM32H743ZI-Nucleo board and on a Raspberry Pi and is currently running on the active
satellite mission Flying Laptop.

## Getting started

The [Hosted FSFW example](https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-hosted) provides a
good starting point and a demo to see the FSFW capabilities.
It is recommended to get started by building and playing around with the demo application.
There are also other examples provided for all OSALs using the popular embedded platforms
Raspberry Pi, Beagle Bone Black and STM32H7.

Generally, the FSFW is included in a project by providing
a configuration folder, building the static library and linking against it. 
There are some functions like `printChar` which are different depending on the target architecture
and need to be implemented by the mission developer.

A template configuration folder was provided and can be copied into the project root to have
a starting point. The [configuration section](doc/README-config.md#top) provides more specific 
information about the possible options.

## Adding the library

The following steps show how to add and use FSFW components. It is still recommended to
try out the example mentioned above to get started, but the following steps show how to
add and link against the FSFW library in general.

1. Add this repository as a submodule

   ```sh
   git submodule add https://egit.irs.uni-stuttgart.de/fsfw/fsfw.git fsfw   
   ```

2. Add the following directive inside the uppermost `CMakeLists.txt` file of your project

   ```cmake
   add_subdirectory(fsfw)
   ```

3. Make sure to provide a configuration folder and supply the path to that folder with
   the `FSFW_CONFIG_PATH` CMake variable from the uppermost `CMakeLists.txt` file.
   It is also necessary to provide the `printChar` function. You can find an example
   implementation for a hosted build 
   [here](https://egit.irs.uni-stuttgart.de/fsfw/fsfw-example-hosted/src/branch/master/bsp_hosted/utility/printChar.c).

4. Link against the FSFW library

	```cmake
	target_link_libraries(<YourProjectName> PRIVATE fsfw)
	```

5. It should now be possible use the FSFW as a static library from the user code.

## Building the unittests

The FSFW also has unittests which use the [Catch2 library](https://github.com/catchorg/Catch2).
These are built by setting the CMake option `FSFW_BUILD_UNITTESTS` to `ON` or `TRUE`
from your project `CMakeLists.txt` file or from the command line.

The fsfw-tests binary will be built as part of the static library and dropped alongside it.
If the unittests are built, the library and the tests will be built with coverage information by
default. This can be disabled by setting the `FSFW_TESTS_COV_GEN` option to `OFF` or `FALSE`.

You can use the following commands inside the `fsfw` folder to set up the build system

```sh
mkdir build-Unittest && cd build-Unittest
cmake -DFSFW_BUILD_UNITTESTS=ON ..
```

Coverage data in HTML format can be generated using the `CodeCoverage`
[CMake module](https://github.com/bilke/cmake-modules/tree/master).
To build the unittests, run them and then generare the coverage data in this format,
the following command can be used inside the build directory after the build system was set up

```sh
cmake --build . -- fsfw-tests_coverage -j
```

The `coverage.py` script located in the `script` folder can also be used to do this conveniently.

## Index

[1. High-level overview](doc/README-highlevel.md#top) <br>
[2. Core components](doc/README-core.md#top) <br>
[3. Configuration](doc/README-config.md#top) <br>
[4. OSAL overview](doc/README-osal.md#top) <br>
[5. PUS services](doc/README-pus.md#top) <br>
[6. Device Handler overview](doc/README-devicehandlers.md#top) <br>
[7. Controller overview](doc/README-controllers.md#top) <br>
[8. Local Data Pools](doc/README-localpools.md#top) <br>



