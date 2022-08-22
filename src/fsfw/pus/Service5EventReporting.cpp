#include "fsfw/pus/Service5EventReporting.h"

#include "fsfw/events/EventManagerIF.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/pus/servicepackets/Service5Packets.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/tmHelpers.h"

Service5EventReporting::Service5EventReporting(PsbParams params, size_t maxNumberReportsPerCycle,
                                               uint32_t messageQueueDepth)
    : PusServiceBase(params),
      storeHelper(params.apid),
      tmHelper(params.serviceId, storeHelper, sendHelper),
      maxNumberReportsPerCycle(maxNumberReportsPerCycle) {
  psbParams.name = "PUS 5 Event Reporting";
  eventQueue = QueueFactory::instance()->createMessageQueue(messageQueueDepth);
}

Service5EventReporting::~Service5EventReporting() {
  QueueFactory::instance()->deleteMessageQueue(eventQueue);
}

ReturnValue_t Service5EventReporting::performService() {
  EventMessage message;
  ReturnValue_t status = returnvalue::OK;
  for (uint8_t counter = 0; counter < maxNumberReportsPerCycle; counter++) {
    // Receive messages even if reporting is disabled for now.
    status = eventQueue->receiveMessage(&message);
    if (status == MessageQueueIF::EMPTY) {
      return returnvalue::OK;
    }

    if (enableEventReport) {
      status = generateEventReport(message);
      if (status != returnvalue::OK) {
        return status;
      }
    }
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Service5EventReporting::generateEventReport: Too many events" << std::endl;
#endif
  return returnvalue::OK;
}

ReturnValue_t Service5EventReporting::generateEventReport(EventMessage message) {
  EventReport report(message.getEventId(), message.getReporter(), message.getParameter1(),
                     message.getParameter2());
  storeHelper.preparePacket(psbParams.serviceId, message.getSeverity(), tmHelper.sendCounter);
  storeHelper.setSourceDataSerializable(report);
  ReturnValue_t result = tmHelper.storeAndSendTmPacket();
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Service5EventReporting::generateEventReport: "
                    "Could not send TM packet"
                 << std::endl;
#else
    sif::printWarning(
        "Service5EventReporting::generateEventReport: "
        "Could not send TM packet\n");
#endif
  }
  return result;
}

ReturnValue_t Service5EventReporting::handleRequest(uint8_t subservice) {
  switch (subservice) {
    case Subservice::ENABLE: {
      enableEventReport = true;
      return returnvalue::OK;
    }
    case Subservice::DISABLE: {
      enableEventReport = false;
      return returnvalue::OK;
    }
    default:
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

// In addition to the default PUSServiceBase initialization, this service needs
// to be registered to the event manager to listen for events.
ReturnValue_t Service5EventReporting::initialize() {
  ReturnValue_t result = PusServiceBase::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  auto* manager = ObjectManager::instance()->get<EventManagerIF>(objects::EVENT_MANAGER);
  if (manager == nullptr) {
    return returnvalue::FAILED;
  }
  // register Service 5 as listener for events
  result = manager->registerListener(eventQueue->getId(), true);
  if (result != returnvalue::OK) {
    return result;
  }
  initializeTmHelpers(sendHelper, storeHelper);
  return result;
}
