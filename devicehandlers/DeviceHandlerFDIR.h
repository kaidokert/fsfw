/*
 * DeviceHandlerFDIR.h
 *
 *  Created on: 09.09.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_DEVICEHANDLERS_DEVICEHANDLERFDIR_H_
#define FRAMEWORK_DEVICEHANDLERS_DEVICEHANDLERFDIR_H_

#include <framework/fdir/FaultCounter.h>
#include <framework/fdir/FDIRBase.h>

class DeviceHandlerFDIR: public FDIRBase {
public:
	DeviceHandlerFDIR(object_id_t owner, object_id_t parent = objects::IO_ASSEMBLY);
	~DeviceHandlerFDIR();
	virtual ReturnValue_t eventReceived(EventMessage* event);
	void eventConfirmed(EventMessage* event);
	void wasParentsFault(EventMessage* event);
	void decrementFaultCounters();
	ReturnValue_t initialize();
	void triggerEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
	bool isFdirActionInProgress();
	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);
protected:
	FaultCounter strangeReplyCount;
	FaultCounter missedReplyCount;
	FaultCounter recoveryCounter;
	enum FDIRState {
		NONE,
		RECOVERY_ONGOING,
		DEVICE_MIGHT_BE_OFF,
		AWAIT_SHUTDOWN
	};
	FDIRState fdirState;
	MessageQueueId_t powerConfirmation;
	//TODO: Arbitrary numbers! Adjust!
	static const uint32_t MAX_REBOOT = 1;
	static const uint32_t REBOOT_TIME_MS = 180000;
	static const uint32_t MAX_STRANGE_REPLIES = 10;
	static const uint32_t STRANGE_REPLIES_TIME_MS = 10000;
	static const uint32_t MAX_MISSED_REPLY_COUNT = 5;
	static const uint32_t MISSED_REPLY_TIME_MS = 10000;
	void handleRecovery(Event reason);
	virtual void clearFaultCounters();
	void setFdirState(FDIRState state);
	void startRecovery(Event reason);
	void setFaulty(Event reason);
};

#endif /* FRAMEWORK_DEVICEHANDLERS_DEVICEHANDLERFDIR_H_ */
