#ifndef FSFW_TESTS_CFDPFAULTHANDLERMOCK_H
#define FSFW_TESTS_CFDPFAULTHANDLERMOCK_H

#include <map>
#include <queue>

#include "fsfw/cfdp/handler/FaultHandlerBase.h"

class CfdpFaultHandlerMock : public cfdp::FaultHandlerBase {
 public:
  struct FaultInfo {
    size_t callCount = 0;
    std::queue<cfdp::ConditionCode> condCodes;
  };

  void noticeOfSuspensionCb(cfdp::ConditionCode code) override;
  void noticeOfCancellationCb(cfdp::ConditionCode code) override;
  void abandonCb(cfdp::ConditionCode code) override;
  void ignoreCb(cfdp::ConditionCode code) override;

  FaultInfo& getFhInfo(cfdp::FaultHandlerCodes fhCode);
  [[nodiscard]] bool faultCbWasCalled() const;
  void reset();

 private:
  std::map<cfdp::FaultHandlerCodes, FaultInfo> fhInfoMap = {
      std::pair{cfdp::FaultHandlerCodes::IGNORE_ERROR, FaultInfo()},
      std::pair{cfdp::FaultHandlerCodes::NOTICE_OF_CANCELLATION, FaultInfo()},
      std::pair{cfdp::FaultHandlerCodes::NOTICE_OF_SUSPENSION, FaultInfo()},
      std::pair{cfdp::FaultHandlerCodes::ABANDON_TRANSACTION, FaultInfo()}};
};
#endif  // FSFW_TESTS_CFDPFAULTHANDLERMOCK_H
