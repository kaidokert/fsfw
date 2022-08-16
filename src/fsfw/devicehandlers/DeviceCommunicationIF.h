#ifndef FSFW_DEVICES_DEVICECOMMUNICATIONIF_H_
#define FSFW_DEVICES_DEVICECOMMUNICATIONIF_H_

#include "../returnvalues/returnvalue.h"
#include "CookieIF.h"
#include "DeviceHandlerIF.h"
/**
 * @defgroup interfaces Interfaces
 * @brief Interfaces for flight software objects
 */

/**
 * @defgroup comm Communication
 * @brief Communication software components.
 */

/**
 * @brief This is an interface to decouple device communication from
 * the device handler to allow reuse of these components.
 * @details
 * Documentation: Dissertation Baetz p.138.
 * It works with the assumption that received data  is polled by a component.
 * There are four generic steps of device communication:
 *
 *   1. Send data to a device
 *   2. Get acknowledgement for sending
 *   3. Request reading data from a device
 *   4. Read received data
 *
 *  To identify different connection over a single interface can return
 *  so-called cookies to components.
 *  The CommunicationMessage message type can be used to extend the
 *  functionality of the ComIF if a separate polling task is required.
 *  @ingroup interfaces
 *  @ingroup comm
 */
class DeviceCommunicationIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::DEVICE_COMMUNICATION_IF;

  //! This is returned in readReceivedMessage() if no reply was reived.
  static const ReturnValue_t NO_REPLY_RECEIVED = MAKE_RETURN_CODE(0x01);

  //! General protocol error. Define more concrete errors in child handler
  static const ReturnValue_t PROTOCOL_ERROR = MAKE_RETURN_CODE(0x02);
  //! If cookie is a null pointer
  static const ReturnValue_t NULLPOINTER = MAKE_RETURN_CODE(0x03);
  static const ReturnValue_t INVALID_COOKIE_TYPE = MAKE_RETURN_CODE(0x04);
  // is this needed if there is no open/close call?
  static const ReturnValue_t NOT_ACTIVE = MAKE_RETURN_CODE(0x05);
  static const ReturnValue_t TOO_MUCH_DATA = MAKE_RETURN_CODE(0x06);

  virtual ~DeviceCommunicationIF() {}

  /**
   * @brief Device specific initialization, using the cookie.
   * @details
   * The cookie is already prepared in the factory. If the communication
   * interface needs to be set up in some way and requires cookie information,
   * this can be performed in this function, which is called on device handler
   * initialization.
   * @param cookie
   * @return
   *  - @c returnvalue::OK if initialization was successfull
   *  - Everything else triggers failure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t initializeInterface(CookieIF *cookie) = 0;

  /**
   * Called by DHB in the SEND_WRITE doSendWrite().
   * This function is used to send data to the physical device
   * by implementing and calling related drivers or wrapper functions.
   * @param cookie
   * @param data
   * @param len If this is 0, nothing shall be sent.
   * @return
   *  - @c returnvalue::OK for successfull send
   *  - Everything else triggers failure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) = 0;

  /**
   * Called by DHB in the GET_WRITE doGetWrite().
   * Get send confirmation that the data in sendMessage() was sent successfully.
   * @param cookie
   * @return
   *  - @c returnvalue::OK if data was sent successfully but a reply is expected
   *  - NO_REPLY_EXPECTED if data was sent successfully and no reply is expected
   *  - Everything else to indicate failure
   */
  virtual ReturnValue_t getSendSuccess(CookieIF *cookie) = 0;

  /**
   * Called by DHB in the SEND_WRITE doSendRead().
   * It is assumed that it is always possible to request a reply
   * from a device. If a requestLen of 0 is supplied, no reply was enabled
   * and communication specific action should be taken (e.g. read nothing
   * or read everything).
   *
   * @param cookie
   * @param requestLen Size of data to read
   * @return - @c returnvalue::OK to confirm the request for data has been sent.
   *         - Everything else triggers failure event with
   *           returnvalue as parameter 1
   */
  virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) = 0;

  /**
   * Called by DHB in the GET_WRITE doGetRead().
   * This function is used to receive data from the physical device
   * by implementing and calling related drivers or wrapper functions.
   * @param cookie
   * @param buffer [out] Set reply here (by using *buffer = ...)
   * @param size [out] size pointer to set (by using *size = ...).
   * 	           Set to 0 if no reply was received
   * @return - @c returnvalue::OK for successfull receive
   * 		   - @c NO_REPLY_RECEIVED if not reply was received. Setting size to
   * 		   	   0 has the same effect
   *         - Everything else triggers failure event with
   *           returnvalue as parameter 1
   */
  virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) = 0;
};

#endif /* FSFW_DEVICES_DEVICECOMMUNICATIONIF_H_ */
