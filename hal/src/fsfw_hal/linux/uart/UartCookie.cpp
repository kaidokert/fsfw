#include "fsfw_hal/linux/uart/UartCookie.h"

#include <fsfw/serviceinterface/ServiceInterface.h>

UartCookie::UartCookie(object_id_t handlerId, std::string deviceFile, UartModes uartMode,
        uint32_t baudrate, size_t maxReplyLen):
        handlerId(handlerId), deviceFile(deviceFile), uartMode(uartMode),
        baudrate(baudrate), maxReplyLen(maxReplyLen) {
}

UartCookie::~UartCookie() {}

uint32_t UartCookie::getBaudrate() const {
    return baudrate;
}

size_t UartCookie::getMaxReplyLen() const {
    return maxReplyLen;
}

std::string UartCookie::getDeviceFile() const {
    return deviceFile;
}

void UartCookie::setParityOdd() {
    parity = Parity::ODD;
}

void UartCookie::setParityEven() {
    parity = Parity::EVEN;
}

Parity UartCookie::getParity() const {
    return parity;
}

void UartCookie::setBitsPerWord(uint8_t bitsPerWord_) {
    switch(bitsPerWord_) {
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        sif::debug << "UartCookie::setBitsPerWord: Invalid bits per word specified" << std::endl;
        return;
    }
    bitsPerWord = bitsPerWord_;
}

uint8_t UartCookie::getBitsPerWord() const {
    return bitsPerWord;
}

StopBits UartCookie::getStopBits() const {
    return stopBits;
}

void UartCookie::setTwoStopBits() {
    stopBits = StopBits::TWO_STOP_BITS;
}

void UartCookie::setOneStopBit() {
    stopBits = StopBits::ONE_STOP_BIT;
}

UartModes UartCookie::getUartMode() const {
    return uartMode;
}

void UartCookie::setReadCycles(uint8_t readCycles) {
    this->readCycles = readCycles;
}

void UartCookie::setToFlushInput(bool enable) {
    this->flushInput = enable;
}

uint8_t UartCookie::getReadCycles() const {
    return readCycles;
}

bool UartCookie::getInputShouldBeFlushed() {
    return this->flushInput;
}

object_id_t UartCookie::getHandlerId() const {
    return this->handlerId;
}

void UartCookie::setNoFixedSizeReply() {
    replySizeFixed = false;
}

bool UartCookie::isReplySizeFixed() {
    return replySizeFixed;
}
