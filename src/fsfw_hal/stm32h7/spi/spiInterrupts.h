#ifndef FSFW_HAL_STM32H7_SPI_INTERRUPTS_H_
#define FSFW_HAL_STM32H7_SPI_INTERRUPTS_H_

#include "../interrupts.h"
#include "spiDefinitions.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace spi {

void assignSpiUserArgs(spi::SpiBus spiBus, user_args_t userArgs);

/**
 * Assign a user interrupt handler for SPI bus 1, allowing to pass an arbitrary argument as well.
 * Generally, this argument will be the related SPI handle.
 * @param user_handler
 * @param user_args
 */
void assignSpiUserHandler(spi::SpiBus spiBus, user_handler_t user_handler, user_args_t user_args);
void getSpiUserHandler(spi::SpiBus spiBus, user_handler_t* user_handler, user_args_t* user_args);

/**
 * Generic interrupt handlers supplied for convenience. Do not call these directly! Set them
 * instead with assign_dma_user_handler and assign_spi_user_handler functions.
 * @param dma_handle
 */
void dmaRxIrqHandler(void* dma_handle);
void dmaTxIrqHandler(void* dma_handle);
void spiIrqHandler(void* spi_handle);

}  // namespace spi

#ifdef __cplusplus
}
#endif

#endif /* FSFW_HAL_STM32H7_SPI_INTERRUPTS_H_ */
