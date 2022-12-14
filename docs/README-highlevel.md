High-level overview
======

# Structure

The general structure is driven by the usage of interfaces provided by objects. 
The FSFW uses C++11 as baseline. The intention behind this is that this C++ Standard should be
widely available, even with older compilers.
The FSFW uses dynamic allocation during the initialization but provides static containers during runtime. 
This simplifies the instantiation of objects and allows the usage of some standard containers. 
Dynamic Allocation after initialization is discouraged and different solutions are provided in the
FSFW to achieve that. The fsfw uses run-time type information but exceptions are not allowed.

# Failure Handling

Functions should return a defined `ReturnValue_t` to signal to the caller that something has
gone wrong. Returnvalues must be unique. For this the function `HasReturnvaluesIF::makeReturnCode`
or the macro `MAKE_RETURN` can be used. The `CLASS_ID` is a unique id for that type of object.
See `returnvalues/FwClassIds` folder. The user can add custom `CLASS_ID`s via the
`fsfwconfig` folder.

# OSAL

The FSFW provides operation system abstraction layers for Linux, FreeRTOS and RTEMS. 
The OSAL provides periodic tasks, message queues, clocks and semaphores as well as mutexes.
The [OSAL README](doc/README-osal.md#top) provides more detailed information on provided components
and how to use them.

# Core Components 

The FSFW has following core components. More detailed informations can be found in the
[core component section](doc/README-core.md#top):

1. Tasks: Abstraction for different (periodic) task types like periodic tasks or tasks 
   with fixed timeslots
2. ObjectManager: This module stores all `SystemObjects` by mapping a provided unique object ID
   to the object handles.
3. Static Stores: Different stores are provided to store data of variable size (like telecommands 
   or small telemetry) in a pool structure without using dynamic memory allocation.
   These pools are allocated up front.
3. Clock: This module provided common time related functions
4. EventManager: This module allows routing of events generated by `SystemObjects`
5. HealthTable: A component which stores the health states of objects

# Static IDs in the framework

Some parts of the framework use a static routing address for communication. 
An example setup of ids can be found in the example config in `defaultcft/fsfwconfig/objects`
 inside the function `Factory::setStaticFrameworkObjectIds()`.

# Events

Events are tied to objects. EventIds can be generated by calling the Macro MAKE_EVENT. 
This works analog to the returnvalues. Every object that needs own EventIds has to get a
unique SUBSYSTEM_ID. Every SystemObject can call triggerEvent from the parent class.
Therefore, event messages contain the specific EventId and the objectId of the object that
has triggered.

# Internal Communication

Components communicate mostly via Messages through Queues. 
Those queues are created by calling the singleton `QueueFactory::instance()->create()` which
will create `MessageQueue` instances for the used OSAL.

# External Communication

The external communication with the mission control system is mostly up to the user implementation.
The FSFW provides PUS Services which can be used to but don't need to be used. 
The services can be seen as a conversion from a TC to a message based communication and back.

## TMTC Communication

The FSFW provides some components to facilitate TMTC handling via the PUS commands.
For example, a UDP or TCP PUS server socket can be opened on a specific port using the
files located in `osal/common`. The FSFW example uses this functionality to allow sending telecommands
and receiving telemetry using the [TMTC commander application](https://github.com/spacefisch/tmtccmd).
Simple commands like the PUS Service 17 ping service can be tested by simply running the
`tmtc_client_cli.py` or `tmtc_client_gui.py` utility in 
the [example tmtc folder](https://egit.irs.uni-stuttgart.de/fsfw/fsfw_example_public/src/branch/master/tmtc)
while the `fsfw_example` application is running.

More generally, any class responsible for handling incoming telecommands and sending telemetry
can implement the generic `TmTcBridge` class located in `tmtcservices`. Many applications
also use a dedicated polling task for reading telecommands which passes telecommands
to the `TmTcBridge` implementation.

## CCSDS Frames, CCSDS Space Packets and PUS

If the communication is based on CCSDS Frames and Space Packets, several classes can be used to
distributed the packets to the corresponding services. Those can be found in `tcdistribution`. 
If Space Packets are used, a timestamper has to be provided by the user. 
An example can be found in the `timemanager` folder, which uses `CCSDSTime::CDS_short`.

# Device Handlers

DeviceHandlers are another important component of the FSFW. 
The idea is, to have a software counterpart of every physical device to provide a simple mode,
health and commanding interface. By separating the underlying Communication Interface with 
`DeviceCommunicationIF`, a device handler (DH) can be tested on different hardware.
The DH has mechanisms to monitor the communication with the physical device which allow
for FDIR reaction.  Device Handlers can be created by implementing `DeviceHandlerBase`. 
A standard FDIR component for the DH will be created automatically but can
be overwritten by the user. More information on DeviceHandlers can be found in the
related [documentation section](doc/README-devicehandlers.md#top).

# Modes and Health

The two interfaces `HasModesIF` and `HasHealthIF` provide access for commanding and monitoring
of components. On-board Mode Management is implement in hierarchy system. 
DeviceHandlers and Controllers are the lowest part of the hierarchy. 
The next layer are Assemblies. Those assemblies act as a component which handle
redundancies of handlers. Assemblies share a common core with the next level which
are the Subsystems. 

Those Assemblies are intended to act as auto-generated components from a database which describes
the subsystem modes. The definitions contain transition and target tables which contain the DH,
Assembly and Controller Modes to be commanded.
Transition tables contain as many steps as needed to reach the mode from any other mode, e.g. a
switch into any higher AOCS mode might first turn on the sensors, than the actuators and the
controller as last component. 
The target table is used to describe the state that is checked continuously by the subsystem. 
All of this allows System Modes to be generated as Subsystem object as well from the same database. 
This System contains list of subsystem modes in the transition and target tables. 
Therefore, it allows a modular system to create system modes and easy commanding of those, because
only the highest components must be commanded.

The health state represents if the component is able to perform its tasks. 
This can be used to signal the system to avoid using this component instead of a redundant one.
The on-board FDIR uses the health state for isolation and recovery. 

# Unit Tests

Unit Tests are provided in the unittest folder. Those use the catch2 framework but do not include
catch2 itself. More information on how to run these tests can be found in the separate
[`fsfw_tests` reposoitory](https://egit.irs.uni-stuttgart.de/fsfw/fsfw_tests)
