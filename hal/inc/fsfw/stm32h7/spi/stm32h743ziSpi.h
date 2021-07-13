#ifndef FSFW_HAL_STM32H7_SPI_STM32H743ZISPI_H_
#define FSFW_HAL_STM32H7_SPI_STM32H743ZISPI_H_

#include "mspInit.h"

namespace spi {

namespace h743zi {

void standardPollingCfg(MspPollingConfigStruct& cfg);
void standardInterruptCfg(MspIrqConfigStruct& cfg, IrqPriorities spiIrqPrio,
        IrqPriorities spiSubprio = HIGHEST);
void standardDmaCfg(MspDmaConfigStruct& cfg, IrqPriorities spiIrqPrio,
        IrqPriorities txIrqPrio, IrqPriorities rxIrqPrio,
        IrqPriorities spiSubprio = HIGHEST, IrqPriorities txSubPrio = HIGHEST,
        IrqPriorities rxSubprio = HIGHEST);

}
}



#endif /* FSFW_HAL_STM32H7_SPI_STM32H743ZISPI_H_ */
