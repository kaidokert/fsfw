#include "PusServiceBaseMock.h"

PsbMock::PsbMock(PsbParams params) : PusServiceBase(params) {}

ReturnValue_t PsbMock::handleRequest(uint8_t subservice) {
  handleRequestCallCnt++;
  subserviceQueue.push(subservice);
  if (handleReqFailPair.first) {
    handleReqFailPair.first = false;
    return handleReqFailPair.second;
  }
  return returnvalue::OK;
}

ReturnValue_t PsbMock::performService() {
  performServiceCallCnt++;
  if (performServiceFailPair.first) {
    performServiceFailPair.first = false;
    return performServiceFailPair.second;
  }
  return returnvalue::OK;
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
bool PsbMock::getAndPopNextSubservice(uint8_t& subservice) {
  if (subserviceQueue.empty()) {
    return false;
  }
  subservice = subserviceQueue.front();
  subserviceQueue.pop();
  return true;
}

PsbParams& PsbMock::getParams() { return psbParams; }

void PsbMock::setStaticPusDistributor(object_id_t pusDistributor) {
  PUS_DISTRIBUTOR = pusDistributor;
}

object_id_t PsbMock::getStaticPusDistributor() { return PUS_DISTRIBUTOR; }

void PsbMock::setStaticTmDest(object_id_t tmDest) { PACKET_DESTINATION = tmDest; }

object_id_t PsbMock::getStaticTmDest() { return PACKET_DESTINATION; }
