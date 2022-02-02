#ifndef LINUX_SPI_SPICOOKIE_H_
#define LINUX_SPI_SPICOOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <linux/spi/spidev.h>

#include "../../common/gpio/gpioDefinitions.h"
#include "spiDefinitions.h"

/**
 * @brief   This cookie class is passed to the SPI communication interface
 * @details
 * This cookie contains device specific properties like speed and SPI mode or the SPI transfer
 * struct required by the Linux SPI driver. It also contains a handle to a GPIO interface
 * to perform slave select switching when necessary.
 *
 * The user can specify gpio::NO_GPIO as the GPIO ID or use a custom send callback to meet
 * special requirements like expander slave select switching (e.g. GPIO or I2C expander)
 * or special timing related requirements.
 */
class SpiCookie : public CookieIF {
 public:
  /**
   * Each SPI device will have a corresponding cookie. The cookie is used by the communication
   * interface and contains device specific information like the largest expected size to be
   * sent and received and the GPIO pin used to toggle the SPI slave select pin.
   * @param spiAddress
   * @param chipSelect    Chip select. gpio::NO_GPIO can be used for hardware slave selects.
   * @param spiDev
   * @param maxSize
   */
  SpiCookie(address_t spiAddress, gpioId_t chipSelect, std::string spiDev, const size_t maxSize,
            spi::SpiModes spiMode, uint32_t spiSpeed);

  /**
   * Like constructor above, but without a dedicated GPIO CS. Can be used for hardware
   * slave select or if CS logic is performed with decoders.
   */
  SpiCookie(address_t spiAddress, std::string spiDev, const size_t maxReplySize,
            spi::SpiModes spiMode, uint32_t spiSpeed);

  /**
   * Use the callback mode of the SPI communication interface. The user can pass the callback
   * function here or by using the setter function #setCallbackMode
   */
  SpiCookie(address_t spiAddress, gpioId_t chipSelect, std::string spiDev, const size_t maxSize,
            spi::SpiModes spiMode, uint32_t spiSpeed, spi::send_callback_function_t callback,
            void* args);

  /**
   * Get the callback function
   * @param callback
   * @param args
   */
  void getCallback(spi::send_callback_function_t* callback, void** args);

  address_t getSpiAddress() const;
  std::string getSpiDevice() const;
  gpioId_t getChipSelectPin() const;
  size_t getMaxBufferSize() const;

  spi::SpiComIfModes getComIfMode() const;

  /** Enables changing SPI speed at run-time */
  void setSpiSpeed(uint32_t newSpeed);
  /** Enables changing the SPI mode at run-time */
  void setSpiMode(spi::SpiModes newMode);

  /**
   * Set the SPI to callback mode and assigns the user supplied callback and an argument
   * passed to the callback.
   * @param callback
   * @param args
   */
  void setCallbackMode(spi::send_callback_function_t callback, void* args);

  /**
   * Can be used to set the callback arguments and a later point than initialization.
   * @param args
   */
  void setCallbackArgs(void* args);

  /**
   * True if SPI transfers should be performed in full duplex mode
   * @return
   */
  bool isFullDuplex() const;

  /**
   * Set transfer type to full duplex or half duplex. Full duplex is the default setting,
   * ressembling common SPI hardware implementation with shift registers, where read and writes
   * happen simultaneosly.
   * @param fullDuplex
   */
  void setFullOrHalfDuplex(bool halfDuplex);

  /**
   * This needs to be called to specify where the SPI driver writes to or reads from.
   * @param readLocation
   * @param writeLocation
   */
  void assignReadBuffer(uint8_t* rx);
  void assignWriteBuffer(const uint8_t* tx);
  /**
   * Set size for the next transfer. Set to 0 for no transfer
   * @param transferSize
   */
  void setTransferSize(size_t transferSize);
  size_t getCurrentTransferSize() const;

  struct UncommonParameters {
    uint8_t bitsPerWord = 8;
    bool noCs = false;
    bool csHigh = false;
    bool threeWireSpi = false;
    /* MSB first is more common */
    bool lsbFirst = false;
  };

  /**
   * Can be used to explicitely disable hardware chip select.
   * Some drivers like the Raspberry Pi Linux driver will not use hardware chip select by default
   * (see https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md)
   * @param enable
   */
  void setNoCs(bool enable);
  void setThreeWireSpi(bool enable);
  void setLsbFirst(bool enable);
  void setCsHigh(bool enable);
  void setBitsPerWord(uint8_t bitsPerWord);

  void getSpiParameters(spi::SpiModes& spiMode, uint32_t& spiSpeed,
                        UncommonParameters* parameters = nullptr) const;

  /**
   * See spidev.h cs_change and delay_usecs
   * @param deselectCs
   * @param delayUsecs
   */
  void activateCsDeselect(bool deselectCs, uint16_t delayUsecs);

  spi_ioc_transfer* getTransferStructHandle();

 private:
  /**
   * Internal constructor which initializes every field
   * @param spiAddress
   * @param chipSelect
   * @param spiDev
   * @param maxSize
   * @param spiMode
   * @param spiSpeed
   * @param callback
   * @param args
   */
  SpiCookie(spi::SpiComIfModes comIfMode, address_t spiAddress, gpioId_t chipSelect,
            std::string spiDev, const size_t maxSize, spi::SpiModes spiMode, uint32_t spiSpeed,
            spi::send_callback_function_t callback, void* args);

  address_t spiAddress;
  gpioId_t chipSelectPin;
  std::string spiDevice;

  spi::SpiComIfModes comIfMode;

  // Required for regular mode
  const size_t maxSize;
  spi::SpiModes spiMode;
  uint32_t spiSpeed;
  bool halfDuplex = false;

  // Required for callback mode
  spi::send_callback_function_t sendCallback = nullptr;
  void* callbackArgs = nullptr;

  struct spi_ioc_transfer spiTransferStruct = {};
  UncommonParameters uncommonParameters;
};

#endif /* LINUX_SPI_SPICOOKIE_H_ */
