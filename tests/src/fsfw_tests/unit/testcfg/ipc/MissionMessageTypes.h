#ifndef CONFIG_IPC_MISSIONMESSAGETYPES_H_
#define CONFIG_IPC_MISSIONMESSAGETYPES_H_

#include <fsfw/ipc/FwMessageTypes.h>

class CommandMessage;

/**
 * Custom command messages are specified here.
 * Most messages needed to use FSFW are already located in
 * <fsfw/ipc/FwMessageTypes.h>
 * @param message Generic Command Message
 */
namespace messagetypes {
enum MESSAGE_TYPE {
  MISSION_MESSAGE_TYPE_START = FW_MESSAGES_COUNT,
};

void clearMissionMessage(CommandMessage* message);
}  // namespace messagetypes

#endif /* CONFIG_IPC_MISSIONMESSAGETYPES_H_ */
