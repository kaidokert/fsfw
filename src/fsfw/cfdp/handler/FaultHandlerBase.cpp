#include "FaultHandlerBase.h"

namespace cfdp {

FaultHandlerBase::FaultHandlerBase() = default;
FaultHandlerBase::~FaultHandlerBase() = default;

bool FaultHandlerBase::getFaultHandler(cfdp::ConditionCode code,
                                       cfdp::FaultHandlerCode& handler) const {
  auto iter = faultHandlerMap.find(code);
  if (iter == faultHandlerMap.end()) {
    return false;
  }
  handler = iter->second;
  return true;
}

bool FaultHandlerBase::setFaultHandler(cfdp::ConditionCode code, cfdp::FaultHandlerCode handler) {
  if (not faultHandlerMap.contains(code)) {
    return false;
  }
  if (handler != FaultHandlerCode::NOTICE_OF_SUSPENSION and
      handler != FaultHandlerCode::ABANDON_TRANSACTION and
      handler != FaultHandlerCode::NOTICE_OF_CANCELLATION and
      handler != FaultHandlerCode::IGNORE_ERROR) {
    return false;
  }
  faultHandlerMap[code] = handler;
  return true;
}

bool FaultHandlerBase::reportFault(cfdp::TransactionId& id, cfdp::ConditionCode code) {
  if (not faultHandlerMap.contains(code)) {
    return false;
  }
  cfdp::FaultHandlerCode fh = faultHandlerMap[code];
  if (fh == cfdp::FaultHandlerCode::IGNORE_ERROR) {
    ignoreCb(id, code);
  } else if (fh == cfdp::FaultHandlerCode::ABANDON_TRANSACTION) {
    abandonCb(id, code);
  } else if (fh == cfdp::FaultHandlerCode::NOTICE_OF_CANCELLATION) {
    noticeOfCancellationCb(id, code);
  } else if (fh == cfdp::FaultHandlerCode::NOTICE_OF_SUSPENSION) {
    noticeOfSuspensionCb(id, code);
  } else {
    // Should never happen, but use defensive programming
    return false;
  }
  return true;
}
}  // namespace cfdp
