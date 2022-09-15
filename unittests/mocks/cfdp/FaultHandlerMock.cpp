#include "FaultHandlerMock.h"

namespace cfdp {

void FaultHandlerMock::noticeOfSuspensionCb(TransactionId& id, cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCode::NOTICE_OF_SUSPENSION);
  info.callCount++;
  info.condCodes.push(code);
}

void FaultHandlerMock::noticeOfCancellationCb(TransactionId& id, cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCode::NOTICE_OF_CANCELLATION);
  info.callCount++;
  info.condCodes.push(code);
}

void FaultHandlerMock::abandonCb(TransactionId& id, cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCode::ABANDON_TRANSACTION);
  info.callCount++;
  info.condCodes.push(code);
}

void FaultHandlerMock::ignoreCb(TransactionId& id, cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCode::IGNORE_ERROR);
  info.callCount++;
  info.condCodes.push(code);
}

FaultHandlerMock::FaultInfo& FaultHandlerMock::getFhInfo(cfdp::FaultHandlerCode fhCode) {
  return fhInfoMap.at(fhCode);
}

void FaultHandlerMock::reset() { fhInfoMap.clear(); }

bool FaultHandlerMock::faultCbWasCalled() const {
  return std::any_of(fhInfoMap.begin(), fhInfoMap.end(),
                     [](const std::pair<cfdp::FaultHandlerCode, FaultInfo>& pair) {
                       return pair.second.callCount > 0;
                     });
}

}  // namespace cfdp
