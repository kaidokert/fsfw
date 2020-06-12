#ifndef FRAMEWORK_IPC_COMMANDMESSAGEIF_H_
#define FRAMEWORK_IPC_COMMANDMESSAGEIF_H_

#include <framework/ipc/MessageQueueMessageIF.h>

#define MAKE_COMMAND_ID( number )	((MESSAGE_ID << 8) + (number))
typedef uint16_t Command_t;

class CommandMessageIF: public MessageQueueMessageIF {
public:
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
};

#endif /* FRAMEWORK_IPC_COMMANDMESSAGEIF_H_ */
