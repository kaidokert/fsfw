#ifndef FRAMEWORK_IPC_COMMANDMESSAGE_H_
#define FRAMEWORK_IPC_COMMANDMESSAGE_H_

#include <framework/ipc/CommandMessageBase.h>
#include <framework/ipc/MessageQueueMessage.h>
#include <framework/ipc/FwMessageTypes.h>

namespace messagetypes {
// Implemented in config.
void clearMissionMessage(CommandMessageIF* message);
}

/**
 * @brief 	Default command message used to pass command messages between tasks.
 * 			Primary message type for IPC. Contains sender, 2-byte command ID
 * 			field, and 2 4-byte parameters.
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
class CommandMessage: public CommandMessageBase {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::COMMAND_MESSAGE;
	static const ReturnValue_t UNKNOWN_COMMAND = MAKE_RETURN_CODE(0x01);

	static const uint8_t MESSAGE_ID = messagetypes::COMMAND;
	//! Used internally, will be ignored
	static const Command_t CMD_NONE = MAKE_COMMAND_ID( 0 );
	static const Command_t REPLY_COMMAND_OK = MAKE_COMMAND_ID( 3 );
	//! Reply indicating that the current command was rejected,
	//! par1 should contain the error code
	static const Command_t REPLY_REJECTED = MAKE_COMMAND_ID( 0xD1 );

	/**
	 * This is the size of a message as it is seen by the MessageQueue.
	 * 14 of the 24 available MessageQueueMessage bytes are used.
	 */
	static const size_t MINIMUM_COMMAND_MESSAGE_SIZE =
			MessageQueueMessage::HEADER_SIZE + sizeof(Command_t) +
			2 * sizeof(uint32_t);

	/**
	 * Default Constructor, does not initialize anything.
	 *
	 * This constructor should be used when receiving a Message, as the
	 * content is filled by the MessageQueue.
	 */
	CommandMessage(MessageQueueMessageIF* receiverMessage);
	/**
	 * This constructor creates a new message with all message content
	 * initialized
	 *
	 * @param command	The DeviceHandlerCommand_t that will be sent
	 * @param parameter1	The first parameter
	 * @param parameter2	The second parameter
	 */
	CommandMessage(MessageQueueMessageIF* messageToSet, Command_t command,
			uint32_t parameter1, uint32_t parameter2);

	/**
	 * @brief 	Default Destructor
	 */
	virtual ~CommandMessage() {}

	/** MessageQueueMessageIF functions used for minimum size check. */
	size_t getMinimumMessageSize() const override;
	/** MessageQueueMessageIF functions used for maximum size check. */
	size_t getMaximumMessageSize() const override;

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

	/**
	 * Get the second parameter of the message
	 * @return the second Parameter of the message
	 */
	uint32_t getParameter2() const;

	/**
	 * Set the second parameter of the message
	 * @param the second parameter of the message
	 */
	void setParameter2(uint32_t parameter2);

	void clear() override;

	/**
	 * Set the command to CMD_NONE and try to find the correct class to handle
	 * a more detailed clear.
	 * Also, calls a mission-specific clearMissionMessage function to separate
	 * between framework and mission messages. Not optimal, may be replaced by
	 * totally different auto-delete solution (e.g. smart pointers).
	 *
	 */
	void clearCommandMessage();

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
	void setToUnknownCommand();

	void setReplyRejected(ReturnValue_t reason,
			Command_t initialCommand = CMD_NONE);
	ReturnValue_t getRejectedReplyReason(
			Command_t* initialCommand = nullptr) const;
};


#endif /* COMMANDMESSAGE_H_ */
