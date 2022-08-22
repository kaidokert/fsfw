#ifndef FSFW_ACTION_ACTIONMESSAGE_H_
#define FSFW_ACTION_ACTIONMESSAGE_H_

#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

using ActionId_t = uint32_t;

/**
 * @brief   These messages are part of the action module of the FSFW.
 * @details
 * These messages are sent amongst objects implementing the HasActionsIF. Classes like the
 * ActionHelper are able to process these messages.
 */
class ActionMessage {
 private:
  ActionMessage();

 public:
  static const uint8_t MESSAGE_ID = messagetypes::ACTION;
  static const Command_t EXECUTE_ACTION = MAKE_COMMAND_ID(1);
  static const Command_t STEP_SUCCESS = MAKE_COMMAND_ID(2);
  static const Command_t STEP_FAILED = MAKE_COMMAND_ID(3);
  static const Command_t DATA_REPLY = MAKE_COMMAND_ID(4);
  static const Command_t COMPLETION_SUCCESS = MAKE_COMMAND_ID(5);
  static const Command_t COMPLETION_FAILED = MAKE_COMMAND_ID(6);

  virtual ~ActionMessage();
  static void setCommand(CommandMessage* message, ActionId_t fid, store_address_t parameters);

  static ActionId_t getActionId(const CommandMessage* message);
  static store_address_t getStoreId(const CommandMessage* message);

  static void setStepReply(CommandMessage* message, ActionId_t fid, uint8_t step,
                           ReturnValue_t result = returnvalue::OK);
  static uint8_t getStep(const CommandMessage* message);
  static ReturnValue_t getReturnCode(const CommandMessage* message);
  static void setDataReply(CommandMessage* message, ActionId_t actionId, store_address_t data);
  static void setCompletionReply(CommandMessage* message, ActionId_t fid, bool success,
                                 ReturnValue_t result = returnvalue::OK);

  static void clear(CommandMessage* message);
};

#endif /* FSFW_ACTION_ACTIONMESSAGE_H_ */
