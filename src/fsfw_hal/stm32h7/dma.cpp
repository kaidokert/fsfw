#include <fsfw_hal/stm32h7/dma.h>

#include <cstddef>
#include <cstdint>

user_handler_t DMA_1_USER_HANDLERS[8];
user_args_t DMA_1_USER_ARGS[8];

user_handler_t DMA_2_USER_HANDLERS[8];
user_args_t DMA_2_USER_ARGS[8];

void dma::assignDmaUserHandler(DMAIndexes dma_idx, DMAStreams stream_idx,
                               user_handler_t user_handler, user_args_t user_args) {
  if (dma_idx == DMA_1) {
    DMA_1_USER_HANDLERS[stream_idx] = user_handler;
    DMA_1_USER_ARGS[stream_idx] = user_args;
  } else if (dma_idx == DMA_2) {
    DMA_2_USER_HANDLERS[stream_idx] = user_handler;
    DMA_2_USER_ARGS[stream_idx] = user_args;
  }
}

// The interrupt handlers in the format required for the IRQ vector table

/* Do not change these function names! They need to be exactly equal to the name of the functions
defined in the startup_stm32h743xx.s files! */

#define GENERIC_DMA_IRQ_HANDLER(DMA_IDX, STREAM_IDX)                                  \
  if (DMA_##DMA_IDX##_USER_HANDLERS[STREAM_IDX] != NULL) {                            \
    DMA_##DMA_IDX##_USER_HANDLERS[STREAM_IDX](DMA_##DMA_IDX##_USER_ARGS[STREAM_IDX]); \
    return;                                                                           \
  }                                                                                   \
  Default_Handler()

extern "C" void DMA1_Stream0_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 0); }
extern "C" void DMA1_Stream1_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 1); }
extern "C" void DMA1_Stream2_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 2); }
extern "C" void DMA1_Stream3_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 3); }
extern "C" void DMA1_Stream4_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 4); }
extern "C" void DMA1_Stream5_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 5); }
extern "C" void DMA1_Stream6_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 6); }
extern "C" void DMA1_Stream7_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(1, 7); }

extern "C" void DMA2_Stream0_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 0); }
extern "C" void DMA2_Stream1_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 1); }
extern "C" void DMA2_Stream2_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 2); }
extern "C" void DMA2_Stream3_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 3); }
extern "C" void DMA2_Stream4_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 4); }
extern "C" void DMA2_Stream5_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 5); }
extern "C" void DMA2_Stream6_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 6); }
extern "C" void DMA2_Stream7_IRQHandler() { GENERIC_DMA_IRQ_HANDLER(2, 7); }
