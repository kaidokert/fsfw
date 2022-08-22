#include "fsfw_hal/stm32h7/devicetest/GyroL3GD20H.h"

#include <cstring>

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/TaskFactory.h"
#include "fsfw_hal/stm32h7/spi/mspInit.h"
#include "fsfw_hal/stm32h7/spi/spiCore.h"
#include "fsfw_hal/stm32h7/spi/spiDefinitions.h"
#include "fsfw_hal/stm32h7/spi/spiInterrupts.h"
#include "fsfw_hal/stm32h7/spi/stm32h743zi.h"
#include "stm32h7xx_hal_rcc.h"
#include "stm32h7xx_hal_spi.h"

alignas(32) std::array<uint8_t, GyroL3GD20H::recvBufferSize> GyroL3GD20H::rxBuffer;
alignas(32) std::array<uint8_t, GyroL3GD20H::txBufferSize> GyroL3GD20H::txBuffer
    __attribute__((section(".dma_buffer")));

TransferStates transferState = TransferStates::IDLE;
spi::TransferModes GyroL3GD20H::transferMode = spi::TransferModes::POLLING;

GyroL3GD20H::GyroL3GD20H(SPI_HandleTypeDef *spiHandle, spi::TransferModes transferMode_)
    : spiHandle(spiHandle) {
  txDmaHandle = new DMA_HandleTypeDef();
  rxDmaHandle = new DMA_HandleTypeDef();
  spi::setSpiHandle(spiHandle);
  spi::assignSpiUserArgs(spi::SpiBus::SPI_1, spiHandle);
  transferMode = transferMode_;
  if (transferMode == spi::TransferModes::DMA) {
    mspCfg = new spi::MspDmaConfigStruct();
    auto typedCfg = dynamic_cast<spi::MspDmaConfigStruct *>(mspCfg);
    spi::setDmaHandles(txDmaHandle, rxDmaHandle);
    stm32h7::h743zi::standardDmaCfg(*typedCfg, IrqPriorities::HIGHEST_FREERTOS,
                                    IrqPriorities::HIGHEST_FREERTOS,
                                    IrqPriorities::HIGHEST_FREERTOS);
    spi::setSpiDmaMspFunctions(typedCfg);
  } else if (transferMode == spi::TransferModes::INTERRUPT) {
    mspCfg = new spi::MspIrqConfigStruct();
    auto typedCfg = dynamic_cast<spi::MspIrqConfigStruct *>(mspCfg);
    stm32h7::h743zi::standardInterruptCfg(*typedCfg, IrqPriorities::HIGHEST_FREERTOS);
    spi::setSpiIrqMspFunctions(typedCfg);
  } else if (transferMode == spi::TransferModes::POLLING) {
    mspCfg = new spi::MspPollingConfigStruct();
    auto typedCfg = dynamic_cast<spi::MspPollingConfigStruct *>(mspCfg);
    stm32h7::h743zi::standardPollingCfg(*typedCfg);
    spi::setSpiPollingMspFunctions(typedCfg);
  }

  spi::assignTransferRxTxCompleteCallback(&spiTransferCompleteCallback, nullptr);
  spi::assignTransferErrorCallback(&spiTransferErrorCallback, nullptr);

  GPIO_InitTypeDef chipSelect = {};
  __HAL_RCC_GPIOD_CLK_ENABLE();
  chipSelect.Pin = GPIO_PIN_14;
  chipSelect.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOD, &chipSelect);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

GyroL3GD20H::~GyroL3GD20H() {
  delete txDmaHandle;
  delete rxDmaHandle;
  if (mspCfg != nullptr) {
    delete mspCfg;
  }
}

