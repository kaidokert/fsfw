#ifndef FRAMEWORK_DATAPOOL_HKSWITCHHELPER_H_
#define FRAMEWORK_DATAPOOL_HKSWITCHHELPER_H_

#include "../tasks/ExecutableObjectIF.h"
#include "../action/CommandsActionsIF.h"
#include "../events/EventReportingProxyIF.h"

//TODO this class violations separation between mission and framework
//but it is only a transitional solution until the Datapool is
//implemented decentrally

class HkSwitchHelper: public ExecutableObjectIF, public CommandsActionsIF {
public:

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::HK;
	static const Event SWITCHING_TM_FAILED = MAKE_EVENT(1, SEVERITY::LOW); //!< Commanding the HK Service failed, p1: error code, p2 action: 0 disable / 1 enable

	HkSwitchHelper(EventReportingProxyIF *eventProxy);
	virtual ~HkSwitchHelper();

	ReturnValue_t initialize();

	virtual ReturnValue_t  performOperation(uint8_t operationCode = 0);

	ReturnValue_t switchHK(SerializeIF *sids, bool enable);

	virtual void setTaskIF(PeriodicTaskIF* task_){};

protected:
	virtual void stepSuccessfulReceived(ActionId_t actionId, uint8_t step);
	virtual void stepFailedReceived(ActionId_t actionId, uint8_t step,
			ReturnValue_t returnCode);
	virtual void dataReceived(ActionId_t actionId, const uint8_t* data,
			uint32_t size);
	virtual void completionSuccessfulReceived(ActionId_t actionId);
	virtual void completionFailedReceived(ActionId_t actionId,
			ReturnValue_t returnCode);
	virtual MessageQueueIF* getCommandQueuePtr();

private:
	CommandActionHelper commandActionHelper;
	MessageQueueIF* actionQueue;
	EventReportingProxyIF *eventProxy;
};

#endif /* FRAMEWORK_DATAPOOL_HKSWITCHHELPER_H_ */
