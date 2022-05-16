#include "ComIFMock.h"

ComIFMock::ComIFMock(obejct_id_t objectId) {}

ComIFMock::~ComIFMock() {}

ReturnValue_t ComIFMock::initializeInterface(CookieIF *cookie) { return RETURN_OK; }

ReturnValue_t ComIFMock::sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) {
  rememberSentByte = *sendData;
  return RETURN_OK;
}

ReturnValue_t ComIFMock::getSendSuccess(CookieIF *cookie) { return RETURN_OK; }

ReturnValue_t ComIFMock::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
  return RETURN_OK;
}

ReturnValue_t ComIFMock::readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) {
  *size = sizeof(rememberSentByte);
  *buffer = &rememberSentByte;
  return RETURN_OK;
}
