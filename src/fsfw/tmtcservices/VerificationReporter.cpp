#include "fsfw/tmtcservices/VerificationReporter.h"

#include "fsfw/objectmanager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

object_id_t VerificationReporter::DEFAULT_RECEIVER = objects::PUS_SERVICE_1_VERIFICATION;
object_id_t VerificationReporter::DEFAULT_REPORTER = objects::VERIFICATION_REPORTER;

VerificationReporter::VerificationReporter(object_id_t objectId, AcceptsVerifyMessageIF* receiver)
    : SystemObject(objectId) {
  if (receiver != nullptr) {
    acknowledgeQueue = receiver->getVerificationQueue();
  }
}

VerificationReporter::~VerificationReporter() = default;

void VerificationReporter::setReceiver(AcceptsVerifyMessageIF& receiver) {
  acknowledgeQueue = receiver.getVerificationQueue();
}

ReturnValue_t VerificationReporter::sendFailureReport(VerifFailureParams params) {
  PusVerificationMessage message(params.reportId, params.ackFlags, params.tcPacketId, params.tcPsc,
                                 params.errorCode, params.step, params.errorParam1,
                                 params.errorParam2);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendSuccessReport: Error writing "
               << "to queue. Code: " << std::hex << status << std::dec << std::endl;
#endif
  }
  return status;
}

ReturnValue_t VerificationReporter::sendSuccessReport(VerifSuccessParams params) {
  PusVerificationMessage message(params.reportId, params.ackFlags, params.tcPacketId, params.tcPsc,
                                 returnvalue::OK, params.step);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendSuccessReport: Error writing "
               << "to queue. Code: " << std::hex << status << std::dec << std::endl;
#endif
  }
  return status;
}

ReturnValue_t VerificationReporter::initialize() {
  if (acknowledgeQueue == MessageQueueIF::NO_QUEUE) {
    auto* receiver = ObjectManager::instance()->get<AcceptsVerifyMessageIF>(DEFAULT_RECEIVER);
    if (receiver != nullptr) {
      acknowledgeQueue = receiver->getVerificationQueue();
    } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error
          << "Could not find a suitable verification message receiver. Please ensure that it is set"
             " via the constructor or creating a global one with the ID "
             "VerificationReporter::DEFAULT_RECEIVER"
          << std::endl;
#else
      sif::printError(
          "Could not find a suitable verification message receiver. Please ensure "
          "that it is set via the constructor or creating a global one with the ID "
          "VerificationReporter::DEFAULT_RECEIVER\n");
#endif
    }
  }
  return SystemObject::initialize();
}
