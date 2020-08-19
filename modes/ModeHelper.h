#ifndef MODEHELPER_H_
#define MODEHELPER_H_

#include "ModeMessage.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../timemanager/Countdown.h"

class HasModesIF;

class ModeHelper {
public:
	MessageQueueId_t theOneWhoCommandedAMode;
	Mode_t commandedMode;
	Submode_t commandedSubmode;

	ModeHelper(HasModesIF *owner);
	virtual ~ModeHelper();

	ReturnValue_t handleModeCommand(CommandMessage *message);

	/**
	 *
	 * @param parentQueue the Queue id of the parent object. Set to 0 if no parent present
	 */
	void setParentQueue(MessageQueueId_t parentQueueId);

	ReturnValue_t initialize(MessageQueueId_t parentQueueId);

	ReturnValue_t initialize(void); //void is there to stop eclipse CODAN from falsely reporting an error

	void modeChanged(Mode_t mode, Submode_t submode);

	void startTimer(uint32_t timeoutMs);

	bool isTimedOut();

	bool isForced();

	void setForced(bool forced);
protected:
	HasModesIF *owner;
	MessageQueueId_t parentQueueId;

	Countdown countdown;

	bool forced;
};

#endif /* MODEHELPER_H_ */
