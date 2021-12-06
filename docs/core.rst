.. _core:

Core Modules
=============

The core modules provide the most important functionalities of the Flight Software Framework.

Clock
------

- This is a class of static functions that can be used at anytime
- Leap Seconds must be set if any time conversions from UTC to other times is used

Object Manager
---------------

- Must be created during program startup
- The component which handles all references. All :cpp:class:`SystemObject`\s register at this
  component.
- All :cpp:class:`SystemObject`\s needs to have a unique Object ID. Those can be managed like
  framework objects.
- A reference to an object can be retrieved by calling the ``get`` function of
  :cpp:class:`ObjectManagerIF`. The target type must be specified as a template argument.
  A ``nullptr`` check of the returning pointer must be done. This function is based on
  run-time type information.

	.. code-block:: cpp

		template <typename T> T* ObjectManagerIF::get(object_id_t id);

- A typical way to create all objects on startup is a handing a static produce function to the
  ObjectManager on creation. By calling ``ObjectManager::instance()->initialize(produceFunc)`` the
  produce function will be called and all :cpp:class:`SystemObject`\s will be initialized
  afterwards.

Event Manager
---------------

- Component which allows routing of events
- Other objects can subscribe to specific events, ranges of events or all events of an object.
- Subscriptions can be done during runtime but should be done during initialization
- Amounts of allowed subscriptions can be configured in ``FSFWConfig.h``

Health Table
---------------

- A component which holds every health state
- Provides a thread safe way to access all health states without the need of message exchanges

Stores
--------------

- The message based communication can only exchange a few bytes of information inside the message
  itself. Therefore, additional information can  be exchanged with Stores. With this, only the
  store address must be exchanged in the message.
- Internally, the FSFW uses an IPC Store to exchange data between processes. For incoming TCs a TC
  Store is used. For outgoing TM a TM store is used.
- All of them should use the Thread Safe Class storagemanager/PoolManager

Tasks
---------

There are two different types of tasks:

- The PeriodicTask just executes objects that are of type ExecutableObjectIF in the order of the
  insertion to the Tasks.
- FixedTimeslotTask executes a list of calls in the order of the given list. This is intended for
  DeviceHandlers, where polling should be in a defined order. An example can be found in 
  ``defaultcfg/fsfwconfig/pollingSequence`` folder

