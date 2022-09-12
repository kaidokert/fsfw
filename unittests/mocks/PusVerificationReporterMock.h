#ifndef FSFW_TESTS_PUSVERIFICATIONREPORTERMOCK_H
#define FSFW_TESTS_PUSVERIFICATIONREPORTERMOCK_H

#include <queue>

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tmtcservices/VerificationReporterIF.h"

class PusVerificationReporterMock : public SystemObject, public VerificationReporterIF {
 public:
  std::queue<VerifSuccessParams> successParams;
  std::queue<VerifFailureParams> failParams;
  PusVerificationReporterMock();
  explicit PusVerificationReporterMock(object_id_t registeredId);
  [[nodiscard]] size_t successCallCount() const;
  VerifSuccessParams& getNextSuccessCallParams();
  void popNextSuccessParams();
  [[nodiscard]] size_t failCallCount() const;
  VerifFailureParams& getNextFailCallParams();
  void popNextFailParams();

  ReturnValue_t sendSuccessReport(VerifSuccessParams params) override;
  ReturnValue_t sendFailureReport(VerifFailureParams params) override;
};
#endif  // FSFW_TESTS_PUSVERIFICATIONREPORTERMOCK_H
