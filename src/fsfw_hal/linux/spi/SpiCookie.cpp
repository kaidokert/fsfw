#include "SpiCookie.h"

SpiCookie::SpiCookie(address_t spiAddress, gpioId_t chipSelect, std::string spiDev,
                     const size_t maxSize, spi::SpiModes spiMode, uint32_t spiSpeed)
    : SpiCookie(spi::SpiComIfModes::REGULAR, spiAddress, chipSelect, spiDev, maxSize, spiMode,
                spiSpeed, nullptr, nullptr) {}

SpiCookie::SpiCookie(address_t spiAddress, std::string spiDev, const size_t maxSize,
                     spi::SpiModes spiMode, uint32_t spiSpeed)
    : SpiCookie(spiAddress, gpio::NO_GPIO, spiDev, maxSize, spiMode, spiSpeed) {}

SpiCookie::SpiCookie(address_t spiAddress, gpioId_t chipSelect, std::string spiDev,
                     const size_t maxSize, spi::SpiModes spiMode, uint32_t spiSpeed,
                     spi::send_callback_function_t callback, void* args)
    : SpiCookie(spi::SpiComIfModes::CALLBACK, spiAddress, chipSelect, spiDev, maxSize, spiMode,
                spiSpeed, callback, args) {}

SpiCookie::SpiCookie(spi::SpiComIfModes comIfMode, address_t spiAddress, gpioId_t chipSelect,
                     std::string spiDev, const size_t maxSize, spi::SpiModes spiMode,
                     uint32_t spiSpeed, spi::send_callback_function_t callback, void* args)
    : spiAddress(spiAddress),
      chipSelectPin(chipSelect),
      spiDevice(spiDev),
      comIfMode(comIfMode),
      maxSize(maxSize),
      spiMode(spiMode),
      spiSpeed(spiSpeed),
      sendCallback(callback),
      callbackArgs(args) {}

spi::SpiComIfModes SpiCookie::getComIfMode() const { return this->comIfMode; }

void SpiCookie::getSpiParameters(spi::SpiModes& spiMode, uint32_t& spiSpeed,
                                 UncommonParameters* parameters) const {
  spiMode = this->spiMode;
  spiSpeed = this->spiSpeed;

  if (parameters != nullptr) {
    parameters->threeWireSpi = uncommonParameters.threeWireSpi;
    parameters->lsbFirst = uncommonParameters.lsbFirst;
    parameters->noCs = uncommonParameters.noCs;
    parameters->bitsPerWord = uncommonParameters.bitsPerWord;
    parameters->csHigh = uncommonParameters.csHigh;
  }
}

gpioId_t SpiCookie::getChipSelectPin() const { return chipSelectPin; }

size_t SpiCookie::getMaxBufferSize() const { return maxSize; }

address_t SpiCookie::getSpiAddress() const { return spiAddress; }

std::string SpiCookie::getSpiDevice() const { return spiDevice; }

void SpiCookie::setThreeWireSpi(bool enable) { uncommonParameters.threeWireSpi = enable; }

void SpiCookie::setLsbFirst(bool enable) { uncommonParameters.lsbFirst = enable; }

void SpiCookie::setNoCs(bool enable) { uncommonParameters.noCs = enable; }

void SpiCookie::setBitsPerWord(uint8_t bitsPerWord) {
  uncommonParameters.bitsPerWord = bitsPerWord;
}

void SpiCookie::setCsHigh(bool enable) { uncommonParameters.csHigh = enable; }

void SpiCookie::activateCsDeselect(bool deselectCs, uint16_t delayUsecs) {
  spiTransferStruct.cs_change = deselectCs;
  spiTransferStruct.delay_usecs = delayUsecs;
}

void SpiCookie::assignReadBuffer(uint8_t* rx) {
  if (rx != nullptr) {
    spiTransferStruct.rx_buf = reinterpret_cast<__u64>(rx);
  }
}

void SpiCookie::assignWriteBuffer(const uint8_t* tx) {
  if (tx != nullptr) {
    spiTransferStruct.tx_buf = reinterpret_cast<__u64>(tx);
  }
}

void SpiCookie::setCallbackMode(spi::send_callback_function_t callback, void* args) {
  this->comIfMode = spi::SpiComIfModes::CALLBACK;
  this->sendCallback = callback;
  this->callbackArgs = args;
}

void SpiCookie::setCallbackArgs(void* args) { this->callbackArgs = args; }

spi_ioc_transfer* SpiCookie::getTransferStructHandle() { return &spiTransferStruct; }

void SpiCookie::setFullOrHalfDuplex(bool halfDuplex) { this->halfDuplex = halfDuplex; }

bool SpiCookie::isFullDuplex() const { return not this->halfDuplex; }

void SpiCookie::setTransferSize(size_t transferSize) { spiTransferStruct.len = transferSize; }

size_t SpiCookie::getCurrentTransferSize() const { return spiTransferStruct.len; }

void SpiCookie::setSpiSpeed(uint32_t newSpeed) { this->spiSpeed = newSpeed; }

void SpiCookie::setSpiMode(spi::SpiModes newMode) { this->spiMode = newMode; }

void SpiCookie::getCallback(spi::send_callback_function_t* callback, void** args) {
  *callback = this->sendCallback;
  *args = this->callbackArgs;
}
