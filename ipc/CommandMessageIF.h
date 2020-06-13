#ifndef FRAMEWORK_IPC_COMMANDMESSAGEIF_H_
#define FRAMEWORK_IPC_COMMANDMESSAGEIF_H_

#include <framework/ipc/MessageQueueMessageIF.h>
#include <framework/ipc/FwMessageTypes.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>


#define MAKE_COMMAND_ID( number )	((MESSAGE_ID << 8) + (number))
typedef uint16_t Command_t;

// TODO: actually, this interface propably does not have to implement
// MQM IF, because there is a getter function for the internal message..
// But it is also convenient to have the full access to all MQM IF functions.
// That way, I can just pass CommandMessages to functions expecting a MQM IF.
// The command message implementations just forwards the calls. Maybe
// we should just leave it like that.
class CommandMessageIF: public MessageQueueMessageIF {
public:
	static constexpr size_t HEADER_SIZE = sizeof(MessageQueueId_t) +
			sizeof(Command_t);

	static const uint8_t INTERFACE_ID = CLASS_ID::COMMAND_MESSAGE;
	static const ReturnValue_t UNKNOWN_COMMAND = MAKE_RETURN_CODE(0x01);

	static const uint8_t MESSAGE_ID = messagetypes::COMMAND;
	//! Used internally, shall be ignored
	static const Command_t CMD_NONE = MAKE_COMMAND_ID( 0 );
	static const Command_t REPLY_COMMAND_OK = MAKE_COMMAND_ID( 1 );
	//! Reply indicating that the current command was rejected,
	//! par1 should contain the error code
	static const Command_t REPLY_REJECTED = MAKE_COMMAND_ID( 2 );

	virtual ~CommandMessageIF() {};

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
	virtual void setReplyRejected(ReturnValue_t reason,
			Command_t initialCommand) = 0;
	/**
	 * Corrensonding getter function.
	 * @param initialCommand
	 * @return
	 */
	virtual ReturnValue_t getReplyRejectedReason(
			Command_t* initialCommand = nullptr) const  = 0;

	/**
	 * This function is used to get a pointer to the internal message, as
	 * the command message implementations always operate on the memory
	 * contained in the message queue message implementation.
	 * This pointer can be used to set the internal message of different
	 * command message implementations.
	 * @return
	 */
	virtual MessageQueueMessageIF* getInternalMessage() const = 0;

};

#endif /* FRAMEWORK_IPC_COMMANDMESSAGEIF_H_ */
