#include "fsfw/ipc/CommandMessageCleaner.h"

#include "fsfw/devicehandlers/DeviceHandlerMessage.h"
#include "fsfw/filesystem/GenericFileSystemMessage.h"
#include "fsfw/health/HealthMessage.h"
#include "fsfw/housekeeping/HousekeepingMessage.h"
#include "fsfw/memory/MemoryMessage.h"
#include "fsfw/modes/ModeMessage.h"
#include "fsfw/monitoring/MonitoringMessage.h"
#include "fsfw/parameters/ParameterMessage.h"
#include "fsfw/subsystem/modes/ModeSequenceMessage.h"
#ifdef FSFW_ADD_TMSTORAGE
#include "fsfw/tmstorage/TmStoreMessage.h"
#endif

void CommandMessageCleaner::clearCommandMessage(CommandMessage* message) {
  switch (message->getMessageType()) {
    case messagetypes::MODE_COMMAND:
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
#ifdef FSFW_ADD_TMSTORAGE
    case messagetypes::TM_STORE:
      TmStoreMessage::clear(message);
      break;
#endif
    case messagetypes::PARAMETER:
      ParameterMessage::clear(message);
      break;
    case messagetypes::HOUSEKEEPING:
      HousekeepingMessage::clear(message);
      break;
    case messagetypes::FILE_SYSTEM_MESSAGE:
      GenericFileSystemMessage::clear(message);
      break;
    default:
      messagetypes::clearMissionMessage(message);
      break;
  }
}
