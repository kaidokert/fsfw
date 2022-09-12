#include "ComIFMock.h"

#include "DeviceHandlerMock.h"

ComIFMock::ComIFMock(object_id_t objectId) : SystemObject(objectId) {}

ComIFMock::~ComIFMock() {}

ReturnValue_t ComIFMock::initializeInterface(CookieIF *cookie) { return returnvalue::OK; }

ReturnValue_t ComIFMock::sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) {
  data = *sendData;
  return returnvalue::OK;
}

ReturnValue_t ComIFMock::getSendSuccess(CookieIF *cookie) { return returnvalue::OK; }

ReturnValue_t ComIFMock::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
  return returnvalue::OK;
}

ReturnValue_t ComIFMock::readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) {
  switch (testCase) {
    case TestCase::MISSED_REPLY: {
      *size = 0;
      return returnvalue::OK;
    }
    case TestCase::SIMPLE_COMMAND_NOMINAL: {
      *size = 1;
      data = DeviceHandlerMock::SIMPLE_COMMAND_DATA;
      *buffer = &data;
      break;
    }
    case TestCase::PERIODIC_REPLY_NOMINAL: {
      *size = 1;
      data = DeviceHandlerMock::PERIODIC_REPLY_DATA;
      *buffer = &data;
      break;
    }
    default:
      break;
  }
  return returnvalue::OK;
}

void ComIFMock::setTestCase(TestCase testCase_) { testCase = testCase_; }
