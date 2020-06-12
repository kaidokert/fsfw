#include <framework/housekeeping/HousekeepingMessage.h>

HousekeepingMessage::HousekeepingMessage(MessageQueueMessage *message):
		CommandMessageBase	(message) {}

HousekeepingMessage::~HousekeepingMessage() {}

