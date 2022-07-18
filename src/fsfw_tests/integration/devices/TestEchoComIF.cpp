#include "TestEchoComIF.h"

#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/tmtcpacket/pus/tm.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>

#include "TestCookie.h"
#include "fsfw/serviceinterface.h"

TestEchoComIF::TestEchoComIF(object_id_t objectId) : SystemObject(objectId) {}

TestEchoComIF::~TestEchoComIF() = default;

ReturnValue_t TestEchoComIF::initializeInterface(CookieIF *cookie) {
  auto *dummyCookie = dynamic_cast<TestCookie *>(cookie);
  if (dummyCookie == nullptr) {
    FSFW_LOGW("TestEchoComIF::initializeInterface: Invalid cookie\n");
    return NULLPOINTER;
  }

  auto resultPair =
      replyMap.emplace(dummyCookie->getAddress(), ReplyBuffer(dummyCookie->getReplyMaxLen()));
  if (not resultPair.second) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return RETURN_OK;
}

ReturnValue_t TestEchoComIF::sendMessage(CookieIF *cookie, const uint8_t *sendData,
                                         size_t sendLen) {
  auto *dummyCookie = dynamic_cast<TestCookie *>(cookie);
  if (dummyCookie == nullptr) {
    return NULLPOINTER;
  }

  ReplyBuffer &replyBuffer = replyMap.find(dummyCookie->getAddress())->second;
  if (sendLen > replyBuffer.capacity()) {
    FSFW_LOGWT("sendMessage: Send length larger than current reply buffer length\n");
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  replyBuffer.resize(sendLen);
  memcpy(replyBuffer.data(), sendData, sendLen);
  return RETURN_OK;
}

ReturnValue_t TestEchoComIF::getSendSuccess(CookieIF *cookie) { return RETURN_OK; }

ReturnValue_t TestEchoComIF::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
  return RETURN_OK;
}

ReturnValue_t TestEchoComIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) {
  auto *dummyCookie = dynamic_cast<TestCookie *>(cookie);
  if (dummyCookie == nullptr) {
    return NULLPOINTER;
  }

  ReplyBuffer &replyBuffer = replyMap.find(dummyCookie->getAddress())->second;
  *buffer = replyBuffer.data();
  *size = replyBuffer.size();

  dummyReplyCounter++;
  if (dummyReplyCounter == 10) {
    // add anything that needs to be read periodically by dummy handler
    dummyReplyCounter = 0;
  }
  return RETURN_OK;
}
