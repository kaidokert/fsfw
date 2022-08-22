#include "fsfw_hal/stm32h7/spi/SpiComIF.h"

#include "fsfw/tasks/SemaphoreFactory.h"
#include "fsfw_hal/stm32h7/gpio/gpio.h"
#include "fsfw_hal/stm32h7/spi/SpiCookie.h"
#include "fsfw_hal/stm32h7/spi/mspInit.h"
#include "fsfw_hal/stm32h7/spi/spiCore.h"
#include "fsfw_hal/stm32h7/spi/spiInterrupts.h"

// FreeRTOS required special Semaphore handling from an ISR. Therefore, we use the concrete
// instance here, because RTEMS and FreeRTOS are the only relevant OSALs currently
// and it is not trivial to add a releaseFromISR to the SemaphoreIF
#if defined FSFW_OSAL_RTEMS
#include "fsfw/osal/rtems/BinarySemaphore.h"
#elif defined FSFW_OSAL_FREERTOS
#include "fsfw/osal/freertos/BinarySemaphore.h"
#include "fsfw/osal/freertos/TaskManagement.h"
#endif

#include "stm32h7xx_hal_gpio.h"

SpiComIF::SpiComIF(object_id_t objectId) : SystemObject(objectId) {
  void *irqArgsVoided = reinterpret_cast<void *>(&irqArgs);
  spi::assignTransferRxTxCompleteCallback(&spiTransferCompleteCallback, irqArgsVoided);
  spi::assignTransferRxCompleteCallback(&spiTransferRxCompleteCallback, irqArgsVoided);
  spi::assignTransferTxCompleteCallback(&spiTransferTxCompleteCallback, irqArgsVoided);
  spi::assignTransferErrorCallback(&spiTransferErrorCallback, irqArgsVoided);
}

void SpiComIF::configureCacheMaintenanceOnTxBuffer(bool enable) {
  this->cacheMaintenanceOnTxBuffer = enable;
}

void SpiComIF::addDmaHandles(DMA_HandleTypeDef *txHandle, DMA_HandleTypeDef *rxHandle) {
  spi::setDmaHandles(txHandle, rxHandle);
}

ReturnValue_t SpiComIF::initialize() { return returnvalue::OK; }

ReturnValue_t SpiComIF::initializeInterface(CookieIF *cookie) {
  SpiCookie *spiCookie = dynamic_cast<SpiCookie *>(cookie);
  if (spiCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error < "SpiComIF::initializeInterface: Invalid cookie" << std::endl;
#else
    sif::printError("SpiComIF::initializeInterface: Invalid cookie\n");
#endif
    return NULLPOINTER;
  }
  auto transferMode = spiCookie->getTransferMode();

  if (transferMode == spi::TransferModes::DMA) {
    DMA_HandleTypeDef *txHandle = nullptr;
    DMA_HandleTypeDef *rxHandle = nullptr;
    spi::getDmaHandles(&txHandle, &rxHandle);
    if (txHandle == nullptr or rxHandle == nullptr) {
      sif::printError("SpiComIF::initialize: DMA handles not set!\n");
      return returnvalue::FAILED;
    }
  }
  // This semaphore ensures thread-safety for a given bus
  spiSemaphore =
      dynamic_cast<BinarySemaphore *>(SemaphoreFactory::instance()->createBinarySemaphore());
  address_t spiAddress = spiCookie->getDeviceAddress();

  auto iter = spiDeviceMap.find(spiAddress);
  if (iter == spiDeviceMap.end()) {
    size_t bufferSize = spiCookie->getMaxRecvSize();
    auto statusPair = spiDeviceMap.emplace(spiAddress, SpiInstance(bufferSize));
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
  }
  auto gpioPin = spiCookie->getChipSelectGpioPin();
  auto gpioPort = spiCookie->getChipSelectGpioPort();

  SPI_HandleTypeDef &spiHandle = spiCookie->getSpiHandle();

  auto spiIdx = spiCookie->getSpiIdx();
  if (spiIdx == spi::SpiBus::SPI_1) {
#ifdef SPI1
    spiHandle.Instance = SPI1;
#endif
  } else if (spiIdx == spi::SpiBus::SPI_2) {
#ifdef SPI2
    spiHandle.Instance = SPI2;
#endif
  } else {
    printCfgError("SPI Bus Index");
    return returnvalue::FAILED;
  }

  auto mspCfg = spiCookie->getMspCfg();

  if (transferMode == spi::TransferModes::POLLING) {
    auto typedCfg = dynamic_cast<spi::MspPollingConfigStruct *>(mspCfg);
    if (typedCfg == nullptr) {
      printCfgError("Polling MSP");
      return returnvalue::FAILED;
    }
    spi::setSpiPollingMspFunctions(typedCfg);
  } else if (transferMode == spi::TransferModes::INTERRUPT) {
    auto typedCfg = dynamic_cast<spi::MspIrqConfigStruct *>(mspCfg);
    if (typedCfg == nullptr) {
      printCfgError("IRQ MSP");
      return returnvalue::FAILED;
    }
    spi::setSpiIrqMspFunctions(typedCfg);
  } else if (transferMode == spi::TransferModes::DMA) {
    auto typedCfg = dynamic_cast<spi::MspDmaConfigStruct *>(mspCfg);
    if (typedCfg == nullptr) {
      printCfgError("DMA MSP");
      return returnvalue::FAILED;
    }
    // Check DMA handles
    DMA_HandleTypeDef *txHandle = nullptr;
    DMA_HandleTypeDef *rxHandle = nullptr;
    spi::getDmaHandles(&txHandle, &rxHandle);
    if (txHandle == nullptr or rxHandle == nullptr) {
      printCfgError("DMA Handle");
      return returnvalue::FAILED;
    }
    spi::setSpiDmaMspFunctions(typedCfg);
  }

  if (gpioPort != nullptr) {
    gpio::initializeGpioClock(gpioPort);
    GPIO_InitTypeDef chipSelect = {};
    chipSelect.Pin = gpioPin;
    chipSelect.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(gpioPort, &chipSelect);
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
  }

  if (HAL_SPI_Init(&spiHandle) != HAL_OK) {
    sif::printWarning("SpiComIF::initialize: Error initializing SPI\n");
    return returnvalue::FAILED;
  }
  // The MSP configuration struct is not required anymore
  spiCookie->deleteMspCfg();

  return returnvalue::OK;
}

