#include <config/objects/translateObjects.h>
#include <config/events/translateEvents.h>

#include <framework/events/EventManager.h>
#include <framework/events/EventMessage.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

const uint16_t EventManager::POOL_SIZES[N_POOLS] = {
		sizeof(EventMatchTree::Node), sizeof(EventIdRangeMatcher),
		sizeof(ReporterRangeMatcher) };
//TODO: Rather arbitrary. Adjust!
const uint16_t EventManager::N_ELEMENTS[N_POOLS] = { 240, 120, 120 };

EventManager::EventManager(object_id_t setObjectId) :
		SystemObject(setObjectId), eventReportQueue(MAX_EVENTS_PER_CYCLE,
				EventMessage::EVENT_MESSAGE_SIZE), mutex(NULL), factoryBackend(
				0, POOL_SIZES, N_ELEMENTS, false, true) {
	mutex = new MutexId_t;
	OSAL::createMutex(setObjectId + 1, (mutex));
}

EventManager::~EventManager() {
	OSAL::deleteMutex(mutex);
	delete mutex;
}

MessageQueueId_t EventManager::getEventReportQueue() {
	return eventReportQueue.getId();
}

ReturnValue_t EventManager::performOperation() {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	while (result == HasReturnvaluesIF::RETURN_OK) {
		EventMessage message;
		result = eventReportQueue.receiveMessage(&message);
		if (result == HasReturnvaluesIF::RETURN_OK) {
			printEvent(&message);
			notifyListeners(&message);
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void EventManager::notifyListeners(EventMessage* message) {
	lockMutex();
	for (auto iter = listenerList.begin(); iter != listenerList.end(); ++iter) {
		if (iter->second.match(message)) {
			eventForwardingSender.sendMessage(iter->first, message,
					message->getSender());
		}
	}
	unlockMutex();
}

ReturnValue_t EventManager::registerListener(MessageQueueId_t listener,
bool forwardAllButSelected) {
	auto result = listenerList.insert(
			std::pair<MessageQueueId_t, EventMatchTree>(listener,
					EventMatchTree(&factoryBackend, forwardAllButSelected)));
	if (!result.second) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t EventManager::subscribeToEvent(MessageQueueId_t listener,
		EventId_t event) {
	return subscribeToEventRange(listener, event);
}

ReturnValue_t EventManager::subscribeToAllEventsFrom(MessageQueueId_t listener,
		object_id_t object) {
	return subscribeToEventRange(listener, 0, 0, true, object);
}

ReturnValue_t EventManager::subscribeToEventRange(MessageQueueId_t listener,
		EventId_t idFrom, EventId_t idTo, bool idInverted,
		object_id_t reporterFrom, object_id_t reporterTo,
		bool reporterInverted) {
	auto iter = listenerList.find(listener);
	if (iter == listenerList.end()) {
		return LISTENER_NOT_FOUND;
	}
	lockMutex();
	ReturnValue_t result = iter->second.addMatch(idFrom, idTo, idInverted,
			reporterFrom, reporterTo, reporterInverted);
	unlockMutex();
	return result;
}

ReturnValue_t EventManager::unsubscribeFromEventRange(MessageQueueId_t listener,
		EventId_t idFrom, EventId_t idTo, bool idInverted,
		object_id_t reporterFrom, object_id_t reporterTo,
		bool reporterInverted) {
	auto iter = listenerList.find(listener);
	if (iter == listenerList.end()) {
		return LISTENER_NOT_FOUND;
	}
	lockMutex();
	ReturnValue_t result = iter->second.removeMatch(idFrom, idTo, idInverted,
			reporterFrom, reporterTo, reporterInverted);
	unlockMutex();
	return result;
}

void EventManager::printEvent(EventMessage* message) {
	const char *string = 0;
	switch (message->getSeverity()) {
	case SEVERITY::INFO:
		string = translateObject(message->getReporter());
		info << "EVENT: ";
		if (string != 0) {
			info << string;
		} else {
			info << "0x" << std::hex << message->getReporter() << std::dec;
		}
		info << " reported " << translateEvents(message->getEvent()) << " ("
				<< std::dec << message->getEventId() << std::hex << ") P1: 0x"
				<< message->getParameter1() << " P2: 0x"
				<< message->getParameter2() << std::dec << std::endl;
		break;
	default:
		string = translateObject(message->getReporter());
		error << "EVENT: ";
		if (string != 0) {
			error << string;
		} else {
			error << "0x" << std::hex << message->getReporter() << std::dec;
		}
		error << " reported " << translateEvents(message->getEvent()) << " ("
				<< std::dec << message->getEventId() << std::hex << ") P1: 0x"
				<< message->getParameter1() << " P2: 0x"
				<< message->getParameter2() << std::dec << std::endl;
		break;
	}

}

void EventManager::lockMutex() {
	OSAL::lockMutex(this->mutex, OSAL::NO_TIMEOUT);
}

void EventManager::unlockMutex() {
	OSAL::unlockMutex(this->mutex);
}
