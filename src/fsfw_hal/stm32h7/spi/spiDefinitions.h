#ifndef FSFW_HAL_STM32H7_SPI_SPIDEFINITIONS_H_
#define FSFW_HAL_STM32H7_SPI_SPIDEFINITIONS_H_

#include "../../common/spi/spiCommon.h"
#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"

namespace spi {

static constexpr uint8_t HAL_SPI_ID = CLASS_ID::HAL_SPI;
static constexpr ReturnValue_t HAL_TIMEOUT_RETVAL = returnvalue::makeCode(HAL_SPI_ID, 0);
static constexpr ReturnValue_t HAL_BUSY_RETVAL = returnvalue::makeCode(HAL_SPI_ID, 1);
static constexpr ReturnValue_t HAL_ERROR_RETVAL = returnvalue::makeCode(HAL_SPI_ID, 2);

enum class TransferStates { IDLE, WAIT, SUCCESS, FAILURE };

enum SpiBus { SPI_1, SPI_2 };

enum TransferModes { POLLING, INTERRUPT, DMA };

void assignSpiMode(SpiModes spiMode, SPI_HandleTypeDef& spiHandle);

/**
 * @brief  Set SPI frequency to calculate correspondent baud-rate prescaler.
 * @param  clock_src_freq  Frequency of clock source
 * @param  baudrate_mbps Baudrate to set to set
 * @retval Baudrate prescaler
 */
uint32_t getPrescaler(uint32_t clock_src_freq, uint32_t baudrate_mbps);

}  // namespace spi

#endif /* FSFW_HAL_STM32H7_SPI_SPIDEFINITIONS_H_ */
