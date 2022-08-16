.. _dhb-prim-doc:

Device Handlers
==================

Device handler components represent, control and monitor equipment, for example sensors or actuators
of a spacecraft or the payload.

Most device handlers have the same common functionality or
requirements, which are fulfilled by implementing certain interfaces:

- The handler/device needs to be commandable: :cpp:class:`HasActionsIF`
- The handler needs to communicate with the physical device via a dedicated
  communication bus, for example SpaceWire, UART or SPI: :cpp:class:`DeviceCommunicationIF`
- The handler has housekeeping data which has to be exposed to the operator and/or other software
  components: :cpp:class:`HasLocalDataPoolIF`
- The handler has configurable parameters: :cpp:class:`ReceivesParameterMessagesIF` which
  also implements :cpp:class:`HasParametersIF`
- The handler has health states, for example to indicate a broken device:
  :cpp:class:`HasHealthIF`
- The handler has modes. For example there are the core modes `MODE_ON`, `MODE_OFF`
  and `MODE_NORMAL` provided by the FSFW. `MODE_ON` means that a device is physically powered
  but that it is not periodically polling data from the
  physical device, `MODE_NORMAL` means that it is able to do that: :cpp:class:`HasModesIF`

The device handler base therefore provides abstractions for a lot of common
functionality, which can potentially avoid high amounts or logic and code duplication.

Template Device Handler Base File
----------------------------------

This is an example template device handler header file with all necessary
functions implemented:

.. code-block:: cpp

	#ifndef __TESTDEVICEHANDLER_H_
	#define __TESTDEVICEHANDLER_H_

	#include <fsfw/devicehandlers/DeviceHandlerBase.h>

	class TestDeviceHandler: DeviceHandlerBase {
	public:
	  TestDeviceHandler(object_id_t objectId, object_id_t comIF, CookieIF* cookie);
	private:
	  void doStartUp() override;
	  void doShutDown() override;
	  ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t* id) override;
	  ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t* id) override;
	  void fillCommandAndReplyMap() override;
	  ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand, const uint8_t* commandData,
	                                        size_t commandDataLen) override;
	  ReturnValue_t scanForReply(const uint8_t* start, size_t remainingSize, DeviceCommandId_t* foundId,
	                             size_t* foundLen) override;
	  ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t* packet) override;
	  uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;
	  ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
	                                        LocalDataPoolManager& poolManager) override;

	};

	#endif /* __TESTDEVICEHANDLER_H_ */

and the respective source file with sensible default return values:

.. code-block:: cpp

	#include "TestDeviceHandler.h"

	TestDeviceHandler::TestDeviceHandler(object_id_t objectId, object_id_t comIF, CookieIF* cookie)
	    : DeviceHandlerBase(objectId, comIF, cookie) {}

	void TestDeviceHandler::doStartUp() {}

	void TestDeviceHandler::doShutDown() {}

	ReturnValue_t TestDeviceHandler::buildNormalDeviceCommand(DeviceCommandId_t* id) {
	  return returnvalue::OK;
	}

	ReturnValue_t TestDeviceHandler::buildTransitionDeviceCommand(DeviceCommandId_t* id) {
	  return returnvalue::OK;
	}

	void TestDeviceHandler::fillCommandAndReplyMap() {}

	ReturnValue_t TestDeviceHandler::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
	                                                         const uint8_t* commandData,
	                                                         size_t commandDataLen) {
	  return returnvalue::OK;
	}

	ReturnValue_t TestDeviceHandler::scanForReply(const uint8_t* start, size_t remainingSize,
	                                              DeviceCommandId_t* foundId, size_t* foundLen) {
	  return returnvalue::OK;
	}

	ReturnValue_t TestDeviceHandler::interpretDeviceReply(DeviceCommandId_t id,
	                                                      const uint8_t* packet) {
	  return returnvalue::OK;
	}

	uint32_t TestDeviceHandler::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
	  return 10000;
	}

	ReturnValue_t TestDeviceHandler::initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
	                                                         LocalDataPoolManager& poolManager) {
	  return returnvalue::OK;
	}
