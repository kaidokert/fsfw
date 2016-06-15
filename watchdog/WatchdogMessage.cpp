#include <framework/watchdog/WatchdogMessage.h>
#include <cstring>

WatchdogMessage::WatchdogMessage() {
	messageSize = WATCHDOG_MESSAGE_SIZE;
}

WatchdogMessage::WatchdogMessage(object_id_t sender,
		WatchdogCommand_t command) {
	messageSize = WATCHDOG_MESSAGE_SIZE;
	setSender(sender);
	setCommand(command);
}

WatchdogMessage::WatchdogCommand_t WatchdogMessage::getCommand() {
	WatchdogCommand_t command;
	memcpy(&command, getData() + sizeof(object_id_t),
			sizeof(WatchdogCommand_t));
	return command;
}

void WatchdogMessage::setCommand(WatchdogCommand_t command) {
	memcpy(getData() + sizeof(object_id_t), &command,
			sizeof(WatchdogCommand_t));
}

object_id_t WatchdogMessage::getSender() {
	object_id_t sender;
	memcpy(&sender, getData(), sizeof(object_id_t));
	return sender;
}

void WatchdogMessage::setSender(object_id_t sender) {
	memcpy(getData(), &sender, sizeof(object_id_t));
}

WatchdogMessage::~WatchdogMessage() {
	
}

size_t WatchdogMessage::getMinimumMessageSize() {
	return WATCHDOG_MESSAGE_SIZE;
}
