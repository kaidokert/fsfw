#ifndef FSFW_HAL_STM32H7_SPI_SPICOOKIE_H_
#define FSFW_HAL_STM32H7_SPI_SPICOOKIE_H_

#include <utility>

#include "../definitions.h"
#include "fsfw/devicehandlers/CookieIF.h"
#include "mspInit.h"
#include "spiDefinitions.h"
#include "stm32h743xx.h"

/**
 * @brief   SPI cookie implementation for the STM32H7 device family
 * @details
 * This cookie contains and caches device specific information to be used by the
 * SPI communication interface
 * @author  R. Mueller
 */
class SpiCookie : public CookieIF {
  friend class SpiComIF;

 public:
  /**
   * Allows construction of a SPI cookie for a connected SPI device
   * @param deviceAddress
   * @param spiIdx                SPI bus, e.g. SPI1 or SPI2
   * @param transferMode
   * @param mspCfg                This is the MSP configuration. The user is expected to supply
   *                              a valid MSP configuration. See mspInit.h for functions
   *                              to create one.
   * @param spiSpeed
   * @param spiMode
   * @param chipSelectGpioPin     GPIO port. Don't use a number here, use the 16 bit type
   *                              definitions supplied in the MCU header file! (e.g. GPIO_PIN_X)
   * @param chipSelectGpioPort    GPIO port (e.g. GPIOA)
   * @param maxRecvSize           Maximum expected receive size. Chose as small as possible.
   * @param csGpio                Optional CS GPIO definition.
   */
  SpiCookie(address_t deviceAddress, spi::SpiBus spiIdx, spi::TransferModes transferMode,
            spi::MspCfgBase* mspCfg, uint32_t spiSpeed, spi::SpiModes spiMode, size_t maxRecvSize,
            stm32h7::GpioCfg csGpio = stm32h7::GpioCfg(nullptr, 0, 0));

  uint16_t getChipSelectGpioPin() const;
  GPIO_TypeDef* getChipSelectGpioPort();
  address_t getDeviceAddress() const;
  spi::SpiBus getSpiIdx() const;
  spi::SpiModes getSpiMode() const;
  spi::TransferModes getTransferMode() const;
  uint32_t getSpiSpeed() const;
  size_t getMaxRecvSize() const;
  SPI_HandleTypeDef& getSpiHandle();

 private:
  address_t deviceAddress;
  SPI_HandleTypeDef spiHandle = {};
  spi::SpiBus spiIdx;
  uint32_t spiSpeed;
  spi::SpiModes spiMode;
  spi::TransferModes transferMode;
  volatile spi::TransferStates transferState = spi::TransferStates::IDLE;
  stm32h7::GpioCfg csGpio;

  // The MSP configuration is cached here. Be careful when using this, it is automatically
  // deleted by the SPI  communication interface if it is not required anymore!
  spi::MspCfgBase* mspCfg = nullptr;
  const size_t maxRecvSize;

  // Only the SpiComIF is allowed to use this to prevent dangling pointers issues
  spi::MspCfgBase* getMspCfg();
  void deleteMspCfg();

  void setTransferState(spi::TransferStates transferState);
  spi::TransferStates getTransferState() const;
};

#endif /* FSFW_HAL_STM32H7_SPI_SPICOOKIE_H_ */
