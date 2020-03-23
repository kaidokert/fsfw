#ifndef COOKIE_H_
#define COOKIE_H_
#include <framework/devicehandlers/DeviceHandlerIF.h>

/**
 * @brief This datatype is used to identify different connection over a single interface
 *        (like RMAP or I2C)
 * @details
 * To use this class, implement a communication specific child cookie. This cookie
 * can be used in the device communication interface by performing
 * a C++ dynamic cast and passing it to a child device handler, which stores
 * it and passes the Cookie to the communication interface where it can be used
 * by again performing a dynamic cast.
 * The cookie can be used to store all kinds of information
 * about the communication, like slave addresses, communication status,
 * communication parameters etc.
 * @ingroup comm
 */
class Cookie{
public:
	Cookie() = default;
	Cookie(address_t logicalAddress_);
	virtual ~Cookie(){}

	void setMaxReplyLen(size_t maxReplyLen_);

	address_t getAddress() const;
	size_t getMaxReplyLen() const;
private:
	address_t logicalAddress = 0;
	size_t maxReplyLen = 0;
};

#endif /* COOKIE_H_ */
