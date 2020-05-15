#ifndef FRAMEWORK_IPC_FWMESSAGETYPES_H_
#define FRAMEWORK_IPC_FWMESSAGETYPES_H_

namespace MESSAGE_TYPE {
//Remember to add new Message Types to the clearCommandMessage function!
enum FW_MESSAGE_TYPE {
	COMMAND = 0,
	MODE_COMMAND,
	HEALTH_COMMAND,
	MODE_SEQUENCE,
	ACTION,
	TM_STORE,
	DEVICE_HANDLER_COMMAND,
	MONITORING,
	MEMORY,
	PARAMETER,
	FW_MESSAGES_COUNT,
	FILE_SYSTEM_MESSAGE
};
}

#endif /* FRAMEWORK_IPC_FWMESSAGETYPES_H_ */
