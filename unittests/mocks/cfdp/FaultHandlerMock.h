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

  void noticeOfSuspensionCb(ConditionCode code) override;
  void noticeOfCancellationCb(ConditionCode code) override;
  void abandonCb(ConditionCode code) override;
  void ignoreCb(ConditionCode code) override;

  FaultInfo& getFhInfo(FaultHandlerCodes fhCode);
  [[nodiscard]] bool faultCbWasCalled() const;
  void reset();

 private:
  std::map<cfdp::FaultHandlerCodes, FaultInfo> fhInfoMap = {
      std::pair{cfdp::FaultHandlerCodes::IGNORE_ERROR, FaultInfo()},
      std::pair{cfdp::FaultHandlerCodes::NOTICE_OF_CANCELLATION, FaultInfo()},
      std::pair{cfdp::FaultHandlerCodes::NOTICE_OF_SUSPENSION, FaultInfo()},
      std::pair{cfdp::FaultHandlerCodes::ABANDON_TRANSACTION, FaultInfo()}};
};

}  // namespace cfdp

#endif  // FSFW_TESTS_FAULTHANDLERMOCK_H
