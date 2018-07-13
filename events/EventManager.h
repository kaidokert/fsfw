#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_

#include <framework/events/eventmatching/EventMatchTree.h>
#include <framework/events/EventManagerIF.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/storagemanager/LocalPool.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/ipc/MutexIF.h>
#include <map>

class EventManager: public EventManagerIF,
		public ExecutableObjectIF,
		public SystemObject {
public:
	static const uint16_t MAX_EVENTS_PER_CYCLE = 80;

	EventManager(object_id_t setObjectId);
	virtual ~EventManager();

	MessageQueueId_t getEventReportQueue();

	ReturnValue_t registerListener(MessageQueueId_t listener, bool forwardAllButSelected = false);
	ReturnValue_t subscribeToEvent(MessageQueueId_t listener, EventId_t event);
	ReturnValue_t subscribeToAllEventsFrom(MessageQueueId_t listener,
			object_id_t object);
	ReturnValue_t subscribeToEventRange(MessageQueueId_t listener,
			EventId_t idFrom = 0, EventId_t idTo = 0, bool idInverted = false,
			object_id_t reporterFrom = 0, object_id_t reporterTo = 0,
			bool reporterInverted = false);
	ReturnValue_t unsubscribeFromEventRange(MessageQueueId_t listener,
			EventId_t idFrom = 0, EventId_t idTo = 0, bool idInverted = false,
			object_id_t reporterFrom = 0, object_id_t reporterTo = 0,
			bool reporterInverted = false);
	ReturnValue_t performOperation(uint8_t opCode);
protected:

	MessageQueueIF* eventReportQueue;

	std::map<MessageQueueId_t, EventMatchTree> listenerList;

	MutexIF* mutex;

	static const uint8_t N_POOLS = 3;
	LocalPool<N_POOLS> factoryBackend;
	static const uint16_t POOL_SIZES[N_POOLS];
	static const uint16_t N_ELEMENTS[N_POOLS];

	void notifyListeners(EventMessage *message);

#ifdef DEBUG
	void printEvent(EventMessage *message);
#endif

	void lockMutex();

	void unlockMutex();
};

#endif /* EVENTMANAGER_H_ */
