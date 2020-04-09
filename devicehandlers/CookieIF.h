#ifndef COOKIE_H_
#define COOKIE_H_
#include <framework/devicehandlers/CookieIF.h>
#include <stdint.h>

/**
 * @brief Physical address type
 */
typedef uint32_t address_t;

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
	virtual ~CookieIF() {};
};

#endif /* COOKIE_H_ */
