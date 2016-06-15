/*
 * ParameterHelper.h
 *
 *  Created on: 28.11.2015
 *      Author: mohr
 */

#ifndef PARAMETERHELPER_H_
#define PARAMETERHELPER_H_

#include <framework/parameters/ParameterMessage.h>
#include <framework/parameters/ReceivesParameterMessagesIF.h>

class ParameterHelper {
public:
	ParameterHelper(ReceivesParameterMessagesIF *owner);
	virtual ~ParameterHelper();

	ReturnValue_t handleParameterMessage(CommandMessage *message);

	ReturnValue_t initialize();
private:
	ReceivesParameterMessagesIF *owner;

	MessageQueueId_t ownerQueueId;

	StorageManagerIF *storage;

	ReturnValue_t sendParameter(MessageQueueId_t to, uint32_t id, const ParameterWrapper *description);

	void rejectCommand(MessageQueueId_t to, ReturnValue_t reason, Command_t initialCommand);
};

#endif /* PARAMETERHELPER_H_ */
