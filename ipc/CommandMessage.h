#ifndef FRAMEWORK_IPC_COMMANDMESSAGE_H_
#define FRAMEWORK_IPC_COMMANDMESSAGE_H_

#include <framework/ipc/MessageQueueMessage.h>
#include <framework/ipc/FwMessageTypes.h>
#include <config/ipc/MissionMessageTypes.h>



#define MAKE_COMMAND_ID( number )	((MESSAGE_ID << 8) + (number))
typedef uint16_t Command_t;

/**
 * @brief 	Used to pass command messages between tasks. Primary message type
 * 			for IPC. Contains sender, 2-byte command field, and 2 4-byte
 * 			parameters.
 * @details
 * It operates on an external memory which is contained inside a
 * MessageQueueMessage by taking its address.
 * This allows for a more flexible designs of message implementations.
 * The pointer can be passed to different message implementations without
 * the need of unnecessary copying.
 * @author	Bastian Baetz
 */
class CommandMessage: public MessageQueueMessageIF {
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
	static const size_t COMMAND_MESSAGE_SIZE = MessageQueueMessage::HEADER_SIZE
			+ sizeof(Command_t) + 2 * sizeof(uint32_t);

	/**
	 * Default Constructor, does not initialize anything.
	 *
	 * This constructor should be used when receiving a Message, as the
	 * content is filled by the MessageQueue.
	 */
	CommandMessage(MessageQueueMessage* receiverMessage);
	/**
	 * This constructor creates a new message with all message content
	 * initialized
	 *
	 * @param command	The DeviceHandlerCommand_t that will be sent
	 * @param parameter1	The first parameter
	 * @param parameter2	The second parameter
	 */
	CommandMessage(MessageQueueMessage* messageToSet, Command_t command,
			uint32_t parameter1, uint32_t parameter2);

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
	Command_t getCommand() const;

	/*
	 * MessageQueueMessageIF functions, which generally just call the
	 * respective functions of the internal message
	 */
	uint8_t * getBuffer() override;
	const uint8_t * getBuffer() const override;
	void setSender(MessageQueueId_t setId) override;
	MessageQueueId_t getSender() const override;
	uint8_t * getData() override;
	const uint8_t* getData() const override;
	size_t getMinimumMessageSize() const override;
	size_t getMessageSize() const override;
	size_t getMaximumMessageSize() const override;

	/**
	 * Extract message ID, which is the first byte of the command ID.
	 * @return
	 */
	uint8_t getMessageType() const;
	/**
	 * Set the command type of the message
	 * @param the Command to be sent
	 */
	void setCommand(Command_t command);

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


	/**
	 * Set the command to CMD_NONE and try to find
	 * the correct class to handle a more detailed
	 * clear.
	 * Also, calls a mission-specific clearMissionMessage
	 * function to separate between framework and mission
	 * messages. Not optimal, may be replaced by totally
	 * different auto-delete solution (e.g. smart pointers).
	 *
	 */
	void clearCommandMessage();
	void clear() override;

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

private:
	/**
	 * @brief 	Pointer to the message containing the data.
	 * @details
	 * The command message does not actually own the memory containing a
	 * message, it just oprates on it via a pointer to a message queue message.
	 */
	MessageQueueMessage* internalMessage;
};


#endif /* COMMANDMESSAGE_H_ */
