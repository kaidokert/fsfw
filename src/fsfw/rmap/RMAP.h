#ifndef FSFW_RMAP_RMAP_H_
#define FSFW_RMAP_RMAP_H_

#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/rmap/RMAPCookie.h"
#include "rmapConf.h"

// SHOULDTODO: clean up includes for RMAP, should be enough to include RMAP.h but right now it's
// quite chaotic...

/**
 * API for a Cookie/Channel based RMAP implementation.
 *
 * The API offers the four basic RMAP actions: sending a Read or Write command and getting the reply
 * to each. As RMAP is an asynchronous protocol, these are implemented as four seperate calls. There
 * are blocking calls which combine a send and get call using a timeout, but these are "real"
 * blocking, looping in between the calls.
 *
 * Cookies are used to contain Information between a send[Read,Write]Command and a
 * get[Read,Write]Reply call, one can think of them like *nix file descriptors. A cookie is valid
 * from a sendX call until the related getX call. That means if a cookie is used in a getX call, the
 * reply to the sendX call the cookie was used with previously is returned. Depending on the
 * underlying RMAPChannel implementation, a cookie can also be valid for more than one getX call,
 * but this should not be assumed generally. A cookie encapsulates information like the RMAP Channel
 * to use, as well as the RMAP Address and a Command Mask used for specifying which RMAP
 * capabilities are used. Cookies are created without interaction with this API, there is no open()
 * call. The RMAP implementation will initialize all fields which are not set by the cookie's
 * constructor.
 *
 * The RMAP implementation relies on Channels. A channel is used to construct the RMAP Headers and
 * handle the protocol. It is access via the RMAPChannelIF. Thus it is possible to use different
 * Implementations which only need to implement the RMAPChannelIF. A channel is also responsible for
 * accessing the lower layers, for example a SPaceWire transport layer.
 *
 * There is one RMAP Channel per physical device. The cookie-channel as well as the device-channel
 * assignment can be changed at runtime to allow for example redundancy switching. This API is
 * static as the information which channel to use is contained within the cookie.
 */
class RMAP {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::RMAP_CHANNEL;

  // static const ReturnValue_t COMMAND_OK = MAKE_RETURN_CODE(0x00);
  static const ReturnValue_t COMMAND_NO_DESCRIPTORS_AVAILABLE =
      MAKE_RETURN_CODE(0xE1);  // no descriptors available for sending command; command was not sent
  static const ReturnValue_t COMMAND_BUFFER_FULL = MAKE_RETURN_CODE(
      0xE2);  // no receiver buffer available for expected len; command was not sent
  static const ReturnValue_t COMMAND_CHANNEL_OUT_OF_RANGE =
      MAKE_RETURN_CODE(0xE3);  // The cookie points to an invalid channel; command was not sent
  // Replaced by DeviceCommunicationIF::TOO_MUCH_DATA	static const ReturnValue_t COMMAND_TOO_BIG =
  // MAKE_RETURN_CODE(0xE4); //the data that was to be sent was too long for the hw to handle (write
  // command) or the expected len was bigger than maximal expected len (read command) command was
  // not sent
  // replaced by DeviceCommunicationIF::NULLPOINTER	static const ReturnValue_t
  // COMMAND_NULLPOINTER = MAKE_RETURN_CODE(0xE5); //datalen was != 0 but data was == NULL in write
  // command, or nullpointer in read command
  static const ReturnValue_t COMMAND_CHANNEL_DEACTIVATED =
      MAKE_RETURN_CODE(0xE6);  // the channel has no port set
  static const ReturnValue_t COMMAND_PORT_OUT_OF_RANGE =
      MAKE_RETURN_CODE(0xE7);  // The specified port is not valid
  static const ReturnValue_t COMMAND_PORT_IN_USE =
      MAKE_RETURN_CODE(0xE8);  // The specified port is already in use
  static const ReturnValue_t COMMAND_NO_CHANNEL =
      MAKE_RETURN_CODE(0xE9);  // The cookie to work with has no channel assigned.
  static const ReturnValue_t NO_HW_CRC = MAKE_RETURN_CODE(
      0xEA);  // The SpW port does not support HW CRC generation, which is unsupported
  // return values for both get_write_reply and get_read_reply
  static const ReturnValue_t REPLY_NO_REPLY = MAKE_RETURN_CODE(0xD0);  // no reply was received
  static const ReturnValue_t REPLY_NOT_SENT =
      MAKE_RETURN_CODE(0xD1);  // command was not sent, implies no reply
  static const ReturnValue_t REPLY_NOT_YET_SENT =
      MAKE_RETURN_CODE(0xD2);  // command is still waiting to be sent
  static const ReturnValue_t REPLY_MISSMATCH = MAKE_RETURN_CODE(
      0xD3);  // a read command was issued, but get_write_rply called, or other way round
  static const ReturnValue_t REPLY_TIMEOUT = MAKE_RETURN_CODE(0xD4);  // timeout
  // replaced by DeviceCommunicationIF::NULLPOINTER	static const ReturnValue_t REPLY_NULLPOINTER
  // = MAKE_RETURN_CODE(0xD5);//one of the arguments in a read reply was NULL return values for
  // get_reply
  static const ReturnValue_t REPLY_INTERFACE_BUSY =
      MAKE_RETURN_CODE(0xC0);  // Interface is busy (transmission buffer still being processed)
  static const ReturnValue_t REPLY_TRANSMISSION_ERROR =
      MAKE_RETURN_CODE(0xC1);  // Interface encountered errors during last operation, data could not
                               // be processed. (transmission error)
  static const ReturnValue_t REPLY_INVALID_DATA =
      MAKE_RETURN_CODE(0xC2);  // Invalid data (amount / value)
  static const ReturnValue_t REPLY_NOT_SUPPORTED = MAKE_RETURN_CODE(0xC3);

