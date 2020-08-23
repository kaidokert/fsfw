#ifndef FRAMEWORK_PARAMETERS_PARAMETERHELPER_H_
#define FRAMEWORK_PARAMETERS_PARAMETERHELPER_H_

#include "../ipc/MessageQueueIF.h"
#include "../parameters/ParameterMessage.h"
#include "../parameters/ReceivesParameterMessagesIF.h"

/**
 * @brief	Helper class to handle parameter messages
 * @details
 * This class simplfiies handling of parameter messages, which are sent
 * to a class which implements ReceivesParameterMessagesIF.
 */
class ParameterHelper {
public:
	ParameterHelper(ReceivesParameterMessagesIF *owner);
	virtual ~ParameterHelper();

	ReturnValue_t handleParameterMessage(CommandMessage *message);

	ReturnValue_t initialize();
private:
	ReceivesParameterMessagesIF *owner;

	MessageQueueId_t ownerQueueId = MessageQueueIF::NO_QUEUE;

	StorageManagerIF *storage = nullptr;

	ReturnValue_t sendParameter(MessageQueueId_t to, uint32_t id,
			const ParameterWrapper *description);

	void rejectCommand(MessageQueueId_t to, ReturnValue_t reason,
			Command_t initialCommand);
};

#endif /* PARAMETERHELPER_H_ */
