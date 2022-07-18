#include "MissionMessageTypes.h"

#include <fsfw/ipc/CommandMessage.h>

void messagetypes::clearMissionMessage(CommandMessage* message) {
  switch (message->getMessageType()) {
    default:
      break;
  }
}
