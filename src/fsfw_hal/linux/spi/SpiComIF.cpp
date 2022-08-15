#include "fsfw_hal/linux/spi/SpiComIF.h"

#include <fcntl.h>
#include <fsfw/globalfunctions/arrayprinter.h>
#include <fsfw/ipc/MutexFactory.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "fsfw/FSFW.h"
#include "fsfw_hal/linux/UnixFileGuard.h"
#include "fsfw_hal/linux/spi/SpiCookie.h"
#include "fsfw_hal/linux/utility.h"

SpiComIF::SpiComIF(object_id_t objectId, GpioIF* gpioComIF)
    : SystemObject(objectId), gpioComIF(gpioComIF) {
  if (gpioComIF == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "SpiComIF::SpiComIF: GPIO communication interface invalid!" << std::endl;
#else
    sif::printError("SpiComIF::SpiComIF: GPIO communication interface invalid!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
  }

  spiMutex = MutexFactory::instance()->createMutex();
}

ReturnValue_t SpiComIF::initializeInterface(CookieIF* cookie) {
  int retval = 0;
  SpiCookie* spiCookie = dynamic_cast<SpiCookie*>(cookie);
  if (spiCookie == nullptr) {
    return NULLPOINTER;
  }

  address_t spiAddress = spiCookie->getSpiAddress();

  auto iter = spiDeviceMap.find(spiAddress);
  if (iter == spiDeviceMap.end()) {
    size_t bufferSize = spiCookie->getMaxBufferSize();
    SpiInstance spiInstance(bufferSize);
    auto statusPair = spiDeviceMap.emplace(spiAddress, spiInstance);
    if (not statusPair.second) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "SpiComIF::initializeInterface: Failed to insert device with address "
                 << spiAddress << "to SPI device map" << std::endl;
#else
      sif::printError(
          "SpiComIF::initializeInterface: Failed to insert device with address "
          "%lu to SPI device map\n",
          static_cast<unsigned long>(spiAddress));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
      return returnvalue::FAILED;
    }
    /* Now we emplaced the read buffer in the map, we still need to assign that location
    to the SPI driver transfer struct */
    spiCookie->assignReadBuffer(statusPair.first->second.replyBuffer.data());
  } else {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "SpiComIF::initializeInterface: SPI address already exists!" << std::endl;
#else
    sif::printError("SpiComIF::initializeInterface: SPI address already exists!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return returnvalue::FAILED;
  }

  /* Pull CS high in any case to be sure that device is inactive */
  gpioId_t gpioId = spiCookie->getChipSelectPin();
  if (gpioId != gpio::NO_GPIO) {
    gpioComIF->pullHigh(gpioId);
  }

  uint32_t spiSpeed = 0;
  spi::SpiModes spiMode = spi::SpiModes::MODE_0;

  SpiCookie::UncommonParameters params;
  spiCookie->getSpiParameters(spiMode, spiSpeed, &params);

  int fileDescriptor = 0;
  UnixFileGuard fileHelper(spiCookie->getSpiDevice(), &fileDescriptor, O_RDWR,
                           "SpiComIF::initializeInterface");
  if (fileHelper.getOpenResult() != returnvalue::OK) {
    return fileHelper.getOpenResult();
  }

  /* These flags are rather uncommon */
  if (params.threeWireSpi or params.noCs or params.csHigh) {
    uint32_t currentMode = 0;
    retval = ioctl(fileDescriptor, SPI_IOC_RD_MODE32, &currentMode);
    if (retval != 0) {
      utility::handleIoctlError("SpiComIF::initialiezInterface: Could not read full mode!");
    }

    if (params.threeWireSpi) {
      currentMode |= SPI_3WIRE;
    }
    if (params.noCs) {
      /* Some drivers like the Raspberry Pi ignore this flag in any case */
      currentMode |= SPI_NO_CS;
    }
    if (params.csHigh) {
      currentMode |= SPI_CS_HIGH;
    }
    /* Write adapted mode */
    retval = ioctl(fileDescriptor, SPI_IOC_WR_MODE32, &currentMode);
    if (retval != 0) {
      utility::handleIoctlError("SpiComIF::initialiezInterface: Could not write full mode!");
    }
  }
  if (params.lsbFirst) {
    retval = ioctl(fileDescriptor, SPI_IOC_WR_LSB_FIRST, &params.lsbFirst);
    if (retval != 0) {
      utility::handleIoctlError("SpiComIF::initializeInterface: Setting LSB first failed");
    }
  }
  if (params.bitsPerWord != 8) {
    retval = ioctl(fileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &params.bitsPerWord);
    if (retval != 0) {
      utility::handleIoctlError(
          "SpiComIF::initializeInterface: "
          "Could not write bits per word!");
    }
  }
  return returnvalue::OK;
}

ReturnValue_t SpiComIF::sendMessage(CookieIF* cookie, const uint8_t* sendData, size_t sendLen) {
  SpiCookie* spiCookie = dynamic_cast<SpiCookie*>(cookie);
  ReturnValue_t result = returnvalue::OK;

  if (spiCookie == nullptr) {
    return NULLPOINTER;
  }

  if (sendLen > spiCookie->getMaxBufferSize()) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "SpiComIF::sendMessage: Too much data sent, send length " << sendLen
                 << "larger than maximum buffer length " << spiCookie->getMaxBufferSize()
                 << std::endl;
#else
    sif::printWarning(
        "SpiComIF::sendMessage: Too much data sent, send length %lu larger "
        "than maximum buffer length %lu!\n",
        static_cast<unsigned long>(sendLen),
        static_cast<unsigned long>(spiCookie->getMaxBufferSize()));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return DeviceCommunicationIF::TOO_MUCH_DATA;
  }

  if (spiCookie->getComIfMode() == spi::SpiComIfModes::REGULAR) {
    result = performRegularSendOperation(spiCookie, sendData, sendLen);
  } else if (spiCookie->getComIfMode() == spi::SpiComIfModes::CALLBACK) {
    spi::send_callback_function_t sendFunc = nullptr;
    void* funcArgs = nullptr;
    spiCookie->getCallback(&sendFunc, &funcArgs);
    if (sendFunc != nullptr) {
      result = sendFunc(this, spiCookie, sendData, sendLen, funcArgs);
    }
  }
  return result;
}

