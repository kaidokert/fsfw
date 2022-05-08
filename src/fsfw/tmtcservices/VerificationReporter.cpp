#include "fsfw/tmtcservices/VerificationReporter.h"

#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tmtcservices/AcceptsVerifyMessageIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

object_id_t VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;

VerificationReporter::VerificationReporter() : acknowledgeQueue(MessageQueueIF::NO_QUEUE) {}

VerificationReporter::~VerificationReporter() {}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id, TcPacketPusBase* currentPacket,
                                             uint8_t set_step) {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    this->initialize();
  }
  if (currentPacket == nullptr) {
    return;
  }
  PusVerificationMessage message(set_report_id, currentPacket->getAcknowledgeFlags(),
                                 currentPacket->getPacketId(),
                                 currentPacket->getPacketSequenceControl(), 0, set_step);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGET("VerificationReporter::sendSuccessReport: Error writing to queue. Code: {}\n",
               status);
  }
}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id, uint8_t ackFlags,
                                             uint16_t tcPacketId, uint16_t tcSequenceControl,
                                             uint8_t set_step) {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    this->initialize();
  }
  PusVerificationMessage message(set_report_id, ackFlags, tcPacketId, tcSequenceControl, 0,
                                 set_step);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGET(
        "VerificationReporter::sendSuccessReport: Error writing "
        "to queue. Code: {}\n",
        status);
  }
}

void VerificationReporter::sendFailureReport(uint8_t report_id, TcPacketPusBase* currentPacket,
                                             ReturnValue_t error_code, uint8_t step,
                                             uint32_t parameter1, uint32_t parameter2) {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    this->initialize();
  }
  if (currentPacket == nullptr) {
    return;
  }
  PusVerificationMessage message(
      report_id, currentPacket->getAcknowledgeFlags(), currentPacket->getPacketId(),
      currentPacket->getPacketSequenceControl(), error_code, step, parameter1, parameter2);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGET(
        "VerificationReporter::sendFailureReport: Error writing "
        "to queue. Code: {}\n",
        status);
  }
}

void VerificationReporter::sendFailureReport(uint8_t report_id, uint8_t ackFlags,
                                             uint16_t tcPacketId, uint16_t tcSequenceControl,
                                             ReturnValue_t error_code, uint8_t step,
                                             uint32_t parameter1, uint32_t parameter2) {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    this->initialize();
  }
  PusVerificationMessage message(report_id, ackFlags, tcPacketId, tcSequenceControl, error_code,
                                 step, parameter1, parameter2);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGE("sendFailureReport: Error writing to queue. Code {:#06x}\n", status);
  }
}

void VerificationReporter::initialize() {
  if (messageReceiver == objects::NO_OBJECT) {
    FSFW_LOGW(
        "initialize: Verification message "
        "receiver object ID not set yet in Factory\n");
    return;
  }
  auto* temp = ObjectManager::instance()->get<AcceptsVerifyMessageIF>(messageReceiver);
  if (temp == nullptr) {
    FSFW_LOGE(
        "VerificationReporter::initialize: Message receiver invalid. "
        "Does it implement AcceptsVerifyMessageIF?\n");
    return;
  }
  this->acknowledgeQueue = temp->getVerificationQueue();
}
