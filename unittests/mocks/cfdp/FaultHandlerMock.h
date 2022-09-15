#ifndef FSFW_TESTS_FAULTHANDLERMOCK_H
#define FSFW_TESTS_FAULTHANDLERMOCK_H

#include <map>
#include <queue>

#include "fsfw/cfdp/handler/FaultHandlerBase.h"

namespace cfdp {

class FaultHandlerMock : public FaultHandlerBase {
 public:
  struct FaultInfo {
    size_t callCount = 0;
    std::queue<cfdp::ConditionCode> condCodes;
  };

  void noticeOfSuspensionCb(TransactionId& id, ConditionCode code) override;
  void noticeOfCancellationCb(TransactionId& id, ConditionCode code) override;
  void abandonCb(TransactionId& id,ConditionCode code) override;
  void ignoreCb(TransactionId& id, ConditionCode code) override;

  FaultInfo& getFhInfo(FaultHandlerCode fhCode);
  [[nodiscard]] bool faultCbWasCalled() const;
  void reset();

 private:
  std::map<cfdp::FaultHandlerCode, FaultInfo> fhInfoMap = {
      std::pair{cfdp::FaultHandlerCode::IGNORE_ERROR, FaultInfo()},
      std::pair{cfdp::FaultHandlerCode::NOTICE_OF_CANCELLATION, FaultInfo()},
      std::pair{cfdp::FaultHandlerCode::NOTICE_OF_SUSPENSION, FaultInfo()},
      std::pair{cfdp::FaultHandlerCode::ABANDON_TRANSACTION, FaultInfo()}};
};

}  // namespace cfdp

#endif  // FSFW_TESTS_FAULTHANDLERMOCK_H
