#ifndef FSFW_HAL_STM32H7_SPI_STM32H743ZISPI_H_
#define FSFW_HAL_STM32H7_SPI_STM32H743ZISPI_H_

#include "mspInit.h"

namespace stm32h7 {

namespace h743zi {

void standardPollingCfg(spi::MspPollingConfigStruct& cfg);
void standardInterruptCfg(spi::MspIrqConfigStruct& cfg, IrqPriorities spiIrqPrio,
                          IrqPriorities spiSubprio = HIGHEST);
void standardDmaCfg(spi::MspDmaConfigStruct& cfg, IrqPriorities spiIrqPrio, IrqPriorities txIrqPrio,
                    IrqPriorities rxIrqPrio, IrqPriorities spiSubprio = HIGHEST,
                    IrqPriorities txSubPrio = HIGHEST, IrqPriorities rxSubprio = HIGHEST);

}  // namespace h743zi
}  // namespace stm32h7

#endif /* FSFW_HAL_STM32H7_SPI_STM32H743ZISPI_H_ */
