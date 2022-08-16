#ifndef FSFW_HAL_STM32H7_DEVICETEST_GYRO_L3GD20H_H_
#define FSFW_HAL_STM32H7_DEVICETEST_GYRO_L3GD20H_H_

#include <array>
#include <cstdint>

#include "../spi/mspInit.h"
#include "../spi/spiDefinitions.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"

#ifndef STM_USE_PERIPHERAL_TX_BUFFER_MPU_PROTECTION
#define STM_USE_PERIPHERAL_TX_BUFFER_MPU_PROTECTION 1
#endif

enum class TransferStates { IDLE, WAIT, SUCCESS, FAILURE };

class GyroL3GD20H {
 public:
  GyroL3GD20H(SPI_HandleTypeDef* spiHandle, spi::TransferModes transferMode);
  ~GyroL3GD20H();

  ReturnValue_t initialize();
  ReturnValue_t performOperation();

 private:
  const uint8_t WHO_AM_I_REG = 0b00001111;
  const uint8_t STM_READ_MASK = 0b10000000;
  const uint8_t STM_AUTO_INCREMENT_MASK = 0b01000000;
  const uint8_t EXPECTED_WHO_AM_I_VAL = 0b11010111;
  const uint8_t CTRL_REG_1 = 0b00100000;
  const uint32_t L3G_RANGE = 245;

  SPI_HandleTypeDef* spiHandle;

  static spi::TransferModes transferMode;
  static constexpr size_t recvBufferSize = 32 * 10;
  static std::array<uint8_t, recvBufferSize> rxBuffer;
  static constexpr size_t txBufferSize = 32;
  static std::array<uint8_t, txBufferSize> txBuffer;

  ReturnValue_t handleDmaTransferInit();
  ReturnValue_t handlePollingTransferInit();
  ReturnValue_t handleInterruptTransferInit();

  ReturnValue_t handleDmaSensorRead();
  HAL_StatusTypeDef performDmaTransfer(size_t sendSize);
  ReturnValue_t handlePollingSensorRead();
  ReturnValue_t handleInterruptSensorRead();

  uint8_t readRegPolling(uint8_t reg);

  static void spiTransferCompleteCallback(SPI_HandleTypeDef* hspi, void* args);
  static void spiTransferErrorCallback(SPI_HandleTypeDef* hspi, void* args);

  void prepareConfigRegs(uint8_t* configRegs);
  void handleSensorReadout();

  DMA_HandleTypeDef* txDmaHandle = {};
  DMA_HandleTypeDef* rxDmaHandle = {};
  spi::MspCfgBase* mspCfg = {};
};

#endif /*  FSFW_HAL_STM32H7_DEVICETEST_GYRO_L3GD20H_H_ */
