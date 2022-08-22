#ifndef FSFW_IPC_COMMANDMESSAGEIF_H_
#define FSFW_IPC_COMMANDMESSAGEIF_H_

#include "../returnvalues/returnvalue.h"
#include "FwMessageTypes.h"
#include "MessageQueueMessageIF.h"

#define MAKE_COMMAND_ID(number) ((MESSAGE_ID << 8) + (number))
typedef uint16_t Command_t;

class CommandMessageIF {
 public:
  /**
   * Header consists of sender ID and command ID.
   */
  static constexpr size_t HEADER_SIZE = MessageQueueMessageIF::HEADER_SIZE + sizeof(Command_t);
  /**
   * This minimum size is derived from the interface requirement to be able
   * to set a rejected reply, which contains a returnvalue and the initial
   * command.
   */
  static constexpr size_t MINIMUM_COMMAND_MESSAGE_SIZE =
      CommandMessageIF::HEADER_SIZE + sizeof(ReturnValue_t) + sizeof(Command_t);

  static constexpr Command_t makeCommandId(uint8_t messageId, uint8_t uniqueId) {
    return ((messageId << 8) | uniqueId);
  }

  static const uint8_t INTERFACE_ID = CLASS_ID::COMMAND_MESSAGE;
  static const ReturnValue_t UNKNOWN_COMMAND = MAKE_RETURN_CODE(1);

  static const uint8_t MESSAGE_ID = messagetypes::COMMAND;
  //! Used internally, shall be ignored
  static const Command_t CMD_NONE = MAKE_COMMAND_ID(0);
  static const Command_t REPLY_COMMAND_OK = MAKE_COMMAND_ID(1);
  //! Reply indicating that the current command was rejected,
  //! par1 should contain the error code
  static const Command_t REPLY_REJECTED = MAKE_COMMAND_ID(2);

  virtual ~CommandMessageIF(){};

  /**
   * A command message shall have a uint16_t command ID field.
   * @return
   */
  virtual Command_t getCommand() const = 0;
  /**
   * A command message shall have a uint8_t message type ID field.
   * @return
   */
  virtual uint8_t getMessageType() const = 0;

  /**
   * A command message can be rejected and needs to offer a function
   * to set a rejected reply
   * @param reason
   * @param initialCommand
   */
  virtual void setReplyRejected(ReturnValue_t reason, Command_t initialCommand) = 0;
  /**
   * Corrensonding getter function.
   * @param initialCommand
   * @return
   */
  virtual ReturnValue_t getReplyRejectedReason(Command_t* initialCommand = nullptr) const = 0;

  virtual void setToUnknownCommand() = 0;

  virtual void clear() = 0;
};

#endif /* FSFW_IPC_COMMANDMESSAGEIF_H_ */
