#ifndef TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEFDIRMOCK_H_
#define TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEFDIRMOCK_H_

#include "fsfw/devicehandlers/DeviceHandlerFailureIsolation.h"

class DeviceFdirMock : public DeviceHandlerFailureIsolation {
 public:
  DeviceFdirMock(object_id_t owner, object_id_t parent);
  virtual ~DeviceFdirMock();

  uint32_t getMissedReplyCount();

 private:
  static const uint8_t STRANGE_REPLY_DOMAIN_ID = 0xF0;
  static const uint8_t MISSED_REPLY_DOMAIN_ID = 0xF1;
};

#endif /* TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEFDIRMOCK_H_ */
