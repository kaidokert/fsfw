#include "fsfw/hal/stm32h7/spi/stm32h743ziSpi.h"
#include "fsfw/hal/stm32h7/spi/spiCore.h"
#include "fsfw/hal/stm32h7/spi/spiInterrupts.h"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_rcc.h"

#include <cstdio>

void spiSetupWrapper() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
}

void spiCleanUpWrapper() {
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();
}

void spiDmaClockEnableWrapper() {
    __HAL_RCC_DMA2_CLK_ENABLE();
}

void spi::h743zi::standardPollingCfg(MspPollingConfigStruct& cfg) {
    cfg.setupMacroWrapper =  &spiSetupWrapper;
    cfg.cleanUpMacroWrapper = &spiCleanUpWrapper;
    cfg.sckPort = GPIOA;
    cfg.sckPin = GPIO_PIN_5;
    cfg.misoPort = GPIOA;
    cfg.misoPin = GPIO_PIN_6;
    cfg.mosiPort = GPIOA;
    cfg.mosiPin = GPIO_PIN_7;
    cfg.sckAlternateFunction = GPIO_AF5_SPI1;
    cfg.mosiAlternateFunction = GPIO_AF5_SPI1;
    cfg.misoAlternateFunction = GPIO_AF5_SPI1;
}

void spi::h743zi::standardInterruptCfg(MspIrqConfigStruct& cfg, IrqPriorities spiIrqPrio,
        IrqPriorities spiSubprio) {
    // High, but works on FreeRTOS as well (priorities range from 0 to 15)
    cfg.preEmptPriority = spiIrqPrio;
    cfg.subpriority = spiSubprio;
    cfg.spiIrqNumber = SPI1_IRQn;
    cfg.spiBus = SpiBus::SPI_1;
    user_handler_t spiUserHandler = nullptr;
    user_args_t spiUserArgs = nullptr;
    getSpiUserHandler(spi::SpiBus::SPI_1, &spiUserHandler, &spiUserArgs);
    if(spiUserHandler == nullptr) {
        printf("spi::h743zi::standardInterruptCfg: Invalid SPI user handlers\n");
        return;
    }
    cfg.spiUserArgs = spiUserArgs;
    cfg.spiIrqHandler = spiUserHandler;
    standardPollingCfg(cfg);
}

void spi::h743zi::standardDmaCfg(MspDmaConfigStruct& cfg, IrqPriorities spiIrqPrio,
        IrqPriorities txIrqPrio, IrqPriorities rxIrqPrio, IrqPriorities spiSubprio,
        IrqPriorities txSubprio, IrqPriorities rxSubprio) {
    cfg.dmaClkEnableWrapper = &spiDmaClockEnableWrapper;
    cfg.rxDmaIndex = dma::DMAIndexes::DMA_2;
    cfg.txDmaIndex = dma::DMAIndexes::DMA_2;
    cfg.txDmaStream = dma::DMAStreams::STREAM_3;
    cfg.rxDmaStream = dma::DMAStreams::STREAM_2;
    DMA_HandleTypeDef* txHandle;
    DMA_HandleTypeDef* rxHandle;
    spi::getDmaHandles(&txHandle, &rxHandle);
    if(txHandle == nullptr or rxHandle == nullptr) {
        printf("spi::h743zi::standardDmaCfg: Invalid DMA handles\n");
        return;
    }
    spi::configureDmaHandle(txHandle, spi::SpiBus::SPI_1, dma::DMAType::TX, cfg.txDmaIndex,
            cfg.txDmaStream, &cfg.txDmaIrqNumber);
    spi::configureDmaHandle(rxHandle, spi::SpiBus::SPI_1, dma::DMAType::RX, cfg.rxDmaIndex,
            cfg.rxDmaStream, &cfg.rxDmaIrqNumber, DMA_NORMAL, DMA_PRIORITY_HIGH);
    cfg.txPreEmptPriority = txIrqPrio;
    cfg.rxPreEmptPriority = txSubprio;
    cfg.txSubpriority = rxIrqPrio;
    cfg.rxSubpriority = rxSubprio;
    standardInterruptCfg(cfg, spiIrqPrio, spiSubprio);
}
