#include "fsfw/events/EventManager.h"

#include "fsfw/events/EventMessage.h"
#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/QueueFactory.h"

MessageQueueId_t EventManagerIF::eventmanagerQueue = MessageQueueIF::NO_QUEUE;

// If one checks registerListener calls, there are around 40 (to max 50)
// objects registering for certain events.
// Each listener requires 1 or 2 EventIdMatcher and 1 or 2 ReportRangeMatcher.
// So a good guess is 75 to a max of 100 pools required for each, which fits well.
const LocalPool::LocalPoolConfig EventManager::poolConfig = {
    {fsfwconfig::FSFW_EVENTMGMR_MATCHTREE_NODES, sizeof(EventMatchTree::Node)},
    {fsfwconfig::FSFW_EVENTMGMT_EVENTIDMATCHERS, sizeof(EventIdRangeMatcher)},
    {fsfwconfig::FSFW_EVENTMGMR_RANGEMATCHERS, sizeof(ReporterRangeMatcher)}};

EventManager::EventManager(object_id_t setObjectId)
    : SystemObject(setObjectId), factoryBackend(0, poolConfig, false, true) {
  mutex = MutexFactory::instance()->createMutex();
  eventReportQueue = QueueFactory::instance()->createMessageQueue(MAX_EVENTS_PER_CYCLE,
                                                                  EventMessage::EVENT_MESSAGE_SIZE);
}

EventManager::~EventManager() {
  QueueFactory::instance()->deleteMessageQueue(eventReportQueue);
  MutexFactory::instance()->deleteMutex(mutex);
}

MessageQueueId_t EventManager::getEventReportQueue() { return eventReportQueue->getId(); }

ReturnValue_t EventManager::performOperation(uint8_t opCode) {
  ReturnValue_t result = returnvalue::OK;
  while (result == returnvalue::OK) {
    EventMessage message;
    result = eventReportQueue->receiveMessage(&message);
    if (result == returnvalue::OK) {
#if FSFW_OBJ_EVENT_TRANSLATION == 1
      printEvent(&message);
#endif
      notifyListeners(&message);
    }
  }
  return returnvalue::OK;
}

void EventManager::notifyListeners(EventMessage* message) {
  lockMutex();
  for (auto iter = listenerList.begin(); iter != listenerList.end(); ++iter) {
    if (iter->second.match(message)) {
      MessageQueueSenderIF::sendMessage(iter->first, message, message->getSender());
    }
  }
  unlockMutex();
}

