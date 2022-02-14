#ifndef FSFW_HAL_STM32H7_SPI_SPICOMIF_H_
#define FSFW_HAL_STM32H7_SPI_SPICOMIF_H_

#include <map>
#include <vector>

#include "fsfw/devicehandlers/DeviceCommunicationIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/SemaphoreIF.h"
#include "fsfw_hal/stm32h7/spi/spiDefinitions.h"
#include "stm32h743xx.h"
#include "stm32h7xx_hal_spi.h"

class SpiCookie;
class BinarySemaphore;

/**
 * @brief   This communication interface allows using generic device handlers with using
 *          the STM32H7 SPI peripherals
 * @details
 * This communication interface supports all three major communcation modes:
 *  - Polling: Simple, but not recommended to real use-cases, blocks the CPU
 *  - Interrupt: Good for small data only arriving occasionally
 *  - DMA: Good for large data which also occur regularly. Please note that the number
 *    of DMA channels in limited
 * The device specific information is usually kept in the SpiCookie class. The current
 * implementation limits the transfer mode for a given SPI bus.
 * @author  R. Mueller
 */
class SpiComIF : public SystemObject, public DeviceCommunicationIF {
 public:
  /**
   * Create a SPI communication interface for the given SPI peripheral (spiInstance)
   * @param objectId
   * @param spiInstance
   * @param spiHandle
   * @param transferMode
   */
  SpiComIF(object_id_t objectId);

  /**
   * Allows the user to disable cache maintenance on the TX buffer. This can be done if the
   * TX buffers are places and MPU protected properly like specified in this link:
   * https://community.st.com/s/article/FAQ-DMA-is-not-working-on-STM32H7-devices
   * The cache maintenace is enabled by default.
   * @param enable
   */
  void configureCacheMaintenanceOnTxBuffer(bool enable);

  void setDefaultPollingTimeout(dur_millis_t timeout);

  /**
   * Add the DMA handles. These need to be set in the DMA transfer mode is used.
   * @param txHandle
   * @param rxHandle
   */
  void addDmaHandles(DMA_HandleTypeDef* txHandle, DMA_HandleTypeDef* rxHandle);

  ReturnValue_t initialize() override;

  // DeviceCommunicationIF overrides
  virtual ReturnValue_t initializeInterface(CookieIF* cookie) override;
  virtual ReturnValue_t sendMessage(CookieIF* cookie, const uint8_t* sendData,
                                    size_t sendLen) override;
  virtual ReturnValue_t getSendSuccess(CookieIF* cookie) override;
  virtual ReturnValue_t requestReceiveMessage(CookieIF* cookie, size_t requestLen) override;
  virtual ReturnValue_t readReceivedMessage(CookieIF* cookie, uint8_t** buffer,
                                            size_t* size) override;

 protected:
  struct SpiInstance {
    SpiInstance(size_t maxRecvSize) : replyBuffer(std::vector<uint8_t>(maxRecvSize)) {}
    std::vector<uint8_t> replyBuffer;
    size_t currentTransferLen = 0;
  };

  struct IrqArgs {
    SpiComIF* comIF = nullptr;
    SpiCookie* spiCookie = nullptr;
  };

  IrqArgs irqArgs;

  uint32_t defaultPollingTimeout = 50;

  SemaphoreIF::TimeoutType timeoutType = SemaphoreIF::TimeoutType::WAITING;
  dur_millis_t timeoutMs = 20;

  BinarySemaphore* spiSemaphore = nullptr;
  bool cacheMaintenanceOnTxBuffer = true;

  using SpiDeviceMap = std::map<address_t, SpiInstance>;
  using SpiDeviceMapIter = SpiDeviceMap::iterator;

  uint8_t* currentRecvPtr = nullptr;
  size_t currentRecvBuffSize = 0;

  SpiDeviceMap spiDeviceMap;

  ReturnValue_t handlePollingSendOperation(uint8_t* recvPtr, SPI_HandleTypeDef& spiHandle,
                                           SpiCookie& spiCookie, const uint8_t* sendData,
                                           size_t sendLen);
  ReturnValue_t handleInterruptSendOperation(uint8_t* recvPtr, SPI_HandleTypeDef& spiHandle,
                                             SpiCookie& spiCookie, const uint8_t* sendData,
                                             size_t sendLen);
  ReturnValue_t handleDmaSendOperation(uint8_t* recvPtr, SPI_HandleTypeDef& spiHandle,
                                       SpiCookie& spiCookie, const uint8_t* sendData,
                                       size_t sendLen);
  ReturnValue_t handleIrqSendOperation(uint8_t* recvPtr, SPI_HandleTypeDef& spiHandle,
                                       SpiCookie& spiCookie, const uint8_t* sendData,
                                       size_t sendLen);
  ReturnValue_t genericIrqSendSetup(uint8_t* recvPtr, SPI_HandleTypeDef& spiHandle,
                                    SpiCookie& spiCookie, const uint8_t* sendData, size_t sendLen);
  ReturnValue_t halErrorHandler(HAL_StatusTypeDef status, spi::TransferModes transferMode);

  static void spiTransferTxCompleteCallback(SPI_HandleTypeDef* hspi, void* args);
  static void spiTransferRxCompleteCallback(SPI_HandleTypeDef* hspi, void* args);
  static void spiTransferCompleteCallback(SPI_HandleTypeDef* hspi, void* args);
  static void spiTransferErrorCallback(SPI_HandleTypeDef* hspi, void* args);

  static void genericIrqHandler(void* irqArgs, spi::TransferStates targetState);

  void printCfgError(const char* const type);
};

#endif /* FSFW_HAL_STM32H7_SPI_SPICOMIF_H_ */
