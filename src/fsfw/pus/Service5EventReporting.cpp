#include "fsfw/pus/Service5EventReporting.h"

#include "fsfw/events/EventManagerIF.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/pus/servicepackets/Service5Packets.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/tmHelpers.h"

Service5EventReporting::Service5EventReporting(object_id_t objectId, uint16_t apid,
                                               uint8_t serviceId, size_t maxNumberReportsPerCycle,
                                               uint32_t messageQueueDepth)
    : PusServiceBase(objectId, apid, serviceId),
      sendHelper(nullptr),
      storeHelper(apid, nullptr),
      maxNumberReportsPerCycle(maxNumberReportsPerCycle) {
  eventQueue = QueueFactory::instance()->createMessageQueue(messageQueueDepth);
}

Service5EventReporting::~Service5EventReporting() {
  QueueFactory::instance()->deleteMessageQueue(eventQueue);
}

ReturnValue_t Service5EventReporting::performService() {
  EventMessage message;
  ReturnValue_t status = RETURN_OK;
  for (uint8_t counter = 0; counter < maxNumberReportsPerCycle; counter++) {
    // Receive messages even if reporting is disabled for now.
    status = eventQueue->receiveMessage(&message);
    if (status == MessageQueueIF::EMPTY) {
      return HasReturnvaluesIF::RETURN_OK;
    }

    if (enableEventReport) {
      status = generateEventReport(message);
      if (status != HasReturnvaluesIF::RETURN_OK) {
        return status;
      }
    }
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Service5EventReporting::generateEventReport: Too many events" << std::endl;
#endif
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service5EventReporting::generateEventReport(EventMessage message) {
  EventReport report(message.getEventId(), message.getReporter(), message.getParameter1(),
                     message.getParameter2());
  storeHelper.preparePacket(serviceId, message.getSeverity(), packetSubCounter);
  storeHelper.setSourceDataSerializable(&report);
  ReturnValue_t result = tm::storeAndSendTmPacket(storeHelper, sendHelper);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Service5EventReporting::generateEventReport: "
                    "Could not send TM packet"
                 << std::endl;
#else
    sif::printWarning(
        "Service5EventReporting::generateEventReport: "
        "Could not send TM packet\n");
#endif
  } else {
    packetSubCounter++;
  }
  return result;
}

ReturnValue_t Service5EventReporting::handleRequest(uint8_t subservice) {
  switch (subservice) {
    case Subservice::ENABLE: {
      enableEventReport = true;
      return HasReturnvaluesIF::RETURN_OK;
    }
    case Subservice::DISABLE: {
      enableEventReport = false;
      return HasReturnvaluesIF::RETURN_OK;
    }
    default:
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

// In addition to the default PUSServiceBase initialization, this service needs
// to be registered to the event manager to listen for events.
ReturnValue_t Service5EventReporting::initialize() {
  ReturnValue_t result = PusServiceBase::initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  auto* manager = ObjectManager::instance()->get<EventManagerIF>(objects::EVENT_MANAGER);
  if (manager == nullptr) {
    return RETURN_FAILED;
  }
  // register Service 5 as listener for events
  result = manager->registerListener(eventQueue->getId(), true);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  initializeTmHelpers(sendHelper, storeHelper);
  return result;
}
