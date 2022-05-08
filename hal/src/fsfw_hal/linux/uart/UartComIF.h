#ifndef BSP_Q7S_COMIF_UARTCOMIF_H_
#define BSP_Q7S_COMIF_UARTCOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>

#include <unordered_map>
#include <vector>

#include "UartCookie.h"

/**
 * @brief 	This is the communication interface to access serial ports on linux based operating
 *          systems.
 *
 * @details The implementation follows the instructions from https://blog.mbedded.ninja/programming/
 *          operating-systems/linux/linux-serial-ports-using-c-cpp/#disabling-canonical-mode
 *
 * @author 	J. Meier
 */
class UartComIF : public DeviceCommunicationIF, public SystemObject {
 public:
  static constexpr uint8_t uartRetvalId = CLASS_ID::HAL_UART;

  static constexpr ReturnValue_t UART_READ_FAILURE =
      HasReturnvaluesIF::makeReturnCode(uartRetvalId, 1);
  static constexpr ReturnValue_t UART_READ_SIZE_MISSMATCH =
      HasReturnvaluesIF::makeReturnCode(uartRetvalId, 2);
  static constexpr ReturnValue_t UART_RX_BUFFER_TOO_SMALL =
      HasReturnvaluesIF::makeReturnCode(uartRetvalId, 3);

  UartComIF(object_id_t objectId);

  virtual ~UartComIF();

  ReturnValue_t initializeInterface(CookieIF* cookie) override;
  ReturnValue_t sendMessage(CookieIF* cookie, const uint8_t* sendData, size_t sendLen) override;
  ReturnValue_t getSendSuccess(CookieIF* cookie) override;
  ReturnValue_t requestReceiveMessage(CookieIF* cookie, size_t requestLen) override;
  ReturnValue_t readReceivedMessage(CookieIF* cookie, uint8_t** buffer, size_t* size) override;

  /**
   * @brief   This function discards all data received but not read in the UART buffer.
   */
  ReturnValue_t flushUartRxBuffer(CookieIF* cookie);

  /**
   * @brief   This function discards all data in the transmit buffer of the UART driver.
   */
  ReturnValue_t flushUartTxBuffer(CookieIF* cookie);

  /**
   * @brief   This function discards both data in the transmit and receive buffer of the UART.
   */
  ReturnValue_t flushUartTxAndRxBuf(CookieIF* cookie);

 private:
  using UartDeviceFile_t = std::string;

  struct UartElements {
    int fileDescriptor;
    std::vector<uint8_t> replyBuffer;
    /** Number of bytes read will be written to this variable */
    size_t replyLen;
  };

  using UartDeviceMap = std::unordered_map<UartDeviceFile_t, UartElements>;
  using UartDeviceMapIter = UartDeviceMap::iterator;

  /**
   * The uart devie map stores informations of initialized uart ports.
   */
  UartDeviceMap uartDeviceMap;

  /**
   * @brief	This function opens and configures a uart device by using the information stored
   *          in the uart cookie.
   * @param uartCookie    Pointer to uart cookie with information about the uart. Contains the
   *                      uart device file, baudrate, parity, stopbits etc.
   * @return  The file descriptor of the configured uart.
   */
  int configureUartPort(UartCookie* uartCookie);

  /**
   * @brief   This function adds the parity settings to the termios options struct.
   *
   * @param options   Pointer to termios options struct which will be modified to enable or disable
   *                  parity checking.
   * @param uartCookie    Pointer to uart cookie containing the information about the desired
   *                      parity settings.
   *
   */
  static void setParityOptions(struct termios* options, UartCookie* uartCookie);

  void setStopBitOptions(struct termios* options, UartCookie* uartCookie);

  /**
   * @brief   This function sets options which are not configurable by the uartCookie.
   */
  void setFixedOptions(struct termios* options);

  /**
   * @brief   With this function the datasize settings are added to the termios options struct.
   */
  void setDatasizeOptions(struct termios* options, UartCookie* uartCookie);

  /**
   * @brief   This functions adds the  baudrate specified in the uartCookie to the termios options
   *          struct.
   */
  void configureBaudrate(struct termios* options, UartCookie* uartCookie);

  void setUartMode(struct termios* options, UartCookie& uartCookie);

  ReturnValue_t handleCanonicalRead(UartCookie& uartCookie, UartDeviceMapIter& iter,
                                    size_t requestLen);
  ReturnValue_t handleNoncanonicalRead(UartCookie& uartCookie, UartDeviceMapIter& iter,
                                       size_t requestLen);
};

#endif /* BSP_Q7S_COMIF_UARTCOMIF_H_ */
