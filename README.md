![FSFW Logo](misc/logo/FSFW_Logo_V3_bw.png)

# Flight Software Framework (FSFW)

The Flight Software Framework is a C++ Object Oriented Framework for unmanned,
automated systems like Satellites. 

The initial version of the Flight Software Framework was developed during
the Flying Laptop Project by the University of Stuttgart in cooperation
with Airbus Defence and Space GmbH.

## Quick facts

The framework is designed for systems, which communicate with external devices, perform control loops, 
receive telecommands and send telemetry, and need to maintain a high level of availability. Therefore,
a mode and health system provides control over the states of the software and the controlled devices.
In addition, a simple mechanism of event based fault detection, isolation and recovery is implemented as well. 

The FSFW provides abstraction layers for operating systems to provide a uniform operating system
abstraction layer (OSAL). Some components of this OSAL are required internally by the FSFW but is
also very useful for developers to implement the same application logic on different operating
systems with a uniform interface.

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
a starting point. The [configuration section](docs/README-config.md#top) provides more specific 
information about the possible options.

## Prerequisites

The Embedded Template Library (etl) is a dependency of the FSFW which is automatically
installed and provided by the build system unless the correction version was installed.
The current recommended version can be found inside the fsfw `CMakeLists.txt` file or by using
`ccmake` and looking up the `FSFW_ETL_LIB_MAJOR_VERSION` variable.

You can install the ETL library like this. On Linux, it might be necessary to add `sudo` before
the install call:

```cpp
git clone https://github.com/ETLCPP/etl
cd etl
git checkout <currentRecommendedVersion>
mkdir build && cd build
cmake ..
cmake --install .
```

It is recommended to install `20.27.2` or newer for the package version handling of
ETL to work.

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

   ```sh
   target_link_libraries(${YourProjectName} PRIVATE fsfw)
   ```

5. It should now be possible use the FSFW as a static library from the user code.

## Building the unittests

The FSFW also has unittests which use the [Catch2 library](https://github.com/catchorg/Catch2).
These are built by setting the CMake option `FSFW_BUILD_UNITTESTS` to `ON` or `TRUE`
from your project `CMakeLists.txt` file or from the command line.

You can install the Catch2 library, which prevents the build system to avoid re-downloading
the dependency if the unit tests are completely rebuilt. The current recommended version
can be found inside the fsfw `CMakeLists.txt` file or by using `ccmake` and looking up
the `FSFW_CATCH2_LIB_VERSION` variable.

```sh
git clone https://github.com/catchorg/Catch2.git
cd Catch2
git checkout <currentRecommendedVersion>
cmake -Bbuild -H. -DBUILD_TESTING=OFF
sudo cmake --build build/ --target install
```

The fsfw-tests binary will be built as part of the static library and dropped alongside it.
If the unittests are built, the library and the tests will be built with coverage information by
default. This can be disabled by setting the `FSFW_TESTS_COV_GEN` option to `OFF` or `FALSE`.

You can use the following commands inside the `fsfw` folder to set up the build system

```sh
mkdir build-tests && cd build-tests
cmake -DFSFW_BUILD_TESTS=ON -DFSFW_OSAL=host -DCMAKE_BUILD_TYPE=Debug ..
```

You can also use `-DFSFW_OSAL=linux` on Linux systems.

Coverage data in HTML format can be generated using the `CodeCoverage`
[CMake module](https://github.com/bilke/cmake-modules/tree/master).
To build the unittests, run them and then generate the coverage data in this format,
the following command can be used inside the build directory after the build system was set up

```sh
cmake --build . -- fsfw-tests_coverage -j
```

The `coverage.py` script located in the `script` folder can also be used to do this conveniently.

## Building the documentations

The FSFW documentation is built using the tools Sphinx, doxygen and breathe based on the
instructions provided in  [this blogpost](https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/). If you
want to do this locally, set up the prerequisites first. This requires a ``python3``
installation as well. Example here is for Ubuntu.

```sh
sudo apt-get install doxygen graphviz
```

And the following Python packages

```sh
python3 -m pip install sphinx breathe
```

You can set up a documentation build system using the following commands

```sh
mkdir build-docs && cd build-docs
cmake -DFSFW_BUILD_DOCS=ON -DFSFW_OSAL=host ..
```

Then you can generate the documentation using

```sh
cmake --build . -- Sphinx -j
```

You can find the generated documentation inside the `docs/sphinx` folder inside the build
folder. Simply open the `index.html` in the webbrowser of your choice.

The `helper.py` script located in the script` folder can also be used to create, build
and open the documentation conveniently. Try `helper.py -h for more information.

## Formatting the sources

The formatting is done by the `clang-format` tool. The configuration is contained within the
`.clang-format` file in the repository root. As long as `clang-format` is installed, you
can run the `auto-format.sh` helper script to format all source files consistently. Furthermore cmake-format is required to format CMake files which can be installed with:
````sh
sudo pip install cmakelang
````

## Index

[1. High-level overview](docs/README-highlevel.md#top) <br>
[2. Core components](docs/README-core.md#top) <br>
[3. Configuration](docs/README-config.md#top) <br>
[4. OSAL overview](docs/README-osal.md#top) <br>
[5. PUS services](docs/README-pus.md#top) <br>
[6. Device Handler overview](docs/README-devicehandlers.md#top) <br>
[7. Controller overview](docs/README-controllers.md#top) <br>
[8. Local Data Pools](docs/README-localpools.md#top) <br>