ReturnValue_t SpiComIF::sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) {
  SpiCookie *spiCookie = dynamic_cast<SpiCookie *>(cookie);
  if (spiCookie == nullptr) {
    return NULLPOINTER;
  }

  SPI_HandleTypeDef &spiHandle = spiCookie->getSpiHandle();

  auto iter = spiDeviceMap.find(spiCookie->getDeviceAddress());
  if (iter == spiDeviceMap.end()) {
    return returnvalue::FAILED;
  }
  iter->second.currentTransferLen = sendLen;

  auto transferMode = spiCookie->getTransferMode();
  switch (spiCookie->getTransferState()) {
    case (spi::TransferStates::IDLE): {
      break;
    }
    case (spi::TransferStates::WAIT):
    case (spi::TransferStates::FAILURE):
    case (spi::TransferStates::SUCCESS):
    default: {
      return returnvalue::FAILED;
    }
  }

  switch (transferMode) {
    case (spi::TransferModes::POLLING): {
      return handlePollingSendOperation(iter->second.replyBuffer.data(), spiHandle, *spiCookie,
                                        sendData, sendLen);
    }
    case (spi::TransferModes::INTERRUPT): {
      return handleInterruptSendOperation(iter->second.replyBuffer.data(), spiHandle, *spiCookie,
                                          sendData, sendLen);
    }
    case (spi::TransferModes::DMA): {
      return handleDmaSendOperation(iter->second.replyBuffer.data(), spiHandle, *spiCookie,
                                    sendData, sendLen);
    }
  }
  return returnvalue::OK;
}

ReturnValue_t SpiComIF::getSendSuccess(CookieIF *cookie) { return returnvalue::OK; }

ReturnValue_t SpiComIF::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
  return returnvalue::OK;
}

ReturnValue_t SpiComIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) {
  SpiCookie *spiCookie = dynamic_cast<SpiCookie *>(cookie);
  if (spiCookie == nullptr) {
    return NULLPOINTER;
  }
  switch (spiCookie->getTransferState()) {
    case (spi::TransferStates::SUCCESS): {
      auto iter = spiDeviceMap.find(spiCookie->getDeviceAddress());
      if (iter == spiDeviceMap.end()) {
        return returnvalue::FAILED;
      }
      *buffer = iter->second.replyBuffer.data();
      *size = iter->second.currentTransferLen;
      spiCookie->setTransferState(spi::TransferStates::IDLE);
      break;
    }
    case (spi::TransferStates::FAILURE): {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "SpiComIF::readReceivedMessage: Transfer failure" << std::endl;
#else
      sif::printWarning("SpiComIF::readReceivedMessage: Transfer failure\n");
#endif
#endif
      spiCookie->setTransferState(spi::TransferStates::IDLE);
      return returnvalue::FAILED;
    }
    case (spi::TransferStates::WAIT):
    case (spi::TransferStates::IDLE): {
      break;
    }
    default: {
      return returnvalue::FAILED;
    }
  }

  return returnvalue::OK;
}

void SpiComIF::setDefaultPollingTimeout(dur_millis_t timeout) {
  this->defaultPollingTimeout = timeout;
}

