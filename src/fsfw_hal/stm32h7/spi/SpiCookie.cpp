#include "fsfw_hal/stm32h7/spi/SpiCookie.h"

SpiCookie::SpiCookie(address_t deviceAddress, spi::SpiBus spiIdx, spi::TransferModes transferMode,
                     spi::MspCfgBase* mspCfg, uint32_t spiSpeed, spi::SpiModes spiMode,
                     size_t maxRecvSize, stm32h7::GpioCfg csGpio)
    : deviceAddress(deviceAddress),
      spiIdx(spiIdx),
      spiSpeed(spiSpeed),
      spiMode(spiMode),
      transferMode(transferMode),
      csGpio(csGpio),
      mspCfg(mspCfg),
      maxRecvSize(maxRecvSize) {
  spiHandle.Init.DataSize = SPI_DATASIZE_8BIT;
  spiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spiHandle.Init.TIMode = SPI_TIMODE_DISABLE;
  spiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  spiHandle.Init.CRCPolynomial = 7;
  spiHandle.Init.CRCLength = SPI_CRC_LENGTH_8BIT;
  spiHandle.Init.NSS = SPI_NSS_SOFT;
  spiHandle.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  spiHandle.Init.Direction = SPI_DIRECTION_2LINES;
  // Recommended setting to avoid glitches
  spiHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  spiHandle.Init.Mode = SPI_MODE_MASTER;
  spi::assignSpiMode(spiMode, spiHandle);
  spiHandle.Init.BaudRatePrescaler = spi::getPrescaler(HAL_RCC_GetHCLKFreq(), spiSpeed);
}

uint16_t SpiCookie::getChipSelectGpioPin() const { return csGpio.pin; }

GPIO_TypeDef* SpiCookie::getChipSelectGpioPort() { return csGpio.port; }

address_t SpiCookie::getDeviceAddress() const { return deviceAddress; }

spi::SpiBus SpiCookie::getSpiIdx() const { return spiIdx; }

spi::SpiModes SpiCookie::getSpiMode() const { return spiMode; }

uint32_t SpiCookie::getSpiSpeed() const { return spiSpeed; }

size_t SpiCookie::getMaxRecvSize() const { return maxRecvSize; }

SPI_HandleTypeDef& SpiCookie::getSpiHandle() { return spiHandle; }

spi::MspCfgBase* SpiCookie::getMspCfg() { return mspCfg; }

void SpiCookie::deleteMspCfg() {
  if (mspCfg != nullptr) {
    delete mspCfg;
  }
}

spi::TransferModes SpiCookie::getTransferMode() const { return transferMode; }

void SpiCookie::setTransferState(spi::TransferStates transferState) {
  this->transferState = transferState;
}

spi::TransferStates SpiCookie::getTransferState() const { return this->transferState; }
