#ifndef FSFW_DEVICEHANDLER_COOKIE_H_
#define FSFW_DEVICEHANDLER_COOKIE_H_

#include <cstdint>

/**
 * @brief Physical address type
 */
using address_t = uint32_t;

/**
 * @brief 	This datatype is used to identify different connection over a
 * 			single interface (like RMAP or I2C)
 * @details
 * To use this class, implement a communication specific child cookie which
 * inherits Cookie. Cookie instances are created in config/Factory.cpp by
 * calling @code{.cpp} CookieIF* childCookie = new ChildCookie(...)
 * @endcode .
 *
 * This cookie is then passed to the child device handlers, which stores the
 * pointer and passes it to the communication interface functions.
 *
 * The cookie can be used to store all kinds of information
 * about the communication, like slave addresses, communication status,
 * communication parameters etc.
 *
 * @ingroup comm
 */
class CookieIF {
 public:
  virtual ~CookieIF(){};
};

#endif /* FSFW_DEVICEHANDLER_COOKIE_H_ */
