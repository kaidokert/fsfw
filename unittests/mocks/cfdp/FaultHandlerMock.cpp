#include "FaultHandlerMock.h"

namespace cfdp {

void FaultHandlerMock::noticeOfSuspensionCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::NOTICE_OF_SUSPENSION);
  info.callCount++;
  info.condCodes.push(code);
}

void FaultHandlerMock::noticeOfCancellationCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::NOTICE_OF_CANCELLATION);
  info.callCount++;
  info.condCodes.push(code);
}

void FaultHandlerMock::abandonCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::ABANDON_TRANSACTION);
  info.callCount++;
  info.condCodes.push(code);
}

void FaultHandlerMock::ignoreCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::IGNORE_ERROR);
  info.callCount++;
  info.condCodes.push(code);
}

FaultHandlerMock::FaultInfo& FaultHandlerMock::getFhInfo(cfdp::FaultHandlerCodes fhCode) {
  return fhInfoMap.at(fhCode);
}

void FaultHandlerMock::reset() { fhInfoMap.clear(); }

bool FaultHandlerMock::faultCbWasCalled() const {
  return std::any_of(fhInfoMap.begin(), fhInfoMap.end(),
                     [](const std::pair<cfdp::FaultHandlerCodes, FaultInfo>& pair) {
                       return pair.second.callCount > 0;
                     });
}

}  // namespace cfdp
