#include "fsfw/tmtcservices/VerificationReporter.h"

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

VerificationReporter::VerificationReporter(AcceptsVerifyMessageIF* receiver, object_id_t objectId)
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
  if (status != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendSuccessReport: Error writing "
               << "to queue. Code: " << std::hex << status << std::dec << std::endl;
#endif
  }
  return status;
}

ReturnValue_t VerificationReporter::sendSuccessReport(VerifSuccessParams params) {
  PusVerificationMessage message(params.reportId, params.ackFlags, params.tcPacketId, params.tcPsc,
                                 retval::OK, params.step);
  ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
  if (status != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VerificationReporter::sendSuccessReport: Error writing "
               << "to queue. Code: " << std::hex << status << std::dec << std::endl;
#endif
  }
  return status;
}
