/**
 * @file	CommandMessage.h
 * @brief	This file defines the CommandMessage class.
 * @date	20.06.2013
 * @author	baetz
 */

#ifndef COMMANDMESSAGE_H_
#define COMMANDMESSAGE_H_

//Remember to add new Message Types to the clear function!
#define MODE_COMMAND_MESSAGE_ID 1
#define HEALTH_COMMAND_MESSAGE_ID 2
#define MODE_SEQUENCE_MESSAGE_ID 3
#define FUNCTION_MESSAGE_ID 4
#define TM_STORE_MESSAGE_ID 5
#define TTC_TM_MESSAGE_ID 0x10
#define DEVICE_HANDLER_COMMAND_MESSAGE_ID 0x44
#define LIMIT_MESSAGE_ID 0x4C
#define MEMORY_MESSAGE_ID 0x4D
#define PAYLOAD_HANDLER_MESSAGE_ID 0x50
#define PARAMETER_MESSAGE_ID 0x60


#include <framework/ipc/MessageQueueMessage.h>

#define MAKE_COMMAND_ID( number )	((MESSAGE_ID << 8) + (number))
typedef ReturnValue_t Command_t;

class CommandMessage : public MessageQueueMessage {
public:
	static const uint8_t INTERFACE_ID = COMMAND_MESSAGE;
	static const ReturnValue_t UNKNOW_COMMAND = MAKE_RETURN_CODE(0x01);


	static const uint8_t MESSAGE_ID = 0;
	static const Command_t CMD_NONE = MAKE_COMMAND_ID( 0 );//!< Used internally, will be ignored
	static const Command_t REPLY_COMMAND_OK = MAKE_COMMAND_ID( 3 );
	static const Command_t REPLY_REJECTED = MAKE_COMMAND_ID( 0xD1 );//!< Reply indicating that the current command was rejected, par1 should contain the error code

	/**
	 * This is the size of a message as it is seen by the MessageQueue
	 */
	static const size_t COMMAND_MESSAGE_SIZE = HEADER_SIZE
			+ sizeof(Command_t) + 2 * sizeof(uint32_t);

	/**
	 * Default Constructor, does not initialize anything.
	 *
	 * This constructor should be used when receiving a Message, as the content is filled by the MessageQueue.
	 */
	CommandMessage();
	/**
	 * This constructor creates a new message with all message content initialized
	 *
	 * @param command	The DeviceHandlerCommand_t that will be sent
	 * @param parameter1	The first parameter
	 * @param parameter2	The second parameter
	 */
	CommandMessage(Command_t command,
			uint32_t parameter1, uint32_t parameter2);

	/**
	 * Default Destructor
	 */
	virtual ~CommandMessage() {
	}

	/**
	 * Read the DeviceHandlerCommand_t that is stored in the message, usually used after receiving
	 *
	 * @return the Command stored in the Message
	 */
	Command_t getCommand() const;

	/**
	 * Set the DeviceHandlerCOmmand_t of the message
	 *
	 * @param the Command to be sent
	 */
	void setCommand(Command_t command);

	/**
	 * Get the first parameter of the message
	 *
	 * @return the first Parameter of the message
	 */
	uint32_t getParameter() const;

	/**
	 * Set the first parameter of the message
	 *
	 * @param the first parameter of the message
	 */
	void setParameter(uint32_t parameter1);

	/**
	 * Get the second parameter of the message
	 *
	 * @return the second Parameter of the message
	 */
	uint32_t getParameter2() const;

	/**
	 * Set the second parameter of the message
	 *
	 * @param the second parameter of the message
	 */
	void setParameter2(uint32_t parameter2);

	/**
	 * Set the command to CMD_NONE and try to find
	 * the correct class to handle a more detailed
	 * clear.
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
	void setToUnknownCommand(Command_t initialCommand);
	void setReplyRejected(ReturnValue_t reason, Command_t initialCommand = CMD_NONE);
	size_t getMinimumMessageSize() const;
};


#endif /* COMMANDMESSAGE_H_ */
