#ifndef TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_COMIFMOCK_H_
#define TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_COMIFMOCK_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>

/**
 * @brief	The ComIFMock supports the simulation of various device communication error cases
 * 		like incomplete or wrong replies and can be used to test the DeviceHandlerBase.
 */
class ComIFMock : public DeviceCommunicationIF, public SystemObject {
 public:
  ComIFMock(obejct_id_t objectId);
  virtual ~ComIFMock();

  virtual ReturnValue_t initializeInterface(CookieIF *cookie) override;
  virtual ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData,
                                    size_t sendLen) override;
  virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
  virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) override;
  virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
                                            size_t *size) override;

 private:
  uint8_t rememberSentByte = 0;
};

#endif /* TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_COMIFMOCK_H_ */
