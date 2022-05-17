#ifndef MISSION_DEVICES_TESTCOOKIE_H_
#define MISSION_DEVICES_TESTCOOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>

#include <cstddef>

/**
 * @brief	Really simple cookie which does not do a lot.
 */
class TestCookie : public CookieIF {
 public:
  TestCookie(address_t address, size_t maxReplyLen);
  virtual ~TestCookie();

  address_t getAddress() const;
  size_t getReplyMaxLen() const;

 private:
  address_t address = 0;
  size_t replyMaxLen = 0;
};

#endif /* MISSION_DEVICES_TESTCOOKIE_H_ */
