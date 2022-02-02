#ifndef FSFW_IPC_COMMANDMESSAGE_H_
#define FSFW_IPC_COMMANDMESSAGE_H_

#include "CommandMessageIF.h"
#include "FwMessageTypes.h"
#include "MessageQueueMessage.h"

/**
 * @brief 	Default command message used to pass command messages between tasks.
 * 			Primary message type for IPC. Contains sender, 2-byte command ID
 * 			field, and 3 4-byte parameter
 * @details
 * It operates on an external memory which is contained inside a
 * class implementing MessageQueueMessageIF by taking its address.
 * This allows for a more flexible designs of message implementations.
 * The pointer can be passed to different message implementations without
 * the need of unnecessary copying.
 *
 * The command message is based of the generic MessageQueueMessage which
 * currently has an internal message size of 28 bytes.
 * @author	Bastian Baetz
 */
class CommandMessage : public MessageQueueMessage, public CommandMessageIF {
 public:
  /**
   * Default size can accomodate 3 4-byte parameters.
   */
  static constexpr size_t DEFAULT_COMMAND_MESSAGE_SIZE =
      CommandMessageIF::MINIMUM_COMMAND_MESSAGE_SIZE + 3 * sizeof(uint32_t);

  /**
   * @brief   Default Constructor, does not initialize anything.
   * @details
   * This constructor should be used when receiving a Message, as the
   * content is filled by the MessageQueue.
   */
  CommandMessage();
  /**
   * This constructor creates a new message with all message content
   * initialized
   *
   * @param command	The DeviceHandlerCommand_t that will be sent
   * @param parameter1	The first parameter
   * @param parameter2	The second parameter
   */
  CommandMessage(Command_t command, uint32_t parameter1, uint32_t parameter2);

  /**
   * @brief 	Default Destructor
   */
  virtual ~CommandMessage() {}

  /**
   * Read the DeviceHandlerCommand_t that is stored in the message,
   * usually used after receiving.
   *
   * @return the Command stored in the Message
   */
  virtual Command_t getCommand() const override;
  /**
   * Set the command type of the message. Default implementation also
   * sets the message type, which will be the first byte of the command ID.
   * @param the Command to be sent
   */
  virtual void setCommand(Command_t command);

  virtual uint8_t* getData() override;
  virtual const uint8_t* getData() const override;

  /**
   * Get the first parameter of the message
   * @return the first Parameter of the message
   */
  uint32_t getParameter() const;
  /**
   * Set the first parameter of the message
   * @param the first parameter of the message
   */
  void setParameter(uint32_t parameter1);
  uint32_t getParameter2() const;
  void setParameter2(uint32_t parameter2);
  uint32_t getParameter3() const;
  void setParameter3(uint32_t parameter3);

  /**
   * check if a message was cleared
   *
   * @return if the command is CMD_NONE
   */
  bool isClearedCommandMessage();

  /**
   * Sets the command to REPLY_REJECTED with parameter UNKNOWN_COMMAND.
   * Is needed quite often, so we better code it once only.
   */
  void setToUnknownCommand() override;

  /**
   * A command message can be rejected and needs to offer a function
   * to set a rejected reply
   * @param reason
   * @param initialCommand
   */
  void setReplyRejected(ReturnValue_t reason, Command_t initialCommand) override;
  /**
   * Corrensonding getter function.
   * @param initialCommand
   * @return
   */
  ReturnValue_t getReplyRejectedReason(Command_t* initialCommand = nullptr) const override;

  virtual void clear() override;
  void clearCommandMessage();

  /**
   * Extract message ID, which is the first byte of the command ID for the
   * default implementation.
   * @return
   */
  virtual uint8_t getMessageType() const override;

  /** MessageQueueMessageIF functions used for minimum size check. */
  size_t getMinimumMessageSize() const override;
};

#endif /* FSFW_IPC_COMMANDMESSAGE_H_ */
