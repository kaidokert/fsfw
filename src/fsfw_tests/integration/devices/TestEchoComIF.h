#ifndef TEST_TESTDEVICES_TESTECHOCOMIF_H_
#define TEST_TESTDEVICES_TESTECHOCOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>

#include <vector>

/**
 * @brief Used to simply returned sent data from device handler
 * @details Assign this com IF in the factory when creating the device handler
 * @ingroup test
 */
class TestEchoComIF : public DeviceCommunicationIF, public SystemObject {
 public:
  TestEchoComIF(object_id_t objectId);
  virtual ~TestEchoComIF();

  /**
   * DeviceCommunicationIF overrides
   * (see DeviceCommunicationIF documentation
   */
  ReturnValue_t initializeInterface(CookieIF *cookie) override;
  ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) override;
  ReturnValue_t getSendSuccess(CookieIF *cookie) override;
  ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) override;
  ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) override;

 private:
  /**
   * Send TM packet which contains received data as TM[17,130].
   * Wiretapping will do the same.
   * @param data
   * @param len
   */
  void sendTmPacket(const uint8_t *data, uint32_t len);

  AcceptsTelemetryIF *funnel = nullptr;
  MessageQueueIF *tmQueue = nullptr;
  size_t replyMaxLen = 0;

  using ReplyBuffer = std::vector<uint8_t>;
  std::map<address_t, ReplyBuffer> replyMap;

  uint8_t dummyReplyCounter = 0;

  uint16_t packetSubCounter = 0;
};

#endif /* TEST_TESTDEVICES_TESTECHOCOMIF_H_ */
