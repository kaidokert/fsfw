#ifndef FSFW_HAL_STM32H7_SPI_MSPINIT_H_
#define FSFW_HAL_STM32H7_SPI_MSPINIT_H_

#include "spiDefinitions.h"
#include "../dma.h"

#include "stm32h7xx_hal_spi.h"

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   This file provides MSP implementation for DMA, IRQ and Polling mode for the
 *          SPI peripheral. This configuration is required for the SPI communication to work.
 */
namespace spi {

struct MspCfgBase {
    virtual ~MspCfgBase() = default;

    void (* cleanUpMacroWrapper) (void) = nullptr;
    void (* setupMacroWrapper) (void) = nullptr;

   GPIO_TypeDef* sckPort = nullptr;
   uint32_t sckPin = 0;
   uint8_t sckAlternateFunction = 0;
   GPIO_TypeDef* mosiPort = nullptr;
   uint32_t mosiPin = 0;
   uint8_t mosiAlternateFunction = 0;
   GPIO_TypeDef* misoPort = nullptr;
   uint32_t misoPin = 0;
   uint8_t misoAlternateFunction = 0;
};

struct MspPollingConfigStruct: public MspCfgBase {};

/* A valid instance of this struct must be passed to the MSP initialization function as a void*
argument */
struct MspIrqConfigStruct: public MspPollingConfigStruct {
    SpiBus spiBus = SpiBus::SPI_1;
    user_handler_t spiIrqHandler = nullptr;
    user_args_t spiUserArgs = nullptr;
    IRQn_Type spiIrqNumber = SPI1_IRQn;
    // Priorities for NVIC
    // Pre-Empt priority ranging from 0 to 15. If FreeRTOS calls are used, only 5-15 are allowed
    IrqPriorities preEmptPriority = IrqPriorities::LOWEST;
    IrqPriorities subpriority = IrqPriorities::LOWEST;
};

/* A valid instance of this struct must be passed to the MSP initialization function as a void*
argument */
struct MspDmaConfigStruct: public MspIrqConfigStruct {
    void (* dmaClkEnableWrapper) (void) = nullptr;
    dma::DMAIndexes txDmaIndex;
    dma::DMAIndexes rxDmaIndex;
    dma::DMAStreams txDmaStream;
    dma::DMAStreams rxDmaStream;
    IRQn_Type txDmaIrqNumber = DMA1_Stream0_IRQn;
    IRQn_Type rxDmaIrqNumber = DMA1_Stream1_IRQn;
    // Priorities for NVIC
    IrqPriorities txPreEmptPriority = IrqPriorities::LOWEST;
    IrqPriorities rxPreEmptPriority = IrqPriorities::LOWEST;
    IrqPriorities txSubpriority = IrqPriorities::LOWEST;
    IrqPriorities rxSubpriority = IrqPriorities::LOWEST;
};

using msp_func_t = void (*) (SPI_HandleTypeDef* hspi, MspCfgBase* cfg);


void getMspInitFunction(msp_func_t* init_func, MspCfgBase **args);
void getMspDeinitFunction(msp_func_t* deinit_func, MspCfgBase **args);

void halMspInitDma(SPI_HandleTypeDef* hspi, MspCfgBase* cfg);
void halMspDeinitDma(SPI_HandleTypeDef* hspi, MspCfgBase* cfg);

void halMspInitInterrupt(SPI_HandleTypeDef* hspi, MspCfgBase* cfg);
void halMspDeinitInterrupt(SPI_HandleTypeDef* hspi, MspCfgBase* cfg);

void halMspInitPolling(SPI_HandleTypeDef* hspi, MspCfgBase* cfg);
void halMspDeinitPolling(SPI_HandleTypeDef* hspi, MspCfgBase* cfg);

/**
 * Assign MSP init functions. Important for SPI configuration
 * @param init_func
 * @param init_args
 * @param deinit_func
 * @param deinit_args
 */
void setSpiDmaMspFunctions(MspDmaConfigStruct* cfg,
        msp_func_t initFunc = &spi::halMspInitDma,
        msp_func_t deinitFunc= &spi::halMspDeinitDma
);
void setSpiIrqMspFunctions(MspIrqConfigStruct* cfg,
        msp_func_t initFunc = &spi::halMspInitInterrupt,
        msp_func_t deinitFunc= &spi::halMspDeinitInterrupt
);
void setSpiPollingMspFunctions(MspPollingConfigStruct* cfg,
        msp_func_t initFunc = &spi::halMspInitPolling,
        msp_func_t deinitFunc= &spi::halMspDeinitPolling
);

void mspErrorHandler(const char* const function, const char *const message);

}


#ifdef __cplusplus
}
#endif

#endif /* FSFW_HAL_STM32H7_SPI_MSPINIT_H_ */