ReturnValue_t EventManager::registerListener(MessageQueueId_t listener,
                                             bool forwardAllButSelected) {
  auto result = listenerList.insert(std::pair<MessageQueueId_t, EventMatchTree>(
      listener, EventMatchTree(&factoryBackend, forwardAllButSelected)));
  if (!result.second) {
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t EventManager::subscribeToEvent(MessageQueueId_t listener, EventId_t event) {
  return subscribeToEventRange(listener, event);
}

ReturnValue_t EventManager::subscribeToAllEventsFrom(MessageQueueId_t listener,
                                                     object_id_t object) {
  return subscribeToEventRange(listener, 0, 0, true, object);
}

ReturnValue_t EventManager::subscribeToEventRange(MessageQueueId_t listener, EventId_t idFrom,
                                                  EventId_t idTo, bool idInverted,
                                                  object_id_t reporterFrom, object_id_t reporterTo,
                                                  bool reporterInverted) {
  auto iter = listenerList.find(listener);
  if (iter == listenerList.end()) {
    return LISTENER_NOT_FOUND;
  }
  lockMutex();
  ReturnValue_t result =
      iter->second.addMatch(idFrom, idTo, idInverted, reporterFrom, reporterTo, reporterInverted);
  unlockMutex();
  return result;
}

ReturnValue_t EventManager::unsubscribeFromAllEvents(MessageQueueId_t listener,
                                                     object_id_t object) {
  return unsubscribeFromEventRange(listener, 0, 0, true, object);
}

ReturnValue_t EventManager::unsubscribeFromEventRange(MessageQueueId_t listener, EventId_t idFrom,
                                                      EventId_t idTo, bool idInverted,
                                                      object_id_t reporterFrom,
                                                      object_id_t reporterTo,
                                                      bool reporterInverted) {
  auto iter = listenerList.find(listener);
  if (iter == listenerList.end()) {
    return LISTENER_NOT_FOUND;
  }
  lockMutex();
  ReturnValue_t result = iter->second.removeMatch(idFrom, idTo, idInverted, reporterFrom,
                                                  reporterTo, reporterInverted);
  unlockMutex();
  return result;
}

void EventManager::lockMutex() { mutex->lockMutex(timeoutType, timeoutMs); }

void EventManager::unlockMutex() { mutex->unlockMutex(); }

void EventManager::setMutexTimeout(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) {
  this->timeoutType = timeoutType;
  this->timeoutMs = timeoutMs;
}

#if FSFW_OBJ_EVENT_TRANSLATION == 1

void EventManager::printEvent(EventMessage* message) {
  switch (message->getSeverity()) {
    case severity::INFO: {
#if FSFW_DEBUG_INFO == 1
      printUtility(sif::OutputTypes::OUT_INFO, message);
#endif /* DEBUG_INFO_EVENT == 1 */
      break;
    }
    default:
      printUtility(sif::OutputTypes::OUT_DEBUG, message);
      break;
  }
}

void EventManager::printUtility(sif::OutputTypes printType, EventMessage* message) {
  const char* string = 0;
  if (printType == sif::OutputTypes::OUT_INFO) {
    string = translateObject(message->getReporter());
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "EventManager: ";
    if (string != 0) {
      sif::info << string;
    } else {
      sif::info << "0x" << std::hex << std::setw(8) << std::setfill('0') << message->getReporter()
                << std::setfill(' ') << std::dec;
    }
    sif::info << " reported event with ID " << message->getEventId() << std::endl;
    sif::info << translateEvents(message->getEvent()) << " | " << std::hex << "P1 Hex: 0x"
              << message->getParameter1() << " | P1 Dec: " << std::dec << message->getParameter1()
              << std::hex << " | P2 Hex: 0x" << message->getParameter2()
              << " | P2 Dec: " << std::dec << message->getParameter2() << std::endl;
#else
    if (string != 0) {
      sif::printInfo("Event Manager: %s reported event with ID %d\n", string,
                     message->getEventId());
    } else {
      sif::printInfo("Event Manager: Reporter ID 0x%08x reported event with ID %d\n",
                     message->getReporter(), message->getEventId());
    }

    sif::printInfo("%s | P1 Hex: 0x%x | P1 Dec: %d | P2 Hex: 0x%x | P2 Dec: %d\n",
                   translateEvents(message->getEvent()), message->getParameter1(),
                   message->getParameter1(), message->getParameter2(), message->getParameter2());
#endif /* FSFW_CPP_OSTREAM_ENABLED == 0 */
  } else {
    string = translateObject(message->getReporter());
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "EventManager: ";
    if (string != 0) {
      sif::debug << string;
    } else {
      sif::debug << "0x" << std::hex << std::setw(8) << std::setfill('0') << message->getReporter()
                 << std::setfill(' ') << std::dec;
    }
    sif::debug << " reported event with ID " << message->getEventId() << std::endl;
    sif::debug << translateEvents(message->getEvent()) << " | " << std::hex << "P1 Hex: 0x"
               << message->getParameter1() << " | P1 Dec: " << std::dec << message->getParameter1()
               << std::hex << " | P2 Hex: 0x" << message->getParameter2()
               << " | P2 Dec: " << std::dec << message->getParameter2() << std::endl;
#else
    if (string != 0) {
      sif::printDebug("Event Manager: %s reported event with ID %d\n", string,
                      message->getEventId());
    } else {
      sif::printDebug("Event Manager: Reporter ID 0x%08x reported event with ID %d\n",
                      message->getReporter(), message->getEventId());
    }
    sif::printDebug("P1 Hex: 0x%x | P1 Dec: %d | P2 Hex: 0x%x | P2 Dec: %d\n",
                    message->getParameter1(), message->getParameter1(), message->getParameter2(),
                    message->getParameter2());
#endif /* FSFW_CPP_OSTREAM_ENABLED == 0 */
  }
}

#endif /* FSFW_OBJ_EVENT_TRANSLATION == 1 */
