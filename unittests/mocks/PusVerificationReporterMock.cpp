#include "PusVerificationReporterMock.h"

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
