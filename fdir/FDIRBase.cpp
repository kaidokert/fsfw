/*
 * FDIRBase.cpp
 *
 *  Created on: 09.09.2015
 *      Author: baetz
 */

#include <framework/events/EventManagerIF.h>
#include <framework/fdir/FDIRBase.h>
#include <framework/health/HasHealthIF.h>
#include <framework/health/HealthMessage.h>
#include <framework/objectmanager/ObjectManagerIF.h>
FDIRBase::FDIRBase(object_id_t owner, object_id_t parent, uint8_t messageDepth, uint8_t parameterDomainBase) :
		eventQueue(messageDepth, EventMessage::EVENT_MESSAGE_SIZE), ownerId(
				owner), owner(NULL), faultTreeParent(parent), parameterDomainBase(parameterDomainBase) {
}

FDIRBase::~FDIRBase() {
}

ReturnValue_t FDIRBase::initialize() {
	EventManagerIF* manager = objectManager->get<EventManagerIF>(
			objects::EVENT_MANAGER);
	if (manager == NULL) {
		return RETURN_FAILED;
	}
	ReturnValue_t result = manager->registerListener(eventQueue.getId());
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	if (ownerId != 0) {
		result = manager->subscribeToAllEventsFrom(eventQueue.getId(), ownerId);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		owner = objectManager->get<HasHealthIF>(ownerId);
		if (owner == NULL) {
			return RETURN_FAILED;
		}
	}
	if (faultTreeParent != 0) {
		ConfirmsFailuresIF* parentIF = objectManager->get<ConfirmsFailuresIF>(
				faultTreeParent);
		if (parentIF == NULL) {
			return RETURN_FAILED;
		}
		eventQueue.setDefaultDestination(parentIF->getEventReceptionQueue());
	}
	return RETURN_OK;
}

void FDIRBase::checkForFailures() {
	EventMessage event;
	for (ReturnValue_t result = eventQueue.receiveMessage(&event);
			result == RETURN_OK; result = eventQueue.receiveMessage(&event)) {
		if (event.getSender() == eventQueue.getId()) {
			//We already got this event, because we sent it.
			continue;
		}
		switch (event.getMessageId()) {
		case EventMessage::EVENT_MESSAGE:
			if (isFdirDisabledForSeverity(event.getSeverity())) {
				//We do not handle events when disabled.
				continue;
			}
			eventReceived(&event);
			break;
		case EventMessage::CONFIRMATION_REQUEST:
			doConfirmFault(&event);
			break;
		case EventMessage::YOUR_FAULT:
			eventConfirmed(&event);
			break;
		case EventMessage::MY_FAULT:
			wasParentsFault(&event);
			break;
		default:
			break;
		}
	}
	decrementFaultCounters();
}

void FDIRBase::setOwnerHealth(HasHealthIF::HealthState health) {
	if (owner != NULL) {
		owner->setHealth(health);
	}
	//else: owner has no health.

}

MessageQueueId_t FDIRBase::getEventReceptionQueue() {
	return eventQueue.getId();
}

ReturnValue_t FDIRBase::sendConfirmationRequest(EventMessage* event,
		MessageQueueId_t destination) {
	event->setMessageId(EventMessage::CONFIRMATION_REQUEST);
	if (destination != 0) {
		return eventQueue.sendMessage(destination, event);
	} else if (faultTreeParent != 0) {
		return eventQueue.sendToDefault(event);
	}
	return RETURN_FAILED;
}

void FDIRBase::eventConfirmed(EventMessage* event) {
}

void FDIRBase::wasParentsFault(EventMessage* event) {
}

void FDIRBase::doConfirmFault(EventMessage* event) {
	ReturnValue_t result = confirmFault(event);
	if (result == YOUR_FAULT) {
		event->setMessageId(EventMessage::YOUR_FAULT);
		eventQueue.reply(event);
	} else if (result == MY_FAULT) {
		event->setMessageId(EventMessage::MY_FAULT);
		eventQueue.reply(event);
	} else {

	}
}

ReturnValue_t FDIRBase::confirmFault(EventMessage* event) {
	return YOUR_FAULT;
}

void FDIRBase::triggerEvent(Event event, uint32_t parameter1,
		uint32_t parameter2) {
	//With this mechanism, all events are disabled for a certain device.
	//That's not so good for visibility.
	if (isFdirDisabledForSeverity(EVENT::getSeverity(event))) {
		return;
	}
	EventMessage message(event, ownerId, parameter1, parameter2);
	EventManagerIF::triggerEvent(&message, eventQueue.getId());
	eventReceived(&message);
}

bool FDIRBase::isFdirDisabledForSeverity(EventSeverity_t severity) {
	if ((owner != NULL) && (severity != SEVERITY::INFO)) {
		if (owner->getHealth() == HasHealthIF::EXTERNAL_CONTROL) {
			//External control disables handling of fault messages.
			return true;
		}
	}
	return false;
}

void FDIRBase::throwFdirEvent(Event event, uint32_t parameter1,
		uint32_t parameter2) {
	EventMessage message(event, ownerId, parameter1, parameter2);
	EventManagerIF::triggerEvent(&message, eventQueue.getId());
}
