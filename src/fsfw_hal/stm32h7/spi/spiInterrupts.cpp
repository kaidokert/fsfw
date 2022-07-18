#include "fsfw_hal/stm32h7/spi/spiInterrupts.h"

#include <stddef.h>

#include "fsfw_hal/stm32h7/spi/spiCore.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_spi.h"

user_handler_t spi1UserHandler = &spi::spiIrqHandler;
user_args_t spi1UserArgs = nullptr;

user_handler_t spi2UserHandler = &spi::spiIrqHandler;
user_args_t spi2UserArgs = nullptr;

/**
 * @brief  This function handles DMA Rx interrupt request.
 * @param  None
 * @retval None
 */
void spi::dmaRxIrqHandler(void *dmaHandle) {
  if (dmaHandle == nullptr) {
    return;
  }
  HAL_DMA_IRQHandler((DMA_HandleTypeDef *)dmaHandle);
}

/**
 * @brief  This function handles DMA Rx interrupt request.
 * @param  None
 * @retval None
 */
void spi::dmaTxIrqHandler(void *dmaHandle) {
  if (dmaHandle == nullptr) {
    return;
  }
  HAL_DMA_IRQHandler((DMA_HandleTypeDef *)dmaHandle);
}

/**
 * @brief  This function handles SPIx interrupt request.
 * @param  None
 * @retval None
 */
void spi::spiIrqHandler(void *spiHandle) {
  if (spiHandle == nullptr) {
    return;
  }
  // auto currentSpiHandle = spi::getSpiHandle();
  HAL_SPI_IRQHandler((SPI_HandleTypeDef *)spiHandle);
}

void spi::assignSpiUserHandler(spi::SpiBus spiIdx, user_handler_t userHandler,
                               user_args_t userArgs) {
  if (spiIdx == spi::SpiBus::SPI_1) {
    spi1UserHandler = userHandler;
    spi1UserArgs = userArgs;
  } else {
    spi2UserHandler = userHandler;
    spi2UserArgs = userArgs;
  }
}

void spi::getSpiUserHandler(spi::SpiBus spiBus, user_handler_t *userHandler,
                            user_args_t *userArgs) {
  if (userHandler == nullptr or userArgs == nullptr) {
    return;
  }
  if (spiBus == spi::SpiBus::SPI_1) {
    *userArgs = spi1UserArgs;
    *userHandler = spi1UserHandler;
  } else {
    *userArgs = spi2UserArgs;
    *userHandler = spi2UserHandler;
  }
}

void spi::assignSpiUserArgs(spi::SpiBus spiBus, user_args_t userArgs) {
  if (spiBus == spi::SpiBus::SPI_1) {
    spi1UserArgs = userArgs;
  } else {
    spi2UserArgs = userArgs;
  }
}

/* Do not change these function names! They need to be exactly equal to the name of the functions
defined in the startup_stm32h743xx.s files! */

extern "C" void SPI1_IRQHandler() {
  if (spi1UserHandler != NULL) {
    spi1UserHandler(spi1UserArgs);
    return;
  }
  Default_Handler();
}

extern "C" void SPI2_IRQHandler() {
  if (spi2UserHandler != nullptr) {
    spi2UserHandler(spi2UserArgs);
    return;
  }
  Default_Handler();
}
