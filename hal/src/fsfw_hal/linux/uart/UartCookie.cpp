#include "UartCookie.h"

#include <fsfw/serviceinterface.h>

UartCookie::UartCookie(object_id_t handlerId, std::string deviceFile, UartModes uartMode,
                       UartBaudRate baudrate, size_t maxReplyLen)
    : handlerId(handlerId),
      deviceFile(deviceFile),
      uartMode(uartMode),
      baudrate(baudrate),
      maxReplyLen(maxReplyLen) {}

UartCookie::~UartCookie() {}

UartBaudRate UartCookie::getBaudrate() const { return baudrate; }

size_t UartCookie::getMaxReplyLen() const { return maxReplyLen; }

std::string UartCookie::getDeviceFile() const { return deviceFile; }

void UartCookie::setParityOdd() { parity = Parity::ODD; }

void UartCookie::setParityEven() { parity = Parity::EVEN; }

Parity UartCookie::getParity() const { return parity; }

void UartCookie::setBitsPerWord(BitsPerWord bitsPerWord_) {
  bitsPerWord = bitsPerWord_;
}

BitsPerWord UartCookie::getBitsPerWord() const { return bitsPerWord; }

StopBits UartCookie::getStopBits() const { return stopBits; }

void UartCookie::setTwoStopBits() { stopBits = StopBits::TWO_STOP_BITS; }

void UartCookie::setOneStopBit() { stopBits = StopBits::ONE_STOP_BIT; }

UartModes UartCookie::getUartMode() const { return uartMode; }

void UartCookie::setReadCycles(uint8_t readCycles) { this->readCycles = readCycles; }

void UartCookie::setToFlushInput(bool enable) { this->flushInput = enable; }

uint8_t UartCookie::getReadCycles() const { return readCycles; }

bool UartCookie::getInputShouldBeFlushed() { return this->flushInput; }

object_id_t UartCookie::getHandlerId() const { return this->handlerId; }

void UartCookie::setNoFixedSizeReply() { replySizeFixed = false; }

bool UartCookie::isReplySizeFixed() { return replySizeFixed; }
