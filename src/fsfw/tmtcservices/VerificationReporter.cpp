#include "fsfw/tmtcservices/VerificationReporter.h"

#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/AcceptsVerifyMessageIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

object_id_t VerificationReporter::messageReceiver = objects::PUS_SERVICE_1_VERIFICATION;

VerificationReporter::VerificationReporter() : acknowledgeQueue(MessageQueueIF::NO_QUEUE) {}

VerificationReporter::~VerificationReporter() = default;

void VerificationReporter::sendSuccessReport(uint8_t set_report_id, PusTcReader* currentPacket,
                                             uint8_t set_step) {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    this->initialize();
  }
  if (currentPacket == nullptr) {
    return;
  }
  PusVerificationMessage message(set_report_id, currentPacket->getAcknowledgeFlags(),
                                 currentPacket->getPacketIdRaw(),
                                 currentPacket->getPacketSeqCtrlRaw(), 0, set_step);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendSuccessReport: Error writing "
               << "to queue. Code: " << std::hex << status << std::dec << std::endl;
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendSuccessReport: Error writing "
               << "to queue. Code: " << std::hex << status << std::dec << std::endl;
#endif
  }
}

void VerificationReporter::sendFailureReport(uint8_t report_id, PusTcReader* currentPacket,
                                             ReturnValue_t error_code, uint8_t step,
                                             uint32_t parameter1, uint32_t parameter2) {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    this->initialize();
  }
  if (currentPacket == nullptr) {
    return;
  }
  PusVerificationMessage message(
      report_id, currentPacket->getAcknowledgeFlags(), currentPacket->getPacketIdRaw(),
      currentPacket->getPacketSeqCtrlRaw(), error_code, step, parameter1, parameter2);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendFailureReport: Error writing "
               << "to queue. Code: " << std::hex << "0x" << status << std::dec << std::endl;
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendFailureReport: Error writing "
               << "to queue. Code: " << std::hex << "0x" << status << std::dec << std::endl;
#endif
  }
}

void VerificationReporter::initialize() {
  if (messageReceiver == objects::NO_OBJECT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "VerificationReporter::initialize: Verification message"
                    " receiver object ID not set yet in Factory!"
                 << std::endl;
#endif
    return;
  }
  auto* temp = ObjectManager::instance()->get<AcceptsVerifyMessageIF>(messageReceiver);
  if (temp == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::initialize: Message "
               << "receiver invalid. Make sure it is set up properly and "
               << "implementsAcceptsVerifyMessageIF" << std::endl;
#endif
    return;
  }
  this->acknowledgeQueue = temp->getVerificationQueue();
}
