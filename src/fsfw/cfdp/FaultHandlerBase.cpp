#include "FaultHandlerBase.h"

CfdpFaultHandlerBase::CfdpFaultHandlerBase() = default;

bool CfdpFaultHandlerBase::getFaultHandler(
    cfdp::ConditionCode code, cfdp::FaultHandlerCodes& handler) const {
  auto iter = faultHandlerMap.find(code);
  if(iter == faultHandlerMap.end()) {
    return false;
  }
  handler = iter->second;
  return true;
}

bool CfdpFaultHandlerBase::setFaultHandler(cfdp::ConditionCode code,
                                           cfdp::FaultHandlerCodes handler) {
  if (not faultHandlerMap.contains(code)) {
    return false;
  }
  faultHandlerMap[code] = handler;
  return true;
}

bool CfdpFaultHandlerBase::faultCallback(cfdp::ConditionCode code) {
  if (not faultHandlerMap.contains(code)) {
    return false;
  }
  cfdp::FaultHandlerCodes fh = faultHandlerMap[code];
  if (fh == cfdp::FaultHandlerCodes::IGNORE_ERROR) {
    ignoreCb(code);
  } else if(fh == cfdp::FaultHandlerCodes::ABANDON_TRANSACTION) {
    abandonCb(code);
  } else if(fh == cfdp::FaultHandlerCodes::NOTICE_OF_CANCELLATION) {
    noticeOfCancellationCb(code);
  } else {
    noticeOfSuspensionCb(code);
  }
  return true;
}

CfdpFaultHandlerBase::~CfdpFaultHandlerBase() = default;
