#ifndef TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_COOKIEIFMOCK_H_
#define TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_COOKIEIFMOCK_H_

#include "fsfw/devicehandlers/CookieIF.h"

class CookieIFMock : public CookieIF {
 public:
  CookieIFMock();
  virtual ~CookieIFMock();
};

#endif /* TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_COOKIEIFMOCK_H_ */
