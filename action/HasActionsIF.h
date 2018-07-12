#ifndef HASACTIONSIF_H_
#define HASACTIONSIF_H_

#include <framework/action/ActionHelper.h>
#include <framework/action/ActionMessage.h>
#include <framework/action/SimpleActionHelper.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/ipc/MessageQueueIF.h>
class HasActionsIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::HAS_ACTIONS_IF;
	static const ReturnValue_t IS_BUSY = MAKE_RETURN_CODE(1);
	static const ReturnValue_t INVALID_PARAMETERS = MAKE_RETURN_CODE(2);
	static const ReturnValue_t EXECUTION_FINISHED = MAKE_RETURN_CODE(3);
	static const ReturnValue_t INVALID_ACTION_ID = MAKE_RETURN_CODE(4);
	virtual ~HasActionsIF() { }
	virtual MessageQueueId_t getCommandQueue() const = 0;
	/**
	 * Execute or initialize the execution of a certain function.
	 * Returning #EXECUTION_FINISHED or a failure code, nothing else needs to be done.
	 * When needing more steps, return RETURN_OK and issue steps and completion manually. One "step failed" or completion report must
	 * be issued!
	 */
	virtual ReturnValue_t executeAction(ActionId_t actionId, MessageQueueId_t commandedBy, const uint8_t* data, uint32_t size) = 0;
};


#endif /* HASACTIONSIF_H_ */
