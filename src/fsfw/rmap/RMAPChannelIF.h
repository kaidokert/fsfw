#ifndef FSFW_RMAP_RMAPCHANNELIF_H_
#define FSFW_RMAP_RMAPCHANNELIF_H_

#include <cstddef>

#include "RMAPCookie.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "rmapConf.h"

class RMAPChannelIF {
 public:
  virtual ~RMAPChannelIF(){};
  /**
   * Reset an RMAP channel
   *
   * Clears the receive buffer (all received messages are deleted) and resets the descriptor table.
   * Also checks for errors in the descriptors and submits them to FDIR (aka stdout)
   *
   * @param channel to reset
   *
   * @return
   *      - @c LINK_DOWN	when the link is down and all replies were missed
   *      - @c COMMAND_CHANNEL_DEACTIVATED if the channel's port is NULL
   *      - @c returnvalue::OK else
   */
  virtual ReturnValue_t reset() = 0;

  /**
   * Check if a channel is active (ie has a port)
   *
   * @param channel_nr
   * @return
   *      - @c COMMAND_OK if channel is active
   *      - @c COMMAND_CHANNEL_DEACTIVATED if channel is deactivated
   */
  virtual ReturnValue_t isActive() = 0;

  /**
   * Assign a SpaceWire port to the Channel
   *
   * @param port	Number of the port. SpaceWire devices are mapped to port numbers to allow
   * checking of the validity
   * @param dest_addr	the destination address used by all packets sent from this channel
   * @param src_addr the source address used by all packets sent from this channel and used when
   * checking incoming packets
   * @return
   *      - @c COMMAND_OK if port was changed
   *      - @c COMMAND_PORT_OUT_OF_RANGE if the port is invalid
   */
  virtual ReturnValue_t setPort(int8_t port, uint8_t dest_addr, uint8_t src_addr) = 0;

  /**
   * Assign a SpaceWire port to the Channel
   *
   * same as setPort(int8_t port, uint8_t dest_addr, uint8_t src_addr), only the addresses are left
   * unchanged
   *
   * @param port	Number of the port. SpaceWire devices are mapped to port numbers to allow
   * checking of the validity
   * @return
   *      - @c COMMAND_OK if port was changed
   *      - @c COMMAND_PORT_OUT_OF_RANGE if the port is invalid
   */
  virtual ReturnValue_t setPort(int8_t port) = 0;

  /**
   * Send an RMAP command
   *
   * @param cookie the cookie used with this call
   * @param instruction the instruction byte that will be sent (this defines if it is a read or
   * write command)
   * @param data data to be sent
   * @param datalen length of data
   * @return
   *      - @c returnvalue::OK
   *      - @c COMMAND_NO_DESCRIPTORS_AVAILABLE 	no descriptors available for sending
   * command; command was not sent
   *      - @c COMMAND_BUFFER_FULL				no receiver buffer available for
   * expected len; command was not sent
   *      - @c COMMAND_TOO_BIG					the data that was to be sent was too
   * long for the hw to handle (write command) or the expected len was bigger than maximal expected
   * len (read command) command was not sent
   *      - @c COMMAND_CHANNEL_DEACTIVATED		the channel has no port set
   *      - @c NOT_SUPPORTED						if you dont feel like
   * implementing something...
   */
  virtual ReturnValue_t sendCommand(RMAPCookie *cookie, uint8_t instruction, const uint8_t *data,
                                    size_t datalen) = 0;

  /**
   * get the reply to an rmap command
   *
   * @param cookie the cookie the command was sent with
   * @param databuffer a pointer to a pointer the location of the reply will be written to
   * @param len a pointer to the variable the length of the reply will be written to
   * @return
   *      - @c REPLY_NO_REPLY				no reply was received
   *      - @c REPLY_NOT_SENT				command was not sent, implies no reply
   *      - @c REPLY_NOT_YET_SENT			command is still waiting to be sent
   *      - @c WRITE_REPLY_INTERFACE_BUSY			Interface is busy (transmission
   * buffer still being processed)
   *      - @c WRITE_REPLY_TRANSMISSION_ERROR		Interface encountered errors during last
   * operation, data could not be processed. (transmission error)
   *      - @c WRITE_REPLY_INVALID_DATA			Invalid data (amount / value)
   *      - @c WRITE_REPLY_NOT_SUPPORTED
   *      - all RMAP standard replies
   */
  virtual ReturnValue_t getReply(RMAPCookie *cookie, uint8_t **databuffer, size_t *len) = 0;

  /**
   *
   * @param cookie
   * @param data
   * @param datalen
   * @param databuffer
   * @param len
   * @param timeout_us
   * @return
   *       - all replies of sendCommand() and getReply()
   *       - @c REPLY_TIMEOUT				timeout
   */
  virtual ReturnValue_t sendCommandBlocking(RMAPCookie *cookie, uint8_t *data, uint32_t datalen,
                                            uint8_t **databuffer, uint32_t *len,
                                            uint32_t timeout_us) = 0;
};

#endif /* FSFW_RMAP_RMAPCHANNELIF_H_ */