  // return values for reset
  static const ReturnValue_t LINK_DOWN = MAKE_RETURN_CODE(0xF0);  // The spw link is down
  // Other SpW codes:
  static const ReturnValue_t SPW_CREDIT = MAKE_RETURN_CODE(0xF1);
  static const ReturnValue_t SPW_ESCAPE = MAKE_RETURN_CODE(0xF2);
  static const ReturnValue_t SPW_DISCONNECT = MAKE_RETURN_CODE(0xF3);
  static const ReturnValue_t SPW_PARITY = MAKE_RETURN_CODE(0xF4);
  static const ReturnValue_t SPW_WRITE_SYNC = MAKE_RETURN_CODE(0xF5);
  static const ReturnValue_t SPW_INVALID_ADDRESS = MAKE_RETURN_CODE(0xF6);
  static const ReturnValue_t SPW_EARLY_EOP = MAKE_RETURN_CODE(0xF7);
  static const ReturnValue_t SPW_DMA = MAKE_RETURN_CODE(0xF8);
  static const ReturnValue_t SPW_LINK_ERROR = MAKE_RETURN_CODE(0xF9);

  // RMAP standard replies
  static const ReturnValue_t REPLY_OK = MAKE_RETURN_CODE(0);
  static const ReturnValue_t REPLY_GENERAL_ERROR_CODE =
      MAKE_RETURN_CODE(1);  // The detected error does not fit into the other
  // error cases or the node does not support
  // further distinction between the errors
  static const ReturnValue_t REPLY_UNUSED_PACKET_TYPE_OR_COMMAND_CODE =
      MAKE_RETURN_CODE(2);  // The Header CRC was decoded correctly but
  // the packet type is reserved or the command
  // is not used by the RMAP protocol.
  static const ReturnValue_t REPLY_INVALID_KEY =
      MAKE_RETURN_CODE(3);  // The Header CRC was decoded correctly but
  // the device key did not match that expected
  // by the target user application
  static const ReturnValue_t REPLY_INVALID_DATA_CRC =
      MAKE_RETURN_CODE(4);  // Error in the CRC of the data field
  static const ReturnValue_t REPLY_EARLY_EOP =
      MAKE_RETURN_CODE(5);  // EOP marker detected before the end of the data
  static const ReturnValue_t REPLY_TOO_MUCH_DATA =
      MAKE_RETURN_CODE(6);  // More than the expected amount of data in a
  // command has been received
  static const ReturnValue_t REPLY_EEP =
      MAKE_RETURN_CODE(7);  // EEP marker detected immediately after the
  // header CRC or during the transfer of data
  // and Data CRC or immediately thereafter.
  // Indicates that there was a communication
  // failure of some sort on the network
  static const ReturnValue_t REPLY_RESERVED = MAKE_RETURN_CODE(8);  // Reserved
  static const ReturnValue_t REPLY_VERIFY_BUFFER_OVERRUN =
      MAKE_RETURN_CODE(9);  // The verify before write bit of the command
  // was set so that the data field was buffered in
  // order to verify the Data CRC before
  // transferring the data to target memory. The
  // data field was longer than able to fit inside
  // the verify buffer resulting in a buffer overrun
  // Note that the command is not executed in
  // this case
  static const ReturnValue_t REPLY_COMMAND_NOT_IMPLEMENTED_OR_NOT_AUTHORISED =
      MAKE_RETURN_CODE(10);  // The target user application did not authorise
  // the requested operation. This may be because
  // the command requested has not been
  // implemented
  static const ReturnValue_t REPLY_RMW_DATA_LENGTH_ERROR =
      MAKE_RETURN_CODE(11);  // The amount of data in a RMW command is
  // invalid (0x01	 0x03	 0x05	 0x07 or  greater
  // than 0x08)
  static const ReturnValue_t REPLY_INVALID_TARGET_LOGICAL_ADDRESS =
      MAKE_RETURN_CODE(12);  // The Header CRC was decoded correctly but
  // the Target Logical Address was not the value
  // expected by the target

