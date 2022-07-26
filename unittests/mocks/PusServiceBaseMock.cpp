#include "PusServiceBaseMock.h"

PsbMock::PsbMock(PsbParams params) : PusServiceBase(params) {}

ReturnValue_t PsbMock::handleRequest(uint8_t subservice) {
  handleRequestCallCnt++;
  subserviceQueue.push(subservice);
  if (handleReqFailPair.first) {
    handleReqFailPair.first = false;
    return handleReqFailPair.second;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PsbMock::performService() {
  performServiceCallCnt++;
  if (performServiceFailPair.first) {
    performServiceFailPair.first = false;
    return performServiceFailPair.second;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

void PsbMock::reset() {
  handleRequestCallCnt = 0;
  performServiceCallCnt = 0;
  std::queue<uint8_t>().swap(subserviceQueue);
}

void PsbMock::makeNextHandleReqCallFail(ReturnValue_t retval) {
  handleReqFailPair.first = true;
  handleReqFailPair.second = retval;
}