ReturnValue_t SpiComIF::performRegularSendOperation(SpiCookie* spiCookie, const uint8_t* sendData,
                                                    size_t sendLen) {
  address_t spiAddress = spiCookie->getSpiAddress();
  auto iter = spiDeviceMap.find(spiAddress);
  if (iter != spiDeviceMap.end()) {
    spiCookie->assignReadBuffer(iter->second.replyBuffer.data());
  }

  ReturnValue_t result = returnvalue::OK;
  int retval = 0;
  /* Prepare transfer */
  int fileDescriptor = 0;
  std::string device = spiCookie->getSpiDevice();
  UnixFileGuard fileHelper(device, &fileDescriptor, O_RDWR, "SpiComIF::sendMessage");
  if (fileHelper.getOpenResult() != returnvalue::OK) {
    return OPENING_FILE_FAILED;
  }
  spi::SpiModes spiMode = spi::SpiModes::MODE_0;
  uint32_t spiSpeed = 0;
  spiCookie->getSpiParameters(spiMode, spiSpeed, nullptr);
  setSpiSpeedAndMode(fileDescriptor, spiMode, spiSpeed);
  spiCookie->assignWriteBuffer(sendData);
  spiCookie->setTransferSize(sendLen);

  bool fullDuplex = spiCookie->isFullDuplex();
  gpioId_t gpioId = spiCookie->getChipSelectPin();

  /* Pull SPI CS low. For now, no support for active high given  */
  if (gpioId != gpio::NO_GPIO) {
    result = spiMutex->lockMutex(timeoutType, timeoutMs);
    if (result != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "SpiComIF::sendMessage: Failed to lock mutex" << std::endl;
#else
      sif::printError("SpiComIF::sendMessage: Failed to lock mutex\n");
#endif
#endif
      return result;
    }
    result = gpioComIF->pullLow(gpioId);
    if (result != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "SpiComIF::sendMessage: Pulling low CS pin failed" << std::endl;
#else
      sif::printWarning("SpiComIF::sendMessage: Pulling low CS pin failed");
#endif
#endif
      return result;
    }
  }

  /* Execute transfer */
  if (fullDuplex) {
    /* Initiate a full duplex SPI transfer. */
    retval = ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), spiCookie->getTransferStructHandle());
    if (retval < 0) {
      utility::handleIoctlError("SpiComIF::sendMessage: ioctl error.");
      result = FULL_DUPLEX_TRANSFER_FAILED;
    }