  /**
   * Resets the underlying channel.
   *
   * @param cookie The cookie which points to the channel to reset
   * @return
   */
  static ReturnValue_t reset(RMAPCookie *cookie);

  /**
   * send an RMAP write command
   *
   * datalen is only 24bit wide, rest will be ignored
   * IMPORTANT: the data buffer must be datalen+1 large, as the driver might
   * write a CRC sum at data[datalen]
   * if you want to send an empty packet, just do datalen = 0 and data = NULL
   *
   * @param cookie The cookie to write to
   * @param buffer the data to write
   * @param length length of data
   * @return
   *      - @c COMMAND_NULLPOINTER				datalen was != 0 but data was ==
   * NULL in write command
   *      - return codes of RMAPChannelIF::sendCommand()
   */
  static ReturnValue_t sendWriteCommand(RMAPCookie *cookie, const uint8_t *buffer, size_t length);

  /**
   * get the reply to a write command
   *
   * @param cookie the cookie the command was sent with
   * @return
   *      - @c REPLY_MISSMATCH			a read command was issued, but getWriteReply called
   *      - return codes of RMAPChannelIF::getReply()
   */
  static ReturnValue_t getWriteReply(RMAPCookie *cookie);

  /**
   * @see sendWriteCommand()
   * @see getWriteReply()
   *
   * @param timeout_us the time after the function returns, if no reply was received
   *
   * @return
   *       - All of sendWriteCommand()
   *       - All of getWriteReply()
   *       - @c REPLY_TIMEOUT				timeout
   */
  static ReturnValue_t writeBlocking(RMAPCookie *cookie, uint8_t *buffer, uint32_t length,
                                     uint32_t timeout_us);

  /**
   * send an RMAP read command
   *
   * @param cookie to cookie to read from
   * @param expLength the expected maximum length of the reply
   * @return
   *      - @c COMMAND_NULLPOINTER				datalen was != 0 but data was ==
   * NULL in write command, or nullpointer in read command
   *      - return codes of RMAPChannelIF::sendCommand()
   */
  static ReturnValue_t sendReadCommand(RMAPCookie *cookie, uint32_t expLength);

  /**
   * get a reply to an RMAP read command
   *
   * @param cookie the cookie that was read from
   * @param[out] buffer the location of the data
   * @param[out] size size of the data
   * @return
   *      - @c COMMAND_NULLPOINTER		buffer or size was NULL
   *      - @c REPLY_MISSMATCH			a write command was issued, but getReadReply called
   *      - return codes of RMAPChannelIF::getReply()
   */
  static ReturnValue_t getReadReply(RMAPCookie *cookie, uint8_t **buffer, size_t *size);

  /**
   * @see sendReadCommand()
   * @see getReadReply()
   *
   * @param timeout_us the time after the function returns, if no reply was received
   *
   * @return
   *       - All of sendReadCommand()
   *       - All of getReadReply()
   *       - @c REPLY_TIMEOUT				timeout
   */
  static ReturnValue_t readBlocking(RMAPCookie *cookie, uint32_t expLength, uint8_t **buffer,
                                    uint32_t *size, uint32_t timeout_us);

 protected:
  RMAP();
};

#endif /* RMAPpp_H_ */
