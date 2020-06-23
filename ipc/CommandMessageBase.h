#ifndef FRAMEWORK_IPC_COMMANDMESSAGEBASE_H_
#define FRAMEWORK_IPC_COMMANDMESSAGEBASE_H_
#include <framework/ipc/CommandMessageIF.h>
#include <framework/ipc/MessageQueueMessage.h>

/**
 * @brief	Base implementation of a generic command message, which has
 * 			a Command_t ID and message type ID in the header in addition
 * 			to the sender message queue ID.
 * @details
 * This is the base implementation serves as a base for other command messages
 * and which implements most functions required for MessageQueueMessageIF.
 * The only functions which have to be supplied by a specific command message
 * impelementations are the size related functions which are used for
 * size checks:
 *
 *  1. getMinimumMessageSize()
 *
 * The maximum message size generally depends on the buffer size of the passed
 * internal message.
 * Don't forget to set the message size of the passed message in the concrete
 * commandmessage implementation!
 */
class CommandMessageBase: public CommandMessageIF {
public:
	//! This minimum size is derived from the interface requirement to be able
	//! to set a rejected reply, which contains a returnvalue and the initial
	//! command.
	static constexpr size_t MINIMUM_COMMAND_MESSAGE_BASE_SIZE =
			CommandMessageIF::HEADER_SIZE + sizeof(ReturnValue_t) +
			sizeof(Command_t);

	CommandMessageBase(MessageQueueMessageIF* message);

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

	/**
	 * Extract message ID, which is the first byte of the command ID for the
	 * default implementation.
	 * @return
	 */
	virtual uint8_t getMessageType() const override;

	/*
	 * MessageQueueMessageIF functions, which generally just call the
	 * respective functions of the internal message queue message.
	 */
	virtual uint8_t * getBuffer() override;
	virtual const uint8_t * getBuffer() const override;
	virtual void setSender(MessageQueueId_t setId) override;
	virtual MessageQueueId_t getSender() const override;
	virtual uint8_t * getData() override;
	virtual const uint8_t* getData() const override;
	virtual size_t getMessageSize() const override;

	/**
	 * A command message can be rejected and needs to offer a function
	 * to set a rejected reply
	 * @param reason
	 * @param initialCommand
	 */
	void setReplyRejected(ReturnValue_t reason,
			Command_t initialCommand) override;
	/**
	 * Corrensonding getter function.
	 * @param initialCommand
	 * @return
	 */
	ReturnValue_t getReplyRejectedReason(
			Command_t* initialCommand = nullptr) const override;

	virtual MessageQueueMessageIF* getInternalMessage() const override;

	virtual void clear() override;
protected:
	/**
	 * @brief 	Pointer to the message containing the data.
	 * @details
	 * The command message does not actually own the memory containing a
	 * message, it just oprates on it via a pointer to a message queue message.
	 */
	MessageQueueMessageIF* internalMessage = nullptr;
};



#endif /* FRAMEWORK_IPC_COMMANDMESSAGEBASE_H_ */
