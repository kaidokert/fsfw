/*
 * FDIRBase.h
 *
 *  Created on: 09.09.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_FDIR_FDIRBASE_H_
#define FRAMEWORK_FDIR_FDIRBASE_H_

#include <framework/events/EventMessage.h>
#include <framework/fdir/ConfirmsFailuresIF.h>
#include <framework/fdir/FaultCounter.h>
#include <framework/health/HealthMessage.h>
#include <framework/ipc/MessageQueue.h>
#include <framework/parameters/HasParametersIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
class FDIRBase: public HasReturnvaluesIF, public ConfirmsFailuresIF, public HasParametersIF {
public:
	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::FDIR_1;
	static const Event FDIR_CHANGED_STATE = MAKE_EVENT(1, SEVERITY::INFO); //!< FDIR has an internal state, which changed from par2 (oldState) to par1 (newState).
	static const Event FDIR_STARTS_RECOVERY = MAKE_EVENT(2, SEVERITY::MEDIUM); //!< FDIR tries to restart device. Par1: event that caused recovery.
	static const Event FDIR_TURNS_OFF_DEVICE = MAKE_EVENT(3, SEVERITY::MEDIUM); //!< FDIR turns off device. Par1: event that caused recovery.
	FDIRBase(object_id_t owner, object_id_t parent = 0, uint8_t messageDepth = 10, uint8_t parameterDomainBase = 0xF0);
	virtual ~FDIRBase();
	virtual ReturnValue_t initialize();
	void checkForFailures();
	MessageQueueId_t getEventReceptionQueue();
	virtual void triggerEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
protected:
	MessageQueue eventQueue;
	object_id_t ownerId;
	HasHealthIF* owner;
	object_id_t faultTreeParent;
	uint8_t parameterDomainBase;
	void setOwnerHealth(HasHealthIF::HealthState health);
	virtual ReturnValue_t eventReceived(EventMessage* event) = 0;
	virtual void eventConfirmed(EventMessage* event);
	virtual void wasParentsFault(EventMessage* event);
	virtual ReturnValue_t confirmFault(EventMessage* event);
	virtual void decrementFaultCounters() = 0;
	ReturnValue_t sendConfirmationRequest(EventMessage* event, MessageQueueId_t destination = 0);
	void throwFdirEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
private:
	void doConfirmFault(EventMessage* event);
	bool isFdirDisabledForSeverity(EventSeverity_t severity);
};

#endif /* FRAMEWORK_FDIR_FDIRBASE_H_ */
