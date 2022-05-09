Change Log
=======

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

# [unreleased]

# [v5.0.0]

## Changes

- Bump C++ required version to C++17. Every project which uses the FSFW and every modern
  compiler supports it
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/622
- HAL Linux SPI: Set the Clock Default State when setting new SPI speed
  and mode
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/573
- GPIO HAL: `Direction`, `GpioOperation` and `Levels` are enum classes now, which prevents
  name clashes with Windows defines.
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/572
- New CMake option `FSFW_HAL_LINUX_ADD_LIBGPIOD` to specifically exclude `gpiod` code.
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/572
- HAL Devicehandlers: Periodic printout is run-time configurable now
- `oneShotAction` flag in the `TestTask` class is not static anymore
- HAL Linux Uart: Baudrate and bits per word are enums now, avoiding misconfigurations
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/585
- IPC Message Queue Handling: Allow passing an optional `MqArgs` argument into the MessageQueue
  creation call. It allows passing context information and an arbitrary user argument into
  the message queue. Also streamlined and simplified `MessageQueue` implementation for all OSALs
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/583
- Clock: 
  - `timeval` to `TimeOfDay_t`
  - Added Mutex for gmtime calls: (compare http://www.opengate.at/blog/2020/01/timeless/)
  - Moved the statics used by Clock in ClockCommon.cpp to this file
  - Better check for leap seconds
  - Added Unittests for Clock (only getter)

## Removed

- Removed the `HkSwitchHelper`. This module should not be needed anymore, now that the local
  datapools have been implemented.
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/557

## Additions

- Linux HAL: Add wiretapping option for I2C. Enabled with `FSFW_HAL_I2C_WIRETAPPING` defined to 1
- Dedicated Version class and constant `fsfw::FSFW_VERSION` containing version information
  inside `fsfw/version.h`
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/559
- Added ETL dependency and improved library dependency management
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/592

## Fixed

- Fix infinite recursion in `prepareHealthSetReply` of PUS Health Service 201.
  Is not currently used right now but might be used in the future
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/617
- Move some CMake directives further up top so they are not ignored
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/621
- Small bugfix in STM32 HAL for SPI
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/599
- HAL GPIO: Improved error checking in `LinuxLibgpioIF::configureGpios(...)`. If a GPIO
  configuration fails, the function will exit prematurely with a dedicated error code
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/602

# [v4.0.0]

## Additions

- CFDP Packet Stack and related tests added. It also refactors the existing TMTC infastructure to
  allow sending of CFDP packets to the CCSDS handlers.
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/528
- added virtual function to print datasets
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/544
- doSendRead Hook
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/545
- Dockumentation for DHB
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/551
  
### HAL additions

- Linux Command Executor, which can execute shell commands in blocking and non-blocking mode
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/536
- uio Mapper
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/543

## Changes

- Applied the `clang-format` auto-formatter to all source code
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/534
- Updated Catch2 to v3.0.0-preview4
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/538
- Changed CI to use prebuilt docker image
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/549

## Bugfix

- CMake fixes in PR https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/533 , was problematic
  if the uppermost user `CMakeLists.txt` did not have the include paths set up properly, which
  could lead to compile errors that `#include "fsfw/FSFW.h"` was not found.
- Fix for build regression in Catch2 v3.0.0-preview4
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/548
- Fix in unittest which failed on CI
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/552
- Fix in helper script
  PR: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/pulls/553

## API Changes

- Aforementioned changes to existing TMTC stack

## Known bugs

- 

# [v3.0.1]

## API Changes

* 

## Bugfixes

* Version number was not updated for v3.0.0 #542

## Enhancement

* 

## Known bugs

* 

# [v3.0.0]

## API Changes

#### TCP Socket Changes

* Keep Open TCP Implementation #496
    * The socket will now kept open after disconnect. This allows reconnecting. 
    * Only one connection is allowed
    * No internal influence but clients need to change their Code.

### GPIO IF

* Add feature to open GPIO by line name #506

### Bitutil

* Unittests for Op Divider and Bitutility #510

### Filesystem IF changed

*  Filesystem Base Interface: Use IF instead of void pointer #511 

### STM32

* STM32 SPI Updates #518

## Bugfixes

* Small bugfix for LIS3 handler #504
* Spelling fixed for function names #509
* CMakeLists fixes #517
* Out of bound reads and writes in unittests #519
* Bug in TmPacketStoredPusC (#478)
* Windows ifdef fixed #529

## Enhancement

* FSFW.h.in more default values #491
* Minor updates for PUS services #498
* HasReturnvaluesIF naming for parameter #499
* Tests can now be built as part of FSFW and versioning moved to CMake #500
*  Added integration test code #508 
* More printouts for rejected TC packets #505
* Arrayprinter format improvements #514
* Adding code for CI with docker and jenkins #520
* Added new function in SerializeAdapter  #513
    *  Enables simple deSerialize if you keep track of the buffer position yourself
    *  `` static ReturnValue_t deSerialize(T *object, const uint8_t* buffer,
            size_t* deserSize, SerializeIF::Endianness streamEndianness) ``
* Unittest helper scripts has a new Parameter to open the coverage html in the webrowser  #525
    * ``'-o', '--open', Open coverage data in webbrowser`` 
* Documentation updated. Sphinx Documentation can now be build with python script #526

## Known bugs

* Version number was not updated for v3.0.0 #542


All Pull Requests:

Milestone: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/milestone/19

# [v2.0.0]

## API Changes


### File Structure changed to fit more common structure

* See pull request (#445)
  * HAL is now part of the main project
    * **See Instructions below:**

#### Instruction how to update existing / user code

* Changes in `#include`:
  * Rename `internalError` in includes to `internalerror`
  * Rename `fsfw/hal` to `fsfw_hal`
  * Rename `fsfw/tests` to `fsfw_tests`
  * Rename `osal/FreeRTOS` to `osal/freertos`

* Changes in `CMakeLists.txt`: 
  * Rename `OS_FSFW` to `FSFW_OSAL`

* Changes in `DleEncoder.cpp`
    * Create an instance of the `DleEncoder` first before calling the `encode` and `decode` functions

### Removed osal/linux/Timer (#486)

* Was redundant to timemanager/Countdown

#### Instruction how to update existing / user code

* Use timemanager/Countdown instead

## Bugfixes

### TM Stack

* Increased TM stack robustness by introducing `nullptr` checks and more printouts (#483)

#### Host OSAL / FreeRTOS 

* QueueMapManager Bugfix (NO_QUEUE was used as MessageQueueId) (#444)

#### Events

* Event output is now consistent (#447)

#### DLE Encoder

* Fixed possible out of bounds access in DLE Encoder (#492)

## Enhancment

* HAL as major new feature, also includes three MEMS devicehandlers as part of #481
* Linux HAL updates (#456)
* FreeRTOS Header cleaning update and Cmake tweaks (#442)
* Printer updates (#453)
* New returnvalue for for empty PST (#485)
* TMTC Bridge: Increase limit of packets stored (#484)

## Known bugs

* Bug in TmPacketStoredPusC (#478)


All Pull Requests:

Milestone: https://egit.irs.uni-stuttgart.de/fsfw/fsfw/milestone/5

# [v1.2.0]

## API Changes

### FSFW Architecture

- New src folder which contains all source files except the HAL, contributed code and test code
- External and internal API mostly stayed the same
- Folder names are now all smaller case: internalError was renamed to internalerror and
  FreeRTOS was renamed to freertos
- Warning if optional headers are used but the modules was not added to the source files to compile

### HAL

- HAL added back into FSFW. It is tightly bound to the FSFW, and compiling it as a static library
  made using it more complicated than necessary

## Bugfixes

### FreeRTOS QueueMapManager

- Fixed a bug which causes the first generated Queue ID to be invalid

## Enhancements

### FSFW Architecture

- See API changes chapter. This change will keep the internal API consistent in the future

# [v1.1.0]

## API Changes

### PUS

- Added PUS C support
- SUBSYSTEM_IDs added for PUS Services
- Added new Parameter which must be defined in config: fsfwconfig::FSFW_MAX_TM_PACKET_SIZE

### ObjectManager

 - ObjectManager is now a singelton


### Configuration

- Additional configuration option fsfwconfig::FSFW_MAX_TM_PACKET_SIZE which 
  need to be specified in FSFWConfig.h

### CMake

- Changed Cmake FSFW_ADDITIONAL_INC_PATH to FSFW_ADDITIONAL_INC_PATHS

## Bugfixes 

- timemanager/TimeStamperIF.h: Timestamp config was not used correctly, leading to different timestamp sizes than configured in fsfwconfig::FSFW_MISSION_TIMESTAMP_SIZE
- TCP server fixes

## Enhancements

### FreeRTOS Queue Handles

- Fixed an internal issue how FreeRTOS MessageQueues were handled

### Linux OSAL

- Better printf error messages

### CMake

- Check for C++11 as mininimum required Version

### Debug Output

- Changed Warning color to magenta, which is well readable on both dark and light mode IDEs


# Changes from ASTP 0.0.1 to 1.0.0

### Host OSAL

- Bugfix in MessageQueue, which caused the sender not to be set properly

### FreeRTOS OSAL

- vRequestContextSwitchFromISR is declared extern "C" so it can be defined in
a C file without issues

### PUS Services

- It is now possible to change the message queue depth for the telecommand verification service (PUS1)
- The same is possible for the event reporting service (PUS5)
- PUS Health Service added, which allows to command and retrieve health via PUS packets


### EnhancedControllerBase

- New base class for a controller which also implements HasActionsIF and HasLocalDataPoolIF

### Local Pool

- Interface of LocalPools has changed. LocalPool is not a template anymore. Instead the size and
bucket number of the pools per page and the number of pages are passed to the ctor instead of
two ctor arguments and a template parameter

### Parameter Service

- The API of the parameter service has been changed to prevent inconsistencies
between documentation and actual code and to clarify usage.
- The parameter ID now consists of:
	1. Domain ID (1 byte)
	2. Unique Identifier (1 byte)
	3. Linear Index (2 bytes)
The linear index can be used for arrays as well as matrices.
The parameter load command now explicitely expects the ECSS PTC and PFC
information as well as  the rows and column number. Rows and column will
default to one, which is equivalent to one scalar parameter (the most
important use-case)
   
### File System Interface

- A new interfaces specifies the functions for a software object which exposes the file system of
a given hardware to use message based file handling (e.g. PUS commanding)

### Internal Error Reporter

- The new internal error reporter uses the local data pools. The pool IDs for
the exisiting three error values and the new error set will be hardcoded for
now, the the constructor for the internal error reporter just takes an object
ID for now.

### Device Handler Base

- There is an additional `PERFORM_OPERATION` step for the device handler base. It is important
that DHB users adapt their polling sequence tables to perform this step. This steps allows for 
a clear distinction between operation and communication steps
- setNormalDatapoolEntriesInvalid is not an abstract method and a default implementation was provided
- getTransitionDelayMs is now an abstract method

### DeviceHandlerIF

- Typo for UNKNOWN_DEVICE_REPLY 

### Events

- makeEvent function: Now takes three input parameters instead of two and
allows setting a unique ID. Event.cpp source file removed, functions now
defined in header directly. Namespaces renamed. Functions declared `constexpr`
now

### Commanding Service Base

- CSB uses the new fsfwconfig::FSFW_CSB_FIFO_DEPTH variable to determine  the FIFO depth for each 
CSB instance. This variable has to be set in the FSFWConfig.h file

### Service Interface

- Proper printf support contained in ServiceInterfacePrinter.h
- CPP ostream support now optional (can reduce executable size by 150 - 250 kB)
- Amalagated header which determines automatically which service interface to use depending on FSFWConfig.h configuration. 
  Users can just use #include <fsfw/serviceinterface/ServiceInterface.h>
- If CPP streams are excluded, sif:: calls won't work anymore and need to be replaced by their printf counterparts. 
  For the fsfw, this can be done by checking the processor define FSFW_CPP_OSTREAM_ENABLED from FSFWConfig.h. 
  For mission code, developers need to replace sif:: calls by the printf counterparts, but only if the CPP stream are excluded. 
  If this is not the case, everything should work as usual.

### ActionHelper and ActionMessage

- ActionHelper finish function and ActionMessage::setCompletionReply now expects explicit 
information whether to report a success or failure message instead of deriving it implicitely 
from returnvalue

### PUS Parameter Service 20

Added PUS parameter service 20 (only custom subservices available).
