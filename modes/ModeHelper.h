#ifndef MODEHELPER_H_
#define MODEHELPER_H_

#include <framework/modes/ModeMessage.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/timemanager/Countdown.h>

class HasModesIF;

class ModeHelper {
public:
	MessageQueueId_t theOneWhoCommandedAMode;
	Mode_t commandedMode;
	Submode_t commandedSubmode;

	ModeHelper(HasModesIF *owner);
	virtual ~ModeHelper();

	/**
	 * This is used by DHB to handle all mode messages issued by a service
	 * @param message
	 * @return
	 */
	ReturnValue_t handleModeCommand(CommandMessage *message);

	/**
	 *
	 * @param parentQueue the Queue id of the parent object (assembly or subsystem object).
	 *        Set to 0 if no parent present
	 */
	void setParentQueue(MessageQueueId_t parentQueueId);

	ReturnValue_t initialize(MessageQueueId_t parentQueueId);

	ReturnValue_t initialize(void); //void is there to stop eclipse CODAN from falsely reporting an error

	/**
	 * Used to notify
	 * @param mode
	 * @param submode
	 */
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