ReturnValue_t GyroL3GD20H::initialize() {
  // Configure the SPI peripheral
  spiHandle->Instance = SPI1;
  spiHandle->Init.BaudRatePrescaler = spi::getPrescaler(HAL_RCC_GetHCLKFreq(), 3900000);
  spiHandle->Init.Direction = SPI_DIRECTION_2LINES;
  spi::assignSpiMode(spi::SpiModes::MODE_3, *spiHandle);
  spiHandle->Init.DataSize = SPI_DATASIZE_8BIT;
  spiHandle->Init.FirstBit = SPI_FIRSTBIT_MSB;
  spiHandle->Init.TIMode = SPI_TIMODE_DISABLE;
  spiHandle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  spiHandle->Init.CRCPolynomial = 7;
  spiHandle->Init.CRCLength = SPI_CRC_LENGTH_8BIT;
  spiHandle->Init.NSS = SPI_NSS_SOFT;
  spiHandle->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  // Recommended setting to avoid glitches
  spiHandle->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  spiHandle->Init.Mode = SPI_MODE_MASTER;
  if (HAL_SPI_Init(spiHandle) != HAL_OK) {
    sif::printWarning("Error initializing SPI\n");
    return returnvalue::FAILED;
  }

  delete mspCfg;
  transferState = TransferStates::WAIT;

  sif::printInfo("GyroL3GD20H::performOperation: Reading WHO AM I register\n");

  txBuffer[0] = WHO_AM_I_REG | STM_READ_MASK;
  txBuffer[1] = 0;

  switch (transferMode) {
    case (spi::TransferModes::DMA): {
      return handleDmaTransferInit();
    }
    case (spi::TransferModes::INTERRUPT): {
      return handleInterruptTransferInit();
    }
    case (spi::TransferModes::POLLING): {
      return handlePollingTransferInit();
    }
    default: {
      return returnvalue::FAILED;
    }
  }

  return returnvalue::OK;
}

