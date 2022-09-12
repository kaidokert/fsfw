#include "fsfw/pus/Service1TelecommandVerification.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/pus/servicepackets/Service1Packets.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"
#include "fsfw/tmtcservices/tmHelpers.h"

Service1TelecommandVerification::Service1TelecommandVerification(object_id_t objectId,
                                                                 uint16_t apid, uint8_t serviceId,
                                                                 object_id_t targetDestination,
                                                                 uint16_t messageQueueDepth,
                                                                 TimeWriterIF* timeStamper)
    : SystemObject(objectId),
      apid(apid),
      serviceId(serviceId),
      targetDestination(targetDestination),
      storeHelper(apid),
      tmHelper(serviceId, storeHelper, sendHelper) {
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
  while (status == returnvalue::OK) {
    status = sendVerificationReport(&message);
    if (status != returnvalue::OK) {
      return status;
    }
    status = tmQueue->receiveMessage(&message);
  }
  if (status == MessageQueueIF::EMPTY) {
    return returnvalue::OK;
  } else {
    return status;
  }
}

ReturnValue_t Service1TelecommandVerification::sendVerificationReport(
    PusVerificationMessage* message) {
  ReturnValue_t result;
  uint8_t reportId = message->getReportId();
  if (reportId == 0 or reportId > 8) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service1TelecommandVerification::sendVerificationReport: Invalid report ID "
               << static_cast<int>(reportId) << " detected" << std::endl;
#else
    sif::printError(
        "Service1TelecommandVerification::sendVerificationReport: Invalid report ID "
        "%d detected\n",
        reportId);
#endif
    return returnvalue::FAILED;
  }
  if (message->getReportId() % 2 == 0) {
    result = generateFailureReport(message);
  } else {
    result = generateSuccessReport(message);
  }
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service1TelecommandVerification::sendVerificationReport: "
                  "Sending verification packet failed !"
               << std::endl;
#endif
  }
  return result;
}

ReturnValue_t Service1TelecommandVerification::generateFailureReport(
    PusVerificationMessage* message) {
  FailureReport report(message->getReportId(), message->getTcPacketId(),
                       message->getTcSequenceControl(), message->getStep(), message->getErrorCode(),
                       message->getParameter1(), message->getParameter2());
  ReturnValue_t result =
      storeHelper.preparePacket(serviceId, message->getReportId(), packetSubCounter++);
  if (result != returnvalue::OK) {
    return result;
  }
  result = storeHelper.setSourceDataSerializable(report);
  if (result != returnvalue::OK) {
    return result;
  }
  return tmHelper.storeAndSendTmPacket();
}

ReturnValue_t Service1TelecommandVerification::generateSuccessReport(
    PusVerificationMessage* message) {
  SuccessReport report(message->getReportId(), message->getTcPacketId(),
                       message->getTcSequenceControl(), message->getStep());
  ReturnValue_t result =
      storeHelper.preparePacket(serviceId, message->getReportId(), packetSubCounter++);
  if (result != returnvalue::OK) {
    return result;
  }
  result = storeHelper.setSourceDataSerializable(report);
  if (result != returnvalue::OK) {
    return result;
  }
  return tmHelper.storeAndSendTmPacket();
}

ReturnValue_t Service1TelecommandVerification::initialize() {
  // Get target object for TC verification messages
  auto* funnel = ObjectManager::instance()->get<AcceptsTelemetryIF>(targetDestination);
  if (funnel == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service1TelecommandVerification::initialize: Specified"
                  " TM funnel invalid. Make sure it is set up and implements"
                  " AcceptsTelemetryIF."
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  if (tmQueue == nullptr) {
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  tmQueue->setDefaultDestination(funnel->getReportReceptionQueue());
  if (tmStore == nullptr) {
    tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (tmStore == nullptr) {
      return ObjectManager::CHILD_INIT_FAILED;
    }
    storeHelper.setTmStore(*tmStore);
  }
  if (timeStamper == nullptr) {
    timeStamper = ObjectManager::instance()->get<TimeWriterIF>(objects::TIME_STAMPER);
    if (timeStamper == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  } else {
  }
  storeHelper.setTimeStamper(*timeStamper);

  sendHelper.setMsgQueue(*tmQueue);
  if (errReporter == nullptr) {
    errReporter =
        ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
    if (errReporter != nullptr) {
      sendHelper.setInternalErrorReporter(*errReporter);
    }
  }
  return SystemObject::initialize();
}
