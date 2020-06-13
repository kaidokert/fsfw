#include <framework/ipc/CommandMessageCleaner.h>

#include <framework/devicehandlers/DeviceHandlerMessage.h>
#include <framework/health/HealthMessage.h>
#include <framework/memory/MemoryMessage.h>
#include <framework/modes/ModeMessage.h>
#include <framework/monitoring/MonitoringMessage.h>
#include <framework/subsystem/modes/ModeSequenceMessage.h>
#include <framework/tmstorage/TmStoreMessage.h>
#include <framework/parameters/ParameterMessage.h>

void CommandMessageCleaner::clearCommandMessage(CommandMessageIF* message) {
	switch(message->getMessageType()){
	case  messagetypes::MODE_COMMAND:
		ModeMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::HEALTH_COMMAND:
		HealthMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::MODE_SEQUENCE:
		ModeSequenceMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::ACTION:
		ActionMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::DEVICE_HANDLER_COMMAND:
		DeviceHandlerMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::MEMORY:
		MemoryMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::MONITORING:
		MonitoringMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::TM_STORE:
		TmStoreMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	case messagetypes::PARAMETER:
		ParameterMessage::clear(dynamic_cast<CommandMessage*>(message));
		break;
	default:
		messagetypes::clearMissionMessage(message);
		break;
	}
}
