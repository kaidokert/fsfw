![FSFW Logo](logo/FSFW_Logo_V3_bw.png)
# Flight Software Framework (FSFW)

The Flight Software Framework is a C++ Object Oriented Framework for unmanned,
automated systems like Satellites. 

The initial version of the Flight Software Framework was developed during
the Flying Laptop Project by the University of Stuttgart in cooperation
with Airbus Defence and Space GmbH.

## Intended Use

The framework is designed for systems, which communicate with external devices, perform control loops, receive telecommands and send telemetry, and need to maintain a high level of availability. 
Therefore, a mode and health system provides control over the states of the software and the controlled devices. 
In addition, a simple mechanism of event based fault detection, isolation and recovery is implemented as well. 

The recommended hardware is a microprocessor with more than 2 MB of RAM and 1 MB of non-volatile Memory. 
For reference, current Applications use a Cobham Gaisler UT699 (LEON3FT), a ISISPACE IOBC or a Zynq-7020 SoC.


## Structure

The general structure is driven by the usage of interfaces provided by objects. The FSFW uses C++11 as baseline. The intention behind this is that this C++ Standard should be widely available, even with older compilers.
The FSFW uses dynamic allocation during the initialization but provides static containers during runtime. 
This simplifies the instantiation of objects and allows the usage of some standard containers. 
Dynamic Allocation after initialization is discouraged and different solutions are provided in the FSFW to achieve that.
The fsfw uses Run-time type information.
Exceptions are not allowed.

### Failure Handling

Functions should return a defined ReturnValue_t to signal to the caller that something is gone wrong. 
Returnvalues must be unique. For this the function HasReturnvaluesIF::makeReturnCode or the Macro MAKE_RETURN can be used.
The CLASS_ID is a unique id for that type of object. See returnvalues/FwClassIds.

### OSAL
The FSFW provides operation system abstraction layers for Linux, FreeRTOS and RTEMS. A independent OSAL called "host" is currently not finished. This aims to be running on windows as well. 
The OSAL provides periodic tasks, message queues, clocks and Semaphores as well as Mutexes.

### Core Components 

Clock:
 * This is a class of static functions that can be used at anytime
 * Leap Seconds must be set if any time conversions from UTC to other times is used

ObjectManager (must be created): 

* The component which handles all references. All SystemObjects register at this component. 
* Any SystemObject needs to have a unique ObjectId. Those can be managed like objects::framework_objects.
* A reference to an object can be get by calling the following function. T must be the specific Interface you want to call.
A nullptr check of the returning Pointer must be done. This function is based on Run-time type information. 

``` c++
	template <typename T> T* ObjectManagerIF::get( object_id_t id )

```
* A typical way to create all objects on startup is a handing a static produce function to the ObjectManager on creation.
By calling objectManager->initialize() the produce function will be called and all SystemObjects will be initialized afterwards.

Event Manager:

* Component which allows routing of events
* Other objects can subscribe to specific events, ranges of events or all events of an object.
* Subscriptions can be done during runtime but should be done during initialization
* Amounts of allowed subscriptions must be configured by setting this parameters:

``` c++
namespace fsfwconfig {
//! Configure the allocated pool sizes for the event manager.
static constexpr size_t FSFW_EVENTMGMR_MATCHTREE_NODES = 240;
static constexpr size_t FSFW_EVENTMGMT_EVENTIDMATCHERS = 120;
static constexpr size_t FSFW_EVENTMGMR_RANGEMATCHERS   = 120;
}
```


Health Table:

* A component which holds every health state 
* Provides a thread safe way to access all health states without the need of message exchanges

Stores

* The message based communication can only exchange a few bytes of information inside the message itself. Therefore, additional information can be exchanged with Stores. With this, only the store address must be exchanged in the message.
* Internally, the FSFW uses an IPC Store to exchange data between processes. For incoming TCs a TC Store is used. For outgoing TM a TM store is used.
* All of them should use the Thread Safe Class storagemanager/PoolManager

Tasks

There are two different types of tasks:
 * The PeriodicTask just executes objects that are of type ExecutableObjectIF in the order of the insertion to the Tasks.
 * FixedTimeslotTask executes a list of calls in the order of the given list. This is intended for DeviceHandlers, where polling should be in a defined order. An example can be found in defaultcfg/fsfwconfig/pollingSequence


### Static Ids in the framework

Some parts of the framework use a static routing address for communication. 
An example setup of ids can be found in the example config in "defaultcft/fsfwconfig/objects/Factory::setStaticFrameworkObjectIds()".

### Events

Events are tied to objects. EventIds can be generated by calling the Macro MAKE_EVENT. This works analog to the returnvalues.
Every object that needs own EventIds has to get a unique SUBSYSTEM_ID. 
Every SystemObject can call triggerEvent from the parent class.
Therefore, event messages contain the specific EventId and the objectId of the object that has triggered.

### Internal Communication

Components communicate mostly over Message through Queues. 
Those queues are created by calling the singleton QueueFactory::instance()->create(). 

### External Communication

The external communication with the mission control system is mostly up to the user implementation.
The FSFW provides PUS Services which can be used to but don't need to be used. 
The services can be seen as a conversion from a TC to a message based communication and back.

#### CCSDS Frames, CCSDS Space Packets and PUS

If the communication is based on CCSDS Frames and Space Packets, several classes can be used to distributed the packets to the corresponding services. Those can be found in tcdistribution. 
If Space Packets are used, a timestamper must be created. 
An example can be found in the timemanager folder, this uses CCSDSTime::CDS_short.

#### DeviceHandling

DeviceHandlers are a core component of the FSFW. 
The idea is, to have a software counterpart of every physical device to provide a simple mode, health and commanding interface.
By separating the underlying Communication Interface with DeviceCommunicationIF, a DH can be tested on different hardware.
The DH has mechanisms to monitor the communication with the physical device which allow for FDIR reaction. 
A standard FDIR component for the DH will be created automatically but can be overwritten by the user.

#### Modes, Health

The two interfaces HasModesIF and HasHealthIF provide access for commanding and monitoring of components.
On-board Mode Management is implement in hierarchy system. 
DeviceHandlers and Controllers are the lowest part of the hierarchy. 
The next layer are Assemblies. Those assemblies act as a component which handle redundancies of handlers. 
Assemblies share a common core with the next level which are the Subsystems. 

Those Assemblies are intended to act as auto-generated components from a database which describes the subsystem modes. 
The definitions contain transition and target tables which contain the DH, Assembly and Controller Modes to be commanded.
Transition tables contain as many steps as needed to reach the mode from any other mode, e.g. a switch into any higher AOCS mode might first turn on the sensors, than the actuators and the controller as last component. 
The target table is used to describe the state that is checked continuously by the subsystem. 
All of this allows System Modes to be generated as Subsystem object as well from the same database. 
This System contains list of subsystem modes in the transition and target tables. 
Therefore, it allows a modular system to create system modes and easy commanding of those, because only the highest components must be commanded.

The health state represents if the component is able to perform its tasks. 
This can be used to signal the system to avoid using this component instead of a redundant one.
The on-board FDIR uses the health state for isolation and recovery. 

## Example config

A example config can be found in defaultcfg/fsfwconfig.

## Unit Tests

Unit Tests are provided in the unittest folder. Those use the catch2 framework but do not include catch2 itself. 
See README.md in the unittest Folder.