#ifndef SIMPLEACTIONHELPER_H_
#define SIMPLEACTIONHELPER_H_

#include "ActionHelper.h"

class SimpleActionHelper: public ActionHelper {
public:
	SimpleActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue);
	virtual ~SimpleActionHelper();
	void step(ReturnValue_t result = HasReturnvaluesIF::RETURN_OK);
	void finish(ReturnValue_t result = HasReturnvaluesIF::RETURN_OK);
	ReturnValue_t reportData(SerializeIF* data);

protected:
	void prepareExecution(MessageQueueId_t commandedBy, ActionId_t actionId, store_address_t dataAddress);
	virtual  void resetHelper();
private:
	bool isExecuting;
	MessageQueueId_t lastCommander;
	ActionId_t lastAction;
	uint8_t stepCount;
};

#endif /* SIMPLEACTIONHELPER_H_ */
