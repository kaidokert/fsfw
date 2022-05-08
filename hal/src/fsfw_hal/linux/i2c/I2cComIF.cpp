#include "I2cComIF.h"

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface.h"
#include "fsfw_hal/linux/UnixFileGuard.h"
#include "fsfw_hal/linux/utility.h"

#if FSFW_HAL_I2C_WIRETAPPING == 1
#include "fsfw/globalfunctions/arrayprinter.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>

I2cComIF::I2cComIF(object_id_t objectId) : SystemObject(objectId) {}

I2cComIF::~I2cComIF() {}

ReturnValue_t I2cComIF::initializeInterface(CookieIF* cookie) {
  address_t i2cAddress;
  std::string deviceFile;

  if (cookie == nullptr) {
    FSFW_FLOGE("{}", "initializeInterface: Invalid cookie\n");
    return NULLPOINTER;
  }
  auto* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
  i2cAddress = i2cCookie->getAddress();

  i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
  if (i2cDeviceMapIter == i2cDeviceMap.end()) {
    size_t maxReplyLen = i2cCookie->getMaxReplyLen();
    I2cInstance i2cInstance = {std::vector<uint8_t>(maxReplyLen), 0};
    auto statusPair = i2cDeviceMap.emplace(i2cAddress, i2cInstance);
    if (not statusPair.second) {
      FSFW_FLOGW("initializeInterface: Failed to insert device with address {} to I2C device map\n",
                 i2cAddress);
      return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
  }

  FSFW_FLOGE("initializeInterface: Device with address {} already in use\n", i2cAddress);
  return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t I2cComIF::sendMessage(CookieIF* cookie, const uint8_t* sendData, size_t sendLen) {
  ReturnValue_t result;
  int fd;
  std::string deviceFile;

  if (sendData == nullptr) {
    FSFW_FLOGW("{}", "sendMessage: Send Data is nullptr\n");
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  if (sendLen == 0) {
    return HasReturnvaluesIF::RETURN_OK;
  }

  auto* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
  if (i2cCookie == nullptr) {
    FSFW_FLOGWT("{}", "sendMessage: Invalid I2C Cookie\n");
    return NULLPOINTER;
  }

  address_t i2cAddress = i2cCookie->getAddress();
  i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
  if (i2cDeviceMapIter == i2cDeviceMap.end()) {
    FSFW_FLOGWT("{}", "sendMessage: I2C address of cookie not registered in I2C device map\n");
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  deviceFile = i2cCookie->getDeviceFile();
  UnixFileGuard fileHelper(deviceFile, &fd, O_RDWR, "I2cComIF::sendMessage");
  if (fileHelper.getOpenResult() != HasReturnvaluesIF::RETURN_OK) {
    return fileHelper.getOpenResult();
  }
  result = openDevice(deviceFile, i2cAddress, &fd);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  if (write(fd, sendData, sendLen) != static_cast<int>(sendLen)) {
    FSFW_FLOGE("sendMessage: Failed to send data to I2C device with error code {} | {}\n", errno,
               strerror(errno));
    return HasReturnvaluesIF::RETURN_FAILED;
  }

#if FSFW_HAL_I2C_WIRETAPPING == 1
  sif::info << "Sent I2C data to bus " << deviceFile << ":" << std::endl;
  arrayprinter::print(sendData, sendLen);
#endif
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::getSendSuccess(CookieIF* cookie) { return HasReturnvaluesIF::RETURN_OK; }

ReturnValue_t I2cComIF::requestReceiveMessage(CookieIF* cookie, size_t requestLen) {
  ReturnValue_t result;
  int fd;
  std::string deviceFile;

  if (requestLen == 0) {
    return HasReturnvaluesIF::RETURN_OK;
  }

  auto* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
  if (i2cCookie == nullptr) {
    FSFW_FLOGWT("{}", "requestReceiveMessage: Invalid I2C Cookie\n");
    i2cDeviceMapIter->second.replyLen = 0;
    return NULLPOINTER;
  }

  address_t i2cAddress = i2cCookie->getAddress();
  i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
  if (i2cDeviceMapIter == i2cDeviceMap.end()) {
    FSFW_FLOGW("requestReceiveMessage: I2C address {} of Cookie not registered in i2cDeviceMap",
               i2cAddress);
    i2cDeviceMapIter->second.replyLen = 0;
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  deviceFile = i2cCookie->getDeviceFile();
  UnixFileGuard fileHelper(deviceFile, &fd, O_RDWR, "I2cComIF::requestReceiveMessage");
  if (fileHelper.getOpenResult() != HasReturnvaluesIF::RETURN_OK) {
    return fileHelper.getOpenResult();
  }
  result = openDevice(deviceFile, i2cAddress, &fd);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    i2cDeviceMapIter->second.replyLen = 0;
    return result;
  }

  uint8_t* replyBuffer = i2cDeviceMapIter->second.replyBuffer.data();

  ssize_t readLen = read(fd, replyBuffer, requestLen);
  if (readLen != static_cast<int>(requestLen)) {
    FSFW_FLOGWT(
        "requestReceiveMessage: Reading from I2C device failed with error code "
        "{} | {}\nRead only {} from {} bytes\n",
        errno, strerror(errno), readLen, requestLen);
    i2cDeviceMapIter->second.replyLen = 0;
    return HasReturnvaluesIF::RETURN_FAILED;
  }

#if FSFW_HAL_I2C_WIRETAPPING == 1
  sif::info << "I2C read bytes from bus " << deviceFile << ":" << std::endl;
  arrayprinter::print(replyBuffer, requestLen);
#endif

  i2cDeviceMapIter->second.replyLen = requestLen;
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::readReceivedMessage(CookieIF* cookie, uint8_t** buffer, size_t* size) {
  auto* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
  if (i2cCookie == nullptr) {
    FSFW_FLOGW("{}", "readReceivedMessage: Invalid I2C Cookie\n");
    return NULLPOINTER;
  }

  address_t i2cAddress = i2cCookie->getAddress();
  i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
  if (i2cDeviceMapIter == i2cDeviceMap.end()) {
    FSFW_FLOGE("readReceivedMessage: I2C address {} of cookie not found in I2C device map\n",
               i2cAddress);
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  *buffer = i2cDeviceMapIter->second.replyBuffer.data();
  *size = i2cDeviceMapIter->second.replyLen;

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::openDevice(std::string deviceFile, address_t i2cAddress,
                                   int* fileDescriptor) {
  if (ioctl(*fileDescriptor, I2C_SLAVE, i2cAddress) < 0) {
    FSFW_FLOGWT("openDevice: Specifying target device failed with error code {} | {}\n", errno,
                strerror(errno));
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return HasReturnvaluesIF::RETURN_OK;
}
