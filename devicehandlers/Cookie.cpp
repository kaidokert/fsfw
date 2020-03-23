/**
 * @file Cookie.cpp
 *
 * @date 23.03.2020
 */
#include <framework/devicehandlers/Cookie.h>

Cookie::Cookie(address_t logicalAddress_): logicalAddress(logicalAddress_) {
}

void Cookie::setAddress(address_t logicalAddress_) {
	logicalAddress = logicalAddress_;
}
void Cookie::setMaxReplyLen(size_t maxReplyLen_) {
	maxReplyLen = maxReplyLen_;
}

address_t Cookie::getAddress() const {
	return logicalAddress;
}

size_t Cookie::getMaxReplyLen() const {
	return maxReplyLen;
}


