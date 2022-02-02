#include "fsfw_hal/linux/i2c/I2cCookie.h"

I2cCookie::I2cCookie(address_t i2cAddress_, size_t maxReplyLen_, std::string deviceFile_)
    : i2cAddress(i2cAddress_), maxReplyLen(maxReplyLen_), deviceFile(deviceFile_) {}

address_t I2cCookie::getAddress() const { return i2cAddress; }

size_t I2cCookie::getMaxReplyLen() const { return maxReplyLen; }

std::string I2cCookie::getDeviceFile() const { return deviceFile; }

I2cCookie::~I2cCookie() {}
