#ifndef FSFW_HAL_STM32H7_SPI_SPICORE_H_
#define FSFW_HAL_STM32H7_SPI_SPICORE_H_

#include "fsfw_hal/stm32h7/dma.h"
#include "fsfw_hal/stm32h7/spi/spiDefinitions.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dma.h"

#ifdef __cplusplus
extern "C" {
#endif

using spi_transfer_cb_t = void (*)(SPI_HandleTypeDef* hspi, void* userArgs);

namespace spi {

void configureDmaHandle(DMA_HandleTypeDef* handle, spi::SpiBus spiBus, dma::DMAType dmaType,
                        dma::DMAIndexes dmaIdx, dma::DMAStreams dmaStream, IRQn_Type* dmaIrqNumber,
                        uint32_t dmaMode = DMA_NORMAL, uint32_t dmaPriority = DMA_PRIORITY_LOW);

/**
 * Assign DMA handles. Required to use DMA for SPI transfers.
 * @param txHandle
 * @param rxHandle
 */
void setDmaHandles(DMA_HandleTypeDef* txHandle, DMA_HandleTypeDef* rxHandle);
void getDmaHandles(DMA_HandleTypeDef** txHandle, DMA_HandleTypeDef** rxHandle);

/**
 * Assign SPI handle. Needs to be done before using the SPI
 * @param spiHandle
 */
void setSpiHandle(SPI_HandleTypeDef* spiHandle);

void assignTransferRxTxCompleteCallback(spi_transfer_cb_t callback, void* userArgs);
void assignTransferRxCompleteCallback(spi_transfer_cb_t callback, void* userArgs);
void assignTransferTxCompleteCallback(spi_transfer_cb_t callback, void* userArgs);
void assignTransferErrorCallback(spi_transfer_cb_t callback, void* userArgs);

/**
 * Get the assigned SPI handle.
 * @return
 */
SPI_HandleTypeDef* getSpiHandle();

}  // namespace spi

#ifdef __cplusplus
}
#endif

#endif /* FSFW_HAL_STM32H7_SPI_SPICORE_H_ */
