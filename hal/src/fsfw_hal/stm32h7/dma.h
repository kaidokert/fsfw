#ifndef FSFW_HAL_STM32H7_DMA_H_
#define FSFW_HAL_STM32H7_DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>

#include "interrupts.h"

namespace dma {

enum DMAType { TX = 0, RX = 1 };

enum DMAIndexes : uint8_t { DMA_1 = 1, DMA_2 = 2 };

enum DMAStreams {
  STREAM_0 = 0,
  STREAM_1 = 1,
  STREAM_2 = 2,
  STREAM_3 = 3,
  STREAM_4 = 4,
  STREAM_5 = 5,
  STREAM_6 = 6,
  STREAM_7 = 7,
};

/**
 * Assign user interrupt handlers for DMA streams, allowing to pass an
 * arbitrary argument as well. Generally, this argument will be the related DMA handle.
 * @param user_handler
 * @param user_args
 */
void assignDmaUserHandler(DMAIndexes dma_idx, DMAStreams stream_idx, user_handler_t user_handler,
                          user_args_t user_args);

}  // namespace dma

#ifdef __cplusplus
}
#endif

#endif /* FSFW_HAL_STM32H7_DMA_H_ */
