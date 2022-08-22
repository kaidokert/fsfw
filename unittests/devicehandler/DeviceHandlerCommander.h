#ifndef TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERCOMMANDER_H_
#define TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERCOMMANDER_H_

#include "fsfw/action/CommandActionHelper.h"
#include "fsfw/action/CommandsActionsIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

class DeviceHandlerCommander : public ExecutableObjectIF,
                               public SystemObject,
                               public CommandsActionsIF {
 public:
  DeviceHandlerCommander(object_id_t objectId);
  virtual ~DeviceHandlerCommander();

  ReturnValue_t performOperation(uint8_t operationCode = 0);
  ReturnValue_t initialize() override;
  MessageQueueIF* getCommandQueuePtr() override;
  void stepSuccessfulReceived(ActionId_t actionId, uint8_t step) override;
  void stepFailedReceived(ActionId_t actionId, uint8_t step, ReturnValue_t returnCode) override;
  void dataReceived(ActionId_t actionId, const uint8_t* data, uint32_t size) override;
  void completionSuccessfulReceived(ActionId_t actionId) override;
  void completionFailedReceived(ActionId_t actionId, ReturnValue_t returnCode) override;

  /**
   * @brief	Calling this function will send the command to the device handler object.
   *
   * @param target	Object ID of the device handler
   * @param actionId	Action ID of the command to send
   */
  ReturnValue_t sendCommand(object_id_t target, ActionId_t actionId);

  ReturnValue_t getReplyReturnCode();
  void resetReplyReturnCode();

 private:
  static const uint32_t QUEUE_SIZE = 20;

  MessageQueueIF* commandQueue = nullptr;

  CommandActionHelper commandActionHelper;

  ReturnValue_t lastReplyReturnCode = returnvalue::FAILED;

  void readCommandQueue();
};

#endif /* TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERCOMMANDER_H_ */
