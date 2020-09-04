#include "../ipc/CommandMessageCleaner.h"

#include "../devicehandlers/DeviceHandlerMessage.h"
#include "../health/HealthMessage.h"
#include "../memory/MemoryMessage.h"
#include "../modes/ModeMessage.h"
#include "../monitoring/MonitoringMessage.h"
#include "../subsystem/modes/ModeSequenceMessage.h"
#include "../tmstorage/TmStoreMessage.h"
#include "../parameters/ParameterMessage.h"

void CommandMessageCleaner::clearCommandMessage(CommandMessage* message) {
	switch(message->getMessageType()){
	case  messagetypes::MODE_COMMAND:
		ModeMessage::clear(message);
		break;
	case messagetypes::HEALTH_COMMAND:
		HealthMessage::clear(message);
		break;
	case messagetypes::MODE_SEQUENCE:
		ModeSequenceMessage::clear(message);
		break;
	case messagetypes::ACTION:
		ActionMessage::clear(message);
		break;
	case messagetypes::DEVICE_HANDLER_COMMAND:
		DeviceHandlerMessage::clear(message);
		break;
	case messagetypes::MEMORY:
		MemoryMessage::clear(message);
		break;
	case messagetypes::MONITORING:
		MonitoringMessage::clear(message);
		break;
	case messagetypes::TM_STORE:
		TmStoreMessage::clear(message);
		break;
	case messagetypes::PARAMETER:
		ParameterMessage::clear(message);
		break;
	default:
		messagetypes::clearMissionMessage(message);
		break;
	}
}