ReturnValue_t GyroL3GD20H::performOperation() {
  switch (transferMode) {
    case (spi::TransferModes::DMA): {
      return handleDmaSensorRead();
    }
    case (spi::TransferModes::POLLING): {
      return handlePollingSensorRead();
    }
    case (spi::TransferModes::INTERRUPT): {
      return handleInterruptSensorRead();
    }
    default: {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t GyroL3GD20H::handleDmaTransferInit() {
  /* Clean D-cache */
  /* Make sure the address is 32-byte aligned and add 32-bytes to length,
  in case it overlaps cacheline */
  // See https://community.st.com/s/article/FAQ-DMA-is-not-working-on-STM32H7-devices
  HAL_StatusTypeDef result = performDmaTransfer(2);
  if (result != HAL_OK) {
    // Transfer error in transmission process
    sif::printWarning("GyroL3GD20H::initialize: Error transmitting SPI with DMA\n");
  }

  // Wait for the transfer to complete
  while (transferState == TransferStates::WAIT) {
    TaskFactory::delayTask(1);
  }

  switch (transferState) {
    case (TransferStates::SUCCESS): {
      uint8_t whoAmIVal = rxBuffer[1];
      if (whoAmIVal != EXPECTED_WHO_AM_I_VAL) {
        sif::printDebug(
            "GyroL3GD20H::initialize: "
            "Read WHO AM I value %d not equal to expected value!\n",
            whoAmIVal);
      }
      transferState = TransferStates::IDLE;
      break;
    }
    case (TransferStates::FAILURE): {
      sif::printWarning("Transfer failure\n");
      transferState = TransferStates::FAILURE;
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }

  sif::printInfo("GyroL3GD20H::initialize: Configuring device\n");
  // Configure the 5 configuration registers
  uint8_t configRegs[5];
  prepareConfigRegs(configRegs);

  result = performDmaTransfer(6);
  if (result != HAL_OK) {
    // Transfer error in transmission process
    sif::printWarning("Error transmitting SPI with DMA\n");
  }

  // Wait for the transfer to complete
  while (transferState == TransferStates::WAIT) {
    TaskFactory::delayTask(1);
  }

  switch (transferState) {
    case (TransferStates::SUCCESS): {
      sif::printInfo("GyroL3GD20H::initialize: Configuration transfer success\n");
      transferState = TransferStates::IDLE;
      break;
    }
    case (TransferStates::FAILURE): {
      sif::printWarning("GyroL3GD20H::initialize: Configuration transfer failure\n");
      transferState = TransferStates::FAILURE;
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }

  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK | STM_READ_MASK;
  std::memset(txBuffer.data() + 1, 0, 5);
  result = performDmaTransfer(6);
  if (result != HAL_OK) {
    // Transfer error in transmission process
    sif::printWarning("Error transmitting SPI with DMA\n");
  }
  // Wait for the transfer to complete
  while (transferState == TransferStates::WAIT) {
    TaskFactory::delayTask(1);
  }

  switch (transferState) {
    case (TransferStates::SUCCESS): {
      if (rxBuffer[1] != configRegs[0] or rxBuffer[2] != configRegs[1] or
          rxBuffer[3] != configRegs[2] or rxBuffer[4] != configRegs[3] or
          rxBuffer[5] != configRegs[4]) {
        sif::printWarning("GyroL3GD20H::initialize: Configuration failure\n");
      } else {
        sif::printInfo("GyroL3GD20H::initialize: Configuration success\n");
      }
      transferState = TransferStates::IDLE;
      break;
    }
    case (TransferStates::FAILURE): {
      sif::printWarning("GyroL3GD20H::initialize: Configuration transfer failure\n");
      transferState = TransferStates::FAILURE;
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t GyroL3GD20H::handleDmaSensorRead() {
  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK | STM_READ_MASK;
  std::memset(txBuffer.data() + 1, 0, 14);

  HAL_StatusTypeDef result = performDmaTransfer(15);
  if (result != HAL_OK) {
    // Transfer error in transmission process
    sif::printDebug("GyroL3GD20H::handleDmaSensorRead: Error transmitting SPI with DMA\n");
  }
  // Wait for the transfer to complete
  while (transferState == TransferStates::WAIT) {
    TaskFactory::delayTask(1);
  }

  switch (transferState) {
    case (TransferStates::SUCCESS): {
      handleSensorReadout();
      break;
    }
    case (TransferStates::FAILURE): {
      sif::printWarning("GyroL3GD20H::handleDmaSensorRead: Sensor read failure\n");
      transferState = TransferStates::FAILURE;
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

HAL_StatusTypeDef GyroL3GD20H::performDmaTransfer(size_t sendSize) {
  transferState = TransferStates::WAIT;
#if STM_USE_PERIPHERAL_TX_BUFFER_MPU_PROTECTION == 0
  SCB_CleanDCache_by_Addr((uint32_t *)(((uint32_t)txBuffer.data()) & ~(uint32_t)0x1F),
                          txBuffer.size() + 32);
#endif

  // Start SPI transfer via DMA
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  return HAL_SPI_TransmitReceive_DMA(spiHandle, txBuffer.data(), rxBuffer.data(), sendSize);
}

ReturnValue_t GyroL3GD20H::handlePollingTransferInit() {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  auto result = HAL_SPI_TransmitReceive(spiHandle, txBuffer.data(), rxBuffer.data(), 2, 1000);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  switch (result) {
    case (HAL_OK): {
      sif::printInfo("GyroL3GD20H::initialize: Polling transfer success\n");
      uint8_t whoAmIVal = rxBuffer[1];
      if (whoAmIVal != EXPECTED_WHO_AM_I_VAL) {
        sif::printDebug(
            "GyroL3GD20H::performOperation: "
            "Read WHO AM I value %d not equal to expected value!\n",
            whoAmIVal);
      }
      break;
    }
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer timeout\n");
      return returnvalue::FAILED;
    }
    case (HAL_ERROR): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer failure\n");
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }

  sif::printInfo("GyroL3GD20H::initialize: Configuring device\n");
  // Configure the 5 configuration registers
  uint8_t configRegs[5];
  prepareConfigRegs(configRegs);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  result = HAL_SPI_TransmitReceive(spiHandle, txBuffer.data(), rxBuffer.data(), 6, 1000);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  switch (result) {
    case (HAL_OK): {
      break;
    }
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer timeout\n");
      return returnvalue::FAILED;
    }
    case (HAL_ERROR): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer failure\n");
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }

  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK | STM_READ_MASK;
  std::memset(txBuffer.data() + 1, 0, 5);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  result = HAL_SPI_TransmitReceive(spiHandle, txBuffer.data(), rxBuffer.data(), 6, 1000);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  switch (result) {
    case (HAL_OK): {
      if (rxBuffer[1] != configRegs[0] or rxBuffer[2] != configRegs[1] or
          rxBuffer[3] != configRegs[2] or rxBuffer[4] != configRegs[3] or
          rxBuffer[5] != configRegs[4]) {
        sif::printWarning("GyroL3GD20H::initialize: Configuration failure\n");
      } else {
        sif::printInfo("GyroL3GD20H::initialize: Configuration success\n");
      }
      break;
    }
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer timeout\n");
      return returnvalue::FAILED;
    }
    case (HAL_ERROR): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer failure\n");
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t GyroL3GD20H::handlePollingSensorRead() {
  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK | STM_READ_MASK;
  std::memset(txBuffer.data() + 1, 0, 14);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  auto result = HAL_SPI_TransmitReceive(spiHandle, txBuffer.data(), rxBuffer.data(), 15, 1000);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

  switch (result) {
    case (HAL_OK): {
      handleSensorReadout();
      break;
    }
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer timeout\n");
      return returnvalue::FAILED;
    }
    case (HAL_ERROR): {
      sif::printDebug("GyroL3GD20H::initialize: Polling transfer failure\n");
      return returnvalue::FAILED;
    }
    default: {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t GyroL3GD20H::handleInterruptTransferInit() {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  switch (HAL_SPI_TransmitReceive_IT(spiHandle, txBuffer.data(), rxBuffer.data(), 2)) {
    case (HAL_OK): {
      sif::printInfo("GyroL3GD20H::initialize: Interrupt transfer success\n");
      // Wait for the transfer to complete
      while (transferState == TransferStates::WAIT) {
        TaskFactory::delayTask(1);
      }

      uint8_t whoAmIVal = rxBuffer[1];
      if (whoAmIVal != EXPECTED_WHO_AM_I_VAL) {
        sif::printDebug(
            "GyroL3GD20H::initialize: "
            "Read WHO AM I value %d not equal to expected value!\n",
            whoAmIVal);
      }
      break;
    }
    case (HAL_BUSY):
    case (HAL_ERROR):
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Initialization failure using interrupts\n");
      return returnvalue::FAILED;
    }
  }

  sif::printInfo("GyroL3GD20H::initialize: Configuring device\n");
  transferState = TransferStates::WAIT;
  // Configure the 5 configuration registers
  uint8_t configRegs[5];
  prepareConfigRegs(configRegs);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  switch (HAL_SPI_TransmitReceive_IT(spiHandle, txBuffer.data(), rxBuffer.data(), 6)) {
    case (HAL_OK): {
      // Wait for the transfer to complete
      while (transferState == TransferStates::WAIT) {
        TaskFactory::delayTask(1);
      }
      break;
    }
    case (HAL_BUSY):
    case (HAL_ERROR):
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Initialization failure using interrupts\n");
      return returnvalue::FAILED;
    }
  }

  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK | STM_READ_MASK;
  std::memset(txBuffer.data() + 1, 0, 5);
  transferState = TransferStates::WAIT;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  switch (HAL_SPI_TransmitReceive_IT(spiHandle, txBuffer.data(), rxBuffer.data(), 6)) {
    case (HAL_OK): {
      // Wait for the transfer to complete
      while (transferState == TransferStates::WAIT) {
        TaskFactory::delayTask(1);
      }
      if (rxBuffer[1] != configRegs[0] or rxBuffer[2] != configRegs[1] or
          rxBuffer[3] != configRegs[2] or rxBuffer[4] != configRegs[3] or
          rxBuffer[5] != configRegs[4]) {
        sif::printWarning("GyroL3GD20H::initialize: Configuration failure\n");
      } else {
        sif::printInfo("GyroL3GD20H::initialize: Configuration success\n");
      }
      break;
    }
    case (HAL_BUSY):
    case (HAL_ERROR):
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Initialization failure using interrupts\n");
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t GyroL3GD20H::handleInterruptSensorRead() {
  transferState = TransferStates::WAIT;
  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK | STM_READ_MASK;
  std::memset(txBuffer.data() + 1, 0, 14);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  switch (HAL_SPI_TransmitReceive_IT(spiHandle, txBuffer.data(), rxBuffer.data(), 15)) {
    case (HAL_OK): {
      // Wait for the transfer to complete
      while (transferState == TransferStates::WAIT) {
        TaskFactory::delayTask(1);
      }
      handleSensorReadout();
      break;
    }
    case (HAL_BUSY):
    case (HAL_ERROR):
    case (HAL_TIMEOUT): {
      sif::printDebug("GyroL3GD20H::initialize: Sensor read failure using interrupts\n");
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

void GyroL3GD20H::prepareConfigRegs(uint8_t *configRegs) {
  // Enable sensor
  configRegs[0] = 0b00001111;
  configRegs[1] = 0b00000000;
  configRegs[2] = 0b00000000;
  // Big endian select
  configRegs[3] = 0b01000000;
  configRegs[4] = 0b00000000;

  txBuffer[0] = CTRL_REG_1 | STM_AUTO_INCREMENT_MASK;
  std::memcpy(txBuffer.data() + 1, configRegs, 5);
}

uint8_t GyroL3GD20H::readRegPolling(uint8_t reg) {
  uint8_t rxBuf[2] = {};
  uint8_t txBuf[2] = {};
  txBuf[0] = reg | STM_READ_MASK;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  auto result = HAL_SPI_TransmitReceive(spiHandle, txBuf, rxBuf, 2, 1000);
  if (result) {
  };
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  return rxBuf[1];
}

void GyroL3GD20H::handleSensorReadout() {
  uint8_t statusReg = rxBuffer[8];
  int16_t gyroXRaw = rxBuffer[9] << 8 | rxBuffer[10];
  float gyroX = static_cast<float>(gyroXRaw) * 0.00875;
  int16_t gyroYRaw = rxBuffer[11] << 8 | rxBuffer[12];
  float gyroY = static_cast<float>(gyroYRaw) * 0.00875;
  int16_t gyroZRaw = rxBuffer[13] << 8 | rxBuffer[14];
  float gyroZ = static_cast<float>(gyroZRaw) * 0.00875;
  sif::printInfo("Status register: 0b" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(statusReg));
  sif::printInfo("Gyro X: %f\n", gyroX);
  sif::printInfo("Gyro Y: %f\n", gyroY);
  sif::printInfo("Gyro Z: %f\n", gyroZ);
}

void GyroL3GD20H::spiTransferCompleteCallback(SPI_HandleTypeDef *hspi, void *args) {
  transferState = TransferStates::SUCCESS;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  if (GyroL3GD20H::transferMode == spi::TransferModes::DMA) {
    // Invalidate cache prior to access by CPU
    SCB_InvalidateDCache_by_Addr((uint32_t *)GyroL3GD20H::rxBuffer.data(),
                                 GyroL3GD20H::recvBufferSize);
  }
}

/**
 * @brief  SPI error callbacks.
 * @param  hspi: SPI handle
 * @note   This example shows a simple way to report transfer error, and you can
 *         add your own implementation.
 * @retval None
 */
void GyroL3GD20H::spiTransferErrorCallback(SPI_HandleTypeDef *hspi, void *args) {
  transferState = TransferStates::FAILURE;
}
