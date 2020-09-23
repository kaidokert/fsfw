#ifndef FSFW_HEALTH_HEALTHMESSAGE_H_
#define FSFW_HEALTH_HEALTHMESSAGE_H_

#include "HasHealthIF.h"
#include "../ipc/CommandMessage.h"

class HealthMessage {
public:
	static const uint8_t MESSAGE_ID = messagetypes::HEALTH_COMMAND;
	// REPLY_COMMAND_OK/REPLY_REJECTED
	static const Command_t HEALTH_SET = MAKE_COMMAND_ID(1);
	// NO REPLY!
	static const Command_t HEALTH_ANNOUNCE = MAKE_COMMAND_ID(3);
	static const Command_t HEALTH_INFO = MAKE_COMMAND_ID(5);
	static const Command_t REPLY_HEALTH_SET = MAKE_COMMAND_ID(6);

	static void setHealthMessage(CommandMessage *message, Command_t command,
			HasHealthIF::HealthState health,
			HasHealthIF::HealthState oldHealth = HasHealthIF::FAULTY);

	static void setHealthMessage(CommandMessage *message, Command_t command);

	static HasHealthIF::HealthState getHealth(const CommandMessage *message);

	static HasHealthIF::HealthState getOldHealth(const CommandMessage *message);

	static void clear(CommandMessage *message);

private:
	HealthMessage();
};

#endif /* FSFW_HEALTH_HEALTHMESSAGE_H_ */
