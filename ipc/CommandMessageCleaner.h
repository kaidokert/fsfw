#ifndef FRAMEWORK_IPC_COMMANDMESSAGECLEANER_H_
#define FRAMEWORK_IPC_COMMANDMESSAGECLEANER_H_
#include <framework/ipc/CommandMessageIF.h>

namespace messagetypes {
// Implemented in config.
void clearMissionMessage(CommandMessageIF* message);
}

class CommandMessageCleaner {
public:
	static void clearCommandMessage(CommandMessageIF* message);
};


#endif /* FRAMEWORK_IPC_COMMANDMESSAGECLEANER_H_ */
