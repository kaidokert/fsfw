#include "CfdpFaultHandlerMock.h"

void CfdpFaultHandlerMock::noticeOfSuspensionCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::NOTICE_OF_SUSPENSION);
  info.callCount++;
  info.condCodes.push(code);
}

void CfdpFaultHandlerMock::noticeOfCancellationCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::NOTICE_OF_CANCELLATION);
  info.callCount++;
  info.condCodes.push(code);
}

void CfdpFaultHandlerMock::abandonCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::ABANDON_TRANSACTION);
  info.callCount++;
  info.condCodes.push(code);
}

void CfdpFaultHandlerMock::ignoreCb(cfdp::ConditionCode code) {
  auto& info = fhInfoMap.at(cfdp::FaultHandlerCodes::IGNORE_ERROR);
  info.callCount++;
  info.condCodes.push(code);
}

CfdpFaultHandlerMock::FaultInfo& CfdpFaultHandlerMock::getFhInfo(cfdp::FaultHandlerCodes fhCode) {
  return fhInfoMap.at(fhCode);
}

void CfdpFaultHandlerMock::reset() { fhInfoMap.clear(); }

bool CfdpFaultHandlerMock::faultCbWasCalled() const {
  return std::any_of(fhInfoMap.begin(), fhInfoMap.end(),
                     [](const std::pair<cfdp::FaultHandlerCodes, FaultInfo>& pair) {
                       return pair.second.callCount > 0;
                     });
}
