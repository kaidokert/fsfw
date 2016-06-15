#ifndef WATCHDOGMESSAGE_H_
#define WATCHDOGMESSAGE_H_

#include <framework/ipc/MessageQueueMessage.h>
#include <framework/objectmanager/SystemObjectIF.h>

class WatchdogMessage: public MessageQueueMessage {
public:
	/**
	 * Commands that can be sent to the watchdog
	 */
	enum WatchdogCommand_t{
		ENABLE, //!< Enables the Watchdog (it is enabled by default)
		DISABLE,//!< Disables the watchdog
		RESET_CPU   //!< Causes a reset of the Processor
	};

	static const uint8_t WATCHDOG_MESSAGE_SIZE = HEADER_SIZE + sizeof(object_id_t) + sizeof(WatchdogCommand_t);

	WatchdogMessage();
	WatchdogMessage(object_id_t sender, WatchdogCommand_t command);
	virtual ~WatchdogMessage();
	WatchdogCommand_t getCommand();
	void setCommand(WatchdogCommand_t command);
	object_id_t getSender();
	void setSender(object_id_t sender);

protected:
	virtual size_t getMinimumMessageSize();
};

#endif /* WATCHDOGMESSAGE_H_ */
