#include "DeviceFdirMock.h"

DeviceFdirMock::DeviceFdirMock(object_id_t owner, object_id_t parent)
    : DeviceHandlerFailureIsolation(owner, parent) {}

DeviceFdirMock::~DeviceFdirMock() = default;

uint32_t DeviceFdirMock::getMissedReplyCount() {
  ParameterWrapper parameterWrapper;
  this->getParameter(MISSED_REPLY_DOMAIN_ID,
                     static_cast<uint8_t>(FaultCounter::ParameterIds::FAULT_COUNT),
                     &parameterWrapper, nullptr, 0);
  uint32_t missedReplyCount = 0;
  parameterWrapper.getElement(&missedReplyCount);
  return missedReplyCount;
}
