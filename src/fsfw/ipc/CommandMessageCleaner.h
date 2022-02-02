#ifndef FRAMEWORK_IPC_COMMANDMESSAGECLEANER_H_
#define FRAMEWORK_IPC_COMMANDMESSAGECLEANER_H_
#include "../ipc/CommandMessage.h"

namespace messagetypes {
// Implemented in config.
void clearMissionMessage(CommandMessage* message);
}  // namespace messagetypes

class CommandMessageCleaner {
 public:
  static void clearCommandMessage(CommandMessage* message);
};

#endif /* FRAMEWORK_IPC_COMMANDMESSAGECLEANER_H_ */
