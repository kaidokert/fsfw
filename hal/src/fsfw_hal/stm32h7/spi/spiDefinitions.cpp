#include "fsfw_hal/stm32h7/spi/spiDefinitions.h"

void spi::assignSpiMode(SpiModes spiMode, SPI_HandleTypeDef& spiHandle) {
  switch (spiMode) {
    case (SpiModes::MODE_0): {
      spiHandle.Init.CLKPolarity = SPI_POLARITY_LOW;
      spiHandle.Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    }
    case (SpiModes::MODE_1): {
      spiHandle.Init.CLKPolarity = SPI_POLARITY_LOW;
      spiHandle.Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    }
    case (SpiModes::MODE_2): {
      spiHandle.Init.CLKPolarity = SPI_POLARITY_HIGH;
      spiHandle.Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    }
    case (SpiModes::MODE_3): {
      spiHandle.Init.CLKPolarity = SPI_POLARITY_HIGH;
      spiHandle.Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    }
  }
}

uint32_t spi::getPrescaler(uint32_t clock_src_freq, uint32_t baudrate_mbps) {
  uint32_t divisor = 0;
  uint32_t spi_clk = clock_src_freq;
  uint32_t presc = 0;
  static const uint32_t baudrate[] = {
      SPI_BAUDRATEPRESCALER_2,   SPI_BAUDRATEPRESCALER_4,   SPI_BAUDRATEPRESCALER_8,
      SPI_BAUDRATEPRESCALER_16,  SPI_BAUDRATEPRESCALER_32,  SPI_BAUDRATEPRESCALER_64,
      SPI_BAUDRATEPRESCALER_128, SPI_BAUDRATEPRESCALER_256,
  };

  while (spi_clk > baudrate_mbps) {
    presc = baudrate[divisor];
    if (++divisor > 7) break;

    spi_clk = (spi_clk >> 1);
  }

  return presc;
}
