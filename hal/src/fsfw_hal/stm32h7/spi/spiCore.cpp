#include "fsfw_hal/stm32h7/spi/spiCore.h"

#include <cstdio>

#include "fsfw_hal/stm32h7/spi/spiDefinitions.h"

SPI_HandleTypeDef* spiHandle = nullptr;
DMA_HandleTypeDef* hdmaTx = nullptr;
DMA_HandleTypeDef* hdmaRx = nullptr;

spi_transfer_cb_t rxTxCb = nullptr;
void* rxTxArgs = nullptr;
spi_transfer_cb_t txCb = nullptr;
void* txArgs = nullptr;
spi_transfer_cb_t rxCb = nullptr;
void* rxArgs = nullptr;
spi_transfer_cb_t errorCb = nullptr;
void* errorArgs = nullptr;

void mapIndexAndStream(DMA_HandleTypeDef* handle, dma::DMAType dmaType, dma::DMAIndexes dmaIdx,
                       dma::DMAStreams dmaStream, IRQn_Type* dmaIrqNumber);
void mapSpiBus(DMA_HandleTypeDef* handle, dma::DMAType dmaType, spi::SpiBus spiBus);

void spi::configureDmaHandle(DMA_HandleTypeDef* handle, spi::SpiBus spiBus, dma::DMAType dmaType,
                             dma::DMAIndexes dmaIdx, dma::DMAStreams dmaStream,
                             IRQn_Type* dmaIrqNumber, uint32_t dmaMode, uint32_t dmaPriority) {
  using namespace dma;
  mapIndexAndStream(handle, dmaType, dmaIdx, dmaStream, dmaIrqNumber);
  mapSpiBus(handle, dmaType, spiBus);

  if (dmaType == DMAType::TX) {
    handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
  } else {
    handle->Init.Direction = DMA_PERIPH_TO_MEMORY;
  }

  handle->Init.Priority = dmaPriority;
  handle->Init.Mode = dmaMode;

  // Standard settings for the rest for now
  handle->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  handle->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  handle->Init.MemBurst = DMA_MBURST_INC4;
  handle->Init.PeriphBurst = DMA_PBURST_INC4;
  handle->Init.PeriphInc = DMA_PINC_DISABLE;
  handle->Init.MemInc = DMA_MINC_ENABLE;
  handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  handle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
}

void spi::setDmaHandles(DMA_HandleTypeDef* txHandle, DMA_HandleTypeDef* rxHandle) {
  hdmaTx = txHandle;
  hdmaRx = rxHandle;
}

void spi::getDmaHandles(DMA_HandleTypeDef** txHandle, DMA_HandleTypeDef** rxHandle) {
  *txHandle = hdmaTx;
  *rxHandle = hdmaRx;
}

void spi::setSpiHandle(SPI_HandleTypeDef* spiHandle_) {
  if (spiHandle_ == NULL) {
    return;
  }
  spiHandle = spiHandle_;
}

void spi::assignTransferRxTxCompleteCallback(spi_transfer_cb_t callback, void* userArgs) {
  rxTxCb = callback;
  rxTxArgs = userArgs;
}

void spi::assignTransferRxCompleteCallback(spi_transfer_cb_t callback, void* userArgs) {
  rxCb = callback;
  rxArgs = userArgs;
}

void spi::assignTransferTxCompleteCallback(spi_transfer_cb_t callback, void* userArgs) {
  txCb = callback;
  txArgs = userArgs;
}

void spi::assignTransferErrorCallback(spi_transfer_cb_t callback, void* userArgs) {
  errorCb = callback;
  errorArgs = userArgs;
}

SPI_HandleTypeDef* spi::getSpiHandle() { return spiHandle; }

/**
 * @brief  TxRx Transfer completed callback.
 * @param  hspi: SPI handle
 */
extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi) {
  if (rxTxCb != NULL) {
    rxTxCb(hspi, rxTxArgs);
  } else {
    printf("HAL_SPI_TxRxCpltCallback: No user callback specified\n");
  }
}

/**
 * @brief  TxRx Transfer completed callback.
 * @param  hspi: SPI handle
 */
extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
  if (txCb != NULL) {
    txCb(hspi, txArgs);
  } else {
    printf("HAL_SPI_TxCpltCallback: No user callback specified\n");
  }
}

/**
 * @brief  TxRx Transfer completed callback.
 * @param  hspi: SPI handle
 */
extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {
  if (rxCb != nullptr) {
    rxCb(hspi, rxArgs);
  } else {
    printf("HAL_SPI_RxCpltCallback: No user callback specified\n");
  }
}

/**
 * @brief  SPI error callbacks.
 * @param  hspi: SPI handle
 * @note   This example shows a simple way to report transfer error, and you can
 *         add your own implementation.
 * @retval None
 */
extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi) {
  if (errorCb != nullptr) {
    errorCb(hspi, rxArgs);
  } else {
    printf("HAL_SPI_ErrorCallback: No user callback specified\n");
  }
}

void mapIndexAndStream(DMA_HandleTypeDef* handle, dma::DMAType dmaType, dma::DMAIndexes dmaIdx,
                       dma::DMAStreams dmaStream, IRQn_Type* dmaIrqNumber) {
  using namespace dma;
  if (dmaIdx == DMAIndexes::DMA_1) {
#ifdef DMA1
    switch (dmaStream) {
      case (DMAStreams::STREAM_0): {
#ifdef DMA1_Stream0
        handle->Instance = DMA1_Stream0;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream0_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_1): {
#ifdef DMA1_Stream1
        handle->Instance = DMA1_Stream1;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream1_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_2): {
#ifdef DMA1_Stream2
        handle->Instance = DMA1_Stream2;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream2_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_3): {
#ifdef DMA1_Stream3
        handle->Instance = DMA1_Stream3;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream3_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_4): {
#ifdef DMA1_Stream4
        handle->Instance = DMA1_Stream4;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream4_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_5): {
#ifdef DMA1_Stream5
        handle->Instance = DMA1_Stream5;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream5_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_6): {
#ifdef DMA1_Stream6
        handle->Instance = DMA1_Stream6;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream6_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_7): {
#ifdef DMA1_Stream7
        handle->Instance = DMA1_Stream7;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA1_Stream7_IRQn;
        }
#endif
        break;
      }
    }
    if (dmaType == DMAType::TX) {
      handle->Init.Request = DMA_REQUEST_SPI1_TX;
    } else {
      handle->Init.Request = DMA_REQUEST_SPI1_RX;
    }
#endif /* DMA1 */
  }
  if (dmaIdx == DMAIndexes::DMA_2) {
#ifdef DMA2
    switch (dmaStream) {
      case (DMAStreams::STREAM_0): {
#ifdef DMA2_Stream0
        handle->Instance = DMA2_Stream0;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream0_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_1): {
#ifdef DMA2_Stream1
        handle->Instance = DMA2_Stream1;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream1_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_2): {
#ifdef DMA2_Stream2
        handle->Instance = DMA2_Stream2;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream2_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_3): {
#ifdef DMA2_Stream3
        handle->Instance = DMA2_Stream3;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream3_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_4): {
#ifdef DMA2_Stream4
        handle->Instance = DMA2_Stream4;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream4_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_5): {
#ifdef DMA2_Stream5
        handle->Instance = DMA2_Stream5;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream5_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_6): {
#ifdef DMA2_Stream6
        handle->Instance = DMA2_Stream6;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream6_IRQn;
        }
#endif
        break;
      }
      case (DMAStreams::STREAM_7): {
#ifdef DMA2_Stream7
        handle->Instance = DMA2_Stream7;
        if (dmaIrqNumber != nullptr) {
          *dmaIrqNumber = DMA2_Stream7_IRQn;
        }
#endif
        break;
      }
    }
#endif /* DMA2 */
  }
}

void mapSpiBus(DMA_HandleTypeDef* handle, dma::DMAType dmaType, spi::SpiBus spiBus) {
  if (dmaType == dma::DMAType::TX) {
    if (spiBus == spi::SpiBus::SPI_1) {
#ifdef DMA_REQUEST_SPI1_TX
      handle->Init.Request = DMA_REQUEST_SPI1_TX;
#endif
    } else if (spiBus == spi::SpiBus::SPI_2) {
#ifdef DMA_REQUEST_SPI2_TX
      handle->Init.Request = DMA_REQUEST_SPI2_TX;
#endif
    }
  } else {
    if (spiBus == spi::SpiBus::SPI_1) {
#ifdef DMA_REQUEST_SPI1_RX
      handle->Init.Request = DMA_REQUEST_SPI1_RX;
#endif
    } else if (spiBus == spi::SpiBus::SPI_2) {
#ifdef DMA_REQUEST_SPI2_RX
      handle->Init.Request = DMA_REQUEST_SPI2_RX;
#endif
    }
  }
}
