#include "PusVerificationReporterMock.h"

#include "fsfw/objectmanager/frameworkObjects.h"

PusVerificationReporterMock::PusVerificationReporterMock()
    : SystemObject(objects::NO_OBJECT, false) {}

PusVerificationReporterMock::PusVerificationReporterMock(object_id_t registeredId)
    : SystemObject(registeredId) {}

size_t PusVerificationReporterMock::successCallCount() const { return successParams.size(); }
size_t PusVerificationReporterMock::failCallCount() const { return failParams.size(); }

VerifSuccessParams& PusVerificationReporterMock::getNextSuccessCallParams() {
  return successParams.front();
}

void PusVerificationReporterMock::popNextFailParams() {
  if (not failParams.empty()) {
    failParams.pop();
  }
}

VerifFailureParams& PusVerificationReporterMock::getNextFailCallParams() {
  return failParams.front();
}

void PusVerificationReporterMock::popNextSuccessParams() {
  if (not successParams.empty()) {
    successParams.pop();
  }
}

ReturnValue_t PusVerificationReporterMock::sendSuccessReport(VerifSuccessParams params) {
  successParams.push(params);
  return returnvalue::OK;
}

ReturnValue_t PusVerificationReporterMock::sendFailureReport(VerifFailureParams params) {
  failParams.push(params);
  return returnvalue::OK;
}
