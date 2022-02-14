#ifndef TEST_TESTDEVICES_TESTDEVICEHANDLER_H_
#define TEST_TESTDEVICES_TESTDEVICEHANDLER_H_

#include "devicedefinitions/testDeviceDefinitions.h"
#include "fsfw/devicehandlers/DeviceHandlerBase.h"
#include "fsfw/globalfunctions/PeriodicOperationDivider.h"
#include "fsfw/timemanager/Countdown.h"

/**
 * @brief 	Basic dummy device handler to test device commanding without a physical device.
 * @details
 * This test device handler provided a basic demo for the device handler object.
 * It can also be commanded with the following PUS services, using
 * the specified object ID of the test device handler.
 *
 * 	1. PUS Service 8 - Functional commanding
 * 	2. PUS Service 2 - Device access, raw commanding
 * 	3. PUS Service 20 - Parameter Management
 * 	4. PUS Service 3 - Housekeeping

 * @author	R. Mueller
 * @ingroup devices
 */
class TestDevice : public DeviceHandlerBase {
 public:
  /**
   * Build the test device in the factory.
   * @param objectId This ID will be assigned to the test device handler.
   * @param comIF The ID of the Communication IF used by test device handler.
   * @param cookie Cookie object used by the test device handler. This is
   * also used and passed to the comIF object.
   * @param onImmediately This will start a transition to MODE_ON immediately
   * so the device handler jumps into #doStartUp. Should only be used
   * in development to reduce need of commanding while debugging.
   * @param changingDataset
   * Will be used later to change the local datasets containeds in the device.
   */
  TestDevice(object_id_t objectId, object_id_t comIF, CookieIF* cookie,
             testdevice::DeviceIndex deviceIdx = testdevice::DeviceIndex::DEVICE_0,
             bool fullInfoPrintout = false, bool changingDataset = true);

  /**
   * This can be used to enable and disable a lot of demo print output.
   * @param enable
   */
  void enableFullDebugOutput(bool enable);

  virtual ~TestDevice();

  //! Size of internal buffer used for communication.
  static constexpr uint8_t MAX_BUFFER_SIZE = 255;

  //! Unique index if the device handler is created multiple times.
  testdevice::DeviceIndex deviceIdx = testdevice::DeviceIndex::DEVICE_0;

 protected:
  testdevice::TestDataSet dataset;
  //! This is used to reset the dataset after a commanded change has been made.
  bool resetAfterChange = false;
  bool commandSent = false;

  /** DeviceHandlerBase overrides (see DHB documentation) */

  /**
   * Hook into the DHB #performOperation call which is executed
   * periodically.
   */
  void performOperationHook() override;

  virtual void doStartUp() override;
  virtual void doShutDown() override;

  virtual ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t* id) override;
  virtual ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t* id) override;
  virtual ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                const uint8_t* commandData,
                                                size_t commandDataLen) override;

  virtual void fillCommandAndReplyMap() override;

  virtual ReturnValue_t scanForReply(const uint8_t* start, size_t len, DeviceCommandId_t* foundId,
                                     size_t* foundLen) override;
  virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t* packet) override;
  virtual uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;

  virtual void doTransition(Mode_t modeFrom, Submode_t subModeFrom) override;

  virtual ReturnValue_t initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
                                                LocalDataPoolManager& poolManager) override;
  virtual LocalPoolObjectBase* getPoolObjectHandle(lp_id_t localPoolId) override;

  /* HasParametersIF  overrides */
  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper* parameterWrapper,
                                     const ParameterWrapper* newValues,
                                     uint16_t startAtIndex) override;

  uint8_t commandBuffer[MAX_BUFFER_SIZE];

  bool fullInfoPrintout = false;
  bool oneShot = true;

  /* Variables for parameter service */
  uint32_t testParameter0 = 0;
  int32_t testParameter1 = -2;
  float vectorFloatParams2[3] = {};

  /* Change device handler functionality, changeable via parameter service */
  uint8_t periodicPrintout = false;
  uint8_t changingDatasets = false;

  ReturnValue_t buildNormalModeCommand(DeviceCommandId_t deviceCommand, const uint8_t* commandData,
                                       size_t commandDataLen);
  ReturnValue_t buildTestCommand0(DeviceCommandId_t deviceCommand, const uint8_t* commandData,
                                  size_t commandDataLen);
  ReturnValue_t buildTestCommand1(DeviceCommandId_t deviceCommand, const uint8_t* commandData,
                                  size_t commandDataLen);
  void passOnCommand(DeviceCommandId_t command, const uint8_t* commandData, size_t commandDataLen);

  ReturnValue_t interpretingNormalModeReply();
  ReturnValue_t interpretingTestReply0(DeviceCommandId_t id, const uint8_t* packet);
  ReturnValue_t interpretingTestReply1(DeviceCommandId_t id, const uint8_t* packet);
  ReturnValue_t interpretingTestReply2(DeviceCommandId_t id, const uint8_t* packet);

  /* Some timer utilities */
  uint8_t divider1 = 2;
  PeriodicOperationDivider opDivider1 = PeriodicOperationDivider(divider1);
  uint8_t divider2 = 10;
  PeriodicOperationDivider opDivider2 = PeriodicOperationDivider(divider2);
  static constexpr uint32_t initTimeout = 2000;
  Countdown countdown1 = Countdown(initTimeout);
};

#endif /* TEST_TESTDEVICES_TESTDEVICEHANDLER_H_ */
