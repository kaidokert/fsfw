#include "TestEchoComIF.h"

#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tmtcpacket/pus/tm.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>

#include "TestCookie.h"

TestEchoComIF::TestEchoComIF(object_id_t objectId) : SystemObject(objectId) {}

TestEchoComIF::~TestEchoComIF() {}

ReturnValue_t TestEchoComIF::initializeInterface(CookieIF *cookie) {
  TestCookie *dummyCookie = dynamic_cast<TestCookie *>(cookie);
  if (dummyCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TestEchoComIF::initializeInterface: Invalid cookie!" << std::endl;
#else
    sif::printWarning("TestEchoComIF::initializeInterface: Invalid cookie!\n");
#endif
    return NULLPOINTER;
  }

  auto resultPair =
      replyMap.emplace(dummyCookie->getAddress(), ReplyBuffer(dummyCookie->getReplyMaxLen()));
  if (not resultPair.second) {
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t TestEchoComIF::sendMessage(CookieIF *cookie, const uint8_t *sendData,
                                         size_t sendLen) {
  TestCookie *dummyCookie = dynamic_cast<TestCookie *>(cookie);
  if (dummyCookie == nullptr) {
    return NULLPOINTER;
  }

  ReplyBuffer &replyBuffer = replyMap.find(dummyCookie->getAddress())->second;
  if (sendLen > replyBuffer.capacity()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TestEchoComIF::sendMessage: Send length " << sendLen
                 << " larger than "
                    "current reply buffer length!"
                 << std::endl;
#else
    sif::printWarning(
        "TestEchoComIF::sendMessage: Send length %d larger than current "
        "reply buffer length!\n",
        sendLen);
#endif
    return returnvalue::FAILED;
  }
  replyBuffer.resize(sendLen);
  memcpy(replyBuffer.data(), sendData, sendLen);
  return returnvalue::OK;
}

ReturnValue_t TestEchoComIF::getSendSuccess(CookieIF *cookie) { return returnvalue::OK; }

ReturnValue_t TestEchoComIF::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
  return returnvalue::OK;
}

ReturnValue_t TestEchoComIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) {
  TestCookie *dummyCookie = dynamic_cast<TestCookie *>(cookie);
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
  return returnvalue::OK;
}
