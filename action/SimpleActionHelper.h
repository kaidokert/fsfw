#ifndef FSFW_ACTION_SIMPLEACTIONHELPER_H_
#define FSFW_ACTION_SIMPLEACTIONHELPER_H_

#include "ActionHelper.h"

/**
 * @brief	This is an action helper which is only able to service one action
 * 			at a time but remembers last commander and last action which
 * 			simplifies usage
 */
class SimpleActionHelper: public ActionHelper {
public:
	SimpleActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue);
	virtual ~SimpleActionHelper();
	void step(ReturnValue_t result = HasReturnvaluesIF::RETURN_OK);
	void finish(ReturnValue_t result = HasReturnvaluesIF::RETURN_OK);
	ReturnValue_t reportData(SerializeIF* data);

protected:
	void prepareExecution(MessageQueueId_t commandedBy, ActionId_t actionId,
			store_address_t dataAddress);
	virtual void resetHelper();
private:
	bool isExecuting;
	MessageQueueId_t lastCommander = MessageQueueIF::NO_QUEUE;
	ActionId_t lastAction = 0;
	uint8_t stepCount = 0;
};

#endif /* SIMPLEACTIONHELPER_H_ */
