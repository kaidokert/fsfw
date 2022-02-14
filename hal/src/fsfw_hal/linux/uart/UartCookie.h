#ifndef SAM9G20_COMIF_COOKIES_UART_COOKIE_H_
#define SAM9G20_COMIF_COOKIES_UART_COOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <fsfw/objectmanager/SystemObjectIF.h>

#include <string>

enum class Parity { NONE, EVEN, ODD };

enum class StopBits { ONE_STOP_BIT, TWO_STOP_BITS };

enum class UartModes { CANONICAL, NON_CANONICAL };

/**
 * @brief   Cookie for the UartComIF. There are many options available to configure the UART driver.
 *          The constructor only requests for common options like the baudrate. Other options can
 *          be set by member functions.
 *
 * @author 	J. Meier
 */
class UartCookie : public CookieIF {
 public:
  /**
   * @brief	Constructor for the uart cookie.
   * @param deviceFile    The device file specifying the uart to use, e.g. "/dev/ttyPS1"
   * @param uartMode      Specify the UART mode. The canonical mode should be used if the
   *                      messages are separated by a delimited character like '\n'. See the
   *                      termios documentation for more information
   * @param baudrate      The baudrate to use for input and output. Possible Baudrates are: 50,
   *                      75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, B19200,
   *                      38400, 57600, 115200, 230400, 460800
   * @param maxReplyLen   The maximum size an object using this cookie expects
   * @details
   * Default configuration: No parity
   *                        8 databits (number of bits transfered with one uart frame)
   *                        One stop bit
   */
  UartCookie(object_id_t handlerId, std::string deviceFile, UartModes uartMode, uint32_t baudrate,
             size_t maxReplyLen);

  virtual ~UartCookie();

  uint32_t getBaudrate() const;
  size_t getMaxReplyLen() const;
  std::string getDeviceFile() const;
  Parity getParity() const;
  uint8_t getBitsPerWord() const;
  StopBits getStopBits() const;
  UartModes getUartMode() const;
  object_id_t getHandlerId() const;

  /**
   * The UART ComIF will only perform a specified number of read cycles for the canonical mode.
   * The user can specify how many of those read cycles are performed for one device handler
   * communication cycle. An example use-case would be to read all available GPS NMEA strings
   * at once.
   * @param readCycles
   */
  void setReadCycles(uint8_t readCycles);
  uint8_t getReadCycles() const;

  /**
   * Allows to flush the data which was received but has not been read yet. This is useful
   * to discard obsolete data at software startup.
   */
  void setToFlushInput(bool enable);
  bool getInputShouldBeFlushed();

  /**
   * Functions two enable parity checking.
   */
  void setParityOdd();
  void setParityEven();

  /**
   * Function two set number of bits per UART frame.
   */
  void setBitsPerWord(uint8_t bitsPerWord_);

  /**
   * Function to specify the number of stopbits.
   */
  void setTwoStopBits();
  void setOneStopBit();

  /**
   * Calling this function prevents the UartComIF to return failed if not all requested bytes
   * could be read. This is required by a device handler when the size of a reply is not known.
   */
  void setNoFixedSizeReply();

  bool isReplySizeFixed();

 private:
  const object_id_t handlerId;
  std::string deviceFile;
  const UartModes uartMode;
  bool flushInput = false;
  uint32_t baudrate;
  size_t maxReplyLen = 0;
  Parity parity = Parity::NONE;
  uint8_t bitsPerWord = 8;
  uint8_t readCycles = 1;
  StopBits stopBits = StopBits::ONE_STOP_BIT;
  bool replySizeFixed = true;
};

#endif
