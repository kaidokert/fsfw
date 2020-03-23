/**
 * @file CookieIF.h
 *
 * @date 23.03.2020
 */

#ifndef FRAMEWORK_DEVICEHANDLERS_COOKIEIF_H_
#define FRAMEWORK_DEVICEHANDLERS_COOKIEIF_H_
#include <framework/devicehandlers/DeviceHandlerIF.h>

class CookieIF {
public:
	/**
	 * Default empty virtual destructor.
	 */
	virtual ~CookieIF() {};

	virtual void setAddress(address_t logicalAddress_) = 0;
	virtual address_t getAddress() const = 0;

	virtual void setMaxReplyLen(size_t maxReplyLen_) = 0;
	virtual size_t getMaxReplyLen() const = 0;
};

#endif /* FRAMEWORK_DEVICEHANDLERS_COOKIEIF_H_ */