#if FSFW_HAL_SPI_WIRETAPPING == 1
    performSpiWiretapping(spiCookie);
#endif /* FSFW_LINUX_SPI_WIRETAPPING == 1 */
  } else {
    /* We write with a blocking half-duplex transfer here */
    if (write(fileDescriptor, sendData, sendLen) != static_cast<ssize_t>(sendLen)) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "SpiComIF::sendMessage: Half-Duplex write operation failed!" << std::endl;
#else
      sif::printWarning("SpiComIF::sendMessage: Half-Duplex write operation failed!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
      result = HALF_DUPLEX_TRANSFER_FAILED;
    }
  }

  if (gpioId != gpio::NO_GPIO) {
    gpioComIF->pullHigh(gpioId);
    result = spiMutex->unlockMutex();
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "SpiComIF::sendMessage: Failed to unlock mutex" << std::endl;
#endif
      return result;
    }
  }
  return result;
}

ReturnValue_t SpiComIF::getSendSuccess(CookieIF* cookie) { return returnvalue::OK; }

ReturnValue_t SpiComIF::requestReceiveMessage(CookieIF* cookie, size_t requestLen) {
  SpiCookie* spiCookie = dynamic_cast<SpiCookie*>(cookie);
  if (spiCookie == nullptr) {
    return NULLPOINTER;
  }

  if (spiCookie->isFullDuplex()) {
    return returnvalue::OK;
  }

  return performHalfDuplexReception(spiCookie);
}

ReturnValue_t SpiComIF::performHalfDuplexReception(SpiCookie* spiCookie) {
  ReturnValue_t result = returnvalue::OK;
  std::string device = spiCookie->getSpiDevice();
  int fileDescriptor = 0;
  UnixFileGuard fileHelper(device, &fileDescriptor, O_RDWR, "SpiComIF::requestReceiveMessage");
  if (fileHelper.getOpenResult() != returnvalue::OK) {
    return OPENING_FILE_FAILED;
  }

  uint8_t* rxBuf = nullptr;
  size_t readSize = spiCookie->getCurrentTransferSize();
  result = getReadBuffer(spiCookie->getSpiAddress(), &rxBuf);
  if (result != returnvalue::OK) {
    return result;
  }

  gpioId_t gpioId = spiCookie->getChipSelectPin();
  if (gpioId != gpio::NO_GPIO) {
    result = spiMutex->lockMutex(timeoutType, timeoutMs);
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "SpiComIF::getSendSuccess: Failed to lock mutex" << std::endl;
#endif
      return result;
    }
    gpioComIF->pullLow(gpioId);
  }

  if (read(fileDescriptor, rxBuf, readSize) != static_cast<ssize_t>(readSize)) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "SpiComIF::sendMessage: Half-Duplex read operation failed!" << std::endl;
