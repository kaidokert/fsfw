#ifndef FSFW_ACTION_COMMANDSACTIONSIF_H_
#define FSFW_ACTION_COMMANDSACTIONSIF_H_

#include "CommandActionHelper.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/returnvalues/returnvalue.h"

/**
 * Interface to separate commanding actions of other objects.
 * In next iteration, IF should be shortened to three calls:
 * - dataReceived(data)
 * - successReceived(id, step)
 * - failureReceived(id, step, cause)
 * or even
 * - replyReceived(id, step, cause) (if cause == OK, it's a success).
 */
class CommandsActionsIF {
  friend class CommandActionHelper;

 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::COMMANDS_ACTIONS_IF;
  static const ReturnValue_t OBJECT_HAS_NO_FUNCTIONS = MAKE_RETURN_CODE(1);
  static const ReturnValue_t ALREADY_COMMANDING = MAKE_RETURN_CODE(2);
  virtual ~CommandsActionsIF() = default;
  virtual MessageQueueIF* getCommandQueuePtr() = 0;

 protected:
  virtual void stepSuccessfulReceived(ActionId_t actionId, uint8_t step) = 0;
  virtual void stepFailedReceived(ActionId_t actionId, uint8_t step, ReturnValue_t returnCode) = 0;
  virtual void dataReceived(ActionId_t actionId, const uint8_t* data, uint32_t size) = 0;
  virtual void completionSuccessfulReceived(ActionId_t actionId) = 0;
  virtual void completionFailedReceived(ActionId_t actionId, ReturnValue_t returnCode) = 0;
};

#endif /* FSFW_ACTION_COMMANDSACTIONSIF_H_ */
