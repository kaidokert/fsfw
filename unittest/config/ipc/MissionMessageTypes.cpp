#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/unittest/config/ipc/MissionMessageTypes.h>

void messagetypes::clearMissionMessage(CommandMessage* message) {
	switch((message->getCommand()>>8) & 0xff) {
	default:
		break;
	}
}