ReturnValue_t SpiComIF::handlePollingSendOperation(uint8_t *recvPtr, SPI_HandleTypeDef &spiHandle,
                                                   SpiCookie &spiCookie, const uint8_t *sendData,
                                                   size_t sendLen) {
  auto gpioPort = spiCookie.getChipSelectGpioPort();
  auto gpioPin = spiCookie.getChipSelectGpioPin();
  auto returnval = spiSemaphore->acquire(timeoutType, timeoutMs);
  if (returnval != returnvalue::OK) {
    return returnval;
  }
  spiCookie.setTransferState(spi::TransferStates::WAIT);
  if (gpioPort != nullptr) {
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);
  }

  auto result = HAL_SPI_TransmitReceive(&spiHandle, const_cast<uint8_t *>(sendData), recvPtr,
                                        sendLen, defaultPollingTimeout);
  if (gpioPort != nullptr) {
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
  }
  spiSemaphore->release();
  switch (result) {
    case (HAL_OK): {
      spiCookie.setTransferState(spi::TransferStates::SUCCESS);
      break;
    }
    case (HAL_TIMEOUT): {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "SpiComIF::sendMessage: Polling Mode | Timeout for SPI device"
                   << spiCookie->getDeviceAddress() << std::endl;
#else
      sif::printWarning("SpiComIF::sendMessage: Polling Mode | Timeout for SPI device %d\n",
                        spiCookie.getDeviceAddress());
#endif
#endif
      spiCookie.setTransferState(spi::TransferStates::FAILURE);
      return spi::HAL_TIMEOUT_RETVAL;
    }
    case (HAL_ERROR):
    default: {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "SpiComIF::sendMessage: Polling Mode | HAL error for SPI device"
                   << spiCookie->getDeviceAddress() << std::endl;
#else
      sif::printWarning("SpiComIF::sendMessage: Polling Mode | HAL error for SPI device %d\n",
                        spiCookie.getDeviceAddress());
#endif
#endif
      spiCookie.setTransferState(spi::TransferStates::FAILURE);
      return spi::HAL_ERROR_RETVAL;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t SpiComIF::handleInterruptSendOperation(uint8_t *recvPtr, SPI_HandleTypeDef &spiHandle,
                                                     SpiCookie &spiCookie, const uint8_t *sendData,
                                                     size_t sendLen) {
  return handleIrqSendOperation(recvPtr, spiHandle, spiCookie, sendData, sendLen);
}

ReturnValue_t SpiComIF::handleDmaSendOperation(uint8_t *recvPtr, SPI_HandleTypeDef &spiHandle,
                                               SpiCookie &spiCookie, const uint8_t *sendData,
                                               size_t sendLen) {
  return handleIrqSendOperation(recvPtr, spiHandle, spiCookie, sendData, sendLen);
}

ReturnValue_t SpiComIF::handleIrqSendOperation(uint8_t *recvPtr, SPI_HandleTypeDef &spiHandle,
                                               SpiCookie &spiCookie, const uint8_t *sendData,
                                               size_t sendLen) {
  ReturnValue_t result = genericIrqSendSetup(recvPtr, spiHandle, spiCookie, sendData, sendLen);
  if (result != returnvalue::OK) {
    return result;
  }
  // yet another HAL driver which is not const-correct..
  HAL_StatusTypeDef status = HAL_OK;
  auto transferMode = spiCookie.getTransferMode();
  if (transferMode == spi::TransferModes::DMA) {
    if (cacheMaintenanceOnTxBuffer) {
      /* Clean D-cache. Make sure the address is 32-byte aligned and add 32-bytes to length,
      in case it overlaps cacheline */
      SCB_CleanDCache_by_Addr((uint32_t *)(((uint32_t)sendData) & ~(uint32_t)0x1F), sendLen + 32);
    }
    status = HAL_SPI_TransmitReceive_DMA(&spiHandle, const_cast<uint8_t *>(sendData),
                                         currentRecvPtr, sendLen);
  } else {
    status = HAL_SPI_TransmitReceive_IT(&spiHandle, const_cast<uint8_t *>(sendData), currentRecvPtr,
                                        sendLen);
  }
  switch (status) {
    case (HAL_OK): {
      break;
    }
    default: {
      return halErrorHandler(status, transferMode);
    }
  }
  return result;
}

ReturnValue_t SpiComIF::halErrorHandler(HAL_StatusTypeDef status, spi::TransferModes transferMode) {
  char modeString[10];
  if (transferMode == spi::TransferModes::DMA) {
    std::snprintf(modeString, sizeof(modeString), "Dma");
  } else {
    std::snprintf(modeString, sizeof(modeString), "Interrupt");
  }
  sif::printWarning("SpiComIF::handle%sSendOperation: HAL error %d occured\n", modeString, status);
  switch (status) {
    case (HAL_BUSY): {
      return spi::HAL_BUSY_RETVAL;
    }
    case (HAL_ERROR): {
      return spi::HAL_ERROR_RETVAL;
    }
    case (HAL_TIMEOUT): {
      return spi::HAL_TIMEOUT_RETVAL;
    }
    default: {
      return returnvalue::FAILED;
    }
  }
}

ReturnValue_t SpiComIF::genericIrqSendSetup(uint8_t *recvPtr, SPI_HandleTypeDef &spiHandle,
                                            SpiCookie &spiCookie, const uint8_t *sendData,
                                            size_t sendLen) {
  currentRecvPtr = recvPtr;
  currentRecvBuffSize = sendLen;

  // Take the semaphore which will be released by a callback when the transfer is complete
  ReturnValue_t result = spiSemaphore->acquire(SemaphoreIF::TimeoutType::WAITING, timeoutMs);
  if (result != returnvalue::OK) {
    // Configuration error
    sif::printWarning(
        "SpiComIF::handleInterruptSendOperation: Semaphore "
        "could not be acquired after %d ms\n",
        timeoutMs);
    return result;
  }
  // Cache the current SPI handle in any case
  spi::setSpiHandle(&spiHandle);
  // Assign the IRQ arguments for the user callbacks
  irqArgs.comIF = this;
  irqArgs.spiCookie = &spiCookie;
  // The SPI handle is passed to the default SPI callback as a void argument. This callback
  // is different from the user callbacks specified above!
  spi::assignSpiUserArgs(spiCookie.getSpiIdx(), reinterpret_cast<void *>(&spiHandle));
  if (spiCookie.getChipSelectGpioPort() != nullptr) {
    HAL_GPIO_WritePin(spiCookie.getChipSelectGpioPort(), spiCookie.getChipSelectGpioPin(),
                      GPIO_PIN_RESET);
  }
  return returnvalue::OK;
}

void SpiComIF::spiTransferTxCompleteCallback(SPI_HandleTypeDef *hspi, void *args) {
  genericIrqHandler(args, spi::TransferStates::SUCCESS);
}

void SpiComIF::spiTransferRxCompleteCallback(SPI_HandleTypeDef *hspi, void *args) {
  genericIrqHandler(args, spi::TransferStates::SUCCESS);
}

void SpiComIF::spiTransferCompleteCallback(SPI_HandleTypeDef *hspi, void *args) {
  genericIrqHandler(args, spi::TransferStates::SUCCESS);
}

void SpiComIF::spiTransferErrorCallback(SPI_HandleTypeDef *hspi, void *args) {
  genericIrqHandler(args, spi::TransferStates::FAILURE);
}

void SpiComIF::genericIrqHandler(void *irqArgsVoid, spi::TransferStates targetState) {
  IrqArgs *irqArgs = reinterpret_cast<IrqArgs *>(irqArgsVoid);
  if (irqArgs == nullptr) {
    return;
  }
  SpiCookie *spiCookie = irqArgs->spiCookie;
  SpiComIF *comIF = irqArgs->comIF;
  if (spiCookie == nullptr or comIF == nullptr) {
    return;
  }

  spiCookie->setTransferState(targetState);

  if (spiCookie->getChipSelectGpioPort() != nullptr) {
    // Pull CS pin high again
    HAL_GPIO_WritePin(spiCookie->getChipSelectGpioPort(), spiCookie->getChipSelectGpioPin(),
                      GPIO_PIN_SET);
  }

#if defined FSFW_OSAL_FREERTOS
  // Release the task semaphore
  BaseType_t taskWoken = pdFALSE;
  ReturnValue_t result =
      BinarySemaphore::releaseFromISR(comIF->spiSemaphore->getSemaphore(), &taskWoken);
#elif defined FSFW_OSAL_RTEMS
  ReturnValue_t result = comIF->spiSemaphore->release();
#endif
  if (result != returnvalue::OK) {
    // Configuration error
    printf("SpiComIF::genericIrqHandler: Failure releasing Semaphore!\n");
  }

  // Perform cache maintenance operation for DMA transfers
  if (spiCookie->getTransferMode() == spi::TransferModes::DMA) {
    // Invalidate cache prior to access by CPU
    SCB_InvalidateDCache_by_Addr((uint32_t *)comIF->currentRecvPtr, comIF->currentRecvBuffSize);
  }
#if defined FSFW_OSAL_FREERTOS
  /* Request a context switch if the SPI ComIF task was woken up and has a higher priority
  than the currently running task */
  if (taskWoken == pdTRUE) {
    TaskManagement::requestContextSwitch(CallContext::ISR);
  }
#endif
}

void SpiComIF::printCfgError(const char *const type) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "SpiComIF::initializeInterface: Invalid " << type << " configuration"
               << std::endl;
#else
  sif::printWarning("SpiComIF::initializeInterface: Invalid %s configuration\n", type);
#endif
}
