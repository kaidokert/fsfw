#include "fsfw/pus/Service1TelecommandVerification.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/pus/servicepackets/Service1Packets.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tmtcpacket/pus/tm/TmPacketStored.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

Service1TelecommandVerification::Service1TelecommandVerification(object_id_t objectId,
                                                                 uint16_t apid, uint8_t serviceId,
                                                                 object_id_t targetDestination,
                                                                 uint16_t messageQueueDepth)
    : SystemObject(objectId),
      apid(apid),
      serviceId(serviceId),
      targetDestination(targetDestination) {
  tmQueue = QueueFactory::instance()->createMessageQueue(messageQueueDepth);
}

Service1TelecommandVerification::~Service1TelecommandVerification() {
  QueueFactory::instance()->deleteMessageQueue(tmQueue);
}

MessageQueueId_t Service1TelecommandVerification::getVerificationQueue() {
  return tmQueue->getId();
}

ReturnValue_t Service1TelecommandVerification::performOperation(uint8_t operationCode) {
  PusVerificationMessage message;
  ReturnValue_t status = tmQueue->receiveMessage(&message);
  while (status == HasReturnvaluesIF::RETURN_OK) {
    status = sendVerificationReport(&message);
    if (status != HasReturnvaluesIF::RETURN_OK) {
      return status;
    }
    status = tmQueue->receiveMessage(&message);
  }
  if (status == MessageQueueIF::EMPTY) {
    return HasReturnvaluesIF::RETURN_OK;
  } else {
    return status;
  }
}

ReturnValue_t Service1TelecommandVerification::sendVerificationReport(
    PusVerificationMessage* message) {
  ReturnValue_t result;
  if (message->getReportId() % 2 == 0) {
    result = generateFailureReport(message);
  } else {
    result = generateSuccessReport(message);
  }
  if (result != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGE(
        "Service1TelecommandVerification::sendVerificationReport: "
        "Sending verification packet failed\n");
  }
  return result;
}

ReturnValue_t Service1TelecommandVerification::generateFailureReport(
    PusVerificationMessage* message) {
  FailureReport report(message->getReportId(), message->getTcPacketId(),
                       message->getTcSequenceControl(), message->getStep(), message->getErrorCode(),
                       message->getParameter1(), message->getParameter2());
#if FSFW_USE_PUS_C_TELEMETRY == 0
  TmPacketStoredPusA tmPacket(apid, serviceId, message->getReportId(), packetSubCounter++, &report);
#else
  TmPacketStoredPusC tmPacket(apid, serviceId, message->getReportId(), packetSubCounter++, &report);
#endif
  ReturnValue_t result = tmPacket.sendPacket(tmQueue->getDefaultDestination(), tmQueue->getId());
  return result;
}

ReturnValue_t Service1TelecommandVerification::generateSuccessReport(
    PusVerificationMessage* message) {
  SuccessReport report(message->getReportId(), message->getTcPacketId(),
                       message->getTcSequenceControl(), message->getStep());
#if FSFW_USE_PUS_C_TELEMETRY == 0
  TmPacketStoredPusA tmPacket(apid, serviceId, message->getReportId(), packetSubCounter++, &report);
#else
  TmPacketStoredPusC tmPacket(apid, serviceId, message->getReportId(), packetSubCounter++, &report);
#endif
  ReturnValue_t result = tmPacket.sendPacket(tmQueue->getDefaultDestination(), tmQueue->getId());
  return result;
}

ReturnValue_t Service1TelecommandVerification::initialize() {
  // Get target object for TC verification messages
  auto* funnel = ObjectManager::instance()->get<AcceptsTelemetryIF>(targetDestination);
  if (funnel == nullptr) {
    FSFW_LOGE("initialize: Specified TM funnel invalid. Does it implement AcceptsTelemetryIF?\n");
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  tmQueue->setDefaultDestination(funnel->getReportReceptionQueue());
  return SystemObject::initialize();
}
