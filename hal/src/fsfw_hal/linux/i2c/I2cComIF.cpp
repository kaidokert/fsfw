#include "fsfw_hal/linux/i2c/I2cComIF.h"
#include "fsfw_hal/linux/utility.h"
#include "fsfw_hal/linux/UnixFileGuard.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#include <cstring>


I2cComIF::I2cComIF(object_id_t objectId): SystemObject(objectId){
}

I2cComIF::~I2cComIF() {}

ReturnValue_t I2cComIF::initializeInterface(CookieIF* cookie) {

    address_t i2cAddress;
    std::string deviceFile;

    if(cookie == nullptr) {
        sif::error << "I2cComIF::initializeInterface: Invalid cookie!" << std::endl;
        return NULLPOINTER;
    }
    I2cCookie* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
    if(i2cCookie == nullptr) {
        sif::error << "I2cComIF::initializeInterface: Invalid I2C cookie!" << std::endl;
        return NULLPOINTER;
    }

    i2cAddress = i2cCookie->getAddress();

    i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
    if(i2cDeviceMapIter == i2cDeviceMap.end()) {
        size_t maxReplyLen = i2cCookie->getMaxReplyLen();
        I2cInstance i2cInstance = {std::vector<uint8_t>(maxReplyLen), 0};
        auto statusPair = i2cDeviceMap.emplace(i2cAddress, i2cInstance);
        if (not statusPair.second) {
            sif::error << "I2cComIF::initializeInterface: Failed to insert device with address " <<
                    i2cAddress << "to I2C device " << "map" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        return HasReturnvaluesIF::RETURN_OK;
    }

    sif::error << "I2cComIF::initializeInterface: Device with address " << i2cAddress <<
            "already in use" << std::endl;
    return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t I2cComIF::sendMessage(CookieIF *cookie,
        const uint8_t *sendData, size_t sendLen) {

    ReturnValue_t result;
    int fd;
    std::string deviceFile;

    if(sendData == nullptr) {
        sif::error << "I2cComIF::sendMessage: Send Data is nullptr"
                << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    if(sendLen == 0) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    I2cCookie* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
    if(i2cCookie == nullptr) {
        sif::error << "I2cComIF::sendMessage: Invalid I2C Cookie!" << std::endl;
        return NULLPOINTER;
    }

    address_t i2cAddress = i2cCookie->getAddress();
    i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
    if (i2cDeviceMapIter == i2cDeviceMap.end()) {
        sif::error << "I2cComIF::sendMessage: i2cAddress of Cookie not "
                << "registered in i2cDeviceMap" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    deviceFile = i2cCookie->getDeviceFile();
    UnixFileGuard fileHelper(deviceFile, &fd, O_RDWR, "I2cComIF::sendMessage");
    if(fileHelper.getOpenResult() != HasReturnvaluesIF::RETURN_OK) {
        return fileHelper.getOpenResult();
    }
    result = openDevice(deviceFile, i2cAddress, &fd);
    if (result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }

    if (write(fd, sendData, sendLen) != (int)sendLen) {
        sif::error << "I2cComIF::sendMessage: Failed to send data to I2C "
                "device with error code " << errno << ". Error description: "
                << strerror(errno) << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::getSendSuccess(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::requestReceiveMessage(CookieIF *cookie,
        size_t requestLen) {
    ReturnValue_t result;
    int fd;
    std::string deviceFile;

    if (requestLen == 0) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    I2cCookie* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
    if(i2cCookie == nullptr) {
        sif::error << "I2cComIF::requestReceiveMessage: Invalid I2C Cookie!" << std::endl;
        i2cDeviceMapIter->second.replyLen = 0;
        return NULLPOINTER;
    }

    address_t i2cAddress = i2cCookie->getAddress();
    i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
    if (i2cDeviceMapIter == i2cDeviceMap.end()) {
        sif::error << "I2cComIF::requestReceiveMessage: i2cAddress of Cookie not "
                << "registered in i2cDeviceMap" << std::endl;
        i2cDeviceMapIter->second.replyLen = 0;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    deviceFile = i2cCookie->getDeviceFile();
    UnixFileGuard fileHelper(deviceFile, &fd, O_RDWR, "I2cComIF::requestReceiveMessage");
    if(fileHelper.getOpenResult() != HasReturnvaluesIF::RETURN_OK) {
        return fileHelper.getOpenResult();
    }
    result = openDevice(deviceFile, i2cAddress, &fd);
    if (result != HasReturnvaluesIF::RETURN_OK){
        i2cDeviceMapIter->second.replyLen = 0;
        return result;
    }

    uint8_t* replyBuffer = i2cDeviceMapIter->second.replyBuffer.data();

    int readLen = read(fd, replyBuffer, requestLen);
    if (readLen != static_cast<int>(requestLen)) {
#if FSFW_VERBOSE_LEVEL >= 1 and FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "I2cComIF::requestReceiveMessage: Reading from I2C "
                << "device failed with error code " << errno <<". Description"
                << " of error: " << strerror(errno) << std::endl;
        sif::error << "I2cComIF::requestReceiveMessage: Read only " << readLen << " from "
                << requestLen << " bytes" << std::endl;
#endif
        i2cDeviceMapIter->second.replyLen = 0;
        sif::debug << "I2cComIF::requestReceiveMessage: Read "  << readLen << " of " << requestLen << " bytes" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    i2cDeviceMapIter->second.replyLen = requestLen;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::readReceivedMessage(CookieIF *cookie,
        uint8_t **buffer, size_t* size) {
    I2cCookie* i2cCookie = dynamic_cast<I2cCookie*>(cookie);
    if(i2cCookie == nullptr) {
        sif::error << "I2cComIF::readReceivedMessage: Invalid I2C Cookie!" << std::endl;
        return NULLPOINTER;
    }

    address_t i2cAddress = i2cCookie->getAddress();
    i2cDeviceMapIter = i2cDeviceMap.find(i2cAddress);
    if (i2cDeviceMapIter == i2cDeviceMap.end()) {
        sif::error << "I2cComIF::readReceivedMessage: i2cAddress of Cookie not "
                << "found in i2cDeviceMap" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    *buffer = i2cDeviceMapIter->second.replyBuffer.data();
    *size = i2cDeviceMapIter->second.replyLen;

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t I2cComIF::openDevice(std::string deviceFile,
        address_t i2cAddress, int* fileDescriptor) {

    if (ioctl(*fileDescriptor, I2C_SLAVE, i2cAddress) < 0) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "I2cComIF: Specifying target device failed with error code " << errno << "."
                << std::endl;
        sif::warning << "Error description " << strerror(errno) << std::endl;
#else
        sif::printWarning("I2cComIF: Specifying target device failed with error code %d.\n");
        sif::printWarning("Error description: %s\n", strerror(errno));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
