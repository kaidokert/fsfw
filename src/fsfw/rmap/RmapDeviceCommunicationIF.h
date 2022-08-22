#ifndef FSFW_RMAP_RMAPDEVICECOMMUNICATIONINTERFACE_H_
#define FSFW_RMAP_RMAPDEVICECOMMUNICATIONINTERFACE_H_

#include "fsfw/devicehandlers/DeviceCommunicationIF.h"
#include "rmapConf.h"

/**
 * @brief	This class is a implementation of a DeviceCommunicationIF for RMAP calls.
 *          It expects RMAPCookies or a derived class of RMAPCookies
 *
 * @details	The open, close and reOpen calls are mission specific
 * 			The open call might return any child of RMAPCookies
 *
 * \ingroup rmap
 */
class RmapDeviceCommunicationIF : public DeviceCommunicationIF {
 public:
  virtual ~RmapDeviceCommunicationIF();

  /**
   * @brief Device specific initialization, using the cookie.
   * @details
   * The cookie is already prepared in the factory. If the communication
   * interface needs to be set up in some way and requires cookie information,
   * this can be performed in this function, which is called on device handler
   * initialization.
   * @param cookie
   * @return -@c returnvalue::OK if initialization was successfull
   * 		   - Everything else triggers failure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t initializeInterface(CookieIF *cookie) = 0;

  /**
   * Called by DHB in the SEND_WRITE doSendWrite().
   * This function is used to send data to the physical device
   * by implementing and calling related drivers or wrapper functions.
   * @param cookie
   * @param data
   * @param len
   * @return -@c returnvalue::OK for successfull send
   *         - Everything else triggers failure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen);

  /**
   * Called by DHB in the GET_WRITE doGetWrite().
   * Get send confirmation that the data in sendMessage() was sent successfully.
   * @param cookie
   * @return -@c returnvalue::OK if data was sent successfull
   * 		   - Everything else triggers falure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t getSendSuccess(CookieIF *cookie);

  /**
   * Called by DHB in the SEND_WRITE doSendRead().
   * It is assumed that it is always possible to request a reply
   * from a device.
   *
   * @param cookie
   * @return -@c returnvalue::OK to confirm the request for data has been sent.
   *         -@c NO_READ_REQUEST if no request shall be made. readReceivedMessage()
   *         	   will not be called in the respective communication cycle.
   *         - Everything else triggers failure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen);

  /**
   * Called by DHB in the GET_WRITE doGetRead().
   * This function is used to receive data from the physical device
   * by implementing and calling related drivers or wrapper functions.
   * @param cookie
   * @param data
   * @param len
   * @return @c returnvalue::OK for successfull receive
   *         - Everything else triggers failure event with returnvalue as parameter 1
   */
  virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size);

  ReturnValue_t setAddress(CookieIF *cookie, uint32_t address);
  uint32_t getAddress(CookieIF *cookie);
  ReturnValue_t setParameter(CookieIF *cookie, uint32_t parameter);
  uint32_t getParameter(CookieIF *cookie);
};

#endif /* FSFW_RMAP_RMAPDEVICECOMMUNICATIONINTERFACE_H_ */
