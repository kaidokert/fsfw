#include "fsfw/rmap/RmapDeviceCommunicationIF.h"

#include "fsfw/rmap/RMAP.h"

// TODO Cast here are all potential bugs
RmapDeviceCommunicationIF::~RmapDeviceCommunicationIF() {}

ReturnValue_t RmapDeviceCommunicationIF::sendMessage(CookieIF *cookie, const uint8_t *sendData,
                                                     size_t sendLen) {
  return RMAP::sendWriteCommand((RMAPCookie *)cookie, sendData, sendLen);
}

ReturnValue_t RmapDeviceCommunicationIF::getSendSuccess(CookieIF *cookie) {
  return RMAP::getWriteReply((RMAPCookie *)cookie);
}

ReturnValue_t RmapDeviceCommunicationIF::requestReceiveMessage(CookieIF *cookie,
                                                               size_t requestLen) {
  return RMAP::sendReadCommand((RMAPCookie *)cookie, ((RMAPCookie *)cookie)->getMaxReplyLen());
}

ReturnValue_t RmapDeviceCommunicationIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
                                                             size_t *size) {
  return RMAP::getReadReply((RMAPCookie *)cookie, buffer, size);
}

ReturnValue_t RmapDeviceCommunicationIF::setAddress(CookieIF *cookie, uint32_t address) {
  ((RMAPCookie *)cookie)->setAddress(address);
  return returnvalue::OK;
}

uint32_t RmapDeviceCommunicationIF::getAddress(CookieIF *cookie) {
  return ((RMAPCookie *)cookie)->getAddress();
}

ReturnValue_t RmapDeviceCommunicationIF::setParameter(CookieIF *cookie, uint32_t parameter) {
  // TODO Empty?
  return returnvalue::FAILED;
}

uint32_t RmapDeviceCommunicationIF::getParameter(CookieIF *cookie) { return 0; }
