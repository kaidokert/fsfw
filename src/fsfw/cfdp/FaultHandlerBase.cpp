#include "FaultHandlerBase.h"

namespace cfdp {

FaultHandlerBase::FaultHandlerBase() = default;
FaultHandlerBase::~FaultHandlerBase() = default;

bool FaultHandlerBase::getFaultHandler(cfdp::ConditionCode code,
                                       cfdp::FaultHandlerCodes& handler) const {
  auto iter = faultHandlerMap.find(code);
  if (iter == faultHandlerMap.end()) {
    return false;
  }
  handler = iter->second;
  return true;
}

bool FaultHandlerBase::setFaultHandler(cfdp::ConditionCode code, cfdp::FaultHandlerCodes handler) {
  if (not faultHandlerMap.contains(code)) {
    return false;
  }
  if (handler != FaultHandlerCodes::NOTICE_OF_SUSPENSION and
      handler != FaultHandlerCodes::ABANDON_TRANSACTION and
      handler != FaultHandlerCodes::NOTICE_OF_CANCELLATION and
      handler != FaultHandlerCodes::IGNORE_ERROR) {
    return false;
  }
  faultHandlerMap[code] = handler;
  return true;
}

bool FaultHandlerBase::reportFault(cfdp::ConditionCode code) {
  if (not faultHandlerMap.contains(code)) {
    return false;
  }
  cfdp::FaultHandlerCodes fh = faultHandlerMap[code];
  if (fh == cfdp::FaultHandlerCodes::IGNORE_ERROR) {
    ignoreCb(code);
  } else if (fh == cfdp::FaultHandlerCodes::ABANDON_TRANSACTION) {
    abandonCb(code);
  } else if (fh == cfdp::FaultHandlerCodes::NOTICE_OF_CANCELLATION) {
    noticeOfCancellationCb(code);
  } else if (fh == cfdp::FaultHandlerCodes::NOTICE_OF_SUSPENSION) {
    noticeOfSuspensionCb(code);
  } else {
    // Should never happen, but use defensive programming
    return false;
  }
  return true;
}
}  // namespace cfdp
