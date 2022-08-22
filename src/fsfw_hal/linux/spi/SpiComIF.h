#ifndef LINUX_SPI_SPICOMIF_H_
#define LINUX_SPI_SPICOMIF_H_

#include <unordered_map>
#include <vector>

#include "fsfw/FSFW.h"
#include "fsfw/devicehandlers/DeviceCommunicationIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw_hal/common/gpio/GpioIF.h"
#include "returnvalues/classIds.h"
#include "spiDefinitions.h"

class SpiCookie;

/**
 * @brief   Encapsulates access to linux SPI driver for FSFW objects
 * @details
 * Right now, only full-duplex SPI is supported. Most device specific transfer properties
 * are contained in the SPI cookie.
 * @author  R. Mueller
 */
class SpiComIF : public DeviceCommunicationIF, public SystemObject {
 public:
  static constexpr uint8_t spiRetvalId = CLASS_ID::HAL_SPI;
  static constexpr ReturnValue_t OPENING_FILE_FAILED = returnvalue::makeCode(spiRetvalId, 0);
  /* Full duplex (ioctl) transfer failure */
  static constexpr ReturnValue_t FULL_DUPLEX_TRANSFER_FAILED =
      returnvalue::makeCode(spiRetvalId, 1);
  /* Half duplex (read/write) transfer failure */
  static constexpr ReturnValue_t HALF_DUPLEX_TRANSFER_FAILED =
      returnvalue::makeCode(spiRetvalId, 2);

  SpiComIF(object_id_t objectId, GpioIF* gpioComIF);

  ReturnValue_t initializeInterface(CookieIF* cookie) override;
  ReturnValue_t sendMessage(CookieIF* cookie, const uint8_t* sendData, size_t sendLen) override;
  ReturnValue_t getSendSuccess(CookieIF* cookie) override;
  ReturnValue_t requestReceiveMessage(CookieIF* cookie, size_t requestLen) override;
  ReturnValue_t readReceivedMessage(CookieIF* cookie, uint8_t** buffer, size_t* size) override;

  /**
   * @brief   This function returns the mutex which can be used to protect the spi bus when
   *          the chip select must be driven from outside of the com if.
   */
  MutexIF* getMutex(MutexIF::TimeoutType* timeoutType = nullptr, uint32_t* timeoutMs = nullptr);

  /**
   * Perform a regular send operation using Linux iotcl. This is public so it can be used
   * in functions like a user callback if special handling is only necessary for certain commands.
   * @param spiCookie
   * @param sendData
   * @param sendLen
   * @return
   */
  ReturnValue_t performRegularSendOperation(SpiCookie* spiCookie, const uint8_t* sendData,
                                            size_t sendLen);

  GpioIF* getGpioInterface();
  void setSpiSpeedAndMode(int spiFd, spi::SpiModes mode, uint32_t speed);
  void performSpiWiretapping(SpiCookie* spiCookie);

  ReturnValue_t getReadBuffer(address_t spiAddress, uint8_t** buffer);

 private:
  struct SpiInstance {
    SpiInstance(size_t maxRecvSize) : replyBuffer(std::vector<uint8_t>(maxRecvSize)) {}
    std::vector<uint8_t> replyBuffer;
  };

  GpioIF* gpioComIF = nullptr;

  MutexIF* spiMutex = nullptr;
  MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
  uint32_t timeoutMs = 20;
  spi_ioc_transfer clockUpdateTransfer = {};

  using SpiDeviceMap = std::unordered_map<address_t, SpiInstance>;
  using SpiDeviceMapIter = SpiDeviceMap::iterator;

  SpiDeviceMap spiDeviceMap;

  ReturnValue_t performHalfDuplexReception(SpiCookie* spiCookie);
};

#endif /* LINUX_SPI_SPICOMIF_H_ */
