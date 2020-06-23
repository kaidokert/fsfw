#include <framework/ipc/CommandMessageCleaner.h>

#include <framework/devicehandlers/DeviceHandlerMessage.h>
#include <framework/health/HealthMessage.h>
#include <framework/memory/MemoryMessage.h>
#include <framework/modes/ModeMessage.h>
#include <framework/monitoring/MonitoringMessage.h>
#include <framework/subsystem/modes/ModeSequenceMessage.h>
#include <framework/tmstorage/TmStoreMessage.h>
#include <framework/parameters/ParameterMessage.h>

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