#else
    sif::printWarning("SpiComIF::sendMessage: Half-Duplex read operation failed!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    result = HALF_DUPLEX_TRANSFER_FAILED;
  }

  if (gpioId != gpio::NO_GPIO) {
    gpioComIF->pullHigh(gpioId);
    result = spiMutex->unlockMutex();
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "SpiComIF::getSendSuccess: Failed to unlock mutex" << std::endl;
#endif
      return result;
    }
  }

  return result;
}

ReturnValue_t SpiComIF::readReceivedMessage(CookieIF* cookie, uint8_t** buffer, size_t* size) {
  SpiCookie* spiCookie = dynamic_cast<SpiCookie*>(cookie);
  if (spiCookie == nullptr) {
    return returnvalue::FAILED;
  }
  uint8_t* rxBuf = nullptr;
  ReturnValue_t result = getReadBuffer(spiCookie->getSpiAddress(), &rxBuf);
  if (result != returnvalue::OK) {
    return result;
  }

  *buffer = rxBuf;
  *size = spiCookie->getCurrentTransferSize();
  spiCookie->setTransferSize(0);
  return returnvalue::OK;
}

MutexIF* SpiComIF::getMutex(MutexIF::TimeoutType* timeoutType, uint32_t* timeoutMs) {
  if (timeoutType != nullptr) {
    *timeoutType = this->timeoutType;
  }
  if (timeoutMs != nullptr) {
    *timeoutMs = this->timeoutMs;
  }
  return spiMutex;
}

void SpiComIF::performSpiWiretapping(SpiCookie* spiCookie) {
  if (spiCookie == nullptr) {
    return;
  }
  size_t dataLen = spiCookie->getTransferStructHandle()->len;
  uint8_t* dataPtr = reinterpret_cast<uint8_t*>(spiCookie->getTransferStructHandle()->tx_buf);
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "Sent SPI data: " << std::endl;
  arrayprinter::print(dataPtr, dataLen, OutputType::HEX, false);
  sif::info << "Received SPI data: " << std::endl;
#else
  sif::printInfo("Sent SPI data: \n");
  arrayprinter::print(dataPtr, dataLen, OutputType::HEX, false);
  sif::printInfo("Received SPI data: \n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
  dataPtr = reinterpret_cast<uint8_t*>(spiCookie->getTransferStructHandle()->rx_buf);
  arrayprinter::print(dataPtr, dataLen, OutputType::HEX, false);
}

ReturnValue_t SpiComIF::getReadBuffer(address_t spiAddress, uint8_t** buffer) {
  if (buffer == nullptr) {
    return returnvalue::FAILED;
  }

  auto iter = spiDeviceMap.find(spiAddress);
  if (iter == spiDeviceMap.end()) {
    return returnvalue::FAILED;
  }

  *buffer = iter->second.replyBuffer.data();
  return returnvalue::OK;
}

GpioIF* SpiComIF::getGpioInterface() { return gpioComIF; }

void SpiComIF::setSpiSpeedAndMode(int spiFd, spi::SpiModes mode, uint32_t speed) {
  int retval = ioctl(spiFd, SPI_IOC_WR_MODE, reinterpret_cast<uint8_t*>(&mode));
  if (retval != 0) {
    utility::handleIoctlError("SpiComIF::setSpiSpeedAndMode: Setting SPI mode failed");
  }

  retval = ioctl(spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (retval != 0) {
    utility::handleIoctlError("SpiComIF::setSpiSpeedAndMode: Setting SPI speed failed");
  }
  // This updates the SPI clock default polarity. Only setting the mode does not update
  // the line state, which can be an issue on mode switches because the clock line will
  // switch the state after the chip select is pulled low
  clockUpdateTransfer.len = 0;
  retval = ioctl(spiFd, SPI_IOC_MESSAGE(1), &clockUpdateTransfer);
  if (retval != 0) {
    utility::handleIoctlError("SpiComIF::setSpiSpeedAndMode: Updating SPI default clock failed");
  }
}
